#pragma once //hope this works on other compilers

#include <GLContext.hpp>

#include <filesystem>
#include <vector>
#include <utility>
#include <optional>
#include <expected>
#include <string>
#include <unordered_map>
#include <array>

namespace cg_raytracing {
	namespace fs = std::filesystem;

	class Shader {
	public :
		/// <summary>
		/// Enum of shader stages
		/// </summary>
		enum class ShaderStage {
			VERTEX,
			GEOMETRY,
			FRAGMENT,
			COMPUTE
		};

		/// <summary>
		/// Construct from r-value reference 
		/// by move constructor. After this
		/// call, _prev no longer contains 
		/// a valid reference to a shader
		/// </summary>
		/// <param name="_prev">The shader to move</param>
		Shader(Shader&& _prev) noexcept;

		/// <summary>
		/// Compile and link a shader from the specified source files,
		/// with the given stages
		/// </summary>
		/// <param name="_stages">Vector of pairs of (path, stage_id)</param>
		/// <returns>Shader handle</returns>
		static std::expected<Shader, std::string> CreateShaderFromFiles(std::vector<std::pair<fs::path, ShaderStage>> const& _stages);


		/// <summary>
		/// Compile and link a shader from the given buffers,
		/// with the given stages
		/// </summary>
		/// <param name="_stages">Vector of pairs of (shader source, stage_id)</param>
		/// <returns>Shader handle</returns>
		static std::expected<Shader, std::string> CreateShaderFromBuffers(std::vector<std::pair<std::string, ShaderStage>> const& _stages);

		/// <summary>
		/// Set label for the linked shader 
		/// (useful for debugging)
		/// </summary>
		/// <param name="_label"></param>
		void SetLabel(std::string const& _label) const;

		/// <summary>
		/// Bind the program to the current OpenGL context
		/// </summary>
		void Bind() const;

		/// <summary>
		/// Get index of uniform _name. Not that the function
		/// will return nullopt if the underlying program never
		/// uses the uniform, since the glsl compiler will
		/// remove it from the binary
		/// </summary>
		/// <param name="_name">Name of the uniform</param>
		/// <returns>The index if it exists, else error</returns>
		std::expected<uint32_t, GLError> GetUniformLocation(std::string const& _name);

		/// <summary>
		/// Same as GetUniformLocation, but this time
		/// for Uniform Buffer Objects
		/// </summary>
		/// <param name="_name">UBO name</param>
		/// <returns>Index else error</returns>
		std::expected<uint32_t, GLError> GetUboIndex(std::string const& _name);

		////////////////////////////////////////////////////////////////////
		/// UNIFORM SETTERS

		std::optional<GLError> SetUniform1f(std::string const& _name, float _value);
		std::optional<GLError> SetUniform2f(std::string const& _name, float _value1, float _value2);
		std::optional<GLError> SetUniform3f(std::string const& _name, float _value1, float _value2, float _value3);
		std::optional<GLError> SetUniform4f(std::string const& _name, float _value1, float _value2, float _value3, float _value4);

		std::optional<GLError> SetUniform1i(std::string const& _name, int32_t _value);
		std::optional<GLError> SetUniform2i(std::string const& _name, int32_t _value1, int32_t _value2);
		std::optional<GLError> SetUniform3i(std::string const& _name, int32_t _value1, int32_t _value2, int32_t _value3);
		std::optional<GLError> SetUniform4i(std::string const& _name, int32_t _value1, int32_t _value2, int32_t _value3, int32_t _value4);

		std::optional<GLError> SetUniform1ui(std::string const& _name, uint32_t _value);
		std::optional<GLError> SetUniform2ui(std::string const& _name, uint32_t _value1, uint32_t _value2);
		std::optional<GLError> SetUniform3ui(std::string const& _name, uint32_t _value1, uint32_t _value2, uint32_t _value3);
		std::optional<GLError> SetUniform4ui(std::string const& _name, uint32_t _value1, uint32_t _value2, uint32_t _value3, uint32_t _value4);

		std::optional<GLError> SetUniformMatrix2(std::string const& _name, bool _transpose, const std::array<float, 4 >&  _values);
		std::optional<GLError> SetUniformMatrix3(std::string const& _name, bool _transpose, const std::array<float, 9 >&  _values);
		std::optional<GLError> SetUniformMatrix4(std::string const& _name, bool _transpose, const std::array<float, 16>&  _values);

		////////////////////////////////////////////////////////////////////

		~Shader();

	private :
		// Allow only the implementation to create an empty shader
		Shader();
		// Delete copy constructor
		Shader(Shader const& _other) = delete;

	private :
		template <typename Key, typename Value>
		using Map = std::unordered_map<Key, Value>;

		/// Ids of all compiled stages
		std::vector<uint32_t>      m_stage_ids;
		/// Id of linked program
		uint32_t                   m_program_id;
		/// Hash-map of uniform name to location
		Map<std::string, uint32_t> m_uniform_locations;
		/// Hash-map of UBO name to index
		Map<std::string, uint32_t> m_ubos_indexes;
	};
}