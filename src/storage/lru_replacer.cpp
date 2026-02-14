#include "storage/lru_replacer.h"

namespace maye_sql {

    LRUReplacer::LRUReplacer(size_t num_pages) {
        max_pages = num_pages;
        current_amount_pages = 0;
    }

    // LRUReplacer::~LRUReplacer() {

    // }

    size_t LRUReplacer::Size() {
        return current_amount_pages;
    }

    bool LRUReplacer::Victim(int *frame_id) {
        if (current_amount_pages == 0) {
            return false;
        }

        Node* curr = map[end->frame_id];
        *frame_id = curr->frame_id;
        map.erase(end->frame_id);
        remove(end);
        delete curr;

        return true;
    }

    void LRUReplacer::Pin(int frame_id) {
        // Did not find it in the map, add the node
        if (map.find(frame_id) == map.end()) {
            Node* n = new Node(frame_id);
            n->pinned = true;
            map[frame_id] = n;
        } else {
            Node* n = map[frame_id];
            // if we already have it pinned then ignore the request, if not remove it from the list
            if (!n->pinned) {
                n->pinned = true;
                remove(n);
                current_amount_pages--;
            }
        }
    }

    void LRUReplacer::Unpin(int frame_id) {
        // if it's not in, add it to
        if (map.find(frame_id) == map.end()) {
            // check first if we have capacity
            if (current_amount_pages + 1 > max_pages) {
                throw new BufferPoolException("can't add a new page, we are too full");
            }
            
            Node* n = new Node(frame_id);
            n->pinned = false;
            map[frame_id] = n;
            insert(n);
            current_amount_pages++;
        } else {
            Node* n = map[frame_id];
            // If it was marked pinned we need to check if we can add it
            if (n->pinned) {
                if (current_amount_pages + 1 > max_pages) {
                    throw new BufferPoolException("can't add a new page, we are too full");
                }
                n->pinned = true;
                insert(n);
                current_amount_pages++;
            // if it is already in the linked list remove it and move it to the back
            } else {
                remove(n);
                insert(n);
            }
        }
    }

    // Only function is to take a node and insert it, the other logic for the datavase
    // Is taken care of in the other
    void LRUReplacer::insert(Node* n) {
        if (head == nullptr) {
            head = n;
            end = n;
        } else {
            n->next = head;
            head->prev = n;
            head = n;
        }
    }

    void LRUReplacer::remove(Node* n) {
        if (n->next != nullptr) {
            n->next->prev = n->prev;
        }

        if (n->prev != nullptr) {
            n->prev->next = n->next;
        }
    }

}