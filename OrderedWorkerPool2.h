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
  typedef std::function<void (WorkItemPtr)> AppCallback;

  OrderedWorkerPool2(const AppCallback& callback);
  ~OrderedWorkerPool2();

  void Add(WorkItemPtr work_item);

private:
  void DoDelivery();
  void ReadyForDelivery(WorkItemPtr work_item);

  const AppCallback callback_;
  std::queue<std::future<WorkItemPtr>> queue_;
  std::thread thread_;
  std::mutex mutex_;
  std::condition_variable cond_;
  bool finished_;
};

#endif // _ORDERED_WORKER_POOL_2_H_
