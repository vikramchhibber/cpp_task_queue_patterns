#ifndef _ORDERED_WORKER_POOL_H_
#define _ORDERED_WORKER_POOL_H_

#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "WorkItem2.h"

class OrderedWorkerPool {
public:
  typedef std::function<void (WorkItemPtr)> AppCallback;

  OrderedWorkerPool(const AppCallback& callback, uint32_t pool_size);
  ~OrderedWorkerPool();

  void Add(WorkItemPtr work_item);

private:
  void DoWork();
  void DoDelivery();
  void ReadyForDelivery(WorkItemPtr work_item);

  const AppCallback callback_;
  // Consumer threads and their queue
  std::queue<WorkItemPtr> queue_;
  std::vector<std::thread> threads_;
  std::mutex mutex_;
  std::condition_variable cond_;
  bool finished_;

  // Delivery thread and its queue
  std::thread delivery_thread_;
  std::queue<WorkItemPtr> delivery_queue_;
  std::mutex delivery_mutex_;
  std::condition_variable delivery_cond_;
  bool delivery_finished_;
};

#endif // _ORDERED_WORKER_POOL_H_

