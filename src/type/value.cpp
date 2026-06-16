#include <cstring>

#include "type/value.h"
#include "type/type_id.h"
#include "exception/exception.h"

namespace maye_sql {

Value Value::DeserializeFrom(const char* src, TypeId type) {
  switch (type) {
    case TypeId::BOOLEAN: {
      bool v;
      memcpy(&v, src, sizeof(v));
      return Value(v);
    }
    case TypeId::INTEGER: {
      int32_t v;
      memcpy(&v, src, sizeof(v));
      return Value(v);
    }
    // case TypeId::VARCHAR: {
    //   stringf v;
    //   memcpy(&v, src, sizeof(v));
    //   return Value(v);
    // }
    default: {
      throw MayeSQLException("DeserializeFrom: unsupported type");
    }
  }
}

void Value::SerializeTo(char* dest) const {
  switch (type_id) {
    case TypeId::BOOLEAN: {
      bool v = GetAs<bool>();
      memcpy(dest, &v, sizeof(v));
      break;
    }
    case TypeId::INTEGER: {
      int32_t v = GetAs<int32_t>();
      memcpy(dest, &v, sizeof(v));
      break;
    }
    default: {
      throw MayeSQLException("SerializeTo: unsupported type");
    }
  }
}
}  // namespace maye_sql