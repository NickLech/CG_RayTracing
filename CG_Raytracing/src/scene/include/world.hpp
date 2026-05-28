#pragma once

#include <kd_tree.hpp>
#include <hittable.hpp>
#include <hit_record.hpp>

#include <memory>
#include <vector>
#include <optional>

namespace cg_raytracing::scene {
	enum class WorldError {
		OUT_OF_BOUNDS
	};

	/// <summary>
	/// Class that contains all the objects and 
	/// the kd-tree that divides those objects
	/// in a hierarchy
	/// </summary>
	class World : public geometry::Hittable {
	public :
		World(World&& _prev) noexcept;
		World& operator=(World&& _other) noexcept;

		/// <summary>
		/// Create world with coordinates that
		/// go from -_world_size/2 to +_world_size/2
		/// </summary>
		/// <param name="_world_size">Size of each coordinate</param>
		/// <returns>Za warudo</returns>
		static World CreateEmpty(float _world_size);

		/// <summary>
		/// Get overarching bounding box
		/// </summary>
		/// <returns>The bbox</returns>
		geometry::BoundingBox GetBoundingBox() const override;

		std::optional<geometry::HitRecord> Hit(const math::Ray& _ray,
			float _t_min = TMIN,
			float _t_max = TMAX) const override;

		/// <summary>
		/// Search for ray-object intersections WITHOUT
		/// allocations of any kind
		/// </summary>
		/// <param name="_ray"></param>
		/// <param name="_t_min"></param>
		/// <param name="_t_max"></param>
		/// <returns></returns>
		std::optional<geometry::HitRecord> HitNoAllocations(const math::Ray& _ray,
			float _t_min = TMIN,
			float _t_max = TMAX) const;

		/// <summary>
		/// Add single object to the hierarchy. Remember to call 
		/// UpdateTree before using any other method
		/// </summary>
		/// <param name="_obj">The object to add</param>
		/// <returns>Error if any</returns>
		std::optional<WorldError> AddObject(std::shared_ptr<geometry::Hittable> _obj);

		/// <summary>
		/// Call this after adding new objects and before
		/// calling other methods
		/// </summary>
		void UpdateTree();

		inline std::vector<std::shared_ptr<geometry::Hittable>> const& GetObjects() const {
			return m_objects;
		}

	private :
		World();
		World& operator=(World const& _other) = delete;

	private :
		float                                            m_world_size;
		std::vector<std::shared_ptr<geometry::Hittable>> m_objects;
		std::unique_ptr<geometry::KDTree>                m_kd_tree;
		bool                                             m_update_tree;
	};
}