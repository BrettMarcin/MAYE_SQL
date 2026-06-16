#pragma once

#include "catalog/column.h"
#include "exception/exception.h"
#include <vector>
#include <string>

namespace maye_sql {

class Schema {
 public:
  explicit Schema(const std::vector<Column>& cols) : columns(cols), fixed_length(0) {
    for (uint32_t i = 0; i < columns.size(); i++) {
      if (!columns[i].IsInlined()) {
        uninlined_columns.push_back(i);
      }
      columns[i].SetColumnOffset(fixed_length);
      fixed_length += columns[i].GetFixedLength();
    }
  }

  uint32_t GetColumnCount() const {
    return columns.size();
  }

  const std::vector<uint32_t>& GetUninlinedColumns() const {
    return uninlined_columns;
  }

  uint32_t GetColIdx(const std::string& name) const {
    for (uint32_t i = 0; i < columns.size(); i++) {
      if (columns[i].GetName() == name) {
        return i;
      }
    }

    throw MayeSQLException("Column not found: " + name);
  }

  const Column& GetColumn(uint32_t index) const {
    return columns[index];
  }

  uint32_t GetFixedLength() const {
    return fixed_length;
  }

 private:
  std::vector<Column> columns;
  uint32_t fixed_length;
  std::vector<uint32_t> uninlined_columns;
};
}  // namespace maye_sql