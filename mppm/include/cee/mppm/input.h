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

#ifndef CEE_INPUT_H_
#define CEE_INPUT_H_

#include <cee/mppm/event.h>

#include <functional>

#include <termios.h>

#include <dirent.h>
#include <sys/poll.h>

#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>

namespace cee {
class Input {
private:
	struct Keyboard {
		char *path;
		int fd;
		xkb_state *state;
		xkb_compose_state *composeState;
		Keyboard *next;
	};

public:
	enum {
		KEY_STATE_RELEASE = 0,
		KEY_STATE_PRESS = 1,
		KEY_STATE_REPEAT = 2
	};

public:
	static int Init();
	static void Shutdown();

	static int Poll();

	static void SetEventCallback(std::function<void(Event &)> fn);

private:
	static int GetKeyboards();
	static void FreeKeyboards();
	static int NewKeyboard(dirent *ent, Keyboard **out);
	static void FreeKeyboard(Keyboard *kbd);
	static bool IsKeyboard(int fd);

	static int ReadKeyboard(Keyboard *keyboard);
	static void ProcessEvent(Keyboard *keyboard, uint16_t type, uint16_t code, int32_t value);

	static void DefaultCallback(Event &);

private:
	static termios s_DefTerm;
	static termios s_Term;

	static xkb_context *s_XkbContext;
	static xkb_keymap *s_XkbKeymap;
	static xkb_compose_table *s_XkbComposeTable;
	static Keyboard *s_Keyboards;

	static int s_EvdevOffset;

	static pollfd *s_Fds;
	static nfds_t s_Nfds;

	static std::function<void(Event &)> s_EventCallback;
};
}

#endif
