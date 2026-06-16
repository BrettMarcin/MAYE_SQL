#pragma once

namespace maye_sql {

enum class TypeId { INVALID, BOOLEAN, INTEGER, VARCHAR };

inline int GetTypeSize(TypeId type) {
  switch (type) {
    case TypeId::INVALID:
    case TypeId::VARCHAR:
      return 0;
    case TypeId::BOOLEAN:
      return 1;
    case TypeId::INTEGER:
      return 4;
    default:
      return 0;
  }
}

}  // namespace maye_sql