#include <thread>
#include <mutex>
#include <condition_variable>
#include <optional>
#include "storage/disk_scheduler.h"


using namespace maye_sql;
using namespace std;

DiskScheduler::DiskScheduler(DiskManager *disk_manager) {
    this->disk_manager = disk_manager;
    background_thread = thread(&DiskScheduler::StartWorkerThread, this);

}

DiskScheduler::~DiskScheduler() {
    {
        lock_guard<std::mutex> lock(latch);
        shutdown = true;
    }
    cv.notify_all();
    
    if (background_thread.joinable()) {
        background_thread.join();
    }
}

void DiskScheduler::StartWorkerThread() {
    unique_lock<mutex> lock(latch);
    while (true) {
        cv.wait(lock, [this] { 
            return shutdown || !request_queue.empty(); 
        });

        if (shutdown && request_queue.empty()) {
            break;
        }

        auto request_opt = std::move(request_queue.front());
        request_queue.pop();
        lock.unlock();

        if (request_opt.has_value()) {
            DiskRequest &req = request_opt.value();
            if (req.is_write) {
                disk_manager->WritePage(req.page_id, req.data);
            } else {
                disk_manager->ReadPage(req.page_id, req.data);
            }

            req.callback.set_value(true);
        }

        lock.lock();
    }
}

void DiskScheduler::Schedule(DiskRequest request) {
    lock_guard<mutex> lock(latch);
    request_queue.emplace(std::move(request));
    cv.notify_one();
}

