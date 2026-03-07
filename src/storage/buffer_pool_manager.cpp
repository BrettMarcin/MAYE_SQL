#include "storage/buffer_pool_manager.h"
#include "storage/page.h"
#include "spdlog/spdlog.h"

using namespace maye_sql;
using namespace std;

namespace maye_sql {

BufferPoolManager::~BufferPoolManager() {}

BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager* disk_manager,
                                     size_t replacer_k) {
  this->pool_size = pool_size;
  this->replacer = make_unique<LRUK_Replacer>(pool_size, replacer_k);
  this->disk_scheduler = make_unique<DiskScheduler>(disk_manager);
  for (size_t i = 0; i < pool_size; i++) {
    free_list.push_back(i);
  }

  pages = new Page[pool_size];

  spdlog::info("Buffer Pool initialized successfully.");
}

Page* BufferPoolManager::NewPage(page_id_t* page_id) {
  lock_guard<mutex> lock(latch);
  if (free_list.size() == 0) {
    frame_id_t frame;
    bool result = replacer->Evict(&frame);
    if (!result) {
      *page_id = INVALID_PAGE_ID;
      return nullptr;
    }
    Page& old_page = pages[frame];

    // if page is hasn't made it to disk, flush it
    if (old_page.is_dirty) {
      DiskRequest request;

      request.is_write = true;
      request.page_id = old_page.page_id;
      request.data = old_page.data;
      promise<bool> promise;
      auto future = promise.get_future();
      request.callback = std::move(promise);

      disk_scheduler->Schedule(std::move(request));
      future.get();
    }

    page_table.erase(old_page.page_id);
    *page_id = disk_scheduler->disk_manager->AllocatePage();
    Page& page = pages[frame];

    page.pin_count = 1;
    memset(page.data, 0, PAGE_SIZE);
    page.page_id = *page_id;
    page.is_dirty = false;
    page_table[page.page_id] = frame;

    replacer->RecordAccess(frame);
    replacer->SetEvictable(frame, false);

    return &page;
  } else {
    *page_id = disk_scheduler->disk_manager->AllocatePage();
    frame_id_t frame = free_list.front();
    free_list.pop_front();
    Page& page = pages[frame];
    page.pin_count = 1;
    memset(page.data, 0, PAGE_SIZE);
    page.page_id = *page_id;
    page.is_dirty = false;
    page_table[page.page_id] = frame;

    replacer->RecordAccess(frame);
    replacer->SetEvictable(frame, false);

    return &page;
  }
}

Page* BufferPoolManager::FetchPage(page_id_t page_id) {
  lock_guard<mutex> lock(latch);
  frame_id_t frame;

  if (page_table.find(page_id) != page_table.end()) {
    frame = page_table[page_id];
  } else {
    if (free_list.size() == 0) {
      bool result = replacer->Evict(&frame);
      if (!result) {
        spdlog::error("Failed to get a new frame");
        return nullptr;
      }

      Page& page = pages[frame];
      if (page.is_dirty) {
        promise<bool> promise;
        future<bool> future = promise.get_future();
        DiskRequest request{true, page.data, page.page_id, std::move(promise)};
        disk_scheduler->Schedule(std::move(request));
        future.get();
      }
      page_table.erase(page.page_id);
    } else {
      frame = free_list.front();
      free_list.pop_front();
    }

    promise<bool> promise;
    future<bool> future = promise.get_future();
    DiskRequest request{false, pages[frame].data, page_id, std::move(promise)};

    disk_scheduler->Schedule(std::move(request));
    future.get();

    pages[frame].page_id = page_id;
    pages[frame].is_dirty = false;
    page_table[page_id] = frame;
  }

  pages[frame].pin_count++;
  replacer->RecordAccess(frame);
  replacer->SetEvictable(frame, false);
  return &pages[frame];
}

bool BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty) {
  lock_guard<mutex> lock(latch);

  if (page_table.find(page_id) == page_table.end()) {
    return false;
  }

  frame_id_t frame = page_table[page_id];
  Page& page = pages[frame];

  if (page.pin_count <= 0) {
    return false;
  }

  if (is_dirty) {
    page.is_dirty = true;
  }

  page.pin_count--;
  if (page.pin_count == 0) {
    replacer->SetEvictable(frame, true);
  }

  return true;
}

bool BufferPoolManager::DeletePage(page_id_t page_id) {
  lock_guard<mutex> lock(latch);
  // if it's in disk, delete it
  if (page_table.find(page_id) == page_table.end()) {
    disk_scheduler->disk_manager->DeallocatePage(page_id);
    return true;
  } else {
    frame_id_t frame = page_table[page_id];
    Page& page = pages[frame];

    if (page.pin_count > 0) {
      spdlog::warn("Can't delete a page that is being accessed by other threads.");
      return false;
    }

    page.page_id = INVALID_PAGE_ID;
    fill(page.data, page.data + PAGE_SIZE, 0);
    page.is_dirty = false;
    page.pin_count = 0;
    replacer->Remove(frame);
    free_list.push_back(frame);
    page_table.erase(page_id);
    disk_scheduler->disk_manager->DeallocatePage(page_id);
    return true;
  }
}

bool BufferPoolManager::FlushPage(page_id_t page_id) {
  lock_guard<mutex> lock(latch);
  if (page_table.find(page_id) == page_table.end()) {
    return false;
  } else {
    frame_id_t frame = page_table[page_id];
    Page& page = pages[frame];

    if (!page.is_dirty) return true;

    promise<bool> promise;
    future<bool> future = promise.get_future();
    DiskRequest request{true, page.data, page.page_id, std::move(promise)};
    disk_scheduler->Schedule(std::move(request));

    if (future.get()) {
      page.is_dirty = false;
      return true;
    }

    return false;
  }
}

}  // namespace maye_sql