#include <world.hpp>

namespace cg_raytracing::scene {
	World::World() :
		m_world_size{},
		m_objects{},
		m_kd_tree{},
		m_update_tree{}
	{ }

	World::World(World&& _prev) noexcept:
		m_world_size{_prev.m_world_size},
		m_objects{std::move(_prev.m_objects)},
		m_kd_tree{std::move(_prev.m_kd_tree)},
		m_update_tree{_prev.m_update_tree}
	{}

	World& World::operator=(World && _other) noexcept {
		m_world_size = _other.m_world_size;
		m_objects = std::move(_other.m_objects);
		m_kd_tree = std::move(_other.m_kd_tree);
		m_update_tree = _other.m_update_tree;

		return *this;
	}

	World World::CreateEmpty(float _world_size) {
		auto world = World();
		world.m_world_size = _world_size;
		return world;
	}

	geometry::BoundingBox World::GetBoundingBox() const {
		if (m_objects.empty() || !m_kd_tree) { 
			return {}; 
		}
		return m_kd_tree->GetTopLevelBoundingBox();
	}

	std::optional<geometry::HitRecord> World::Hit(const math::Ray& _ray, float _t_min, float _t_max) const {
		if (0 == m_objects.size()) { 
			return std::nullopt; 
		}
		if (!m_kd_tree) {
			return std::nullopt;
		}
		
		auto intersected_nodes = m_kd_tree->RayIntersectsObjects(_ray);
		if (0 == intersected_nodes.size()) { 
			return std::nullopt; 
		}

		// List of intersected nodes is already ordered by distance from the origin
		// Pick the first object that is truly hit by the ray

		for (auto [node_ref, point] : intersected_nodes) {
			auto hit_record = m_objects[node_ref->obj_index.value()]->Hit(_ray, _t_min, _t_max);
			if (hit_record.has_value()) {
				return hit_record;
			}
		}

		return std::nullopt;
	}

	std::optional<geometry::HitRecord> World::HitNoAllocations(const math::Ray& _ray, float _t_min, float _t_max) const {
		if (0 == m_objects.size()) {
			return std::nullopt;
		}
		if (!m_kd_tree) {
			return std::nullopt;
		}
		auto hit = m_kd_tree->RayIntersectsSingleObjectIterative(_ray, m_objects, _t_min, _t_max);
		if (!hit.has_value()) {
			return std::nullopt;
		}
		return hit->first;
	}

	std::optional<WorldError> World::AddObject(std::shared_ptr<geometry::Hittable> _obj) {
		auto bbox = _obj->GetBoundingBox();
		auto half_size = m_world_size / 2.f;
		auto is_out_of_bounds = (
			std::abs(bbox.min_x) >= half_size ||
			std::abs(bbox.max_x) >= half_size ||

			std::abs(bbox.min_y) >= half_size ||
			std::abs(bbox.max_y) >= half_size ||

			std::abs(bbox.min_z) >= half_size ||
			std::abs(bbox.max_z) >= half_size
		);
		if (is_out_of_bounds) {
			return WorldError::OUT_OF_BOUNDS;
		}
		m_objects.push_back(_obj);
		m_update_tree = true;
		return std::nullopt;
	}

	void World::UpdateTree() {
		if (!m_update_tree) {
			return;
		}
		m_update_tree = false;
		if (0 == m_objects.size()) {
			return;
		}

		m_kd_tree = geometry::KDTree::CreateUniqueFromHittables(m_objects, m_world_size);
	}
}