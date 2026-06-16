#pragma once

#include <string>
#include "type/type_id.h"

namespace maye_sql {

class Value {
 public:
  Value() : type_id(TypeId::INVALID), is_null(true) {}
  Value(TypeId type_id) : type_id(type_id), is_null(true) {}
  Value(bool value) : type_id(TypeId::BOOLEAN), is_null(false), value(value) {}
  Value(int32_t value) : type_id(TypeId::INTEGER), is_null(false), value(value) {}
  Value(const std::string& value) : type_id(TypeId::VARCHAR), is_null(false), value(value) {}

  TypeId GetTypeId() const {
    return type_id;
  }

  bool IsNull() const {
    return is_null;
  }

  void SerializeTo(char* dest) const;

  static Value DeserializeFrom(const char* src, TypeId type);

  template <typename T>
  T GetAs() const {
    return std::get<T>(value);
  }

 private:
  TypeId type_id;
  bool is_null;
  std::variant<bool, int32_t, std::string> value;
};

}  // namespace maye_sql