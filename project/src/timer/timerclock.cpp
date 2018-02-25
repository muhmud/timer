
#include <timer/timerclock.hpp>
#include <util/datetime.hpp>

// Timer constants
#define SECONDS_MINUTE 60
#define SECONDS_HOUR (SECONDS_MINUTE * SECONDS_MINUTE)

namespace timer {
  std::string TimerClock::format(time_t dateTime) {
    return util::datetime::format(dateTime, "%Y-%m-%d %H:%M:%S %z");
  }

  std::string TimerClock::format(std::chrono::seconds duration) {
    int64_t timeDifference = duration.count();

    // Calculate hours, minutes, and seconds
    const int hours = (timeDifference - (timeDifference % SECONDS_HOUR)) / SECONDS_HOUR;
    timeDifference -= hours * SECONDS_HOUR;

    const int minutes = (timeDifference - (timeDifference % SECONDS_MINUTE)) / SECONDS_MINUTE;
    const int seconds = timeDifference - minutes * SECONDS_MINUTE;

    // Format the result into a string
    auto pad = [&](int value) { return value < 10 ? "0" : ""; };
    std::ostringstream result;
    result << pad(hours) << hours << ":" << pad(minutes) << minutes << ":" << pad(seconds)
           << seconds;

    return result.str();
  }

  float TimerClock::toHours(std::chrono::seconds seconds) {
    return ((float)seconds.count()) / 60 / 60;
  }

  time_t TimerClock::parseDate(const std::string &dateTime) {
    static auto const locale = util::datetime::toLocale("%Y-%m-%d");

    auto result = util::datetime::parse(dateTime, locale);

    if (result.is_not_a_date_time()) {
      throw std::runtime_error("invalid date value");
    }

    return boost::posix_time::to_time_t(result);
  }

  time_t TimerClock::parseMonthYear(const std::string &dateTime) {
    static auto const locale = util::datetime::toLocale("%b %Y");

    auto result = util::datetime::parse(dateTime, locale);

    if (result.is_not_a_date_time()) {
      auto const today = boost::gregorian::day_clock::local_day();

      return util::datetime::toDate(today.year(), today.month(), 1);
    }

    // Validate the year
    auto year = result.date().year();

    if (year < 1900) {
      // Invalid year: use the current year
      return util::datetime::toDate(boost::gregorian::day_clock::local_day().year(),
                                    result.date().month(), 1);
    }

    return boost::posix_time::to_time_t(result);
  }

  time_t TimerClock::endOfMonth(const time_t date) {
    auto const endOfMonth = boost::posix_time::from_time_t(date).date().end_of_month();

    return util::datetime::toDate(endOfMonth.year(), endOfMonth.month(), endOfMonth.day());
  }
} // namespace timer
