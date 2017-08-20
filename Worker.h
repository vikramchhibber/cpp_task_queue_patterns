#ifndef _WORKER_H_
#define _WORKER_H_

#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "WorkItem.h"

class Worker {
public:
  Worker();
  ~Worker() = default;

  void Add(WorkItemPtr work_item);
  void Finalize();

private:
  void DoWork();
  void ReadyForDelivery(WorkItemPtr work_item);

  std::queue<WorkItemPtr> queue_;
  std::thread thread_;
  std::mutex mutex_;
  std::condition_variable cond_;
  bool finished_;
};

#endif // _WORKER_H_

