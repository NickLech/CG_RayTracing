#pragma once //hope this works on other compilers

#include <filesystem>
#include <vector>
#include <utility>
#include <optional>
#include <expected>

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
		void Bind();

		~Shader();

	private :
		// Allow only the implementation to create an empty shader
		Shader();
		// Delete copy constructor
		Shader(Shader const& _other) = delete;

	private :
		/// <summary>
		/// Ids of all compiled stages
		/// </summary>
		std::vector<uint32_t> m_stage_ids;
		/// <summary>
		/// Id of linked program
		/// </summary>
		uint32_t              m_program_id;
	};
}