#ifndef _ORDERED_WORKER_POOL_2_H_
#define _ORDERED_WORKER_POOL_2_H_

#include <iostream>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "WorkItem.h"

class OrderedWorkerPool2 {
public:
  OrderedWorkerPool2();
  ~OrderedWorkerPool2() = default;

  void Add(WorkItemPtr work_item);
  void Finalize();

private:
  void DoDelivery();
  void ReadyForDelivery(WorkItemPtr work_item);

  std::queue<std::future<WorkItemPtr>> queue_;
  std::thread thread_;
  std::mutex mutex_;
  std::condition_variable cond_;
  bool finished_;
};

#endif // _ORDERED_WORKER_POOL_2_H_
