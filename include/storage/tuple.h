#pragma once

#include <vector>
#include <string>
#include "common/rid.h"
#include "common/config.h"

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

  Tuple(const Tuple& other) : rid(other.rid), size(other.size) {
    if (other.data != nullptr) {
      data = new char[size];
      std::memcpy(data, other.data, size);
    } else {
      data = nullptr;
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