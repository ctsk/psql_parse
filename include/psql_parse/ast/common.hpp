#pragma once

#include "location.hh"

#include <memory>
#include <variant>
#include <optional>
#include <vector>

namespace psql_parse {

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