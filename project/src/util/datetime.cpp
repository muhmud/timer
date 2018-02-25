
#include <util/datetime.hpp>

namespace util {
  namespace datetime {
    std::locale toLocale(const std::string &format) {
      return std::locale(std::locale::classic(), new boost::posix_time::time_input_facet(format));
    }

    boost::posix_time::ptime parse(const std::string &value, const std::locale &locale) {
      // Parse the date/time
      boost::posix_time::ptime result;
      std::istringstream input(value);
      input.imbue(locale);
      input >> result;

      return result;
    }

    boost::posix_time::ptime parse(const std::string &value, const std::string &format) {
      return parse(value, toLocale(format));
    }

    std::string format(time_t dateTime, const std::string &format) {
      std::stringstream result;

      result << std::put_time(std::localtime(&dateTime), format.c_str());
      return result.str();
    }

    time_t toDate(int year, int month, int day) {
      std::tm tm = {0};
      tm.tm_mday = day;
      tm.tm_mon = month - 1;
      tm.tm_year = year - 1900;

      return timegm(&tm);
    }

    time_t now() { return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); }

    std::chrono::system_clock::time_point toTimePoint(const time_t time) {
      return std::chrono::system_clock::from_time_t(time);
    }

    time_t toTime(const std::chrono::system_clock::time_point &timePoint) {
      return std::chrono::system_clock::to_time_t(timePoint);
    }

    std::chrono::seconds duration(time_t start, time_t end) {
      return std::chrono::duration_cast<std::chrono::seconds>(toTimePoint(end) -
                                                              toTimePoint(start));
    }

  } // namespace datetime
} // namespace util
