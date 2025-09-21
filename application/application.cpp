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

#include <cee/mppm/mppm.h>

#include <memory>

#include <cstdlib>

int main(int argc, char **argv) {
	std::unique_ptr<cee::MPPM> app(nullptr);
	try {
		app = std::make_unique<cee::MPPM>();
	} catch(const std::exception& e) {
		fprintf(stderr, "Caught initialization exception:\n");
		fprintf(stderr, "%s\n", e.what());
		std::exit(0);
	}

	int ret = 0;
	try {
		ret = app->Run();
	} catch(const std::exception& e) {
		fprintf(stderr, "Caught runtime exception\n");
		fprintf(stderr, "%s\n", e.what());
		std::exit(0);
	}
	app.reset();

	return ret;
}

