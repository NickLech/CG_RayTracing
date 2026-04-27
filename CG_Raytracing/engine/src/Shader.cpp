#include <Shader.hpp>
#include <Utility.hpp>
#include <GLContext.hpp>

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

		common::Unreachable();
		return GLuint(-1);
	}

	Shader::Shader(Shader&& _prev) noexcept :
		m_stage_ids{std::move(_prev.m_stage_ids)},
		m_program_id{_prev.m_program_id},
		m_uniform_locations{std::move(_prev.m_uniform_locations)},
		m_ubos_indexes{std::move(_prev.m_ubos_indexes)} {
		_prev.m_stage_ids.clear();
		_prev.m_program_id = 0;
		_prev.m_uniform_locations.clear();
		_prev.m_ubos_indexes.clear();
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

	void cg_raytracing::Shader::Bind() const {
		auto gl_ctx = GetCurrentGLContext();
		gl_ctx->BindProgram(m_program_id);
	}

	Shader::~Shader() {
		auto gl_ctx = GetCurrentGLContext();
		// If the current bound program is managed
		// by this object, we should unbind it
		// to prevent delayed shader deletion
		if (gl_ctx->GetCurrentProgram() == m_program_id) {
			gl_ctx->BindProgram(0);
		}

		if (0 != m_program_id) {
			glDeleteProgram(m_program_id);
		}

		for (auto stage_id : m_stage_ids) {
			glDeleteShader(stage_id);
		}

		m_program_id = 0;
		m_stage_ids.clear();
	}

	std::expected<uint32_t, GLError> Shader::GetUniformLocation(std::string const& _name) {
		if (m_uniform_locations.contains(_name)) {
			return m_uniform_locations[_name];
		}

		auto location = glGetUniformLocation(m_program_id, _name.c_str());
		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return std::unexpected{ maybe_err };
		}

		if (GL_INVALID_INDEX == location) {
			return std::unexpected{ GLError::UNIFORM_NOT_FOUND };
		}

		m_uniform_locations[_name] = location;	
		return (uint32_t)location;
	}

	std::expected<uint32_t, GLError> Shader::GetUboIndex(std::string const& _name) {
		if (m_ubos_indexes.contains(_name)) {
			return m_ubos_indexes[_name];
		}
		
		auto index = glGetUniformBlockIndex(m_program_id, _name.c_str());
		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return std::unexpected{ maybe_err };
		}

		if (GL_INVALID_INDEX == index) {
			return std::unexpected{ GLError::UNIFORM_NOT_FOUND };
		}

		m_ubos_indexes[_name] = index;
		return (uint32_t)index;
	}

	Shader::Shader() :
		m_stage_ids{},
		m_program_id{},
		m_uniform_locations{},
		m_ubos_indexes{}
	{}

	std::optional<GLError> Shader::SetUniform1f(std::string const& _name, float _value) {
		auto maybe_index = GetUniformLocation(_name);
		if (!maybe_index.has_value()) {
			return maybe_index.error();
		}
		auto index = maybe_index.value();
		
		glUniform1f(index, _value);

		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}

		return std::nullopt;
	}

	std::optional<GLError> Shader::SetUniform2f(std::string const& _name, float _value1, float _value2) {
		auto maybe_index = GetUniformLocation(_name);
		if (!maybe_index.has_value()) {
			return maybe_index.error();
		}
		auto index = maybe_index.value();

		glUniform2f(index, _value1, _value2);

		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}

		return std::nullopt;
	}

	std::optional<GLError> Shader::SetUniform3f(std::string const& _name, float _value1, float _value2, float _value3) {
		auto maybe_index = GetUniformLocation(_name);
		if (!maybe_index.has_value()) {
			return maybe_index.error();
		}
		auto index = maybe_index.value();

		glUniform3f(index, _value1, _value2, _value3);

		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}

		return std::nullopt;
	}

	std::optional<GLError> Shader::SetUniform4f(std::string const& _name, float _value1, float _value2, float _value3, float _value4) {
		auto maybe_index = GetUniformLocation(_name);
		if (!maybe_index.has_value()) {
			return maybe_index.error();
		}
		auto index = maybe_index.value();

		glUniform4f(index, _value1, _value2, _value3, _value4);

		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}

		return std::nullopt;
	}

	std::optional<GLError> Shader::SetUniform1i(std::string const& _name, int32_t _value) {
		auto maybe_index = GetUniformLocation(_name);
		if (!maybe_index.has_value()) {
			return maybe_index.error();
		}
		auto index = maybe_index.value();

		glUniform1i(index, _value);

		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}

		return std::nullopt;
	}
	std::optional<GLError> Shader::SetUniform2i(std::string const& _name, int32_t _value1, int32_t _value2) {
		auto maybe_index = GetUniformLocation(_name);
		if (!maybe_index.has_value()) {
			return maybe_index.error();
		}
		auto index = maybe_index.value();

		glUniform2i(index, _value1, _value2);

		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}

		return std::nullopt;
	}
	std::optional<GLError> Shader::SetUniform3i(std::string const& _name, int32_t _value1, int32_t _value2, int32_t _value3) {
		auto maybe_index = GetUniformLocation(_name);
		if (!maybe_index.has_value()) {
			return maybe_index.error();
		}
		auto index = maybe_index.value();

		glUniform3i(index, _value1, _value2, _value3);

		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}

		return std::nullopt;
	}
	std::optional<GLError> Shader::SetUniform4i(std::string const& _name, int32_t _value1, int32_t _value2, int32_t _value3, int32_t _value4) {
		auto maybe_index = GetUniformLocation(_name);
		if (!maybe_index.has_value()) {
			return maybe_index.error();
		}
		auto index = maybe_index.value();

		glUniform4i(index, _value1, _value2, _value3, _value4);

		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}

		return std::nullopt;
	}

	std::optional<GLError> Shader::SetUniform1ui(std::string const& _name, uint32_t _value) {
		auto maybe_index = GetUniformLocation(_name);
		if (!maybe_index.has_value()) {
			return maybe_index.error();
		}
		auto index = maybe_index.value();

		glUniform1ui(index, _value);

		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}

		return std::nullopt;
	}
	std::optional<GLError> Shader::SetUniform2ui(std::string const& _name, uint32_t _value1, uint32_t _value2) {
		auto maybe_index = GetUniformLocation(_name);
		if (!maybe_index.has_value()) {
			return maybe_index.error();
		}
		auto index = maybe_index.value();

		glUniform2ui(index, _value1, _value2);

		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}

		return std::nullopt;
	}
	std::optional<GLError> Shader::SetUniform3ui(std::string const& _name, uint32_t _value1, uint32_t _value2, uint32_t _value3) {
		auto maybe_index = GetUniformLocation(_name);
		if (!maybe_index.has_value()) {
			return maybe_index.error();
		}
		auto index = maybe_index.value();

		glUniform3ui(index, _value1, _value2, _value3);

		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}

		return std::nullopt;
	}
	std::optional<GLError> Shader::SetUniform4ui(std::string const& _name, uint32_t _value1, uint32_t _value2, uint32_t _value3, uint32_t _value4) {
		auto maybe_index = GetUniformLocation(_name);
		if (!maybe_index.has_value()) {
			return maybe_index.error();
		}
		auto index = maybe_index.value();

		glUniform4ui(index, _value1, _value2, _value3, _value4);

		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}

		return std::nullopt;
	}


	std::optional<GLError> Shader::SetUniformMatrix2(std::string const& _name, bool _transpose, const std::array<float, 4 >& _values) {
		auto maybe_index = GetUniformLocation(_name);
		if (!maybe_index.has_value()) {
			return maybe_index.error();
		}
		auto index = maybe_index.value();

		glUniformMatrix2fv(index, 1, _transpose, _values.data());

		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}

		return std::nullopt;
	}
	std::optional<GLError> Shader::SetUniformMatrix3(std::string const& _name, bool _transpose, const std::array<float, 9 >& _values) {
		auto maybe_index = GetUniformLocation(_name);
		if (!maybe_index.has_value()) {
			return maybe_index.error();
		}
		auto index = maybe_index.value();

		glUniformMatrix3fv(index, 1, _transpose, _values.data());

		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}

		return std::nullopt;
	}
	std::optional<GLError> Shader::SetUniformMatrix4(std::string const& _name, bool _transpose, const std::array<float, 16>& _values) {
		auto maybe_index = GetUniformLocation(_name);
		if (!maybe_index.has_value()) {
			return maybe_index.error();
		}
		auto index = maybe_index.value();

		glUniformMatrix4fv(index, 1, _transpose, _values.data());

		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}

		return std::nullopt;
	}
}