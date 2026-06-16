#pragma once

#include <vector>
#include <string>
#include "common/rid.h"
#include "common/config.h"
#include "catalog/schema.h"
#include "type/value.h"

namespace maye_sql {

class Tuple {
 public:
  Tuple() : rid(RID()), data(nullptr), size(0) {}

  Tuple(char* source, uint32_t length, RID rid_val) : rid(rid_val), size(length) {
    data = new char[size];
    std::memcpy(data, source, size);
  }

  Tuple(const char* source, uint32_t length) : rid(RID()), size(length) {
    if (source != nullptr && length > 0) {
      data = new char[size];
      std::memcpy(data, source, size);
    } else {
      data = nullptr;
      size = 0;
    }
  }

  // TODO: handle varchars
  Tuple(const std::vector<Value>& values, const Schema& schema) {
    size = schema.GetFixedLength();

    for (uint32_t idx : schema.GetUninlinedColumns()) {
      std::string s = values[idx].GetAs<std::string>();
      size += sizeof(uint32_t) + s.length();
    }

    data = new char[size];
    uint32_t var_offset = schema.GetFixedLength();
    for (uint32_t i = 0; i < values.size(); i++) {
      if (!schema.GetColumn(i).IsInlined()) {
        std::string s = values[i].GetAs<std::string>();
        int32_t offset = schema.GetColumn(i).GetColumnOffset();
        uint32_t len = s.length();
        memcpy(data + offset, &var_offset, 4);
        memcpy(data + var_offset, &len, 4);
        memcpy(data + var_offset + 4, s.data(), s.length());
        var_offset += sizeof(uint32_t) + s.length();
      } else {
        uint32_t offset = schema.GetColumn(i).GetColumnOffset();
        values[i].SerializeTo(data + offset);
      }
    }
  }

  Tuple(const Tuple& other) : rid(other.rid), size(other.size) {
    if (other.data != nullptr) {
      data = new char[size];
      std::memcpy(data, other.data, size);
    } else {
      data = nullptr;
    }
  }

  Value GetValue(const Schema& schema, uint32_t col_idx) const {
    if (schema.GetColumn(col_idx).IsInlined()) {
      uint32_t offset = schema.GetColumn(col_idx).GetColumnOffset();
      return Value::DeserializeFrom(data + offset, schema.GetColumn(col_idx).GetColumnType());
    } else {
      uint32_t offset = schema.GetColumn(col_idx).GetColumnOffset();
      uint32_t var_offset;
      memcpy(&var_offset, data + offset, sizeof(uint32_t));
      uint32_t len;
      memcpy(&len, data + var_offset, sizeof(uint32_t));
      std::string s(data + var_offset + sizeof(uint32_t), len);
      return Value(s);
    }
  }

  Tuple& operator=(const Tuple& other) {
    if (this != &other) {
      delete[] data;
      size = other.size;
      rid = other.rid;
      if (other.data != nullptr) {
        data = new char[size];
        std::memcpy(data, other.data, size);
      } else {
        data = nullptr;
      }
    }
    return *this;
  }

  ~Tuple() {
    delete[] data;
  }

  inline RID GetRid() const {
    return rid;
  }
  inline void SetRid(RID rid_val) {
    rid = rid_val;
  }
  inline uint32_t GetLength() const {
    return size;
  }
  inline char* GetData() const {
    return data;
  }

 private:
  RID rid;
  char* data;
  uint32_t size;
};

}  // namespace maye_sql