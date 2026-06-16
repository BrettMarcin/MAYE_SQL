#include <gtest/gtest.h>
#include <cstring>
#include <vector>
#include "catalog/schema.h"
#include "catalog/column.h"
#include "storage/tuple.h"
#include "type/value.h"

using namespace std;
using namespace maye_sql;

TEST(CatalogTest, FixedTypes) {
  // INTEGER (4 bytes) followed by BOOLEAN (1 byte).
  Column id("id", TypeId::INTEGER);
  Column active("active", TypeId::BOOLEAN);

  Schema schema(std::vector<Column>{id, active});

  EXPECT_EQ(schema.GetColumnCount(), 2u);

  // Offsets should be laid out back-to-back in the fixed section.
  EXPECT_EQ(schema.GetColumn(0).GetColumnOffset(), 0);
  EXPECT_EQ(schema.GetColumn(1).GetColumnOffset(), 4);

  // Total fixed-section size = 4 (int) + 1 (bool).
  EXPECT_EQ(schema.GetFixedLength(), 5u);

  // No varchar columns, so nothing is uninlined.
  EXPECT_TRUE(schema.GetUninlinedColumns().empty());
}

TEST(CatalogTest, VarVharTypes) {
  Column id("id", TypeId::INTEGER);
  Column active("active", TypeId::VARCHAR, 10);

  Schema schema(std::vector<Column>{id, active});

  EXPECT_EQ(schema.GetColumnCount(), 2);

  EXPECT_EQ(schema.GetColumn(0).GetColumnOffset(), 0);
  EXPECT_EQ(schema.GetColumn(1).GetColumnOffset(), 4);

  EXPECT_EQ(schema.GetFixedLength(), 8);
  EXPECT_EQ(schema.GetUninlinedColumns().size(), 1u);
  EXPECT_EQ(schema.GetUninlinedColumns()[0], 1u);
}

TEST(CatalogTest, MultipleVarchars) {
  Column a("a", TypeId::VARCHAR, 10);
  Column b("b", TypeId::INTEGER);
  Column c("c", TypeId::VARCHAR, 5);

  Schema schema(std::vector<Column>{a, b, c});

  EXPECT_EQ(schema.GetColumnCount(), 3u);

  EXPECT_EQ(schema.GetColumn(0).GetColumnOffset(), 0);
  EXPECT_EQ(schema.GetColumn(1).GetColumnOffset(), 4);
  EXPECT_EQ(schema.GetColumn(2).GetColumnOffset(), 8);

  EXPECT_EQ(schema.GetFixedLength(), 12u);

  EXPECT_EQ(schema.GetUninlinedColumns(), std::vector<uint32_t>({0, 2}));
}

TEST(CatalogTest, GetColIdx) {
  Column id("id", TypeId::INTEGER);
  Column active("active", TypeId::BOOLEAN);

  Schema schema(std::vector<Column>{id, active});

  EXPECT_EQ(schema.GetColIdx("id"), 0u);
  EXPECT_EQ(schema.GetColIdx("active"), 1u);
  EXPECT_THROW(schema.GetColIdx("missing"), MayeSQLException);
}

TEST(CatalogTest, EmptySchema) {
  Schema schema(std::vector<Column>{});

  EXPECT_EQ(schema.GetColumnCount(), 0u);
  EXPECT_EQ(schema.GetFixedLength(), 0u);
  EXPECT_TRUE(schema.GetUninlinedColumns().empty());
}

TEST(CatalogTest, TupleRoundTrip) {
  Column id("id", TypeId::INTEGER);
  Column active("active", TypeId::BOOLEAN);
  Schema schema(std::vector<Column>{id, active});

  Tuple tuple(std::vector<Value>{Value(int32_t{42}), Value(true)}, schema);

  EXPECT_EQ(tuple.GetValue(schema, 0).GetTypeId(), TypeId::INTEGER);
  EXPECT_EQ(tuple.GetValue(schema, 0).GetAs<int32_t>(), 42);

  EXPECT_EQ(tuple.GetValue(schema, 1).GetTypeId(), TypeId::BOOLEAN);
  EXPECT_EQ(tuple.GetValue(schema, 1).GetAs<bool>(), true);
}

TEST(CatalogTest, TupleRoundTripValues) {
  Column a("a", TypeId::INTEGER);
  Column b("b", TypeId::INTEGER);
  Column c("c", TypeId::BOOLEAN);
  Schema schema(std::vector<Column>{a, b, c});

  Tuple tuple(std::vector<Value>{Value(int32_t{-7}), Value(int32_t{1000}), Value(false)}, schema);

  EXPECT_EQ(tuple.GetValue(schema, 0).GetAs<int32_t>(), -7);
  EXPECT_EQ(tuple.GetValue(schema, 1).GetAs<int32_t>(), 1000);
  EXPECT_EQ(tuple.GetValue(schema, 2).GetAs<bool>(), false);
}

TEST(CatalogTest, TupleVarcharRoundTrip) {
  Column id("id", TypeId::INTEGER);
  Column name("name", TypeId::VARCHAR, 20);
  Column active("active", TypeId::BOOLEAN);
  Schema schema(std::vector<Column>{id, name, active});

  Tuple tuple(std::vector<Value>{Value(int32_t{42}), Value(std::string("hello")), Value(true)},
              schema);

  EXPECT_EQ(tuple.GetValue(schema, 0).GetAs<int32_t>(), 42);

  EXPECT_EQ(tuple.GetValue(schema, 1).GetTypeId(), TypeId::VARCHAR);
  EXPECT_EQ(tuple.GetValue(schema, 1).GetAs<std::string>(), "hello");

  EXPECT_EQ(tuple.GetValue(schema, 2).GetAs<bool>(), true);
}

TEST(CatalogTest, TupleEmptyString) {
  Column id("id", TypeId::INTEGER);
  Column name("name", TypeId::VARCHAR, 20);
  Schema schema(std::vector<Column>{id, name});

  Tuple tuple(std::vector<Value>{Value(int32_t{7}), Value(std::string(""))}, schema);

  EXPECT_EQ(tuple.GetValue(schema, 0).GetAs<int32_t>(), 7);
  EXPECT_EQ(tuple.GetValue(schema, 1).GetAs<std::string>(), "");
}

TEST(CatalogTest, TupleTwoVarchars) {
  Column first("first", TypeId::VARCHAR, 20);
  Column second("second", TypeId::VARCHAR, 20);
  Schema schema(std::vector<Column>{first, second});

  Tuple tuple(std::vector<Value>{Value(std::string("alice")), Value(std::string("bob"))}, schema);

  EXPECT_EQ(tuple.GetValue(schema, 0).GetAs<std::string>(), "alice");
  EXPECT_EQ(tuple.GetValue(schema, 1).GetAs<std::string>(), "bob");
}