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

#ifndef CEE_SHADER_H_
#define CEE_SHADER_H_

#include <cee/mppm/log.h>

#include <string>
#include <unordered_map>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace cee {
// Forward declare instead of include header to avoid cyclical inclusion
class Renderer;

class Shader {
private:
	enum class ShaderType : uint16_t {
		NONE  = 0,
		FLOAT = 1,
		INT   = 2,
		MAT   = 3
	};

public:
	Shader(std::shared_ptr<Renderer> renderer);
	Shader(const Shader&) = delete;
	Shader(Shader&& other);
	~Shader();

	int SetVertexSourceFile(const std::string &path);
	int SetFragmentSourceFile(const std::string &path);

	void SetVertexSourceString(const std::string &source);
	void SetFragmentSourceString(const std::string &source);

	void FreeSourceStrings();

	int Compile();
	void Destroy();
	
	void Bind();
	void Unbind();

	template<typename... Args>
	inline void SetUniform(const std::string& name, Args&&... args) {
		SetUniform(GetUnifromLocation(name), std::forward<Args>(args)...);
	}

	Shader& operator=(const Shader& other) = delete;
	Shader& operator=(Shader&& other);

private:
	static std::string ReadFile(const std::string &path);

	void GetVersionAndApi(std::shared_ptr<Renderer> renderer);

	int ValidateShaderVersion(const std::string& shaderSource);

	GLuint GetUnifromLocation(const std::string &name);

	template<typename T>
	static constexpr ShaderType GetShaderType() {
		if constexpr (std::is_same_v<T, float>) {
			return ShaderType::FLOAT;
		} else if constexpr (std::is_same_v<T, int>) {
			return ShaderType::INT;
		} else if constexpr (std::is_same_v<T, glm::mat4>) {
			return ShaderType::MAT;
		}
		else {
			return ShaderType::NONE;
		}
	}

	template<typename... Args>
	static void SetUniform(GLuint location, const Args&... args) {
		static_assert(sizeof...(Args) > 0,
			"No arguements provided");
		using ArgType = typename std::tuple_element_t<0, std::tuple<Args...>>;
		static_assert((std::is_same_v<ArgType, Args> && ...),
			"Cannot use different types in the same uniform");

		constexpr size_t n = sizeof...(Args);
		constexpr ShaderType st = GetShaderType<ArgType>();

		if constexpr (st == ShaderType::FLOAT) {
			if constexpr (n == 1) {
				glUniform1f(location, args...);
			} else if constexpr (n == 2) {
				glUniform2f(location, args...);
			} else if constexpr (n == 3) {
				glUniform3f(location, args...);
			} else if constexpr (n == 4) {
				glUniform4f(location, args...);
			} else {
				CEE_ERROR("Unexpected shader uniform value count");
			}
		} else if constexpr (st == ShaderType::INT) {
			if constexpr (n == 1) {
				glUniform1i(location, args...);
			} else if constexpr (n == 2) {
				glUniform2i(location, args...);
			} else if constexpr (n == 3) {
				glUniform3i(location, args...);
			} else if constexpr (n == 4) {
				glUniform4i(location, args...);
			} else {
				CEE_ERROR("Unexpected shader uniform value count");
			}
		} else if constexpr (st == ShaderType::MAT) {
			if constexpr (n == 1) {
				const glm::mat4& mat = {args...};
				glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
			} else {
				CEE_ERROR("Unexpected shader uniform value count");
			}
		} else {
			CEE_ERROR("Unexpected shader uniform type: {}", static_cast<uint16_t>(st));
		}
	}

private:
	enum class glAPI {
		OpenGL_ES = 0,
		OpenGL,
		ENUM_MAX = OpenGL_ES

	} m_glAPI;
	enum class glVersion {
		OpenGL_ES_2_0 = 0,
		OpenGL_ES_3_0,
		OpenGL_ES_3_1,
		OpenGL_ES_3_2,
		OpenGL_3_0,
		OpenGL_3_1,
		OpenGL_3_2,
		OpenGL_3_3,
		OpenGL_4_0,
		OpenGL_4_1,
		OpenGL_4_2,
		OpenGL_4_3,
		OpenGL_4_4,
		OpenGL_4_5,
		OpenGL_4_6,
		ENUM_MAX = OpenGL_4_6

	} m_glVersion;

	std::string m_FragmentSource;
	std::string m_VertexSource;

	GLuint m_Program;
	std::unordered_map<std::string, GLuint> m_UniformLoacations;
};
}

#endif

