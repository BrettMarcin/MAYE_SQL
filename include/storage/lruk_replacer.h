#pragma once

#include <vector>
#include <cstddef>
#include <list>
#include "common/config.h"

using namespace std;

namespace maye_sql {

struct LRUKNode;

class LRUK_Replacer {
 public:
  explicit LRUK_Replacer(size_t num_frames, size_t k);
  bool Evict(frame_id_t* frame_id);
  void RecordAccess(frame_id_t frame_id);
  void Remove(frame_id_t frame_id);
  void SetEvictable(frame_id_t frame_id, bool set_evictable);
  int Size();

 private:
  size_t current_size;
  size_t k;
  size_t replacer_size;
  size_t current_timestamp;
  unordered_map<frame_id_t, LRUKNode*> map;
  mutex latch;
};

struct LRUKNode {
  frame_id_t frame_id;
  size_t k;
  list<size_t> history;
  bool is_evictable{false};
};

}  // namespace maye_sql