#pragma once

#include <chrono>
#include <ctime>
#include <string>

namespace timer {
  struct TimerClock {
    // Constants
    static time_t const NULL_TIME = 0;
    static time_t const MIN_TIME = -2209013206;
    static time_t const MAX_TIME = 253402271999;

    // Format a datetime to %Y-%m-%d %H:%M:%S %z
    static std::string format(time_t dateTime);

    // Format a duration in seconds as HH:MM:SS
    static std::string format(std::chrono::seconds duration);

    // Convert seconds to hours
    static float toHours(std::chrono::seconds seconds);

    // Parse a local datetime in %Y-%m-%dT%H:%M:%S format (throws for bad values)
    static time_t parseLocalDateTime(const std::string &dateTime);

    // Parse a month, and optionally year from a string (throws for bad values)
    static time_t parseMonthYear(const std::string &dateTime);

    // Find the date of the end of the month for a given value
    static time_t endOfMonth(const time_t date);
  };
} // namespace timer
