#pragma once

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <memory>
#include <sqlite/connection.hpp>
#include <sqlite/query.hpp>

#include <timer/timerlog.hpp>

namespace timer {
  class TimerDB {
  private:
    std::unique_ptr<sqlite::connection> m_con;

  public:
    typedef std::function<void(const TimerLog &)> TimerLogProcessor;

    // Constructors
    TimerDB(std::unique_ptr<sqlite::connection> con);
    TimerDB(TimerDB &&timerDB);
    TimerDB(const TimerDB &timerDB) = delete;

    // Timer log management
    void addTimerLog(const TimerLog &timerLog);
    TimerLog latestTimerLog();

    // Reporting functions
    void queryTimerLog(time_t start_datetime, time_t end_datetime, TimerLogProcessor processor);
    void queryTimerLog(TimerLogProcessor processor);

    // Factory method
    static TimerDB create(const boost::filesystem::path &timerDatabaseFilePath);
  };
} // namespace timer
