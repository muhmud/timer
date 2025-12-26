#pragma once

#include <bits/stdint-intn.h>
#include <ctime>
#include <string>

namespace timer {
  struct TimerStatus {
    enum class Value { INVALID, RUNNING, PAUSED, STOPPED };

    // Conversion functions
    static Value toValue(const std::string &value);
    static std::string toString(Value value);
  };

  struct TimerLog {
    int id;
    time_t timer_datetime;
    int64_t timer_work_done;
    TimerStatus::Value status;
    time_t start_datetime;
    time_t end_datetime;
    std::string task;

    TimerLog(int id, time_t timer_datetime, int64_t timer_work_done, TimerStatus::Value status,
             time_t start_datetime, time_t end_datetime, const std::string &task);

    TimerLog(time_t timer_datetime, int64_t timer_work_done, TimerStatus::Value status,
             time_t start_datetime, time_t end_datetime, const std::string &task);

    TimerLog();
  };
} // namespace timer
