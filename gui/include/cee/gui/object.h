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

#ifndef CEE_GUI_OBJECT_H_
#define CEE_GUI_OBJECT_H_

#include <any>
#include <functional>
#include <memory>
#include <set>
#include <utility>

#include <cstdint>

namespace cee {
namespace gui {
	enum class Signal : int64_t {
		None = 0,
		Activate = 1,
		Destroy = 2,
		Hide = 3,
		Show = 4,
		Clicked = 5
	};

	template<typename... Args>
	struct SignalHandler {
		Signal sig;
		std::function<void(Args...)> f;

		void invoke(Args&&... args);

		void operator()(Args... args) {
			invoke(std::forward<Args>(args)...);
		}

		bool operator==(const SignalHandler<Args...>& other) {
			return (this->sig == other.sig) &&
			(std::hash<SignalHandler<Args...>>(*this) == std::hash<SignalHandler<Args...>>(other));
		}
	};


	class Object {
	public:
		template <typename... Args>
		int64_t SignalConnect(const Signal sig, std::function<void(Args...)> f);
		void SignalDisconnect(int64_t handlerId);
		template <typename... Args>
		void SignalDisconnectByFunc(const Signal sig, std::function<void(Args...)> f);

	protected:
		std::shared_ptr<Object> m_Parent;

	private:
		std::set<std::any> m_SignalCallbacks;
	};
}
}

#endif

