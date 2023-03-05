#pragma once

#include "location.hh"

#include <memory>
#include <variant>
#include <optional>
#include <vector>

#define DEFAULT_SPACESHIP(Type) \
	friend auto operator<=>(const Type&, const Type&) = default

#define DEFAULT_EQ(Type) \
	friend bool operator==(const Type&, const Type&) noexcept = default

namespace psql_parse {

	struct Node {
		const location loc;
	};



	using Name = std::string;

	struct QualifiedName {
        DEFAULT_SPACESHIP(QualifiedName);

        std::vector<Name> qualifier;
		Name name;

        explicit QualifiedName(Name name);
	};

	using DomainName = QualifiedName;

	using V_NULL = std::monostate;

	enum class StringLiteralType {
		BIT,
		CHAR,
		HEX,
		NATIONAL
	};

	template <class T>
	class box {
		std::unique_ptr<T> ref_;

	public:
		box()
		: ref_(nullptr) {}

		box(T* t)
		: ref_(t) {}

		template <class... Args>
		static box make(Args... args) {
			return box(new T(std::forward<Args>(args)...));
		}

		T &operator*() { return *ref_; }
		const T &operator*() const { return *ref_; }

		T *operator->() { return ref_.get(); }
		const T *operator->() const { return ref_.get(); }


		friend auto operator<=>(const box<T>& l, const box<T>& r) {
			if (l.ref_.get() == nullptr && r.ref_.get() == nullptr) {
				return true;
			}

			if (l.ref_.get() == nullptr || r.ref_.get() == nullptr) {
				return false;
			}

			return *(l.ref_) <=> *(r.ref_);
		}

		friend auto operator==(const box<T>& l, const box<T>& r) {
			if (l.ref_.get() == nullptr && r.ref_.get() == nullptr) {
				return true;
			}

			if (l.ref_.get() == nullptr || r.ref_.get() == nullptr) {
				return false;
			}

			return *(l.ref_) == *(r.ref_);
		}

	};
}
