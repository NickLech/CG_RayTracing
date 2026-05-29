#include <kd_tree.hpp>

#include <algorithm>
#include <cmath>
#include <functional>
#include <stack>
#include <stdexcept>
#include <unordered_set>
#include <deque>
#include <tuple>

namespace cg_raytracing::geometry {
	namespace detail {
		/// <summary>
		/// Compute 16 bit morton code (interleaved positional
		/// data, normalized based on the size of the
		/// overall bounding box)
		/// </summary>
		/// <param name="_x"></param>
		/// <param name="_y"></param>
		/// <param name="_z"></param>
		/// <param name="_world_size_x"></param>
		/// <param name="_world_size_y"></param>
		/// <param name="_world_size_z"></param>
		/// <returns></returns>
		static uint64_t ComputeMortonCode(float _x, float _y, float _z, float _world_size_x, float _world_size_y, 
			float _world_size_z) {
			auto normalize = [](float _coord, float _world_size) {
				return (_coord + _world_size / 2) / _world_size;
			};

			auto bit_expand = [](uint64_t _val) {
				// - First separate the bytes FFFF -> 00FF 00FF
				// - Separate nibbles 0FF0FF -> 0F0F0F0F
				// - Create pairs of two bits
				// - Isolate all bits
				// - Add another empty bit
				_val = (_val | (_val << 8)) & 0x00FF'00FFULL;
				_val = (_val | (_val << 4)) & 0x0F0F'0F0FULL;
				_val = (_val | (_val << 2)) & 0x3333'3333ULL;
				_val = (_val | (_val << 1)) & 0x5555'5555ULL;
				return _val;
			};

			auto bit_expand64 = [](uint64_t _val) {
				_val = (_val | (_val << 16)) & 0x0000'FFFF'0000'FFFFULL;
				_val = (_val | (_val << 8))  & 0x00FF'00FF'00FF'00FFULL;
				_val = (_val | (_val << 4))  & 0x0F0F'0F0F'0F0F'0F0FULL;
				_val = (_val | (_val << 2))  & 0x3333'3333'3333'3333ULL;
				_val = (_val | (_val << 1))  & 0x5555'5555'5555'5555ULL;
				return _val;
			};

			_x = normalize(_x, _world_size_x);
			_y = normalize(_y, _world_size_y);
			_z = normalize(_z, _world_size_z);

			_x = std::clamp(_x, .0f, 1.f);
			_y = std::clamp(_y, .0f, 1.f);
			_z = std::clamp(_z, .0f, 1.f);

			constexpr auto MAX_VALUE = (float)((1 << 16) - 1);

			_x = std::min(std::floor(_x * MAX_VALUE), MAX_VALUE);
			_y = std::min(std::floor(_y * MAX_VALUE), MAX_VALUE);
			_z = std::min(std::floor(_z * MAX_VALUE), MAX_VALUE);

			auto _morton_x = bit_expand64(bit_expand((uint64_t)_x));
			auto _morton_y = bit_expand64(bit_expand((uint64_t)_y));
			auto _morton_z = bit_expand64(bit_expand((uint64_t)_z));

			return _morton_x | (_morton_y << 1) | (_morton_z << 2);
		}

		static uint64_t ComputeMortonCode(float _x, float _y, float _z, float _world_size) {
			return ComputeMortonCode(_x, _y, _z, _world_size, _world_size, _world_size);
		}
	}

	KDTree::KDTree(KDTree&& _prev) noexcept :
		m_flat_tree{std::move(_prev.m_flat_tree)},
		m_world_size{_prev.m_world_size},
		m_num_tracked_objects{_prev.m_num_tracked_objects}
	{}

	KDTree& KDTree::operator=(KDTree && _other) noexcept {
		m_flat_tree = std::move(_other.m_flat_tree);
		m_world_size = _other.m_world_size;
		m_num_tracked_objects = _other.m_num_tracked_objects;
		return *this;
	}

	using SplitFunction = std::function<size_t(std::vector<std::pair<size_t, uint64_t>> const&, size_t, size_t)>;
	static std::pair<std::shared_ptr<KDNode>, size_t> CreateSubTree(
		std::vector<std::pair<size_t, uint64_t>> const& _list, 
		std::vector<std::shared_ptr<Hittable>> const& _hittables,
		size_t _begin, size_t _end, SplitFunction const& _split_function,
		size_t _level = 0) {
		if (_begin == _end) {
			auto leaf = std::make_shared<KDNode>();
			leaf->bbox = _hittables[_list[_begin].first]->GetBoundingBox();
			leaf->obj_index = _list[_begin].first;
			leaf->level = _level;
			leaf->num_subnodes = 0;
			return { leaf, 1 };
		}
		else {
			auto node = std::make_shared<KDNode>();
			auto split_pos = _split_function(_list, _begin, _end);
			auto [l, count_l] = CreateSubTree(_list, _hittables, _begin, split_pos, _split_function, _level + 1);
			auto [r, count_r] = CreateSubTree(_list, _hittables, split_pos + 1, _end, _split_function, _level + 1);

			node->num_subnodes = 2 + l->num_subnodes + r->num_subnodes;
			node->left  = l;
			node->right = r;
			node->bbox = node->left->bbox.Union(node->right->bbox);
			node->level = _level;

			return { node, 1 + count_l + count_r };
		}
	}

	/// <summary>
	/// Create flat kd-tree without constructing a 
	/// temporary pointer-based tree
	/// </summary>
	/// <param name="_list">List of (object index, morton code)</param>
	/// <param name="_hittables">List of objects to be tracked</param>
	/// <param name="_split_function">Function to split between left/right at each non-leaf node</param>
	/// <returns>The flat tree</returns>
	static std::vector<FlatKDNode> CreateTreeIterative(
		std::vector<std::pair<size_t, uint64_t>> const& _list,
		std::vector<std::shared_ptr<Hittable>> const& _hittables,
		SplitFunction const& _split_function) {
		FlatKDNode root{};
		std::vector<FlatKDNode> flat_tree{};

		auto num_codes = _list.size();
		auto num_levels = std::ceilf(std::log2f((float)num_codes)) + 1;
		// Preallocate 2^N nodes
		// If the number of objects is not a power of two,
		// the preallocation is bigger than needed
		auto alloc_size = (size_t)std::powf(2.f, num_levels);

		flat_tree.reserve(alloc_size);
		flat_tree.push_back(root);

		if (1 == num_codes) {
			auto& node = flat_tree.back();
			node.bbox = _hittables[_list[0].first]->GetBoundingBox();
			node.obj_index = _list[0].first;
			return flat_tree;
		}

		std::stack<std::pair<size_t, size_t>> ranges_to_visit_l{};
		std::stack<std::pair<size_t, size_t>> ranges_to_visit_r{};
		std::stack<size_t> parents{};
		std::stack<size_t> nodes_without_bbox{};

		auto split_point = _split_function(_list, 0, _list.size() - 1);

		ranges_to_visit_l.push({ 0, split_point });
		ranges_to_visit_r.push({ split_point + 1, _list.size() - 1 });

		parents.push(0);

		while (!ranges_to_visit_l.empty() || !ranges_to_visit_r.empty()) {
			std::pair<size_t, size_t> curr_range{};
			// Are we at the right of a node?
			bool is_r = false;
			// Pop a range to visit either from the left
			// or the right
			if (!ranges_to_visit_l.empty()) {
				curr_range = ranges_to_visit_l.top();
				ranges_to_visit_l.pop();
			}
			else {
				curr_range = ranges_to_visit_r.top();
				ranges_to_visit_r.pop();
				is_r = true;
			}

			if (curr_range.first > curr_range.second) {
				continue;
			}

			FlatKDNode new_node{};
			if (curr_range.first == curr_range.second) {
				// We reached a leaf, assign bbox and index
				new_node.bbox = _hittables[_list[curr_range.first].first]->GetBoundingBox();
				new_node.obj_index = _list[curr_range.first].first;
			}

			auto curr_pos = flat_tree.size();
			flat_tree.push_back(new_node);

			// Get index of parent node
			auto parent_index = parents.top();
			if (is_r) {
				// Right node
				{
					auto& parent = flat_tree[parent_index];
					// Assign index of this node
					parent.right = curr_pos;
					if (curr_range.first == curr_range.second) {
						// If this node is a leaf, we can complete 
						// the parent node with the merged bbox
						parent.bbox = flat_tree[parent.left.value()].bbox.Union(flat_tree[parent.right.value()].bbox);
					}
					else {
						// Else we still need to traverse other nodes before 
						// completing the node
						nodes_without_bbox.push(parent_index);
					}
				}

				parents.pop();
			}
			else {
				// Left node
				flat_tree[parent_index].left = curr_pos;
			}

			if (curr_range.first != curr_range.second) {
				// This range is yet not complete
				// Split it again
				auto split_point = _split_function(_list, curr_range.first, curr_range.second);

				ranges_to_visit_l.push({ curr_range.first, split_point });
				ranges_to_visit_r.push({ split_point + 1, curr_range.second });

				parents.push(curr_pos);
			}
		}

		// Complete the flat tree by grouping the various boxes
		// from the bottom to top (considering only the nodes
		// that are still without the bbox)
		while (!nodes_without_bbox.empty()) {
			auto curr_node = nodes_without_bbox.top();

			if (0 != flat_tree[curr_node].bbox.Volume()) {
				// Already complete, probably this
				// node has been reinsterted in the stack
				nodes_without_bbox.pop();
				continue;
			}

			// Get the bboxes of the left and right nodes

			auto bbox_left = flat_tree[flat_tree[curr_node].left.value()].bbox;
			auto bbox_right = flat_tree[flat_tree[curr_node].right.value()].bbox;

			// Is any of the boxes missing?

			auto missing_left = 0 == bbox_left.Volume();
			auto missing_right = 0 == bbox_right.Volume();

			// If a box is missing:
			// Reinsert possibly duplicate indices before the current index

			if (missing_right) {
				nodes_without_bbox.push(flat_tree[curr_node].right.value());
			}

			if (missing_left) {
				nodes_without_bbox.push(flat_tree[curr_node].left.value());
			}

			// None of the nodes has missing bbox: merge the two

			if (!missing_left && !missing_right) {
				nodes_without_bbox.pop();
				flat_tree[curr_node].bbox = bbox_left.Union(bbox_right);
			}	
		}

		return flat_tree;
	}

	static std::vector<FlatKDNode> CreateFlattenedTree(std::shared_ptr<KDNode> _tree, size_t _node_count) {
		std::vector<FlatKDNode> flat_tree{};
		flat_tree.reserve(_node_count);

		std::stack<std::shared_ptr<KDNode>> nodes_to_visit{};
		nodes_to_visit.push(_tree);

		while (!nodes_to_visit.empty()) {
			auto curr_node = nodes_to_visit.top();
			nodes_to_visit.pop();

			auto i = flat_tree.size();
			auto level = curr_node->level + 1;

			FlatKDNode new_node{};
			new_node.bbox = curr_node->bbox;
			new_node.obj_index = curr_node->obj_index;
			new_node.left = std::nullopt;
			new_node.right = std::nullopt;
			
			if (!curr_node->obj_index.has_value()) {
				//non-leaf
				if (curr_node->right) { nodes_to_visit.push(curr_node->right); }
				if (curr_node->left) { nodes_to_visit.push(curr_node->left);   }

				new_node.left = i + 1;
				auto right_offset = curr_node->left ? curr_node->left->num_subnodes : 0;
				// Sub node count does not include (obviously) the current node and the left child,
				// so we add 2
				new_node.right = i + right_offset + 2;
			}
			
			flat_tree.push_back(new_node);
		}

		return flat_tree;
	}

	/// <summary>
	/// Create kd-tree given the objects to track and the max coord size for all
	/// dimensions
	/// </summary>
	/// <param name="_hittables">List of objects</param>
	/// <param name="_world_size">Max coord size</param>
	/// <returns>The tree</returns>
	KDTree KDTree::CreateFromHittables(std::vector<std::shared_ptr<Hittable>> const& _hittables, float _world_size) {
		std::vector<std::pair<size_t, uint64_t>> code_list{};
		code_list.reserve(_hittables.size());

		for (size_t index = 0; auto const& object : _hittables) {
			auto box = object->GetBoundingBox();
			auto center = box.pos;
			auto morton_code = detail::ComputeMortonCode(center.x, center.y, center.z, _world_size);
			code_list.push_back({ index, morton_code });
			index++;
		}

		// Sort by spatial locality
		std::sort(code_list.begin(), code_list.end(), [](auto _l, auto _r) { return _r.second > _l.second; });

		// Simple middle split between lef/right
		// If any other method is used, it is expected that
		// at each node, we have subtrees on both the left
		// and right
		auto split_middle = [](std::vector<std::pair<size_t, uint64_t>> const&, size_t _begin, size_t _end) {
			return (_begin + _end) / 2;
		};

		//auto [tree, node_count] = CreateSubTree(code_list, _hittables, 0, code_list.size() - 1, split_middle);
		//auto flattened = CreateFlattenedTree(tree, node_count);

		auto flattened = CreateTreeIterative(code_list, _hittables, split_middle);

		auto kd_tree = KDTree();
		kd_tree.m_flat_tree = std::move(flattened);
		kd_tree.m_world_size = _world_size;
		kd_tree.m_num_tracked_objects = _hittables.size();

		return kd_tree;
	}

	std::unique_ptr<KDTree> KDTree::CreateUniqueFromHittables(std::vector<std::shared_ptr<Hittable>> const& _hittables, float _world_size) {
		auto ptr = std::make_unique<KDTree>();
		*ptr = CreateFromHittables(_hittables, _world_size);
		return ptr;
	}

	void KDTree::VisitDSF(std::function<void(FlatKDNode const&)>&& _fun, bool _leafs_only) const {
		std::stack<size_t> nodes_to_visit{};
		std::unordered_set<size_t> visited_nodes{};

		// Start with the root
		nodes_to_visit.push(0);

		while (!nodes_to_visit.empty()) {
			// Get index in flat tree
			auto curr_node_index = nodes_to_visit.top();
			if (visited_nodes.contains(curr_node_index)) {
				// Somehow the node was already visited
				nodes_to_visit.pop();
				continue;
			}

			auto const& node = m_flat_tree[curr_node_index];
			if (node.obj_index.has_value()) { // Leaf 
				// Just call the function, no more
				// links to follow in this subtree
				_fun(node);
				visited_nodes.insert(curr_node_index);
				nodes_to_visit.pop();
				continue;
			}

			auto l = node.left;
			auto r = node.right;

			// Check if either subtree is unexplored
			// (and that it is explorable)

			auto is_l_not_visited = l.has_value() && !visited_nodes.contains(l.value());
			auto is_r_not_visited = r.has_value() && !visited_nodes.contains(r.value());

			// First append right node, then left, so the next
			// iteration will pick the left one first

			if (is_r_not_visited) {
				nodes_to_visit.push(r.value());
			}

			if (is_l_not_visited) {
				nodes_to_visit.push(l.value());
			}

			if (!is_l_not_visited && !is_r_not_visited) {
				// All subtrees have been visited
				// Call the function on this node
				if (!_leafs_only) {
					_fun(node);
				}
				
				visited_nodes.insert(curr_node_index);
				nodes_to_visit.pop();
			}
		}
	}

	void KDTree::VisitBSF(std::function<void(FlatKDNode const&)>&& _fun) const {
		// Use fifo
		std::deque<size_t> nodes_to_visit{};
		nodes_to_visit.push_front(0);

		while (!nodes_to_visit.empty()) {
			auto curr_node_index = nodes_to_visit.back();

			auto const& node = m_flat_tree[curr_node_index];
			// Breadth-first, visit node immediately
			_fun(node);
			nodes_to_visit.pop_back();

			auto l = node.left;
			auto r = node.right;

			// Check if either subtree is unexplored
			// (and that it is explorable)

			auto is_l_not_visited = l.has_value();
			auto is_r_not_visited = r.has_value();

			if (is_l_not_visited) {
				nodes_to_visit.push_front(node.left.value());
			}

			if (is_r_not_visited) {
				nodes_to_visit.push_front(node.right.value());
			}
		}
	}

	template<KDTree::SearchType _SearchType>
	std::vector<std::pair<FlatKDNode const*, math::Vec3>> KDTree::SearchIntersections(
		KDTree::SearchParams const& _params,
		size_t _count) const {
		// Having O the origin of ray/bbox/sphere and P the point of intersection with a bbox
		// And D = |P - O|
		// Contains a list of (D, OBJ_INDEX, P)
		std::vector<std::tuple<float, size_t, math::Vec3>> unordered_nodes{};

		std::stack<size_t> nodes_to_visit{};
		nodes_to_visit.push(0); // Start from the root

		constexpr auto MAX_COUNT_VALUE = std::numeric_limits<size_t>::max();

		// Reference point
		math::Vec3 _p{};

		// Extract reference point from the correct structure
		if constexpr (_SearchType == SearchType::RAY) {
			_p = _params.ray.m_origin;
		}
		else if constexpr (_SearchType == SearchType::SPHERE) {
			_p = _params.center;
		}
		else if constexpr (_SearchType == SearchType::BBOX) {
			_p = _params.bbox.pos;
		}

		while (!nodes_to_visit.empty()) {
			auto curr_node_index = nodes_to_visit.top();
			nodes_to_visit.pop();

			auto const& node = m_flat_tree[curr_node_index];
			std::optional<math::Vec3> intersect_point{};

			// Check intersection based on type
			if constexpr (_SearchType == SearchType::RAY) {
				intersect_point = node.bbox.RayIntersect(_params.ray);
			}
			else if constexpr (_SearchType == SearchType::SPHERE) {
				intersect_point = node.bbox.SphereIntersect(_params.center, _params.radius);
			} 
			else if constexpr (_SearchType == SearchType::BBOX) {
				intersect_point = node.bbox.BBIntersect(_params.bbox);
			}

			if (!intersect_point.has_value()) {
				continue;
			}

			if (node.obj_index.has_value()) {
				// Intersected node is leaf, 
				// get hit point
				auto hit_coord = intersect_point.value();
				// Compute distance from origin of 
				// ray/sphere/bbox
				auto distance = (hit_coord - _p).length();
				// Append the distance, node index and point of intersection
				unordered_nodes.push_back({ (float)distance, curr_node_index, hit_coord });

				if (MAX_COUNT_VALUE != _count) { // If the KNN is bound
					--_count; // Check if we have reached the limit
					if (0 == _count) {
						break;
					}
				}
				continue;
			}

			// Append left and right nodes to the nodes to visit, if present

			if (node.right.has_value()) {
				nodes_to_visit.push(node.right.value());
			}

			if (node.left.has_value()) {
				nodes_to_visit.push(node.left.value());
			}
		}

		if (unordered_nodes.size() == 0) {
			return {};
		}

		// The list contains the distance and a reference to the node
		// Ordering by increasing distance will also order the references
		std::sort(unordered_nodes.begin(), unordered_nodes.end(), [](auto const& _l, auto const& _r) {
			return std::get<0>(_r) > std::get<0>(_l);
		});

		std::vector<std::pair<FlatKDNode const*, math::Vec3>> intersected_nodes{};
		intersected_nodes.resize(unordered_nodes.size());

		// Extract ordered references and intersection points
		for (size_t o_index = 0; auto const& hit_info : unordered_nodes) {
			intersected_nodes[o_index] = { &m_flat_tree[std::get<1>(hit_info)], std::get<2>(hit_info) };
			o_index++;
		}

		return intersected_nodes;
	}

	std::vector<std::pair<FlatKDNode const*, math::Vec3>> KDTree::RayIntersectsObjects(math::Ray const& _ray) const {
		SearchParams params{};
		params.ray = _ray;

		return SearchIntersections<SearchType::RAY>(params);
	}

	std::vector<std::pair<FlatKDNode const*, math::Vec3>> KDTree::NearestNeighbours(
		math::Vec3 const& _p, 
		float _radius, 
		size_t _count) const {
		SearchParams params{};
		params.center = _p;
		params.radius = _radius;

		return SearchIntersections<SearchType::SPHERE>(params, _count);
	}

	size_t KDTree::GetObjectCount() const {
		size_t object_count{};
		VisitDSF([&object_count](auto const& _node) { object_count++; }, true);
		return object_count;
	}

	BoundingBox KDTree::GetTopLevelBoundingBox() const {
		return m_flat_tree[0].bbox;
	}

	std::optional<KDTreeMessage> KDTree::AddHittable(std::shared_ptr<Hittable> _hittable, size_t _index) {
		if (!_hittable) {
			return KDTreeMessage::INVALID_OBJECT;
		}
		auto bbox = _hittable->GetBoundingBox();
		// Search key
		auto morton_code = detail::ComputeMortonCode(bbox.pos.x, bbox.pos.y, bbox.pos.z, m_world_size);

		size_t prev_parent{std::numeric_limits<size_t>::max()};
		size_t curr_parent{0};
		std::stack<size_t> bboxes_to_update{};
		bboxes_to_update.push(curr_parent);

		bool go_right = false;
		// Follow search key until leaf node
		while (true) {
			auto const& node = m_flat_tree[curr_parent];
			// Compute morton code of this node
			auto curr_morton_code = detail::ComputeMortonCode(node.bbox.pos.x, node.bbox.pos.y, node.bbox.pos.z, m_world_size);

			go_right = morton_code > curr_morton_code;
			
			if (node.obj_index.has_value()) {
				// We have reached a leaf
				break;
			}

			prev_parent = curr_parent;
			if (go_right) {
				curr_parent = node.right.value();
			}
			else {
				curr_parent = node.left.value();
			}

			bboxes_to_update.push(curr_parent);
		}

		FlatKDNode new_node{};
		new_node.obj_index = _index;
		new_node.bbox = bbox;
		new_node.left = std::nullopt;
		new_node.right = std::nullopt;

		// Append new node at the end
		auto new_node_index = m_flat_tree.size();
		m_flat_tree.push_back(new_node);

		{
			// Both left and right exist
			// Since the parent node is a leaf, 
			// move the object index to the left
			// or the right of the new node
			auto old_node = m_flat_tree[curr_parent];
			auto new_old_node_index = m_flat_tree.size();
			// Move leaf node to another position
			m_flat_tree.push_back(old_node);

			auto& parent_ref = m_flat_tree[curr_parent];
			// Node is no longer leaf
			parent_ref.obj_index = std::nullopt;

			// Create references to new nodes
			if (go_right) {
				std::swap(m_flat_tree[new_node_index], m_flat_tree[new_old_node_index]);
				std::swap(new_node_index, new_old_node_index);
				parent_ref.left = new_old_node_index;
				parent_ref.right = new_node_index;
			}
			else {
				parent_ref.right = new_old_node_index;
				parent_ref.left = new_node_index;
			}
		}

		auto curr_depth = bboxes_to_update.size();

		while (!bboxes_to_update.empty()) {
			auto curr_node_index = bboxes_to_update.top();
			bboxes_to_update.pop();

			auto& node = m_flat_tree[curr_node_index];
			node.bbox = m_flat_tree[node.left.value()].bbox.Union(m_flat_tree[node.right.value()].bbox);
		}

		m_num_tracked_objects++;

		if (curr_depth < UNBALANCED_THRESHOLD_MIN) {
			return std::nullopt;
		}

		auto expected_levels = std::ceilf(std::log2f((float)m_num_tracked_objects)) + 1;
		auto max_depth = expected_levels * UNBALANCED_THRESHOLD_MULTIPLIER;

		return (curr_depth >= max_depth) ? std::optional{ KDTreeMessage::UNBALANCED } : std::nullopt;
	}

	std::optional<std::pair<HitRecord, size_t>> KDTree::RayIntersectsSingleObject(
		math::Ray const& _ray,
		std::vector<std::shared_ptr<Hittable>> const& _hittables,
		float _tmin,
		float _tmax
	) const {
		return RayIntersectsSingleObject(_ray, _hittables, m_flat_tree[0], _tmin, _tmax);
	}

	std::optional<std::pair<HitRecord, size_t>> KDTree::RayIntersectsSingleObject(
		math::Ray const& _ray, 
		std::vector<std::shared_ptr<Hittable>> const& _hittables,
		FlatKDNode const& _curr_node,
		float _tmin,
		float _tmax) const {
		// Check if top level bbox is intersected
		if (!_curr_node.bbox.RayIntersect(_ray, _tmin, _tmax)) {
			return std::nullopt;
		}

		if (_curr_node.obj_index.has_value()) { // Node is leaf
			// Check if we hit
			auto hit = _hittables[_curr_node.obj_index.value()]->Hit(_ray, _tmin, _tmax);
			if (!hit.has_value()) {
				return std::nullopt;
			}
			return { {hit.value(), _curr_node.obj_index.value()} };
		}

		auto left = _curr_node.left;
		auto right = _curr_node.right;

		// Check if we hit left
		auto node_left = RayIntersectsSingleObject(_ray, _hittables, m_flat_tree[left.value()], _tmin, _tmax);

		if (node_left.has_value()) { // If we hit something, it is closer to the origin
			_tmax = node_left->first.m_t; // Update tmax
		}

		// Check right with possibly new tmin
		auto node_right = RayIntersectsSingleObject(_ray, _hittables, m_flat_tree[right.value()], _tmin, _tmax);

		if (node_right.has_value()) {
			return node_right; // We hit something, it is closer to the origin
		}
		else if (node_left.has_value()) {
			return node_left; // Else left was closer
		}

		return std::nullopt;
	}

	KDTree::KDTree() :
		m_flat_tree{},
		m_world_size{},
		m_num_tracked_objects{} {}

	KDNode::KDNode() :
		bbox{},
		obj_index{},
		left{},
		right{},
		level{},
		num_subnodes{}
	{}
}