#include <future>
#include <optional>
#include <queue>
#include <thread>
#include "common/config.h"
#include "storage/disk_manager.h"

using namespace std;

namespace maye_sql {

    struct DiskRequest {
        bool is_write;
        char *data;
        page_id_t page_id;
        promise<bool> callback;
    };

    class DiskScheduler {

        friend class DiskSchedulerTestPeer;
        
        public:
            explicit DiskScheduler(DiskManager *disk_manager);
            ~DiskScheduler();

            void Schedule(DiskRequest request);

            void StartWorkerThread();

        private:
            DiskManager *disk_manager;
            thread background_thread;
            queue<optional<DiskRequest>> request_queue;
            mutex latch;
            condition_variable cv;
    
            bool shutdown{false};

    };
}