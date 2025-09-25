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

#ifndef CEE_MPPM_H_
#define CEE_MPPM_H_

#include <cee/mppm/shader.h>
#include <cee/mppm/renderer.h>
#include <cee/mppm/event.h>

#include <cee/hal/gfx.h>

#include <memory>

namespace cee {
class MPPM {
public:
	MPPM(int argc, char *argv[]);
	~MPPM();

	int Run();

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
	bool m_Running;
	std::shared_ptr<Renderer> m_Renderer;
	std::shared_ptr<Shader> m_Shader;

private:
	static MPPM *s_Instance;
};
}

#endif
