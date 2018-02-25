#pragma once

#include <boost/filesystem.hpp>
#include <chrono>
#include <json/json.h>
#include <timer/timerdb.hpp>

#define TIMER_FILE ".timer.db"

namespace timer {
  class Timer {
  private:
    boost::filesystem::path m_timerDirectory;
    TimerDB m_timerDB;

  public:
    // Constructors
    Timer(const boost::filesystem::path &timerDirectory, TimerDB timerDB);
    Timer(Timer &&timer);
    Timer(const Timer &timer) = delete;

    // Timer control functions
    void start(const std::string &task);
    void pause();
    void resume(const std::string &task);
    void stop();

    // Returns the current status of the timer as JSON
    Json::Value status();

    // Reporting
    std::chrono::seconds totalWorkDone(time_t start_datetime, time_t end_datetime);

    std::unique_ptr<std::map<std::string, std::chrono::seconds>>
    totalWorkDoneByTask(time_t start_datetime, time_t end_datetime);

    std::unique_ptr<std::map<time_t, std::chrono::seconds>>
    totalWorkDoneByDate(time_t start_datetime, time_t end_datetime);

    // Factory methods
    static Timer create(const boost::filesystem::path &timerDatabaseFilePath);

    // This method will search for a timer database file in the current directory.
    // If not found, it will confinue the search through the parents. If still not
    // found, it will create a new file in the current directory
    static Timer create();
  };
} // namespace timer
