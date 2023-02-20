//
// Created by christian on 18.02.23.
//
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

	struct DecimalType { std::optional<uint64_t> precision, scale; };
	struct FloatType { std::optional<uint64_t> precision; };
	struct IntegerType {};
	struct NumericType { std::optional<uint64_t> precision, scale; };
	struct SmallIntType {};
	struct RealType {};
	struct DoublePrecisionType {};
	struct CharType { uint64_t length; };
	struct VarCharType { uint64_t length; };
	struct NationalCharType { uint64_t length; };
	struct NationalVarCharType { uint64_t length; };
	struct Bit { uint64_t length; };
	struct VarBit { uint64_t length; };
	struct DateType {};
	struct TimeType { uint64_t precision; bool with_timezone; };
	struct TimeStampType { uint64_t precision; bool with_timezone; };


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
		std::optional<Name> catalog;
		std::optional<Name> schema;
		Name name;
	};

	using DomainName = QualifiedName;

	enum class StringLiteralType {
		BIT,
		CHAR,
		HEX,
		NATIONAL
	};
}