#include <vector>
#include <cstddef>
#include <list>

using namespace std;

namespace maye_sql {

struct LRUKNode;

class LRUK_Replacer {

    public:
        explicit LRUK_Replacer(size_t num_frames, size_t k);
        bool Evict(int* frame_id);
        void RecordAccess(int frame_id);
        void Remove(int frame_id);
        void SetEvictable(int frame_id, bool set_evictable);
        int Size();

    private:
        size_t current_size;
        size_t k;
        size_t replacer_size;
        size_t current_timestamp;
        unordered_map<int, LRUKNode*> map;
        mutex latch;

};

struct LRUKNode {
    int frame_id;
    size_t k;
    list<size_t> history;
    bool is_evictable{false};
};

}