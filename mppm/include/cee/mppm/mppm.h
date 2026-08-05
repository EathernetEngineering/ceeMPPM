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

#ifndef CEE_MPPM_H_
#define CEE_MPPM_H_

#include <cee/mppm/event.h>

#include <cee/core/log.h>

#include <cee/platform/gfx.h>
#include <cee/platform/i2c.h>

#include <memory>
#include <vector>

namespace cee {
class MPPM {
public:
	MPPM(int argc, char *argv[]);
	~MPPM();

	int Run();

	static Logger &GetLogger() { return s_Instance->m_Log->GetLogger(); }

private:
	void OnEvent(Event &e);

	void OnKeyPress(KeyDownEvent &e);
	void OnKeyPress(KeyUpEvent &e);
	void OnTick(ApplicationTickEvent &e);
	void OnPageFlip(ApplicationPageFlip &e);
	void OnExit(ApplicationExitEvent &e);

	void DisableTerminal();
	void EnableTerminal();

	void SetSigHandlers();
	static void SigHandler(int SIG);

private:
	void ParseCommandLineArgs(int argc, char *argv[]);

private:
	bool m_Running;
	std::unique_ptr<Log> m_Log;
	spdlog::level::level_enum m_LogLevel;
	std::string m_LogFile;
	platform::GfxContextType m_GfxBackend = platform::GfxContextType::PLATFORM_GFX_CONTEXT_NONE;
	platform::I2CContextType m_I2CBackend = platform::I2CContextType::PLATFORM_I2C_CONTEXT_NONE;
	std::shared_ptr<platform::I2CController> m_I2CController;
	std::unique_ptr<platform::PCF8591> m_Adc;
	std::unique_ptr<platform::GraphicsContext> m_GfxContext;

	std::vector<float> m_LeadII;
	int m_LeadIIPos;
	std::vector<float> m_Pres;
	int m_PresPos;
	std::vector<float> m_Osc;
	int m_OscPos;

private:
	static MPPM *s_Instance;
};
}

#endif
