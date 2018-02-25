#pragma once

#include <boost/program_options.hpp>
#include <util/cmdline.hpp>

namespace timer {
  namespace options = boost::program_options;

  struct TimerCommand {
    // Enum containing timer commands
    enum class Value { invalid, go, pause, resume, stop, report, status };

    // Map of timer commands to CommandLineArg objects
    static std::map<Value, util::cmd::CommandLineArg<Value>> const values;

    // Helper method to add CommandLineArg objects to boost program options
    static void add(options::options_description_easy_init &builder);
  };

  struct TimerParameter {
    // Enum containing timer parameters
    enum class Value { invalid, timerDirectory, task, startDate, endDate, month };

    // Map  of timer parameter to CommandLineArg objects
    static std::map<Value, util::cmd::CommandLineArg<Value>> const values;

    // Helper method to add CommandLineArg objects to boost program options
    static void add(options::options_description_easy_init &builder);

    // Returns a helper object for easy access to command line parameter values
    static util::cmd::CommandLineArgValue arg(Value value, const options::variables_map &vm);
  };

  class TimerCommandLineExecutor {
  private:
    options::options_description m_cmd;

    // Constructor (use the factory method)
    TimerCommandLineExecutor(options::options_description cmd);

  public:
    // Execute the application using command line arguments
    int execute(int argc, char **argv);

    // Factory method for creating a command line executor
    static TimerCommandLineExecutor create();
  };
} // namespace timer
