#ifndef _WORKER_POOL_H_
#define _WORKER_POOL_H_

#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "WorkItem.h"

class WorkerPool {
public:
  WorkerPool(uint32_t pool_size);
  ~WorkerPool() = default;

  void Add(WorkItemPtr work_item);
  void Finalize();

private:
  void DoWork();
  void ReadyForDelivery(WorkItemPtr work_item);

  std::queue<WorkItemPtr> queue_;
  std::vector<std::thread> threads_;
  std::mutex mutex_;
  std::condition_variable cond_;
  bool finished_;
};

#endif // _WORKER_POOL_H_

