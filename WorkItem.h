#ifndef _WORK_ITEM_H_
#define _WORK_ITEM_H_

#include <iostream>
#include <memory>
#include <chrono>
#include <thread>

class WorkItem {
public:
  WorkItem() = default;
  virtual ~WorkItem() = default;

  virtual void Process() = 0;

};
typedef std::shared_ptr<WorkItem> WorkItemPtr;


class SleepyWorkItem: public WorkItem {
public:
  SleepyWorkItem(uint32_t id, uint32_t msec_duration):
    id_(id), msec_duration_(msec_duration) {}
  ~SleepyWorkItem() = default;

  void Process() override {
    std::this_thread::sleep_for(std::chrono::milliseconds(msec_duration_));
  }

  uint32_t id() const {
    return id_;
  }

  uint32_t msec_duration() const {
    return msec_duration_;
  }

private:
  uint32_t id_;
  uint32_t msec_duration_;

};

#endif // _WORK_ITEM_H_

