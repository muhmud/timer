
#include <timer/timerlog.hpp>
#include <util/enum.hpp>

namespace timer {
  TimerStatus::Value TimerStatus::toValue(const std::string &value) {
    CHECK_STRING_TO_ENUM(RUNNING)
    CHECK_STRING_TO_ENUM(PAUSED)
    CHECK_STRING_TO_ENUM(STOPPED)

    return Value::INVALID;
  }

  std::string TimerStatus::toString(TimerStatus::Value value) {
    switch (value) {
      CHECK_ENUM_TO_STRING(RUNNING)
      CHECK_ENUM_TO_STRING(PAUSED)
      CHECK_ENUM_TO_STRING(STOPPED)
    default:
      return "INVALID";
    }
  }

  TimerLog::TimerLog(int id, time_t timer_datetime, int64_t timer_work_done,
                     TimerStatus::Value status, time_t start_datetime, time_t end_datetime,
                     const std::string &task)
      : id(id), timer_datetime(timer_datetime), timer_work_done(timer_work_done), status(status),
        start_datetime(start_datetime), end_datetime(end_datetime), task(task) {}

  TimerLog::TimerLog(time_t timer_datetime, int64_t timer_work_done, TimerStatus::Value status,
                     time_t start_datetime, time_t end_datetime, const std::string &task)
      : TimerLog(-1, timer_datetime, timer_work_done, status, start_datetime, end_datetime, task) {}

  TimerLog::TimerLog() : TimerLog(-1, -1, -1, TimerStatus::Value::INVALID, -1, -1, "") {}
} // namespace timer
