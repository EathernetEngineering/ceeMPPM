/*
 * CeeHealth
 * Copyright (C) 2025 Chloe Eather
 *
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * vcyclycial cyclycial ersion.
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
#include <config.h>

#include <cee/hal/hal.h>
#include <cee/hal/logger.h>

#include <chrono>
#include <csignal>
#include <functional>

#include <signal.h>

#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon-keysyms.h>

namespace cee {
MPPM* MPPM::s_Instance = nullptr;

MPPM::MPPM() {
	if (s_Instance) {
		std::fprintf(stderr, "More than one instance of cee::CardiacMonitor is not allowed.");
		std::exit(EXIT_FAILURE);
	}
	s_Instance = this;
	bool running = true;

	Log::Init();
	HALLogInit();
	Log::AddLogger(hal::GetLogger());

	rng<int>::Init();

	if (Input::Init() < 0) {
		throw std::runtime_error("Failed to initialize input system");
	}
	Input::SetEventCallback(std::bind(&MPPM::OnEvent, this, std::placeholders::_1));

#if BUILD_HAL_DRM
	HALSetGfxBackend(HAL_GFX_BACKEND_DRM);
#elif BUILD_HAL_EGL_X11
	HALSetGfxBackend(HAL_GFX_BACKEND_EGL_X);
#elif BUILD_HAL_GLX
	HALSetGfxBackend(HAL_GFX_BACKEND_GLX);
#endif
#if BUILD_HAL_I2C
	// FIXME: Change this to I2C once implemented.
	HALSetI2CBackend(HAL_I2C_BACKEND_SIM);
#elif BUILD_HAL_I2C_SIM
	HALSetI2CBackend(HAL_I2C_BACKEND_SIM);
#endif

	if (HALInit()) {
		CEE_ERROR("Failed to initialize hardware abstration layer!");
		throw std::runtime_error("Failed to initialize hardware abstration layer!");
	}

	try {
		m_Renderer = cee::Renderer::Create();
	} catch (const std::runtime_error& e) {
		CEE_ERROR("Renderer uninitialized!");
		throw std::runtime_error("Renderer uninitialized!");
	}

	m_Shader = std::make_shared<Shader>(m_Renderer);
#if BUILD_GLES
	m_Shader->SetVertexSourceString(
		"#version 100\n"
		"\n"
		"attribute vec4 a_Position;\n"
		"attribute vec4 a_Color;\n"
		"attribute vec2 a_UV;\n"
		"\n"
		"precision mediump float;\n"
		"\n"
		"uniform vec2 u_Viewport;\n"
		"\n"
		"varying vec4 v_Color;\n"
		"\n"
		"void main() {\n"
		"	gl_Position = vec4(2.0 * a_Position.xy / u_Viewport.xy - 1.0, 0.0, 1.0);\n"
		"	v_Color = a_Color;\n"
		"}\n");
	m_Shader->SetFragmentSourceString(
		"#version 100\n"
		"\n"
		"precision mediump float;\n"
		"\n"
		"varying vec4 v_Color;\n"
		"\n"
		"void main() {\n"
		// "	gl_FragColor = v_Color;\n"
		"	gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
		"}\n");
	if (m_Shader->Compile() != 0) {
		throw std::runtime_error("Failed to compile and link shader");
	}
#elif BUILD_GL
	m_Shader->SetVertexSourceString(
		"#version 330 core\n"
		"\n"
		"layout (location = 0) in vec4 a_Position;\n"
		"layout (location = 1) in vec4 a_Color;\n"
		"layout (location = 2) in vec2 a_UV;\n"
		"\n"
		"uniform vec2 u_Viewport;\n"
		"\n"
		"out vec4 v_Color;\n"
		"\n"
		"void main() {\n"
		"	gl_Position = vec4(2.0 * a_Position.xy / u_Viewport.xy - 1.0, 0.0, 1.0);\n"
		"	v_Color = a_Color;\n"
		"}\n");
	m_Shader->SetFragmentSourceString(
		"#version 330 core\n"
		"\n"
		"in vec4 v_Color;\n"
		"\n"
		"layout (location = 0) out vec4 fragColor;\n"
		"\n"
		"void main() {\n"
		"	fragColor = v_Color;\n"
		"}\n");
	if (m_Shader->Compile() != 0) {
		throw std::runtime_error("Failed to compile and link shader");
	}
#endif

}

MPPM::~MPPM() {
	m_Shader.reset();

	m_Renderer.reset();
	cee::Input::Shutdown();
}

int MPPM::Run() {
	SetSigHandlers();

	std::chrono::time_point start = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float, std::milli> delta;

	m_Running = true;
	while (m_Running) {
		m_Renderer->StartFrame();
		m_Renderer->ClearColor({ 1.0f, 0.36f, 0.835f, 1.0f });
		m_Renderer->Clear();

		m_Shader->Bind();
		m_Shader->SetUniform(
			"u_Viewport",
				static_cast<float>(m_Renderer->GetWidth()),
				static_cast<float>(m_Renderer->GetHeight())
			);
		m_Renderer->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
		m_Renderer->DrawQuad({ 0.0f, 0.0f, 0.0f }, { 200.f, 200.f, 200.f });
		m_Renderer->EndFrame();

		m_Renderer->SwapBuffers();
		ApplicationPageFlip flip;
		OnEvent(flip);
		Input::Poll();

		delta = std::chrono::high_resolution_clock::now() - start;;
		start = std::chrono::high_resolution_clock::now();
		ApplicationTickEvent tick(delta.count());
		OnEvent(tick);
	}

	return EXIT_SUCCESS;
}

void MPPM::OnEvent(Event& e) {
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
}

