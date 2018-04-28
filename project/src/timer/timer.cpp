
#include <timer/timer.hpp>
#include <timer/timerclock.hpp>
#include <util/datetime.hpp>

#include <boost/algorithm/string/trim.hpp>

namespace timer {
  class TimerState {
  private:
    TimerLog m_latestTimerLog;

  public:
    TimerState(TimerLog latestTimerLog) : m_latestTimerLog(latestTimerLog) {}

    void checkTransitionTo(TimerStatus::Value newTimerStatus) const {
      switch (newTimerStatus) {
      case TimerStatus::Value::RUNNING: {
        if (m_latestTimerLog.status == TimerStatus::Value::RUNNING) {
          throw std::runtime_error("timer is already running");
        }

        break;
      }
      case TimerStatus::Value::STOPPED: {
        if (m_latestTimerLog.status != TimerStatus::Value::RUNNING &&
            m_latestTimerLog.status != TimerStatus::Value::PAUSED) {
          throw std::runtime_error("timer is not running");
        }

        break;
      }
      case TimerStatus::Value::PAUSED: {
        if (m_latestTimerLog.status == TimerStatus::Value::PAUSED) {
          throw std::runtime_error("timer is already paused");
        }

        if (m_latestTimerLog.status != TimerStatus::Value::RUNNING) {
          throw std::runtime_error("timer is not running");
        }

        break;
      }
      case TimerStatus::Value::INVALID:
        throw std::runtime_error("invalid timer status transition");
      }
    }

    int64_t timerWorkDone(time_t end) const {
      return m_latestTimerLog.timer_work_done +
             util::datetime::duration(m_latestTimerLog.start_datetime, end).count();
    }
  };

  static std::chrono::seconds workDone(const TimerLog &latestTimerLog) {
    return std::chrono::seconds(latestTimerLog.timer_work_done) +
           (latestTimerLog.end_datetime == TimerClock::NULL_TIME
                ? util::datetime::duration(latestTimerLog.start_datetime, util::datetime::now())
                : std::chrono::seconds(0));
  }

  Timer::Timer(const boost::filesystem::path &timerDirectory, TimerDB timerDB)
      : m_timerDirectory(timerDirectory), m_timerDB(std::move(timerDB)) {}
  Timer::Timer(Timer &&timer) : m_timerDB(std::move(timer.m_timerDB)) {}

  void Timer::start(const std::string &task) {
    // Check if the state transition is valid
    TimerState const timerState(m_timerDB.latestTimerLog());
    timerState.checkTransitionTo(TimerStatus::Value::RUNNING);

    // Add a new log entry
    m_timerDB.addTimerLog(TimerLog(util::datetime::now(), 0, TimerStatus::Value::RUNNING,
                                   util::datetime::now(), TimerClock::NULL_TIME, task));
  }

  void Timer::pause() {
    // Check if the state transition is valid
    TimerLog const latestTimerLog = m_timerDB.latestTimerLog();
    TimerState const timerState(latestTimerLog);
    timerState.checkTransitionTo(TimerStatus::Value::PAUSED);

    // Record the time
    auto endDateTime = util::datetime::now();

    // Add a new log entry
    m_timerDB.addTimerLog(TimerLog(latestTimerLog.timer_datetime,
                                   timerState.timerWorkDone(endDateTime),
                                   TimerStatus::Value::PAUSED, latestTimerLog.start_datetime,
                                   endDateTime, latestTimerLog.task));
  }

  void Timer::resume(const std::string &task) {
    // Check if the state transition is valid
    TimerLog const latestTimerLog = m_timerDB.latestTimerLog();
    TimerState const timerState(latestTimerLog);
    timerState.checkTransitionTo(TimerStatus::Value::RUNNING);

    // Get and validate the task
    auto newTask = task;
    boost::algorithm::trim(newTask);
    if (newTask == "") {
      if (latestTimerLog.task == "") {
        throw std::runtime_error("invalid task");
      }

      newTask = latestTimerLog.task;
    }

    // Add a new log entry
    m_timerDB.addTimerLog(TimerLog(latestTimerLog.timer_datetime, latestTimerLog.timer_work_done,
                                   TimerStatus::Value::RUNNING, util::datetime::now(),
                                   TimerClock::NULL_TIME, newTask));
  }

  void Timer::stop() {
    // Check if the state transition is valid
    TimerLog const latestTimerLog = m_timerDB.latestTimerLog();
    TimerState const timerState(latestTimerLog);
    timerState.checkTransitionTo(TimerStatus::Value::STOPPED);

    // Record the time
    auto startDateTime = util::datetime::now(), endDateTime = startDateTime;

    // Add a new log entry
    m_timerDB.addTimerLog(TimerLog(
        latestTimerLog.timer_datetime,
        latestTimerLog.status == TimerStatus::Value::RUNNING ? timerState.timerWorkDone(endDateTime)
                                                             : latestTimerLog.timer_work_done,
        TimerStatus::Value::STOPPED,
        latestTimerLog.status == TimerStatus::Value::RUNNING ? latestTimerLog.start_datetime
                                                             : startDateTime,
        endDateTime, latestTimerLog.task));
  }

  void Timer::unstop() {
    // Check if the state transition is valid
    TimerLog const latestTimerLog = m_timerDB.latestTimerLog();

    if (latestTimerLog.status != TimerStatus::Value::STOPPED) {
      throw std::runtime_error("invalid timer status (timer must be stopped)");
    }

    // Remove the latest log
    m_timerDB.removeTimerLog(latestTimerLog.id);

    // If the previous entry was a paused one then there's nothing more to do;
    // otherwise, add a paused entry with the details from the stopped one
    TimerLog const newLatestTimerLog = m_timerDB.latestTimerLog();
    if (newLatestTimerLog.status != TimerStatus::Value::PAUSED) {
      m_timerDB.addTimerLog(TimerLog(latestTimerLog.timer_datetime, latestTimerLog.timer_work_done,
                                     TimerStatus::Value::PAUSED, latestTimerLog.start_datetime,
                                     latestTimerLog.end_datetime, latestTimerLog.task));
    }
  }

  Json::Value Timer::status() {
    TimerLog const latestTimerLog = m_timerDB.latestTimerLog();
    Json::Value root;

    root["project"] = m_timerDirectory.filename().c_str();
    root["directory"] = m_timerDirectory.c_str();

    root["timer"]["datetime"] = TimerClock::format(latestTimerLog.timer_datetime);
    root["timer"]["workDone"] = TimerClock::format(workDone(latestTimerLog));

    root["timer"]["current"]["task"] = latestTimerLog.task;
    root["timer"]["current"]["status"] = TimerStatus::toString(latestTimerLog.status);
    root["timer"]["current"]["start"] = TimerClock::format(latestTimerLog.start_datetime);
    if (latestTimerLog.end_datetime != TimerClock::NULL_TIME) {
      root["timer"]["current"]["end"] = TimerClock::format(latestTimerLog.end_datetime);
    }

    return root;
  }

  std::chrono::seconds Timer::totalWorkDone(time_t start_datetime, time_t end_datetime) {
    std::chrono::seconds workDone(0);

    // Go through all log entries that contain both a start and end time,
    // calculate the duration difference between the two and sum the values up
    m_timerDB.queryTimerLog(start_datetime, end_datetime, [&](const TimerLog &timerLog) {
      if (timerLog.start_datetime != TimerClock::NULL_TIME &&
          timerLog.end_datetime != TimerClock::NULL_TIME) {
        workDone += util::datetime::duration(timerLog.start_datetime, timerLog.end_datetime);
      }
    });

    return workDone;
  }

  std::unique_ptr<std::map<std::string, std::chrono::seconds>>
  Timer::totalWorkDoneByTask(time_t start_datetime, time_t end_datetime) {
    auto workDoneByTask = std::make_unique<std::map<std::string, std::chrono::seconds>>();

    // Go through all log entries that contain both a start and end time,
    // calculate the duration difference between the two and sum the values up
    m_timerDB.queryTimerLog(start_datetime, end_datetime, [&](const TimerLog &timerLog) {
      if (timerLog.start_datetime != TimerClock::NULL_TIME &&
          timerLog.end_datetime != TimerClock::NULL_TIME) {

        // Find the existing work done for the task, if any
        auto workDone = workDoneByTask->find(timerLog.task);
        if (workDone == workDoneByTask->end()) {
          // No work done was found: insert a value of second and make this
          // part of the map
          workDoneByTask->insert(std::make_pair(timerLog.task, std::chrono::seconds(0)));
          workDone = workDoneByTask->find(timerLog.task);
        }

        // Add the duration in this log
        workDone->second +=
            util::datetime::duration(timerLog.start_datetime, timerLog.end_datetime);
      }
    });

    return workDoneByTask;
  }

  std::unique_ptr<std::map<time_t, std::chrono::seconds>>
  Timer::totalWorkDoneByDate(time_t start_datetime, time_t end_datetime) {
    auto workDoneByDate = std::make_unique<std::map<time_t, std::chrono::seconds>>();

    // Go through all log entries that contain both a start and end time,
    // calculate the duration difference between the two and sum the values up
    m_timerDB.queryTimerLog(start_datetime, end_datetime, [&](const TimerLog &timerLog) {
      if (timerLog.start_datetime != TimerClock::NULL_TIME &&
          timerLog.end_datetime != TimerClock::NULL_TIME) {

        // Find the existing work done for the task, if any
        auto workDone = workDoneByDate->find(timerLog.timer_datetime);
        if (workDone == workDoneByDate->end()) {
          // No work done was found: insert a value of second and make this
          // part of the map
          workDoneByDate->insert(std::make_pair(timerLog.timer_datetime, std::chrono::seconds(0)));
          workDone = workDoneByDate->find(timerLog.timer_datetime);
        }

        // Add the duration in this log
        workDone->second +=
            util::datetime::duration(timerLog.start_datetime, timerLog.end_datetime);
      }
    });

    return workDoneByDate;
  }

  void Timer::processTimerLogs(time_t start_datetime, time_t end_datetime,
                               std::function<void(const TimerLog &)> processor) {
    m_timerDB.queryTimerLog(start_datetime, end_datetime, processor);
  }

  Timer Timer::create(const boost::filesystem::path &timerDatabaseFilePath) {
    return Timer(
        timerDatabaseFilePath,
        TimerDB::create(boost::filesystem::path(timerDatabaseFilePath).append(TIMER_FILE)));
  }

  Timer Timer::create() {
    // Try to find a timer file in the current directory and if not found,
    // then in it's parents. Keep going until we hit root, at which point
    // we will throw an error
    auto timerDatabasePath = boost::filesystem::current_path();
    do {
      auto timerDatabaseFilePath = timerDatabasePath;
      timerDatabaseFilePath.append(TIMER_FILE);

      if (boost::filesystem::exists(timerDatabaseFilePath)) {
        return create(timerDatabasePath);
      }

      timerDatabasePath = timerDatabasePath.parent_path();
    } while (!timerDatabasePath.empty());

    // No database file was found, so return the location of a non-existent
    // file in the current directory. This will cause a database to be
    // implicitly created
    return create(boost::filesystem::current_path());
  }
} // namespace timer
