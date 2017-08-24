#include <vector>

#include "WorkerPool.h"

WorkerPool::WorkerPool(const AppCallback& callback, uint32_t pool_size):
  callback_(callback), finished_(false) {
  for (uint32_t i = 0; i < pool_size; ++i) {
    threads_.emplace_back(&WorkerPool::DoWork, this);
  }
}

WorkerPool::~WorkerPool() {
  { // Lock scope
    std::unique_lock<std::mutex> lock(mutex_);
    finished_ = true;
  }
  cond_.notify_all();
  for (std::thread& thread: threads_) {
    thread.join();
  }
}

// Producer thread context
void WorkerPool::Add(WorkItemPtr work_item) {
  { // Lock scope
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.emplace(work_item);
  }
  cond_.notify_all();
}

// Consumer threads context
void WorkerPool::DoWork() {
  while (true) {
    WorkItemPtr work_item;
    { // Lock scope
      std::unique_lock<std::mutex> lock(mutex_);
      cond_.wait(lock, [this] () { 
        return !queue_.empty() || finished_;
      });
      if (!queue_.empty()) {
        work_item = std::move(queue_.front());
        queue_.pop();
      } else if (finished_) {
        break;
      }
    }
    if (work_item) {
      work_item->Process();
      callback_(work_item);
    }
  }
}

void ReadyForDelivery(WorkItemPtr work_item) {
  std::shared_ptr<SleepyWorkItem> sleepy_work_item = 
      std::dynamic_pointer_cast<SleepyWorkItem> (work_item);
  std::cout << "Id " << sleepy_work_item->id() << ", " << 
      sleepy_work_item->msec_duration() << 
      " millisec is ready for delivery!" << std::endl;
}

int main(int argc, char** argv) {
  std::vector<WorkItemPtr> items;
  items.emplace_back(std::make_shared<SleepyWorkItem>(1, 100));
  items.emplace_back(std::make_shared<SleepyWorkItem>(2, 700));
  items.emplace_back(std::make_shared<SleepyWorkItem>(3, 500));
  items.emplace_back(std::make_shared<SleepyWorkItem>(4, 200));
  items.emplace_back(std::make_shared<SleepyWorkItem>(5, 300));

  auto start = std::chrono::high_resolution_clock::now();
  { // Object scope
    WorkerPool worker_pool(std::bind(&ReadyForDelivery, std::placeholders::_1), 4);
    for (WorkItemPtr item: items) {
      worker_pool.Add(item);
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "Completed work items in " << 
    std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " milliseconds." << std::endl;

  return 0;
}
