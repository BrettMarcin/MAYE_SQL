#pragma once

#include <string>
#include "type/type_id.h"

namespace maye_sql {
class Column {
 public:
  Column(const std::string& name, TypeId column_type) : name(name), column_type(column_type) {
    fixed_length = GetTypeSize(column_type);
    variable_length = 0;
  }

  Column(const std::string& name, TypeId column_type, int32_t max_length) {
    this->name = name;
    this->column_type = column_type;
    this->fixed_length = sizeof(uint32_t);
    this->variable_length = max_length;
  }

  const std::string& GetName() const {
    return name;
  }

  TypeId GetColumnType() const {
    return column_type;
  }

  int32_t GetFixedLength() const {
    return fixed_length;
  }

  int32_t GetVariableLength() const {
    return variable_length;
  }

  int32_t GetColumnOffset() const {
    return column_offset;
  }

  void SetColumnOffset(int32_t offset) {
    column_offset = offset;
  }

  bool IsInlined() const {
    return column_type != TypeId::VARCHAR;
  }

 private:
  std::string name;
  TypeId column_type;
  int32_t fixed_length;
  int32_t variable_length;
  int32_t column_offset;
};
}  // namespace maye_sql