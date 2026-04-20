#include <Shader.hpp>
#include <Utility.hpp>

#include <format>
#include <fstream>

#include <GL/glew.h>

namespace cg_raytracing {
	using ShaderStage = Shader::ShaderStage;

	static GLuint GetOpengGLShaderStageFromEnum(ShaderStage _stage) {
		switch (_stage)
		{
		case ShaderStage::VERTEX:
			return GL_VERTEX_SHADER;
		case ShaderStage::GEOMETRY:
			return GL_GEOMETRY_SHADER;
		case ShaderStage::FRAGMENT:
			return GL_FRAGMENT_SHADER;
		case ShaderStage::COMPUTE:
			return GL_COMPUTE_SHADER;
		default:
			common::Unreachable();
			break;
		}
	}

	Shader::Shader(Shader&& _prev) noexcept :
		m_stage_ids{std::move(_prev.m_stage_ids)},
		m_program_id{_prev.m_program_id} {
		_prev.m_stage_ids.clear();
		_prev.m_program_id = 0;
	}

	static std::optional<std::string> CheckShaderCompilationStatus(uint32_t _shader) {
		int32_t status{};
		glGetShaderiv(_shader, GL_COMPILE_STATUS, &status);

		if (GL_TRUE == status) {
			return std::nullopt;
		}

		int32_t info_len{}, true_len{};
		glGetShaderiv(_shader, GL_INFO_LOG_LENGTH, &info_len);
		std::string info_log{};
		info_log.resize(info_len, '\0');
		glGetShaderInfoLog(_shader, info_len, &true_len, info_log.data());

		return info_log;
	}

	static std::optional<std::string> CheckProgramLinkStatus(uint32_t _program) {
		int32_t status{};
		glGetProgramiv(_program, GL_LINK_STATUS, &status);

		if (GL_TRUE == status) {
			return std::nullopt;
		}

		int32_t info_len{}, true_len{};
		glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &info_len);
		std::string info_log{};
		info_log.resize(info_len, '\0');
		glGetProgramInfoLog(_program, info_len, &true_len, info_log.data());

		return info_log;
	}

	std::expected<Shader, std::string> Shader::CreateShaderFromFiles(std::vector<std::pair<fs::path, ShaderStage>> const& _stages) {
		std::vector<std::pair<std::string, ShaderStage>> shader_sources{};
		for (auto const& [path, stage] : _stages) {
			if (!std::filesystem::exists(path)) {
				return std::unexpected{ std::format("Could not find file {}", path.string()) };
			}

			auto fsize = std::filesystem::file_size(path);

			std::ifstream shader_fd{ path };
			if (!shader_fd.is_open()) {
				return std::unexpected{ std::format("Could not open file {}", path.string()) };
			}

			std::string buf{};
			buf.resize(fsize, '\0');
			shader_fd.read(buf.data(), fsize);

			shader_sources.push_back({ buf, stage });
		}

		return CreateShaderFromBuffers(shader_sources);
	}

	std::expected<Shader, std::string> Shader::CreateShaderFromBuffers(std::vector<std::pair<std::string, ShaderStage>> const& _stages) {
		auto shader = Shader();

		for (auto const& [buf, stage] : _stages) {
			auto shader_id = glCreateShader(GetOpengGLShaderStageFromEnum(stage));
			if (0 == shader_id) {
				return std::unexpected{"Shader creation failed"};
			}
			auto buf_raw_ptr = buf.c_str();
			auto buf_len = (GLint)buf.size();
			glShaderSource(shader_id, 1, &buf_raw_ptr, &buf_len);
			glCompileShader(shader_id);

			auto maybe_err = CheckShaderCompilationStatus(shader_id);
			if (maybe_err.has_value()) {
				return std::unexpected{ maybe_err.value() };
			}

			shader.m_stage_ids.push_back(shader_id);
		}

		auto program_id = glCreateProgram();
		if (0 == program_id) {
			return std::unexpected{ "Program creation failed" };
		}

		for (auto shader_id : shader.m_stage_ids) {
			glAttachShader(program_id, shader_id);
		}

		glLinkProgram(program_id);
		
		auto maybe_err = CheckProgramLinkStatus(program_id);
		if (maybe_err.has_value()) {
			return std::unexpected{ maybe_err.value() };
		}

		shader.m_program_id = program_id;
		return shader;
	}

	void Shader::SetLabel(std::string const& _label) const {
		glObjectLabel(GL_PROGRAM, m_program_id, _label.size(), _label.c_str());
	}

	void Shader::Bind() {
		glUseProgram(m_program_id);
	}

	Shader::~Shader() {
		//Unbind all programs
		glUseProgram(0);
		if (0 != m_program_id) {
			glDeleteProgram(m_program_id);
		}

		for (auto stage_id : m_stage_ids) {
			glDeleteShader(stage_id);
		}

		m_program_id = 0;
		m_stage_ids.clear();
	}

	Shader::Shader() :
		m_stage_ids{},
		m_program_id{}
	{}
}