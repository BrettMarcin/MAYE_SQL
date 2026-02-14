#include <stdexcept>
#include <string>

namespace maye_sql {

class MayeSQLException : public std::runtime_error {
public:
    explicit MayeSQLException(const std::string& message) 
        : std::runtime_error("MayeSQL Error: " + message) {}
};

class BufferPoolException : public MayeSQLException {
public:
    explicit BufferPoolException(const std::string& message) 
        : MayeSQLException("Buffer Pool - " + message) {}
};

} // namespace maye_sql