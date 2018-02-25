
#include <timer/timer.hpp>
#include <timer/timerclock.hpp>
#include <timer/timercmd.hpp>
#include <timer/version.hpp>
#include <util/cmdline.hpp>
#include <util/datetime.hpp>

#include <boost/date_time.hpp>
#include <boost/program_options.hpp>
#include <iomanip>
#include <iostream>

// Other constants
#define MAX_TASK_LENGTH 100

namespace timer {
  namespace options = boost::program_options;
  using namespace util::cmd;

  void TimerCommand::add(options::options_description_easy_init &builder) {
    for (const auto &command : values) {
      command.second.add(builder);
    }
  }

  std::map<TimerCommand::Value, util::cmd::CommandLineArg<TimerCommand::Value>> const
      TimerCommand::values = std::map<TimerCommand::Value, CommandLineArg<TimerCommand::Value>>{
          {Value::go,
           CommandLineArg<Value>(Value::go, "go", "g", "starts the timer for the specified task")},
          {Value::pause, CommandLineArg<Value>(Value::pause, "pause", "p", "pauses the timer")},
          {Value::resume,
           CommandLineArg<Value>(Value::resume, "resume", "r",
                                 "resumes the timer with the last used task or a new task")},
          {Value::stop, CommandLineArg<Value>(Value::stop, "stop", "s", "stops the timer")},
          {Value::report,
           CommandLineArg<Value>(Value::report, "report", "e", options::value<std::string>(),
                                 "generates a report specified by name [date, task]")},
          {Value::status,
           CommandLineArg<Value>(Value::status, "status", "u", "returns the status of the timer")}};

  void TimerParameter::add(options::options_description_easy_init &builder) {
    for (const auto &command : values) {
      command.second.add(builder);
    }
  }

  util::cmd::CommandLineArgValue TimerParameter::arg(Value value,
                                                     const options::variables_map &vm) {
    return values.find(value)->second.arg(vm);
  }

  std::map<TimerParameter::Value, CommandLineArg<TimerParameter::Value>> const
      TimerParameter::values = std::map<Value, CommandLineArg<Value>>{
          {Value::timerDirectory,
           CommandLineArg<Value>(Value::timerDirectory, "timer-directory", "D",
                                 options::value<std::string>(), "project directory")},
          {Value::task,
           CommandLineArg<Value>(Value::task, "task", "T", options::value<std::string>(),
                                 "name of the task to be started/resumed")},
          {Value::startDate,
           CommandLineArg<Value>(Value::startDate, "start-date", "S", options::value<std::string>(),
                                 "start date to be used for a report")},
          {Value::endDate,
           CommandLineArg<Value>(Value::endDate, "end-date", "E", options::value<std::string>(),
                                 "end date to be used for a report")},
          {Value::month,
           CommandLineArg<Value>(Value::month, "month", "M", options::value<std::string>(),
                                 "month/year to be used for a report")}};

  TimerCommandLineExecutor::TimerCommandLineExecutor(options::options_description cmd)
      : m_cmd(cmd) {}

  int TimerCommandLineExecutor::execute(int argc, char **argv) {
    try {
      // Parse the command line
      options::variables_map vm;
      options::store(options::parse_command_line(argc, argv, m_cmd), vm);

      // Check for conflicting options
      util::cmd::CommandLineArg<TimerCommand::Value> timerCommand;
      for (const auto &command : TimerCommand::values) {
        bool const thisCommandFound = vm.count(command.second.name);
        if (thisCommandFound) {
          if (timerCommand.id != TimerCommand::Value::invalid) {
            throw std::logic_error("only one command can be specified");
          }

          timerCommand = command.second;
        }
      }

      // Ensure that a command has been specified
      if (timerCommand.id == TimerCommand::Value::invalid) {
        std::cout << "timer (v" << VERSION << ") - time work that you do\n\n";
        std::cout << m_cmd << '\n';
        return -1;
      }

      // Create the timer
      const auto timerDirectoryArg = TimerParameter::arg(TimerParameter::Value::timerDirectory, vm);
      auto timer = timerDirectoryArg.exists() ? Timer::create(timerDirectoryArg.asNonEmptyString())
                                              : Timer::create();

      // Create common parameters
      const auto taskArg = TimerParameter::arg(TimerParameter::Value::task, vm);

      // Execute the command
      switch (timerCommand.id) {
      case TimerCommand::Value::go: {
        // Start the timer
        timer.start(taskArg.asNonEmptyString());
        break;
      }

      case TimerCommand::Value::pause: {
        // Pause the timer
        timer.pause();
        break;
      }

      case TimerCommand::Value::resume: {
        // Resume the timer
        timer.resume(taskArg.value<std::string>());
        break;
      }

      case TimerCommand::Value::stop: {
        // Stops the timer
        timer.stop();
        break;
      }

      case TimerCommand::Value::report: {
        // Get the name of the report and verify that it is valid
        const auto report = timerCommand.arg(vm).asNonEmptyString();
        if (report != "date" && report != "task") {
          throw std::logic_error("invalid report (should one of [date, task])");
        }

        // Create arg objects for the all date range parameters
        auto const startDateArg = TimerParameter::arg(TimerParameter::Value::startDate, vm);
        auto const endDateArg = TimerParameter::arg(TimerParameter::Value::endDate, vm);
        auto const monthArg = TimerParameter::arg(TimerParameter::Value::month, vm);

        // Check for existence of these various arguments
        bool const useDates = startDateArg.exists() || endDateArg.exists();
        bool const useMonthYear = monthArg.exists();

        // Check that only one pair has been provided
        if (useDates && useMonthYear) {
          std::cout << "use either start/end dates or month/year\n";
          return -1;
        }

        // Process the date parameters
        const auto start = startDateArg.exists()
                               ? TimerClock::parseDate(startDateArg.asNonEmptyString())
                               : TimerClock::parseMonthYear(monthArg.value<std::string>());
        const auto end = endDateArg.exists() ? TimerClock::parseDate(endDateArg.asNonEmptyString())
                                             : TimerClock::endOfMonth(start);

        // Produce the report
        if (report == "date") {
          const auto data = *timer.totalWorkDoneByDate(start, end);
          for (const auto & [ date, value ] : data) {
            std::cout << util::datetime::format(date, "%Y-%m-%d (%H:%M:%S %z)") << '\t'
                      << TimerClock::format(value) << " (" << std::fixed << std::setprecision(2)
                      << TimerClock::toHours(value) << ")" << '\n';
          }
        } else {
          const auto data = *timer.totalWorkDoneByTask(start, end);
          for (const auto & [ task, value ] : data) {
            std::cout << (task.length() > MAX_TASK_LENGTH
                              ? task.substr(0, MAX_TASK_LENGTH - 4) + " ..."
                              : task + std::string(MAX_TASK_LENGTH - task.length(), ' '))
                      << "\t" << TimerClock::format(value) << " (" << std::fixed
                      << std::setprecision(2) << TimerClock::toHours(value) << ")" << '\n';
          }
        }

        break;
      }

      case TimerCommand::Value::status: {
        std::cout << timer.status() << '\n';
        break;
      }

      default:
        throw std::logic_error("invalid timer command");
      }
    } catch (const std::exception &e) {
      std::cout << e.what() << '\n';

      return -1;
    }

    return 0;
  }

  TimerCommandLineExecutor TimerCommandLineExecutor::create() {
    using namespace timer;

    options::options_description cmd("options", 100);
    auto builder = cmd.add_options();

    // Add timer commands
    TimerCommand::add(builder);

    // Add command arguments
    TimerParameter::add(builder);

    // The help command
    builder("help,h", "print usage message");

    return TimerCommandLineExecutor(cmd);
  }
} // namespace timer
