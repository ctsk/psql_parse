#pragma once

#include <unordered_map>

#include "expr.hpp"
namespace psql_parse {
	class NodeFactory {

	protected:
		std::unordered_map<void *, location> locations;

	public:
		template<class T, class... Args>
		auto node(location loc, Args... args) -> T* {
			T* ptr = new T(std::forward<Args>(args)...);
			locations[ptr] = loc;
			return ptr;
		}

		template <class T, class... Args>
		auto notNode(location loc, Args...args) -> UnaryOp* {
			T* ptr = node<T>(loc, std::forward<Args>(args)...);
			return node<UnaryOp>(loc, UnaryOp::Op::NOT, ptr);
		}

		void clear() {
			locations.clear();
		}

	};
}
