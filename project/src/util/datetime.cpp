
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <util/datetime.hpp>

namespace util {
  namespace datetime {
    // boost::date_time::c_local_adjustor uses the C-API to adjust a
    // moment given in utc to the same moment in the local time zone.
    typedef boost::date_time::c_local_adjustor<boost::posix_time::ptime> local_adj;

    boost::posix_time::time_duration getUtcOffset() {
      using namespace boost::posix_time;

      const ptime utc_now = second_clock::universal_time();
      const ptime now = local_adj::utc_to_local(utc_now);

      return now - utc_now;
    }

    std::locale toLocale(const std::string &format) {
      return std::locale(std::locale::classic(), new boost::posix_time::time_input_facet(format));
    }

    boost::posix_time::ptime parseLocal(const std::string &value, const std::locale &locale) {
      // Parse the date/time
      boost::posix_time::ptime result;
      std::istringstream input(value);
      input.imbue(locale);
      input >> result;

      return result - getUtcOffset();
    }

    boost::posix_time::ptime parseLocal(const std::string &value, const std::string &format) {
      return parseLocal(value, toLocale(format));
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

    time_t addDays(const time_t date, const int days) {
      return date + ((days < 0 ? 0 : days) * (60 * 60 * 24));
    }
  } // namespace datetime
} // namespace util
