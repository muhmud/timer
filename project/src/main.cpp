
#include <timer/timercmd.hpp>

int main(int argc, char **argv) {
  return timer::TimerCommandLineExecutor::create().execute(argc, argv);
}
