
#include <timer/timerclock.hpp>
#include <timer/timerdb.hpp>

#include <sqlite/execute.hpp>

// SQL
#define INSERT_TIMER_LOG                                                                           \
  "insert into timer_log (timer_datetime, timer_work_done, status, "                               \
  "start_datetime, end_datetime, task) values (?, ?, ?, ?, ?, ?);"
#define DELETE_TIMER_LOG "delete from timer_log where id = ?"
#define SELECT_LATEST_TIMER_LOG                                                                    \
  "select id, timer_datetime, timer_work_done, status, start_datetime, "                           \
  "       end_datetime, task "                                                                     \
  "from latest_timer_log;"
#define SELECT_QUERY_TIMER_LOG                                                                     \
  "select id, timer_datetime, timer_work_done, status, start_datetime, "                           \
  "       end_datetime, task "                                                                     \
  "from timer_log where timer_datetime between ? and ? order by "                                  \
  "start_datetime;"

namespace timer {
  TimerLog toTimerLog(boost::shared_ptr<sqlite::result> result) {
    return TimerLog(result->get_int(0), result->get_int64(1), result->get_int64(2),
                    TimerStatus::toValue(result->get_string(3)), result->get_int64(4),
                    result->get_int64(5), result->get_string(6));
  }

  TimerDB::TimerDB(std::unique_ptr<sqlite::connection> con) : m_con(std::move(con)) {}
  TimerDB::TimerDB(TimerDB &&timerDB) : m_con(std::move(timerDB.m_con)) {}

  void TimerDB::addTimerLog(const TimerLog &timerLog) {
    // Create a prepared statement
    sqlite::execute insert(*m_con, INSERT_TIMER_LOG);

    // Set parameters
    insert % timerLog.timer_datetime % timerLog.timer_work_done %
        TimerStatus::toString(timerLog.status) % timerLog.start_datetime % timerLog.end_datetime %
        timerLog.task;

    // Execute
    insert();
  }

  void TimerDB::removeTimerLog(int id) {
    // Create a prepared statement
    sqlite::execute deleteSql(*m_con, DELETE_TIMER_LOG);

    // Set parameters
    deleteSql % id;

    // Execute
    deleteSql();
  }

  TimerLog TimerDB::latestTimerLog() {
    // Create a query
    sqlite::query latestTimerLogQuery(*m_con, SELECT_LATEST_TIMER_LOG);

    // Execute the query & get the results
    boost::shared_ptr<sqlite::result> result = latestTimerLogQuery.get_result();

    if (result->next_row()) {
      return toTimerLog(result);
    }

    return TimerLog();
  }

  void TimerDB::queryTimerLog(time_t start_datetime, time_t end_datetime,
                              TimerLogProcessor processor) {
    // Create a query
    sqlite::query timerLogQuery(*m_con, SELECT_QUERY_TIMER_LOG);

    // Set parameters
    timerLogQuery % start_datetime % end_datetime;

    // Execute the query & get the results
    boost::shared_ptr<sqlite::result> result = timerLogQuery.get_result();

    // Go through each result and pass it to the processor to perform
    // the required operations on the data
    while (result->next_row()) {
      processor(toTimerLog(result));
    }
  }

  void TimerDB::queryTimerLog(TimerLogProcessor processor) {
    queryTimerLog(TimerClock::MIN_TIME, TimerClock::MAX_TIME, processor);
  }

  TimerDB TimerDB::create(const boost::filesystem::path &timerDatabaseFilePath) {
    // Check if the database already exists. If it doesn't, we will initialise
    // it's structure with the required tables
    bool const databaseExisted = boost::filesystem::exists(timerDatabaseFilePath);

    // Create sqlite connection
    auto con = std::make_unique<sqlite::connection>(timerDatabaseFilePath.c_str());

    if (!databaseExisted) {
      // Initialise the database structure

      // Create the main timer log table
      sqlite::execute(*con,
                      "create table timer_log("
                      "  id integer primary key,"
                      "  timer_datetime big int,"
                      "  timer_work_done big int,"
                      "  status text,"
                      "  start_datetime big int,"
                      "  end_datetime big int,"
                      "  task text"
                      ");",
                      true);

      // Create index to improve common query performance
      sqlite::execute(*con, "create index ix on timer_log(start_datetime);", true);

      // Create a view for timer status
      sqlite::execute(*con,
                      "create view latest_timer_log as"
                      "  select id, timer_datetime, timer_work_done, status, "
                      "         start_datetime, end_datetime, task"
                      "  from timer_log"
                      "  indexed by ix"
                      "  order by start_datetime desc"
                      "  limit 1;",
                      true);
    }

    return TimerDB(std::move(con));
  }
} // namespace timer
