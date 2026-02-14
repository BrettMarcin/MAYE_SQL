#pragma once
#include <vector>
#include "exception/exception.h"

namespace maye_sql {

class Replacer {
public:
    Replacer() = default;
    virtual ~Replacer() = default;


    virtual bool Victim(int *frame_id) = 0;

    virtual void Pin(int frame_id) = 0;

    virtual void Unpin(int frame_id) = 0;

    virtual size_t Size() = 0;
};

}