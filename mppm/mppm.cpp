/*
 * CeeHealth
 * Copyright (C) 2025 2026 Chloe Eather
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

#include <cee/mppm/mppm.h>
#include <cee/mppm/event.h>
#include <cee/mppm/log.h>
#include <cee/mppm/input.h>
#include <cee/mppm/rng.h>
#include <cee/mppm/config.h>

#include <cee/profiler/profiler.h>

#include <cee/hal/hal.h>

#include <cee/gui/gui.h>
#include <cee/gui/box.h>

#include <glad/gles2.h>

#include <chrono>
#include <csignal>
#include <filesystem>
#include <functional>

#include <getopt.h>
#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon-keysyms.h>

enum {
	ARG_LOGFILE = 1
};

static const char *g_OptString = "g:i:l:hv";
static const option g_LongOptions[] = {
	{ "help", no_argument, nullptr, 'h' },
	{ "version", no_argument, nullptr, 'v' },
	{ "logfile", required_argument, nullptr, ARG_LOGFILE },
	{ nullptr, 0, nullptr, 0 }
};

namespace cee {
static void PrintHelpMessage(const char *cmd);
static void PrintVersion(const char *cmd);

MPPM* MPPM::s_Instance = nullptr;

MPPM::MPPM(int argc, char *argv[]) {
	(void)argc, (void)argv; // Supress unused warning
	ParseCommandLineArgs(argc, argv);
	PROFILE_SCOPE("Initialization");
	if (s_Instance) {
		std::fprintf(stderr, "More than one instance of cee::MPPM is not allowed.");
		std::exit(EXIT_FAILURE);
		throw std::runtime_error("More than one instance of cee::MPPM is not allowed.");
	}
	s_Instance = this;
	bool running = true;

	Log::Init();
	hal::InitLogger();
	Log::AddLogger(hal::GetLogger());
	gui::InitLogger();
	Log::AddLogger(gui::GetLogger());

	rng<int>::Init();

	if (Input::Init() < 0) {
		throw std::runtime_error("Failed to initialize input system");
	}
	Input::SetEventCallback(std::bind(&MPPM::OnEvent, this, std::placeholders::_1));

	if (hal::GetGfxBackend() == HAL_GFX_BACKEND_NONE) {
#if BUILD_HAL_DRM
		hal::SetGfxBackend(HAL_GFX_BACKEND_DRM);
#elif BUILD_HAL_X11
		CEE_CORE_TRACE("Graphics DRM backend not detected, falling back to X11");
		hal::SetGfxBackend(HAL_GFX_BACKEND_X11);
#else
		CEE_CORE_ERROR("No graphics backend detected!");
		throw std::runtime_error("No graphics backend detected");
#endif
	}
	if (hal::GetI2CBackend() == HAL_I2C_BACKEND_NONE) {
#if BUILD_HAL_I2C_HW
		hal::SetI2CBackend(HAL_I2C_BACKEND_HW);
#elif BUILD_HAL_I2C_MOCK
		CEE_CORE_TRACE("I2C hardware backend not detected, falling back to mock");
		hal::SetI2CBackend(HAL_I2C_BACKEND_MOCK);
#else
		CEE_CORE_ERROR("No I2C backend detected!");
		throw std::runtime_error("No I2C backend detected");
#endif
	}

	if (hal::Init()) {
		CEE_CORE_ERROR("Failed to initialize hardware abstration layer!");
		throw std::runtime_error("Failed to initialize hardware abstration layer!");
	}

	m_HalGfx = hal::GraphicsContext::Create();
	m_HalGfx->Init();

	cee::gui::Init();
}

MPPM::~MPPM() {
	cee::gui::Shutdown();
	m_HalGfx->Shutdown();
	hal::Shutdown();
	cee::Input::Shutdown();
}

int MPPM::Run() {
	SetSigHandlers();

	std::unique_ptr<cee::gui::Box> root = std::make_unique<cee::gui::Box>();
	std::unique_ptr<cee::gui::Box> hbox = std::make_unique<cee::gui::Box>();
	std::unique_ptr<cee::gui::Box> padBox = std::make_unique<cee::gui::Box>();
	std::unique_ptr<cee::gui::Box> transBox = std::make_unique<cee::gui::Box>();
	std::unique_ptr<cee::gui::Box> lesbianBox = std::make_unique<cee::gui::Box>();
	std::unique_ptr<cee::gui::Box> transFlagStrip1 = std::make_unique<cee::gui::Box>();
	std::unique_ptr<cee::gui::Box> transFlagStrip2 = std::make_unique<cee::gui::Box>();
	std::unique_ptr<cee::gui::Box> transFlagStrip3 = std::make_unique<cee::gui::Box>();
	std::unique_ptr<cee::gui::Box> transFlagStrip4 = std::make_unique<cee::gui::Box>();
	std::unique_ptr<cee::gui::Box> transFlagStrip5 = std::make_unique<cee::gui::Box>();
	std::unique_ptr<cee::gui::Box> lesbianFlagStrip1 = std::make_unique<cee::gui::Box>();
	std::unique_ptr<cee::gui::Box> lesbianFlagStrip2 = std::make_unique<cee::gui::Box>();
	std::unique_ptr<cee::gui::Box> lesbianFlagStrip3 = std::make_unique<cee::gui::Box>();
	std::unique_ptr<cee::gui::Box> lesbianFlagStrip4 = std::make_unique<cee::gui::Box>();
	std::unique_ptr<cee::gui::Box> lesbianFlagStrip5 = std::make_unique<cee::gui::Box>();
	std::unique_ptr<cee::gui::Box> lesbianFlagStrip6 = std::make_unique<cee::gui::Box>();
	std::unique_ptr<cee::gui::Box> lesbianFlagStrip7 = std::make_unique<cee::gui::Box>();

	{
		PROFILE_SCOPE("Setup GUI");
		root->SetStackDirection(cee::gui::Box::StackDirection::Horizontal);
		cee::gui::SetRootNode(root.get());

		hbox->SetStackDirection(cee::gui::Box::StackDirection::Horizontal);

		transBox->SetStackDirection(cee::gui::Box::StackDirection::Vertical);
		padBox->Resize(100.f, 500.f);
		padBox->SetStackDirection(cee::gui::Box::StackDirection::Vertical);
		lesbianBox->SetStackDirection(cee::gui::Box::StackDirection::Vertical);

		root->AddChild(hbox.get());
		hbox->AddChild(lesbianBox.get());
		hbox->AddChild(padBox.get());
		hbox->AddChild(transBox.get());

		transFlagStrip1->Resize(250.f, 100.f);
		transFlagStrip1->SetColor({ .332f, .8f, .984f, 1.f });
		transBox->AddChild(transFlagStrip1.get());
		transFlagStrip2->Resize(250.f, 100.f);
		transFlagStrip2->SetColor({ .965f, .656f, .719f, 1.f });
		transBox->AddChild(transFlagStrip2.get());
		transFlagStrip3->Resize(250.f, 100.f);
		transFlagStrip3->SetColor({ 1.f, 1.f, 1.f, 1.f });
		transBox->AddChild(transFlagStrip3.get());
		transFlagStrip4->Resize(250.f, 100.f);
		transFlagStrip4->SetColor({ .965f, .656f, .719f, 1.f });
		transBox->AddChild(transFlagStrip4.get());
		transFlagStrip5->Resize(250.f, 100.f);
		transFlagStrip5->SetColor({ .332f, .8f, .984f, 1.f });
		transBox->AddChild(transFlagStrip5.get());

		lesbianFlagStrip1->Resize(250.f, 71.429f);
		lesbianFlagStrip1->SetColor(gui::HexToColor(0xD52D00FF));
		lesbianBox->AddChild(lesbianFlagStrip1.get());
		lesbianFlagStrip2->Resize(250.f, 71.429f);
		lesbianFlagStrip2->SetColor(gui::HexToColor(0xEF7627FF));
		lesbianBox->AddChild(lesbianFlagStrip2.get());
		lesbianFlagStrip3->Resize(250.f, 71.429f);
		lesbianFlagStrip3->SetColor(gui::HexToColor(0xFF9A56FF));
		lesbianBox->AddChild(lesbianFlagStrip3.get());
		lesbianFlagStrip4->Resize(250.f, 71.429f);
		lesbianFlagStrip4->SetColor(gui::HexToColor(0xFFFFFFFF));
		lesbianBox->AddChild(lesbianFlagStrip4.get());
		lesbianFlagStrip5->Resize(250.f, 71.429f);
		lesbianFlagStrip5->SetColor(gui::HexToColor(0xD162A4FF));
		lesbianBox->AddChild(lesbianFlagStrip5.get());
		lesbianFlagStrip6->Resize(250.f, 71.429f);
		lesbianFlagStrip6->SetColor(gui::HexToColor(0xB55690FF));
		lesbianBox->AddChild(lesbianFlagStrip6.get());
		lesbianFlagStrip7->Resize(250.f, 71.429f);
		lesbianFlagStrip7->SetColor(gui::HexToColor(0xA30262FF));
		lesbianBox->AddChild(lesbianFlagStrip7.get());
	}

	std::chrono::time_point start = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float, std::milli> delta;

	m_Running = true;
	while (m_Running) {
		PROFILE_SCOPE("Main loop");
		uint32_t windowWidth = m_HalGfx->GetWidth();
		uint32_t windowHeight = m_HalGfx->GetHeight();
		cee::gui::BeginFrame({ windowWidth, windowHeight });
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		cee::gui::Render(windowWidth, windowHeight);
		cee::gui::EndFrame();
		m_HalGfx->SwapBuffers();
		PROFILER_FRAME_MARK();
		{
			PROFILE_SCOPE("Input");
			ApplicationPageFlip flip;
			OnEvent(flip);
			Input::Poll();
			if (cee::gui::HandleEvents() < 0) {
				CEE_CORE_WARN("Failed to handle GUI events");
			}

			delta = std::chrono::high_resolution_clock::now() - start;;
			start = std::chrono::high_resolution_clock::now();
			ApplicationTickEvent tick(delta.count());
			OnEvent(tick);
		}
	}

	return EXIT_SUCCESS;
}

void MPPM::OnEvent(Event& e) {
	PROFILE_SCOPE("Event dispatch");
	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<KeyDownEvent>([this](auto &&...args) -> decltype(auto) { this->OnKeyPress(std::forward<decltype(args)>(args)...); });
	dispatcher.Dispatch<KeyUpEvent>([this](auto &&...args) -> decltype(auto) { this->OnKeyPress(std::forward<decltype(args)>(args)...); });
	dispatcher.Dispatch<ApplicationTickEvent>([this](auto &&...args) -> decltype(auto) { this->OnTick(std::forward<decltype(args)>(args)...); });
	dispatcher.Dispatch<ApplicationPageFlip>([this](auto &&...args) -> decltype(auto) { this->OnPageFlip(std::forward<decltype(args)>(args)...); });
	dispatcher.Dispatch<ApplicationExitEvent>([this](auto &&...args) -> decltype(auto) { this->OnExit(std::forward<decltype(args)>(args)...); });
}

void MPPM::OnKeyPress(KeyDownEvent &e) {

}

void MPPM::OnKeyPress(KeyUpEvent &e) {
	if (e.GetKeycode() == KEY_Q) {
		CEE_CORE_INFO("q key pressed, exiting...");
		ApplicationExitEvent exitEvent;
		OnEvent(exitEvent);
	}
}

void MPPM::OnTick(ApplicationTickEvent &e) {

}

void MPPM::OnPageFlip(ApplicationPageFlip &e) {

}

void MPPM::OnExit(ApplicationExitEvent &e) {
	m_Running = false;
}

void MPPM::SetSigHandlers() {
	PROFILE_FUNCTION();
	signal(SIGTERM, MPPM::SigHandler);
	signal(SIGTRAP, MPPM::SigHandler);
	signal(SIGINT, MPPM::SigHandler);
	signal(SIGHUP, MPPM::SigHandler);
	signal(SIGQUIT, MPPM::SigHandler);
}

static const char *SigMessageStart = "Recieved signal: ";
static const char *SigMessageEnd = "... Shutting down.\n";
static const char *OtherSigMessageStart = "Recieved other signal: ";
static const char *OtherSigMessageEnd = "... Exiting\n";

// This is not thread safe, there is still risk of race conditions, it is
// okay for signals and probably will not catastrophically break.
static void SigSafePrintMessage(int SIG, FILE *file, const char *start, const char *end) {
	write(fileno(file), start, strlen(start));
	const char *sigStr = strsignal(SIG);
	write(fileno(file), sigStr, strlen(sigStr));
	write(fileno(file), end, strlen(end));
}

void MPPM::SigHandler(int SIG) {
	switch (SIG) {
	case SIGTERM:
	case SIGTRAP:
	case SIGINT:
	case SIGHUP:
	case SIGQUIT:
		SigSafePrintMessage(SIG, stdout, SigMessageStart, SigMessageEnd);
		s_Instance->m_Running = false;
		break;
	default:
		SigSafePrintMessage(SIG, stderr, OtherSigMessageStart, OtherSigMessageEnd);
		exit(1);
		break;
	}
}

void MPPM::ParseCommandLineArgs(int argc, char *argv[]) {
	int opt;
	while ((opt = getopt_long(argc, argv, g_OptString, g_LongOptions, nullptr)) != -1) {
		switch (opt) {
		case 'g':
			if (strcmp(optarg, "drm") == 0) {
				hal::SetGfxBackend(HAL_GFX_BACKEND_DRM);
			} else if (strcmp(optarg, "x11") == 0) {
				hal::SetGfxBackend(HAL_GFX_BACKEND_X11);
			} else {
				std::fprintf(stderr, "Invalid graphics backend: %s\n", optarg);
				PrintHelpMessage(argv[0]);
			}
			break;
		case 'i':
			if (strcmp(optarg, "hw") == 0) {
				hal::SetI2CBackend(HAL_I2C_BACKEND_HW);
			} else if (strcmp(optarg, "mock") == 0) {
				hal::SetI2CBackend(HAL_I2C_BACKEND_MOCK);
			} else {
				std::fprintf(stderr, "Invalid i2c backend: %s\n", optarg);
				PrintHelpMessage(argv[0]);
			}
			break;
		case 'l':
			if (strcmp(optarg, "debug") == 0) {
				Log::SetLogLevel(spdlog::level::debug);
			} else if (strcmp(optarg, "trace") == 0) {
				Log::SetLogLevel(spdlog::level::trace);
			} else if (strcmp(optarg, "info") == 0) {
				Log::SetLogLevel(spdlog::level::info);
			} else if (strcmp(optarg, "warn") == 0) {
				Log::SetLogLevel(spdlog::level::warn);
			} else if (strcmp(optarg, "error") == 0) {
				Log::SetLogLevel(spdlog::level::err);
			} else {
				std::fprintf(stderr, "Invalid log level: %s\n", optarg);
				PrintHelpMessage(argv[0]);
			}
			break;
		case ARG_LOGFILE: {
			if (!std::filesystem::is_directory(optarg)) {
				std::fprintf(stderr, "Log file path must be absolute: %s\n", optarg);
				PrintHelpMessage(argv[0]);
			}
			Log::SetLogLocation(optarg);
			break;
		}
		case 'h':
			PrintHelpMessage(argv[0]);
			break;
		case 'v':
			PrintVersion(argv[0]);
			break;
		default:
			PrintHelpMessage(argv[0]);
			break;
		}
	}
}

static void PrintHelpMessage(const char *cmd) {
	std::printf("Usage: %s [options]\n", cmd);
	std::printf("Options:\n");
	std::printf("\t-g <backend>     Select graphics backend. {drm|x11} default: drm\n");
	std::printf("\t-h, --help       Show this help message and exit\n");
	std::printf("\t-i <backend>     Select i2c backend. {hw|mock} default: hw\n");
	std::printf("\t-l <level>       Set log level {debug|trace|info|warn|error} default: info\n");
	std::printf("\t--logfile=<file> Set log file path.");
	std::printf("\t                 default: $HOME/.local/share/ceeMPPM/\n");
	std::printf("\t-v, --version    Show version information and exit\n");
	std::exit(0);
}

static void PrintVersion(const char *cmd) {
	(void)cmd; // Supress unused warning
	std::printf("ceeMPPM version %d.%d\n", MPPM_VERSION_MAJOR, MPPM_VERSION_MINOR);
	std::exit(0);
}
}

