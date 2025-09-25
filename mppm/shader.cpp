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

#include <cee/mppm/shader.h>
#include <cee/mppm/renderer.h>
#include <cee/mppm/log.h>

#include <glad/gl.h>
#include <glm/ext/matrix_transform.hpp>

#include <fstream>
#include <stdexcept>
#include <string>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

namespace cee {
static int GetShaderStatus(GLuint shader, const std::string& stageName)
{
	int success;
	int logLength = 0;
	std::string log;
	
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

	if (!success) {
		log.reserve(logLength + 1);
		log.resize(logLength + 1);
		glGetShaderInfoLog(shader, log.size(), nullptr, log.data());
		CEE_CORE_WARN("Failed to compile {} shader:", stageName);
		CEE_CORE_WARN("{}", log);
		return -1;
	} else if (logLength) {
		log.reserve(logLength + 1);
		log.resize(logLength + 1);
		glGetShaderInfoLog(shader, log.size(), nullptr, log.data());
		CEE_CORE_DEBUG("GLSL compiler {} shader log:", stageName);
		CEE_CORE_DEBUG("{}", log);
	}
	return 0;
}

static int GetProgramStatus(GLuint prgram)
{
	int success;
	int logLength = 0;
	std::string log;
	
	glGetProgramiv(prgram, GL_LINK_STATUS, &success);
	glGetProgramiv(prgram, GL_INFO_LOG_LENGTH, &logLength);

	if (!success) {
		log.reserve(logLength + 1);
		log.resize(logLength + 1);
		glGetProgramInfoLog(prgram, log.size(), nullptr, log.data());
		CEE_CORE_WARN("Failed to link program:");
		CEE_CORE_WARN("{}", log);
		return -1;
	} else if (logLength) {
		log.reserve(logLength + 1);
		log.resize(logLength + 1);
		glGetProgramInfoLog(prgram, log.size(), nullptr, log.data());
		CEE_CORE_DEBUG("GLSL linker log:");
		CEE_CORE_DEBUG("{}", log);
	}
	return 0;
}

Shader::Shader(std::shared_ptr<Renderer> renderer)
{
	GetVersionAndApi(renderer);
}

Shader::Shader(Shader&& other)
{
	*this = std::move(other);
}

Shader::~Shader()
{
	Destroy();
}


int Shader::SetVertexSourceFile(const std::string &path)
{
	m_VertexSource = ReadFile(path);
	if (m_VertexSource.length() == 0)
		return -1;
	return 0;
}

int Shader::SetFragmentSourceFile(const std::string &path)
{
	m_FragmentSource = ReadFile(path);
	if (m_FragmentSource.length() == 0)
		return -1;
	return 0;
}

void Shader::SetVertexSourceString(const std::string &source)
{
	m_VertexSource = source;
}

void Shader::SetFragmentSourceString(const std::string &source)
{
	m_FragmentSource = source;
}

void Shader::FreeSourceStrings()
{
	m_VertexSource.clear();
	m_VertexSource.shrink_to_fit();
	m_FragmentSource.clear();
	m_FragmentSource.shrink_to_fit();
}

int Shader::Compile()
{
	int ret = 0;
	if ((ret = ValidateShaderVersion(m_VertexSource)))
		return ret;
	if ((ret = ValidateShaderVersion(m_FragmentSource)))
		return ret;

	GLuint vshader, fshader;
	const char *vshaderSource = m_VertexSource.c_str(), *fshaderSource = m_FragmentSource.c_str(); 

	if (!(vshader = glCreateShader(GL_VERTEX_SHADER))) {
		CEE_CORE_ERROR("Failed to create gl vertex shader. glGetError(): 0x{:X}", glGetError());
		return -1;
	}
	if (!(fshader = glCreateShader(GL_FRAGMENT_SHADER))) {
		CEE_CORE_ERROR("Failed to create gl fragement shader. glGetError(): 0x{:X}", glGetError());
		glDeleteShader(vshader);
		return -1;
	}

	glShaderSource(vshader, 1, &vshaderSource, nullptr);
	glCompileShader(vshader);
	if (GetShaderStatus(vshader, "vertex")) {
		glDeleteShader(vshader);
		glDeleteShader(fshader);

		return -1;
	}

	glShaderSource(fshader, 1, &fshaderSource, nullptr);
	glCompileShader(fshader);
	if (GetShaderStatus(fshader, "fragment")) {
		glDeleteShader(vshader);
		glDeleteShader(fshader);

		return -1;
	}

	if (!(m_Program = glCreateProgram())) {
		glDeleteShader(vshader);
		glDeleteShader(fshader);
		CEE_CORE_ERROR("Failed to create gl program. glGetError(): 0x{:X}", glGetError());
		return -1;
	}
	glAttachShader(m_Program, vshader);
	glAttachShader(m_Program, fshader);
	glLinkProgram(m_Program);

	glDeleteShader(vshader);
	glDeleteShader(fshader);

	if (GetProgramStatus(m_Program)) {
		return -1;
	}

	return 0;
};

void Shader::Destroy()
{
	if (m_Program) {
		glDeleteProgram(m_Program);
	}
}

void Shader::Bind()
{
	if (m_Program)
		glUseProgram(m_Program);
}

void Shader::Unbind()
{
	glUseProgram(0);
}

Shader& Shader::operator=(Shader&& other)
{
	this->m_Program = other.m_Program;
	this->m_VertexSource = std::move(other.m_VertexSource);
	this->m_FragmentSource = std::move(other.m_FragmentSource);
	this->m_UniformLoacations = std::move(other.m_UniformLoacations);

	other.m_Program = 0;
	other.m_VertexSource = {};
	other.m_FragmentSource = {};
	other.m_UniformLoacations = {};

	return *this;
}

std::string Shader::ReadFile(const std::string &path)
{
	std::string fileContents;
	std::ifstream file(path);
	if (!file.is_open()) {
		CEE_CORE_WARN("Failed to open file {}", path);
		return {};
	}

	file.seekg(0, std::ios_base::end);
	ssize_t size = file.tellg();
	file.seekg(0);

	fileContents.reserve(size + 1);
	fileContents.resize(size + 1);

	file.read(fileContents.data(), size);
	if (file.fail()) {
		CEE_CORE_WARN("Failed to read file {}", path);
		return {};
	}

	file.close();
	
	return fileContents;
}

void Shader::GetVersionAndApi(std::shared_ptr<Renderer> renderer)
{
	/* 
	 * The output expected from this is in the format:
	 * OpenGL ES major.minor[.release] [vendor info]
	 * for OpenGL ES 2.0, and for OpenGL ES >3.0 it is:
	 * major.minor[.release] [vendor info]
	 * and for OpenGL it is:
	 * major.minor[.release] [vendor info]
	 */
	auto versionString = renderer->GetVersionString();

	if (versionString.find("ES") != versionString.npos) {
		m_glAPI = glAPI::OpenGL_ES;
	} else {
		m_glAPI = glAPI::OpenGL;
	}

	auto majorBegin = versionString.find_first_of("0123456789");
	auto majorEnd = versionString.find_first_not_of("0123456789", majorBegin);
	auto minorBegin = versionString.find_first_of("0123456789", majorEnd);
	auto minorEnd = versionString.find_first_not_of("0123456789", minorBegin);
	if ((majorBegin == versionString.npos) ||
		(majorEnd   == versionString.npos) ||
		(minorBegin == versionString.npos) ||
		(minorEnd   == versionString.npos)) {
		throw std::logic_error("GL version string not formatted as expexted.");
	}

	// Use string instead of string_view here so that we can use stoi nicely
	std::string majorStr(&versionString[majorBegin], majorEnd - majorBegin);
	std::string minorStr(&versionString[minorBegin], minorEnd - minorBegin);

	int major = std::stoi(majorStr), minor = std::stoi(minorStr);

	/*
	 * I am sorry for what you are about to lay your eyes on, if your editor
	 * allows collapsing of blocks I would highly reccomend hiding what follows
	 * this because nobody deserves to see it...
	 *  - Chloe, 2025
	 *  
	 *  And maybe TODO: Whatever this is
	 */
	if (m_glAPI == glAPI::OpenGL_ES) {
		switch (major) {
			case 2: {
				switch (minor) {
					case 0: {
						m_glVersion = glVersion::OpenGL_ES_3_0;
					} break;

					default:
						throw std::logic_error(std::format("Unknown OpenGL version {}.{}", major, minor));
				}
			} break;

			case 3: {
				switch (minor) {
					case 0: {
						m_glVersion = glVersion::OpenGL_ES_3_0;
					} break;

					case 1: {
						m_glVersion = glVersion::OpenGL_ES_3_1;
					} break;

					case 2: {
						m_glVersion = glVersion::OpenGL_ES_3_2;
					} break;

					default:
						throw std::logic_error(std::format("Unknown OpenGL version {}.{}", major, minor));
				}
			} break;

			default:
				throw std::logic_error(std::format("Unknown OpenGL version {}.{}", major, minor));
		}
	} else if (m_glAPI == glAPI::OpenGL) {
		switch (major) {
			case 3: {
				switch (minor) {
					case 0: {
						m_glVersion = glVersion::OpenGL_3_0;
					} break;

					case 1: {
						m_glVersion = glVersion::OpenGL_3_1;
					} break;

					case 2: {
						m_glVersion = glVersion::OpenGL_3_2;
					} break;

					case 3: {
						m_glVersion = glVersion::OpenGL_3_3;
					} break;

					default:
						throw std::logic_error(std::format("Unknown OpenGL version {}.{}", major, minor));
				}
			} break;

			case 4: {
				switch (minor) {
					case 0: {
						m_glVersion = glVersion::OpenGL_4_0;
					} break;

					case 1: {
						m_glVersion = glVersion::OpenGL_4_1;
					} break;

					case 2: {
						m_glVersion = glVersion::OpenGL_4_2;
					} break;

					case 3: {
						m_glVersion = glVersion::OpenGL_4_3;
					} break;

					case 4: {
						m_glVersion = glVersion::OpenGL_4_4;
					} break;

					case 5: {
						m_glVersion = glVersion::OpenGL_4_5;
					} break;

					case 6: {
						m_glVersion = glVersion::OpenGL_4_6;
					} break;

					default:
						throw std::logic_error(std::format("Unknown OpenGL version {}.{}", major, minor));
				}
			} break;

			default:
				throw std::logic_error(std::format("Unknown OpenGL version {}.{}", major, minor));
		}
	}
}

int Shader::ValidateShaderVersion(const std::string& shaderSource) {
	std::string_view line;
	int lineNum = 0;
	auto poundPos = 0, newlinePos = 0;
	do { 
		poundPos = shaderSource.find_first_of('#', newlinePos);
		newlinePos = shaderSource.find_first_of('\n', poundPos);
		if ((poundPos == shaderSource.npos) || (newlinePos == shaderSource.npos)) {
			CEE_CORE_WARN("Shaders without explicit version strings are not supported");
			return -1;
		}
		line = std::string_view(&shaderSource[poundPos], newlinePos - poundPos);
		if (line.find("version") != line.npos) {
			break;
		}
		lineNum++;
	} while (1);

	auto versionNumPos = line.npos;
	if ((versionNumPos = line.find_first_of("0123456789")) == line.npos) {
		CEE_CORE_WARN("Invalid version string in shader:");
		CEE_CORE_WARN("Line {}:", lineNum);
		CEE_CORE_WARN(" >  ", line);
		return -1;
	}

	int version = std::stoi(&line[versionNumPos]);

	switch (m_glVersion) {
		case glVersion::OpenGL_ES_2_0: {
			if (version != 100) {
				CEE_CORE_WARN("Incompatible glsl version");
				CEE_CORE_WARN("Line {}:", lineNum);
				CEE_CORE_WARN(" >  {}", line);
				return -1;
			}
		} break;
		case glVersion::OpenGL_ES_3_0:
		case glVersion::OpenGL_ES_3_1:
		case glVersion::OpenGL_ES_3_2:
		case glVersion::OpenGL_3_0:
		case glVersion::OpenGL_3_1:
		case glVersion::OpenGL_3_2:
		case glVersion::OpenGL_3_3: 
		case glVersion::OpenGL_4_0:
		case glVersion::OpenGL_4_1:
		case glVersion::OpenGL_4_2:
		case glVersion::OpenGL_4_3:
		case glVersion::OpenGL_4_4:
		case glVersion::OpenGL_4_5:
		case glVersion::OpenGL_4_6: {
			if (version < 300) {
				CEE_CORE_WARN("Incompatible glsl version");
				CEE_CORE_WARN("Line {}:", lineNum);
				CEE_CORE_WARN(" >  {}", line);
				return -1;
			}
		} break;
		default:
			CEE_CORE_WARN("Shader has invalid version");
			return -1;
	}

	return 0;
}

GLuint Shader::GetUnifromLocation(const std::string &name)
{
	std::unordered_map<std::string, GLuint>::iterator loc;
	if ((loc = m_UniformLoacations.find(name)) == m_UniformLoacations.end()) {
		m_UniformLoacations[name] = glGetUniformLocation(m_Program, name.c_str());
		loc = m_UniformLoacations.find(name);
	}

	return loc->second;
}
}

