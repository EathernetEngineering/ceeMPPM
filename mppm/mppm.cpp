/*
 * ceeMPPM
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

#include <cee/core/except.h>

#include <cee/profiler/profiler.h>

#include <cee/gui/gui.h>
#include <cee/gui/box.h>
#include <cee/gui/text.h>
#include <cee/gui/plot.h>

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
	ParseCommandLineArgs(argc, argv);
	PROFILE_SCOPE("Initialization");
	if (s_Instance) {
		std::fprintf(stderr, "More than one instance of cee::MPPM is not allowed.");
		throw core::UsageError("More than one instance of cee::MPPM is not allowed.");
	}
	s_Instance = this;

	m_Log = std::make_unique<Log>("MPPM", m_LogFile, m_LogLevel);

	rng<int>::Init();

	if (Input::Init() < 0) {
		throw core::InternalError("Failed to initialize input system");
	}
	Input::SetEventCallback(std::bind(&MPPM::OnEvent, this, std::placeholders::_1));

	if (m_GfxBackend == platform::GfxContextType::PLATFORM_GFX_CONTEXT_NONE) {
#if BUILD_PLATFORM_DRM
		m_GfxBackend = platform::GfxContextType::PLATFORM_GFX_CONTEXT_DRM;
#else
		m_GfxBackend = platform::GfxContextType::PLATFORM_GFX_CONTEXT_X11;
#endif
	}

	if (m_GfxBackend == platform::GfxContextType::PLATFORM_GFX_CONTEXT_DRM) {
		CEE_CORE_DEBUG("Using DRM for rendering");
		m_GfxContext = platform::GraphicsContext::Create(platform::GfxContextType::PLATFORM_GFX_CONTEXT_DRM,
				m_Log->CreateChild("DRM"));
	} else if (m_GfxBackend == platform::GfxContextType::PLATFORM_GFX_CONTEXT_X11) {
		CEE_CORE_DEBUG("Using X11 for rendering");
		m_GfxContext = platform::GraphicsContext::Create(platform::GfxContextType::PLATFORM_GFX_CONTEXT_X11,
				m_Log->CreateChild("X11"));
	} else {
		CEE_CORE_ERROR("No graphics backend detected!");
		throw core::UsageError("No graphics backend detected");
	}

	if (!m_GfxContext)
		throw core::InternalError("Failed to create graphics context");

	if (m_I2CBackend == platform::I2CContextType::PLATFORM_I2C_CONTEXT_NONE) {
#if BUILD_PLATFORM_I2C_HW
		m_I2CBackend = platform::I2CContextType::PLATFORM_I2C_CONTEXT_HW;
#else
		m_I2CBackend = platform::I2CContextType::PLATFORM_I2C_CONTEXT_MOCK;
#endif
	}

	if (m_I2CBackend == platform::I2CContextType::PLATFORM_I2C_CONTEXT_HW) {
		CEE_CORE_DEBUG("Using I2C interface {}", "/dev/i2c-0");
		m_I2CController = platform::I2CController::Create("/dev/i2c-0",
				platform::I2CContextType::PLATFORM_I2C_CONTEXT_HW,
				m_Log->CreateChild("I2C"));
	} else if (m_I2CBackend == platform::I2CContextType::PLATFORM_I2C_CONTEXT_MOCK) {
		CEE_CORE_DEBUG("Using mock I2C interface");
		m_I2CController = platform::I2CController::Create("/dev/i2c-0",
				platform::I2CContextType::PLATFORM_I2C_CONTEXT_MOCK,
				m_Log->CreateChild("I2C"));
	} else {
		CEE_CORE_ERROR("No I2C backend detected!");
		throw core::UsageError("No I2C backend detected");
	}

	if (!m_I2CController)
		throw core::InternalError("Failed to create I2C interface");

	m_GfxContext->Init();
	m_Adc = std::make_unique<platform::PCF8591>(m_I2CController, 0x48);

	gui::Init(m_Log->CreateChild("GUI"));
}

MPPM::~MPPM() {
	gui::Shutdown();
	m_GfxContext->Shutdown();
	m_I2CController.reset();
	Input::Shutdown();
	m_Log.reset();
}

int MPPM::Run() {
	SetSigHandlers();

	auto root = gui::CreateNode<gui::Box>();
	auto vbox = gui::CreateNode<gui::Box>();
	auto line1Box = gui::CreateNode<gui::Box>();
	auto line1GraphBox = gui::CreateNode<gui::Box>();
	auto line1TextBox = gui::CreateNode<gui::Box>();
	std::unique_ptr<gui::Plot> line1Plot = gui::CreateNode<gui::Plot>(
			gui::Color{ 0.1f, 1.0f, 0.1f, 1.0f });
	std::unique_ptr<gui::Text> line1Num = gui::CreateNode<gui::Text>(
			"167", 48, gui::Color{ 0.1f, 1.0f, 0.1f, 1.0f });
	auto line2Box = gui::CreateNode<gui::Box>();
	auto line2GraphBox = gui::CreateNode<gui::Box>();
	auto line2TextBox = gui::CreateNode<gui::Box>();
	std::unique_ptr<gui::Plot> line2Plot = gui::CreateNode<gui::Plot>(
			gui::Color{ 1.0f, 0.1f, 0.1f, 1.0f });
	std::unique_ptr<gui::Text> line2Num = gui::CreateNode<gui::Text>(
			"0", 48, gui::Color{ 1.0f, 0.1f, 0.1f, 1.0f });
	auto line3Box = gui::CreateNode<gui::Box>();
	auto line3GraphBox = gui::CreateNode<gui::Box>();
	auto line3TextBox = gui::CreateNode<gui::Box>();
	std::unique_ptr<gui::Plot> line3Plot = gui::CreateNode<gui::Plot>(
			gui::Color{ 0.5f, 0.2f, 0.2f, 1.0f });
	std::unique_ptr<gui::Text> line3Num = gui::CreateNode<gui::Text>(
			"0", 48, gui::Color{ 0.5f, 0.2f, 0.2f, 1.0f });

	{
		PROFILE_SCOPE("Setup GUI");

		m_LeadII.resize(1000, 0.f);
		m_LeadIIPos = 0;
		m_Pres.resize(1000, 0.f);
		m_PresPos = 0;
		m_Osc.resize(1000, 0.f);
		m_OscPos = 0;

		root->SetDebugName("root");
		root->SetStackDirection(gui::Box::StackDirection::Horizontal);
		gui::SetRootNode(root.get());

		vbox->SetDebugName("vbox");
		line1Box->SetDebugName("line1Box");
		line1GraphBox->SetDebugName("line1GraphBox");
		line1TextBox->SetDebugName("line1TextBox");
		line1Plot->SetDebugName("line1Graph");
		line1Num->SetDebugName("line1Num");
		line2Box->SetDebugName("line2Box");
		line2GraphBox->SetDebugName("line2GraphBox");
		line2TextBox->SetDebugName("line2TextBox");
		line2Plot->SetDebugName("line2Graph");
		line2Num->SetDebugName("line2Num");
		line3Box->SetDebugName("line3Box");
		line3GraphBox->SetDebugName("line3GraphBox");
		line3TextBox->SetDebugName("line3TextBox");
		line3Plot->SetDebugName("line3Graph");
		line3Num->SetDebugName("line3Num");
		
		vbox->SetStackDirection(gui::Box::StackDirection::Vertical);
		line1Box->SetStackDirection(gui::Box::StackDirection::Horizontal);
		line1GraphBox->SetStackDirection(gui::Box::StackDirection::Horizontal);
		line1TextBox->SetStackDirection(gui::Box::StackDirection::Horizontal);
		line2Box->SetStackDirection(gui::Box::StackDirection::Horizontal);
		line2GraphBox->SetStackDirection(gui::Box::StackDirection::Horizontal);
		line2TextBox->SetStackDirection(gui::Box::StackDirection::Horizontal);
		line3Box->SetStackDirection(gui::Box::StackDirection::Horizontal);
		line3GraphBox->SetStackDirection(gui::Box::StackDirection::Horizontal);
		line3TextBox->SetStackDirection(gui::Box::StackDirection::Horizontal);

		line1Box->Resize(620.f, 250.f);
		line1GraphBox->Resize(500.f, 250.f);
		line1TextBox->Resize(120.f, 250.f);
		line1Plot->ResizeData(1000);
		line2Box->Resize(620.f, 250.f);
		line2GraphBox->Resize(500.f, 250.f);
		line2TextBox->Resize(120.f, 250.f);
		line2Plot->ResizeData(1000);
		line3Box->Resize(620.f, 250.f);
		line3GraphBox->Resize(500.f, 250.f);
		line3TextBox->Resize(120.f, 250.f);
		line3Plot->ResizeData(1000);

		line1Plot->SetLineBreakWidth(10);
		line2Plot->SetLineBreakWidth(10);
		line3Plot->SetLineBreakWidth(10);

		line1Plot->Show(true);
		line1Num->Show(true);
		line2Plot->Show(true);
		line2Num->Show(true);
		line3Plot->Show(true);
		line3Num->Show(true);

		root->AddChild(vbox.get());
		vbox->AddChild(line1Box.get());
		line1Box->AddChild(line1GraphBox.get());
		line1Box->AddChild(line1TextBox.get());
		line1GraphBox->AddChild(line1Plot.get());
		line1TextBox->AddChild(line1Num.get());
		vbox->AddChild(line2Box.get());
		line2Box->AddChild(line2GraphBox.get());
		line2Box->AddChild(line2TextBox.get());
		line2GraphBox->AddChild(line2Plot.get());
		line2TextBox->AddChild(line2Num.get());
		vbox->AddChild(line3Box.get());
		line3Box->AddChild(line3GraphBox.get());
		line3Box->AddChild(line3TextBox.get());
		line3GraphBox->AddChild(line3Plot.get());
		line3TextBox->AddChild(line3Num.get());
	}

	std::chrono::time_point start = std::chrono::high_resolution_clock::now();
	std::chrono::duration<size_t, std::micro> delta;

	m_Running = true;
	while (m_Running) {
		PROFILE_SCOPE("Main loop");

		float windowWidth = static_cast<float>(m_GfxContext->GetWidth());
		float windowHeight = static_cast<float>(m_GfxContext->GetHeight());
		gui::BeginFrame({ windowWidth, windowHeight });
		line1Plot->SetData(m_LeadII.data(), m_LeadII.size());
		line1Plot->SetLineBreakPos(m_LeadIIPos);
		line2Plot->SetData(m_Pres.data(), m_Pres.size());
		line2Plot->SetLineBreakPos(m_PresPos);
		line3Plot->SetData(m_Osc.data(), m_Osc.size());
		line3Plot->SetLineBreakPos(m_OscPos);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		gui::Render({ windowWidth, windowHeight });
		gui::EndFrame();
		m_GfxContext->SwapBuffers();
		PROFILER_FRAME_MARK();
		{
			using std::chrono::high_resolution_clock;
			PROFILE_SCOPE("Input");

			ApplicationPageFlip flip;
			OnEvent(flip);

			m_Adc->SendControl(0, false, platform::PCF8591::InputMode::SINGLE_ENDED, false);
			uint8_t adcCh0 = m_Adc->Read();
			m_LeadII[m_LeadIIPos++] = adcCh0 / 255.f;
			if (m_LeadIIPos == 1000) {
				m_LeadIIPos = 0;
			}
			m_Adc->SendControl(1, false, platform::PCF8591::InputMode::SINGLE_ENDED, false);
			uint8_t adcCh1 = m_Adc->Read();
			m_Pres[m_PresPos++] = adcCh1 / 255.f;
			if (m_PresPos == 1000) {
				m_PresPos = 0;
			}
			m_Adc->SendControl(2, false, platform::PCF8591::InputMode::SINGLE_ENDED, false);
			uint8_t adcCh2 = m_Adc->Read();
			m_Osc[m_OscPos++] = adcCh2 / 255.f;
			if (m_OscPos == 1000) {
				m_OscPos = 0;
			}

			Input::Poll();
			if (gui::HandleEvents() < 0) {
				CEE_CORE_WARN("Failed to handle GUI events");
			}

			delta = std::chrono::duration_cast<std::chrono::microseconds>(high_resolution_clock::now() - start);
			start = high_resolution_clock::now();
			ApplicationTickEvent tick(static_cast<float>(delta.count()) / 1000.f);
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
				m_GfxBackend = platform::GfxContextType::PLATFORM_GFX_CONTEXT_DRM;
			} else if (strcmp(optarg, "x11") == 0) {
				m_GfxBackend = platform::GfxContextType::PLATFORM_GFX_CONTEXT_X11;
			} else {
				std::fprintf(stderr, "Invalid graphics backend: %s\n", optarg);
				PrintHelpMessage(argv[0]);
			}
			break;
		case 'i':
			if (strcmp(optarg, "hw") == 0) {
				m_I2CBackend = platform::I2CContextType::PLATFORM_I2C_CONTEXT_HW;
			} else if (strcmp(optarg, "mock") == 0) {
				m_I2CBackend = platform::I2CContextType::PLATFORM_I2C_CONTEXT_MOCK;
			} else {
				std::fprintf(stderr, "Invalid i2c backend: %s\n", optarg);
				PrintHelpMessage(argv[0]);
			}
			break;
		case 'l':
			if (strcmp(optarg, "debug") == 0) {
				m_LogLevel = spdlog::level::debug;
			} else if (strcmp(optarg, "trace") == 0) {
				m_LogLevel = spdlog::level::trace;
			} else if (strcmp(optarg, "info") == 0) {
				m_LogLevel = spdlog::level::info;
			} else if (strcmp(optarg, "warn") == 0) {
				m_LogLevel = spdlog::level::warn;
			} else if (strcmp(optarg, "error") == 0) {
				m_LogLevel = spdlog::level::err;
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
			m_LogFile = optarg;
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
	std::printf("\t--logfile=<file> Set log file location.");
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

