#include <gtest/gtest.h>
#include <string>
#include "type/value.h"

using namespace std;
using namespace maye_sql;

TEST(ValueTest, RoundTripInteger) {
  Value v(int32_t{42});

  EXPECT_EQ(v.GetTypeId(), TypeId::INTEGER);
  EXPECT_FALSE(v.IsNull());
  EXPECT_EQ(v.GetAs<int32_t>(), 42);
}

TEST(ValueTest, RoundTripBoolean) {
  Value v(true);

  EXPECT_EQ(v.GetTypeId(), TypeId::BOOLEAN);
  EXPECT_FALSE(v.IsNull());
  EXPECT_EQ(v.GetAs<bool>(), true);
}

TEST(ValueTest, RoundTripVarchar) {
  Value v(std::string("alice"));

  EXPECT_EQ(v.GetTypeId(), TypeId::VARCHAR);
  EXPECT_FALSE(v.IsNull());
  EXPECT_EQ(v.GetAs<std::string>(), "alice");
}

TEST(ValueTest, DefaultIsNull) {
  Value v;

  EXPECT_TRUE(v.IsNull());
  EXPECT_EQ(v.GetTypeId(), TypeId::INVALID);
}

TEST(ValueTest, TypedNull) {
  Value v(TypeId::INTEGER);

  EXPECT_TRUE(v.IsNull());
  EXPECT_EQ(v.GetTypeId(), TypeId::INTEGER);
}

TEST(ValueTest, CheckSerialize) {
  char buffer[4];

  Value v3(int32_t{42});
  v3.SerializeTo(buffer);

  Value v4 = Value::DeserializeFrom(buffer, TypeId::INTEGER);

  EXPECT_EQ(v4.GetTypeId(), TypeId::INTEGER);
  EXPECT_FALSE(v4.IsNull());
  EXPECT_EQ(v4.GetAs<int32_t>(), 42);

  Value v6(true);
  v6.SerializeTo(buffer);

  Value v5 = Value::DeserializeFrom(buffer, TypeId::BOOLEAN);

  EXPECT_EQ(v5.GetTypeId(), TypeId::BOOLEAN);
  EXPECT_FALSE(v5.IsNull());
  EXPECT_EQ(v5.GetAs<bool>(), true);
}
