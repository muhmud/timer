
#include <boost/algorithm/string/trim.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <util/cmdline.hpp>

namespace util {
  namespace cmd {
    namespace options = boost::program_options;

    CommandLineArgValue::CommandLineArgValue(const std::string *name,
                                             const options::variables_map *vm)
        : m_name(name), m_vm(vm) {}

    bool CommandLineArgValue::exists() const { return m_vm->count(*m_name); }

    std::string CommandLineArgValue::asNonEmptyString() const {
      auto result = value<std::string>();
      boost::algorithm::trim(result);

      if (result == "") {
        throw std::logic_error(FORMAT_STRING("invalid " << *m_name));
      }

      return result;
    }
  } // namespace cmd
} // namespace util
