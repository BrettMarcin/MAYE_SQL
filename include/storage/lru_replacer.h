#include "replacer.h"
#include <unordered_map>

namespace maye_sql {

class Node {

    public:
        Node(int frame_id) {
            this->frame_id = frame_id;
            pinned = false;
            next = nullptr;
            prev = nullptr;
        }
        int frame_id;
        bool pinned;
        Node* next;
        Node* prev;
};

class LRUReplacer: public Replacer {
    public:
        explicit LRUReplacer(size_t num_pages);
        
        ~LRUReplacer() override = default;

        bool Victim(int *frame_id) override;
        void Pin(int frame_id) override;
        void Unpin(int frame_id) override;
        size_t Size() override;

    private:
        int max_pages;
        int current_amount_pages;
        Node* head;
        Node* end;
        std::unordered_map<int, Node*> map;

        void insert(Node* n);
        void remove(Node* n);

};

}