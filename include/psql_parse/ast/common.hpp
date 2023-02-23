#pragma once

#include "location.hh"

#include <memory>
#include <variant>
#include <optional>
#include <vector>

namespace psql_parse {

	struct Node {
		const location loc;
	};

	template <typename T>
	class box {
		std::unique_ptr<T> ref;

	protected:

	public:
		box(): ref(nullptr) {}
		box(T&& inner): ref(new T(std::move(inner))) {}
		box(T* inner): ref(inner) {}
		box(std::unique_ptr<T>&& ptr): ref(std::move(ptr)) {}
		box(const T& inner): ref(new T(inner)) {}
		box(const box& other): box(*other.ref) {}
		box(box<T>&& other) noexcept: ref(std::move(other.ref)) {}

		box &operator=(const box &other) {
			*ref = *other.ref;
			return *this;
		}

		~box() = default;

		T &operator*() { return *ref; };
		const T &operator*() const { return *ref; };

		T *operator->() { return ref.get(); }
		const T *operator->() const { return ref.get(); }

		friend auto operator==(const box<T>& l, const box<T> r) {
			return *(l.ref) == *(r.ref);
		}

		friend auto operator<=>(const box<T>& l, const box<T> r) {
			return *(l.ref) <=> *(r.ref);
		}

		template <typename... Args>
		static box<T> make(Args... args) {
			return box(std::make_unique<T>(args...));
		}

	};

#define DEFAULT_SPACESHIP(Type) \
	friend auto operator<=>(const Type&, const Type&) = default

	struct DecimalType { std::optional<uint64_t> precision, scale; DEFAULT_SPACESHIP(DecimalType); };
	struct FloatType { std::optional<uint64_t> precision; DEFAULT_SPACESHIP(FloatType); };
	struct IntegerType { DEFAULT_SPACESHIP(IntegerType); };
	struct NumericType { std::optional<uint64_t> precision, scale;DEFAULT_SPACESHIP(NumericType); };
	struct SmallIntType { DEFAULT_SPACESHIP(SmallIntType); };
	struct RealType {DEFAULT_SPACESHIP(RealType); };
	struct DoublePrecisionType {DEFAULT_SPACESHIP(DoublePrecisionType); };
	struct CharType { uint64_t length; DEFAULT_SPACESHIP(CharType); };
	struct VarCharType { uint64_t length; DEFAULT_SPACESHIP(VarCharType); };
	struct NationalCharType { uint64_t length; DEFAULT_SPACESHIP(NationalCharType); };
	struct NationalVarCharType { uint64_t length; DEFAULT_SPACESHIP(NationalVarCharType); };
	struct Bit { uint64_t length; DEFAULT_SPACESHIP(Bit); };
	struct VarBit { uint64_t length; DEFAULT_SPACESHIP(VarBit); };
	struct DateType {DEFAULT_SPACESHIP(DateType); };
	struct TimeType { uint64_t precision; bool with_timezone; DEFAULT_SPACESHIP(TimeType); };
	struct TimeStampType { uint64_t precision; bool with_timezone; DEFAULT_SPACESHIP(TimeStampType); };


	using DataType = std::variant<
			DecimalType,
			FloatType,
			IntegerType,
			NumericType,
			SmallIntType,
			RealType,
			DoublePrecisionType,
			CharType,
			VarCharType,
			NationalCharType,
			NationalVarCharType,
			Bit,
			VarBit,
			DateType,
			TimeType,
			TimeStampType>;

	using Name = std::string;

	struct QualifiedName {
		std::optional<Name> catalog = std::nullopt;
		std::optional<Name> schema = std::nullopt;
		Name name;
		friend auto operator<=>(const QualifiedName&, const QualifiedName&) = default;
	};

	using DomainName = QualifiedName;

	using V_NULL = std::monostate;

	enum class StringLiteralType {
		BIT,
		CHAR,
		HEX,
		NATIONAL
	};
}