#pragma once

#include <vector>
#include <string>
#include "common/rid.h"
#include "common/config.h"

namespace maye_sql {

class Tuple {
 public:
  Tuple() : rid(RID()), data(nullptr), size(0) {}

  Tuple(std::vector<char> data) : rid(RID()) {
    size = data.size();
    data = new char[size];
    std::memcpy(data, data.data(), size);
  }

  Tuple(const Tuple& other) {
    size = other.size;
    rid = other.rid;
    if (other.data != nullptr) {
      data = new char[size];
      std::memcpy(data, other.data, size);
    } else {
      data = nullptr;
    }
  }

  ~Tuple() {
    delete[] data;
  }

  inline RID GetRid() const {
    return rid;
  }
  inline void SetRid(RID rid) {
    rid = rid;
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