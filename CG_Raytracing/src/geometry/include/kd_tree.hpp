#pragma once

#include <hittable.hpp>
#include <ray.hpp>
#include <vec3.hpp>

#include <vector>
#include <memory>
#include <optional>
#include <functional>
#include <limits>

namespace cg_raytracing::geometry {
	struct KDNode {
		KDNode();

		BoundingBox bbox;
		std::optional<size_t> obj_index;
		std::shared_ptr<KDNode> left;
		std::shared_ptr<KDNode> right;
		size_t level;
		size_t num_subnodes;
	};

	struct FlatKDNode {
		BoundingBox           bbox;
		std::optional<size_t> obj_index;
		std::optional<size_t> left, right;
	};

	enum class KDTreeMessage {
		INVALID_OBJECT,
		UNBALANCED
	};

	class KDTree {
	public :
		static constexpr size_t UNBALANCED_THRESHOLD_MIN = 7;
		static constexpr double UNBALANCED_THRESHOLD_MULTIPLIER = 1.5;

		KDTree(KDTree&& _prev) noexcept;
		KDTree& operator=(KDTree&& _other) noexcept;

		static KDTree CreateFromHittables(std::vector<std::shared_ptr<Hittable>> const& _hittables, float _world_size);

		static std::unique_ptr<KDTree> CreateUniqueFromHittables(std::vector<std::shared_ptr<Hittable>> const& _hittables, 
			float _world_size);

		/// <summary>
		/// NOT RECOMMENDED: Add a new hittable to
		/// the tree in a non-balanced manner,
		/// rebuild the tree if adding many
		/// hittables
		/// </summary>
		/// <param name="_hittable">Pointer to object</param>
		/// <param name="_hittable">Index of object in the object list</param>
		/// <returns>If the tree is too deep wrt the number of objects</returns>
		std::optional<KDTreeMessage> AddHittable(std::shared_ptr<Hittable> _hittable, size_t _index);

		/// <summary>
		/// Visit all nodes depth-search-first 
		/// </summary>
		/// <param name="_fun">Callback, a reference to the current node is passed</param>
		/// <param name="_leafs_only"></param>
		void VisitDSF(std::function<void(FlatKDNode const&)>&& _fun, bool _leafs_only = false) const;

		/// <summary>
		/// Visit all nodes breadth-search-first 
		/// </summary>
		/// <param name="_fun">Callback, a reference to the current node is passed</param>
		void VisitBSF(std::function<void(FlatKDNode const&)>&& _fun) const;

		/// <summary>
		/// Return list of leaf nodes that the ray intersects, 
		/// together with the point where the ray and the point
		/// where they intersect, order by the distance from 
		/// the origin of the ray
		/// </summary>
		/// <param name="_ray">The ray</param>
		/// <returns>List of nodes + hit points</returns>
		std::vector<std::pair<FlatKDNode const*, math::Vec3>> RayIntersectsObjects(math::Ray const& _ray) const;

		/// <summary>
		/// Return N or all NN nodes centered at _p 
		/// inside the radius _radius and their distance
		/// from _p, ordered by distance from _p
		/// </summary>
		/// <param name="_p"></param>
		/// <param name="_radius"></param>
		/// <param name="_count"></param>
		/// <returns></returns>
		std::vector<std::pair<FlatKDNode const*, math::Vec3>> NearestNeighbours(
			math::Vec3 const& _p, 
			float _radius,
			size_t _count = std::numeric_limits<size_t>::max()) const;

		/// <summary>
		/// Check if a ray intersect one object, e.g. a COMPLETE
		/// intersection, not only bbox
		/// </summary>
		/// <param name="_ray"></param>
		/// <returns></returns>
		std::optional<std::pair<HitRecord, size_t>> RayIntersectsSingleObject(
			math::Ray const& _ray,
			std::vector<std::shared_ptr<Hittable>> const& _hittables, 
			float _tmin,
			float _tmax
		) const;

		/// <summary>
		/// Get total number of tracked objects in the BVH
		/// </summary>
		/// <returns>Number of objects</returns>
		size_t GetObjectCount() const;

		BoundingBox GetTopLevelBoundingBox() const;

		/// <summary>
		/// USE ONLY IF NECESSARY
		/// </summary>
		KDTree();

	private :
		KDTree(KDTree const& _prev) = delete;
		KDTree& operator=(KDTree const& _other) = delete;

		enum class SearchType {
			RAY,
			SPHERE,
			BBOX
		};

		// I'd like to use a union here,
		// but the underlying types
		// are not POD
		struct SearchParams {
			math::Ray ray;
			math::Vec3 center;
			float radius;
			BoundingBox bbox;
		};

		/// <summary>
		/// Perform search of bboxes intersecting one
		/// of the selected object types. 
		/// The type of intersection is based on
		/// _SearchType
		/// </summary>
		/// <typeparam name="_SearchType">Type of search</typeparam>
		/// <param name="_params">Search parameters (such as ray or bbox)</param>
		/// <param name="_count">Max number of objects to return</param>
		/// <returns>List of hit objecs and where they intersect</returns>
		template <SearchType _SearchType>
		std::vector<std::pair<FlatKDNode const*, math::Vec3>> SearchIntersections(
			SearchParams const& _params,
			size_t _count = std::numeric_limits<size_t>::max()
		) const;

		std::optional<std::pair<HitRecord, size_t>> RayIntersectsSingleObject(
			math::Ray const& _ray,
			std::vector<std::shared_ptr<Hittable>> const& _hittables,
			FlatKDNode const& _curr_node,
			float _tmin,
			float _tmax
		) const;

	private :
		std::vector<FlatKDNode> m_flat_tree;
		float                   m_world_size;
		size_t                  m_num_tracked_objects;
	};
}