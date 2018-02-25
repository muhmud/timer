#pragma once

#include <boost/date_time.hpp>
#include <chrono>

namespace util {
  namespace datetime {
    // Create a locale for date/time format parsing
    std::locale toLocale(const std::string &format);

    // Parse a datetime from a string value using the specified locale
    boost::posix_time::ptime parse(const std::string &value, const std::locale &locale);

    // Parse a datetime from a string value where the specified format is expected
    boost::posix_time::ptime parse(const std::string &value, const std::string &format);

    // Format a datetime as a string
    std::string format(time_t dateTime, const std::string &format);

    // Manufacture a date from it's constituent parts
    time_t toDate(int year, int month, int day);

    // The date/time now
    time_t now();

    // Time & timepoint conversion
    std::chrono::system_clock::time_point toTimePoint(const time_t time);
    time_t toTime(const std::chrono::system_clock::time_point &timePoint);

    // Calculate difference in seconds between two times
    std::chrono::seconds duration(time_t start, time_t end);
  } // namespace datetime
} // namespace util
