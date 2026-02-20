#include "storage/lruk_replacer.h"

using namespace std;
using namespace maye_sql;

namespace maye_sql {
    LRUK_Replacer::LRUK_Replacer(size_t num_frames, size_t k) {
        current_size = 0;
        this->k = k;
        replacer_size = num_frames;
        current_timestamp = 1;
    }

    bool LRUK_Replacer::Evict(int* frame_id) {
        lock_guard<mutex> lock(latch);
        int victim_id = -1;
        size_t earliest_timestamp = numeric_limits<size_t>::max();
    
        int finite_victim_id = -1;
        size_t earliest_k_timestamp = numeric_limits<size_t>::max();
        for (auto const& [id, node] : map) {
            if (!node->is_evictable) {
                continue;
            }

            if (node->history.size() < k) {
                if (node->history.front() < earliest_timestamp) {
                    earliest_timestamp = node->history.front();
                    victim_id = id;
                }
            } else if (victim_id == -1) {
                if (node->history.front() < earliest_k_timestamp) {
                    earliest_k_timestamp = node->history.front();
                    finite_victim_id = id;
                }
            }
        }

        int final_victim = (victim_id != -1) ? victim_id : finite_victim_id;

        if (final_victim != -1) {
            *frame_id = final_victim;
            
            current_size--;
            delete map[final_victim];
            map.erase(final_victim);
            return true;
        }

        return false;
    }

    int LRUK_Replacer::Size() {
        lock_guard<mutex> lock(latch);
        return current_size;
    }

    void LRUK_Replacer::Remove(int frame_id) {
        lock_guard<mutex> lock(latch);
        if (map.find(frame_id) != map.end()) {
            if (map[frame_id]->is_evictable) {
                current_size--;
            }
            delete map[frame_id];
            map.erase(frame_id);   
        }
    }

    void LRUK_Replacer::SetEvictable(int frame_id, bool set_evictable) {
        lock_guard<mutex> lock(latch);
        if (map.find(frame_id) != map.end()) {
            if (!map[frame_id]->is_evictable && set_evictable) {
                current_size++;
            } else if (map[frame_id]->is_evictable && !set_evictable) {
                current_size--;
            }
            map[frame_id]->is_evictable = set_evictable;
        }
    }

    void LRUK_Replacer::RecordAccess(int frame_id) {
        lock_guard<mutex> lock(latch);
        current_timestamp++;
        if (map.find(frame_id) == map.end()) {
            LRUKNode* node = new LRUKNode();
            node->frame_id = frame_id;
            node->history.push_back(current_timestamp);
            map[frame_id] = node;
        } else {
            LRUKNode* node = map[frame_id];
            node->history.push_back(current_timestamp);
            if (node->history.size() > this->k) {
                node->history.pop_front();
            }
        }
    }

}