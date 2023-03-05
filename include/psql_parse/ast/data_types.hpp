#pragma once

#include "common.hpp"

namespace psql_parse {
    /*
     * Exact Numeric Type
     */
    struct NumericType { std::optional<uint64_t> precision, scale; DEFAULT_EQ(NumericType); };
    struct DecimalType { std::optional<uint64_t> precision, scale; DEFAULT_EQ(DecimalType); };
    struct SmallIntType { DEFAULT_EQ(SmallIntType); };
    struct IntegerType { DEFAULT_EQ(IntegerType); };
    struct BigIntType { DEFAULT_EQ(BigIntType); };

    /*
     * Approximate Numeric Type
     */
    struct FloatType { std::optional<uint64_t> precision; DEFAULT_EQ(FloatType); };
    struct RealType { DEFAULT_EQ(RealType); };
    struct DoublePrecisionType { DEFAULT_EQ(DoublePrecisionType); };

    /*
     * Binary String Type
     */
    struct BinaryType { std::optional<uint64_t> length; DEFAULT_EQ(BinaryType); };
    struct VarBinaryType { std::optional<uint64_t> length; DEFAULT_EQ(VarBinaryType); };
    struct BlobType { std::optional<uint64_t> length; DEFAULT_EQ(BlobType); };

    /*
     * Character String Type
     */
    struct CharLength {
        DEFAULT_EQ(CharLength);
        enum class Unit {
            CHARACTERS,
            OCTETS,
        };
        uint64_t length;
        std::optional<Unit> unit;
    };

    struct CharType { std::optional<CharLength> length; DEFAULT_EQ(CharType); };
    struct VarCharType { CharLength length; DEFAULT_EQ(VarCharType); };
    struct ClobType { std::optional<CharLength> length; DEFAULT_EQ(ClobType); };
    struct NationalCharType { std::optional<CharLength> length; DEFAULT_EQ(NationalCharType); };
    struct NationalVarCharType { CharLength length; DEFAULT_EQ(NationalVarCharType); };
    struct NationalClobType { std::optional<CharLength> length; DEFAULT_EQ(NationalClobType); };

    /*
     * Boolean Type
     */
    struct BooleanType { DEFAULT_EQ(BooleanType); };

    /*
     * Datetime Type
     */
    struct DateType { DEFAULT_EQ(DateType); };
    struct TimeType { std::optional<uint64_t> precision; std::optional<bool> with_timezone; DEFAULT_EQ(TimeType); };
    struct TimestampType { std::optional<uint64_t> precision; std::optional<bool> with_timezone; DEFAULT_EQ(TimestampType); };

    /*
     * Interval Type
     */
    // struct IntervalType { DEFAULT_EQ(IntervalType); };

    struct UserDefinedType {
        DEFAULT_EQ(UserDefinedType);

        box<QualifiedName> name;
    };
    struct RowType;
    struct RefType;
    struct ArrayType;
    struct MultiSetType;

    using DataType = std::variant<
            NumericType,
            DecimalType,
            SmallIntType,
            IntegerType,
            BigIntType,
            FloatType,
            RealType,
            DoublePrecisionType,
            BinaryType,
            VarBinaryType,
            BlobType,
            CharType,
            VarCharType,
            ClobType,
            NationalCharType,
            NationalVarCharType,
            NationalClobType,
            BooleanType,
            DateType,
            TimeType,
            TimestampType,
            UserDefinedType,
            box<RowType>,
            box<RefType>,
            box<ArrayType>,
            box<MultiSetType>>;

    /*
     * non-predefined  / recursive types
     */

    struct RowType {
        DEFAULT_EQ(RowType);

        using FieldDef = std::pair<Name, DataType>;
        std::vector<FieldDef> fields;
    };

    struct RefType {
        DEFAULT_EQ(RefType);

        UserDefinedType type;
        std::optional<box<QualifiedName>> scope;
    };

    struct ArrayType {
        DEFAULT_EQ(ArrayType);

        DataType type;
        std::optional<uint64_t> max_cardinality;
    };

    struct MultiSetType {
        DEFAULT_EQ(MultiSetType);

        DataType type;
    };

}
