/*
 * CeeHealth
 * Copyright (C) 2025 Chloe Eather
 *
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <cee/mppm/input.h>
#include <cee/mppm/event.h>
#include <cee/mppm/log.h>

#include <cerrno>
#include <climits>

#include <dirent.h>
#include <linux/input-event-codes.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <fnmatch.h>
#include <sys/poll.h>

#include <xkbcommon/xkbcommon-compose.h>
#include <xkbcommon/xkbcommon.h>
#include <linux/input.h>

#define NLONGS(n) (((n) + LONG_BIT - 1) / LONG_BIT)

static bool evdev_bit_is_set(const unsigned long *array, int bit) {
	return array[bit / LONG_BIT] & (1LL << (bit % LONG_BIT));
}

namespace cee {
termios Input::s_DefTerm;
termios Input::s_Term;
xkb_context *Input::s_XkbContext;
xkb_keymap *Input::s_XkbKeymap;
xkb_compose_table *Input::s_XkbComposeTable;
Input::Keyboard *Input::s_Keyboards;
int Input::s_EvdevOffset;

pollfd *Input::s_Fds;
nfds_t Input::s_Nfds;
std::function<void(Event &)> Input::s_EventCallback;

int Input::Init() {
	int ret;
	Keyboard *keyboard;
	nfds_t i;
	int numIncludes = 0;
	const char* includes[64];

	tcgetattr(fileno(stdin), &s_DefTerm);
	memcpy(&s_Term, &s_DefTerm, sizeof(termios));

	s_Term.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(fileno(stdin), TCSANOW, &s_Term);

	s_XkbComposeTable = nullptr;
	s_EvdevOffset = 0;
	s_EventCallback = &Input::DefaultCallback;
	
	s_XkbContext = xkb_context_new(XKB_CONTEXT_NO_DEFAULT_INCLUDES);
	if (s_XkbContext == nullptr) {
		CEE_ERROR("Failed to setup xkb context");
		return -1;
	}

	if (numIncludes == 0) {
		includes[numIncludes++] = "__defaults__";
	}

	for (i = 0; i < numIncludes; i++) {
		const char* include = includes[i];
		if (strcmp(include, "__defaults__") == 0) {
			xkb_context_include_path_append_default(s_XkbContext);
		} else {
			xkb_context_include_path_append(s_XkbContext, include);
		}
	}

	s_XkbKeymap = xkb_keymap_new_from_names(s_XkbContext, nullptr, XKB_KEYMAP_COMPILE_NO_FLAGS);
	if (s_XkbKeymap == nullptr) {
		CEE_ERROR("Failed to compile keymap");
		return -1;
	}


	ret = GetKeyboards();
	if (ret < 0) {
		CEE_ERROR("Failed to get keyboards");
		goto out;
	}

	for (keyboard = s_Keyboards, s_Nfds = 0; keyboard; keyboard = keyboard->next, s_Nfds++){}
	s_Nfds++;
	s_Fds = reinterpret_cast<pollfd*>(calloc(s_Nfds, sizeof(*s_Fds)));
	if (s_Fds == nullptr) {
		CEE_ERROR("Out of memory");
		ret = -1;
		goto out;
	}

	for (i = 0, keyboard = s_Keyboards; keyboard; keyboard = keyboard->next, i++) {
		s_Fds[i].fd = keyboard->fd;
		s_Fds[i].events = POLLIN;
	}
	s_Fds[s_Nfds - 1].fd = fileno(stdin);
	s_Fds[s_Nfds - 1].events = POLLIN;

out:
	return ret;
}

void Input::Shutdown() {
	FreeKeyboards();
	xkb_compose_table_unref(s_XkbComposeTable);
	s_XkbComposeTable = nullptr;
	xkb_keymap_unref(s_XkbKeymap);
	s_XkbKeymap = nullptr;
	xkb_context_unref(s_XkbContext);
	s_XkbContext = nullptr;

	tcsetattr(fileno(stdin), 0, &s_DefTerm);
}

int Input::Poll() {
	int ret = 0;
	Keyboard *keyboard;
	nfds_t i;

	ret = poll(s_Fds, s_Nfds, 0);
	if (ret < 0) {
		if (errno == EINTR) {
			goto out;	
		}
		CEE_WARN("poll failed: {0}", strerror(errno));
		goto out;
	}

	for (i = 0, keyboard = s_Keyboards; keyboard; keyboard = keyboard->next, i++) {
		if (s_Fds[i].revents != 0) {
			ret = ReadKeyboard(keyboard);
			if (ret != 0) {
				goto out;
			}
		}
	}

out:
	return ret;
}

void Input::SetEventCallback(std::function<void(Event &)> fn) {
	s_EventCallback = fn;
}

int Input::GetKeyboards() {
	int ret = 0, nents = 0;
	Keyboard *kbd;
	dirent **ents;
	s_Keyboards = nullptr;

	nents = scandir("/dev/input", &ents, [](const dirent* ent) -> int {
		return !fnmatch("event*", ent->d_name, 0);
	}, alphasort);
	if (nents < 0) {
		CEE_ERROR("Couldn't scan /dev/input: {0}", strerror(errno));
		return -1;
	}

	for (int i = 0; i < nents; i++) {
		ret = NewKeyboard(ents[i], &kbd);
		if (ret) {
			if (ret == -EACCES) {
				CEE_ERROR("Couldn't open /dev/input/{0}: {1}", ents[i]->d_name, strerror(-ret));
				break;
			}
			if (ret == -ENOTSUP) {
				//CEE_DEBUG("Couldn't open /dev/input/{0}: {1}. Skipping...", ents[i]->d_name, strerror(-ret));
			}
			continue;
		}
		if (kbd == nullptr) {
			CEE_ERROR("kbd is nullptr");
			continue;
		}
		kbd->next = s_Keyboards;
		s_Keyboards = kbd;
	}

	for (int i = 0; i < nents; i++) {
		free(ents[i]);
	}
	free(ents);
	
	if (s_Keyboards == nullptr) {
		CEE_ERROR("Couldn't find any keyboards");
		return -1;
	}

	return 0;
}

void Input::FreeKeyboards() {
	Keyboard *next;

	while (s_Keyboards) {
		next = s_Keyboards->next;
		FreeKeyboard(s_Keyboards);
		s_Keyboards = next;
	}
}

int Input::NewKeyboard(dirent *ent, Keyboard **out) {
	int ret = 0, fd = 0;
	char *path;
	xkb_state *state;
	xkb_compose_state *composeState = nullptr;
	Keyboard *keyboard;

	ret = asprintf(&path, "/dev/input/%s", ent->d_name);
	if (ret < 0) {
		return -ENOMEM;
	}

	fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	if (fd < 0) {
		ret = -errno;
		goto err_path;
	}

	if (!IsKeyboard(fd)) {
		ret = -ENOTSUP;
		goto err_fd;
	}
	
	state = xkb_state_new(s_XkbKeymap);
	if (state == nullptr) {
		CEE_ERROR("Couldn't create state for {0}", path);
		ret = -EFAULT;
		goto err_fd;
	}

	keyboard = reinterpret_cast<Keyboard*>(calloc(1, sizeof(*keyboard)));
	if (keyboard == nullptr) {
		ret = -ENOMEM;
		goto err_state;
	}

	keyboard->path = path;
	keyboard->fd = fd;
	keyboard->state = state;
	keyboard->composeState = composeState;
	*out = keyboard;

	return 0;

err_state:
	xkb_state_unref(state);
err_fd:
	close(fd);
err_path:
	free(path);
	return ret;
}

void Input::FreeKeyboard(Keyboard *kbd) {
	if (kbd == nullptr) {
		return;
	}
	if (kbd->fd >= 0) {
		close(kbd->fd);
	}
	free(kbd->path);
	xkb_state_unref(kbd->state);
	xkb_compose_state_unref(kbd->composeState);
	free(kbd);
}

bool Input::IsKeyboard(int fd) {
	unsigned long evbits[NLONGS(EV_CNT)] = { 0 };
	unsigned long keybits[NLONGS(KEY_CNT)] = { 0 };

	errno = 0;
	ioctl(fd, EVIOCGBIT(0, sizeof(evbits)), evbits);
	if (errno) {
		return false;
	}

	if (!evdev_bit_is_set(evbits, EV_KEY)) {
		return false;
	}

	errno = 0;
	ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybits)), keybits);
	if (errno) {
		return false;
	}

	for (int i = KEY_RESERVED; i <= KEY_MIN_INTERESTING; i++) {
		if (evdev_bit_is_set(keybits, i)) {
			return true;
		}
	}

	return false;
}

int Input::ReadKeyboard(Keyboard *keyboard) {
	ssize_t len;
	input_event evs[16];

	while ((len = read(keyboard->fd, &evs, sizeof(evs))) > 0) {
		const size_t nevs = len / sizeof(input_event);
		for (size_t i = 0; i < nevs; i++) {
			ProcessEvent(keyboard, evs[i].type, evs[i].code, evs[i].value);
		}
	}
	if (len < 0 && errno != EWOULDBLOCK) {
		CEE_ERROR("Couldn't read {0}: {1}", keyboard->path, strerror(errno));
		return 1;
	}
	return 0;
}

void Input::ProcessEvent(Keyboard *keyboard, uint16_t type, uint16_t code, int32_t value) {
	xkb_keycode_t keycode;
	xkb_keymap *keymap;
	xkb_state_component changed;
	xkb_compose_status status;

	if (type != EV_KEY) {
		return;
	}

	keycode = s_EvdevOffset + code;
	keymap = xkb_state_get_keymap(keyboard->state);

	if (value == KEY_STATE_REPEAT && !xkb_keymap_key_repeats(keymap, keycode)) {
		KeyRepeatEvent e(keycode);
		s_EventCallback(e);
		return;
	}

	if (value == KEY_STATE_RELEASE) {
		changed = xkb_state_update_key(keyboard->state, keycode, XKB_KEY_UP);
		KeyUpEvent e(keycode);
		s_EventCallback(e);
	} else {
		changed = xkb_state_update_key(keyboard->state, keycode, XKB_KEY_DOWN);
		KeyDownEvent e(keycode);
		s_EventCallback(e);
	}
}

void Input::DefaultCallback(Event &e) {
	(void)e; /* Unused variable */
}
}

