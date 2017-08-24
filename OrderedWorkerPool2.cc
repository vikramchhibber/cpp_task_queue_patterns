#include "OrderedWorkerPool2.h"

OrderedWorkerPool2::OrderedWorkerPool2(const AppCallback& callback):
  callback_(callback), thread_(&OrderedWorkerPool2::DoDelivery, this), finished_(false) {
}

OrderedWorkerPool2::~OrderedWorkerPool2() {
  { // Lock scope
    std::unique_lock<std::mutex> lock(mutex_);
    finished_ = true;
  }
  cond_.notify_one();
  thread_.join();
}

// Producer thread context
void OrderedWorkerPool2::Add(WorkItemPtr work_item) {
  { // Lock scope
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.emplace(std::async(std::launch::async,
    [work_item] {
      // Do real work of processing the WorkItem asynchronous 
      work_item->Process();
      return work_item;
    }));
  }
  cond_.notify_one();
}

// Delivery thread context
void OrderedWorkerPool2::DoDelivery() {
  while (true) {
    std::future<WorkItemPtr> future;
    { // Lock scope
      std::unique_lock<std::mutex> lock(mutex_);
      // Wait if queue is empty and finished_ is false
      cond_.wait(lock, [this] () { 
        return !queue_.empty() || finished_;
      });
      if (!queue_.empty()) {
        future = std::move(queue_.front());
        queue_.pop();
      } else if (finished_) {
        break;
      } else {
        continue;
      }
    }
    // We must call "get" outside the lock scope as it may
    // block in case the WorkItem is deferred or is being processed
    // and the producer thread will also block.
    // The "get" call also ensures that the 
    // deferred WorkItem is processed.
    WorkItemPtr work_item = future.get();
    if (work_item) {
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
    OrderedWorkerPool2 worker_pool(std::bind(&ReadyForDelivery, std::placeholders::_1));
    for (WorkItemPtr item: items) {
      worker_pool.Add(item);
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "Completed work items in " << 
    std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " milliseconds." << std::endl;

  return 0;
}
