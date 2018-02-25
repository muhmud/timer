#pragma once

#include <boost/algorithm/string/trim.hpp>
#include <boost/program_options.hpp>
#include <util/string.hpp>

namespace util {
  namespace cmd {
    namespace options = boost::program_options;

    class CommandLineArgValue {
    private:
      const std::string *m_name;
      const options::variables_map *m_vm;

    public:
      // Constructor
      CommandLineArgValue(const std::string *name, const options::variables_map *vm);

      // Checks whether the arg value exists in the command line
      bool exists() const;

      // Functions to retrieve the value of the argument
      template <typename T> T value() const { return exists() ? (*m_vm)[*m_name].as<T>() : T(); }

      std::string asNonEmptyString() const;
    };

    template <typename Enum> struct CommandLineArg {
      // Properties
      Enum id;
      std::string name;
      std::string shortName;
      options::value_semantic *value;
      std::string description;

      // Constructors
      CommandLineArg(Enum id, std::string name, std::string shortName,
                     options::value_semantic *value, std::string description)
          : id(id), name(name), shortName(shortName), value(value), description(description) {}
      CommandLineArg(Enum id, std::string name, std::string option, std::string description)
          : CommandLineArg(id, name, option, nullptr, description) {}
      CommandLineArg() : id(Enum::invalid) {}

      // Adds the option to the supplied options builder
      void add(options::options_description_easy_init &builder) const {
        const std::string optionName = FORMAT_STRING(name << "," << shortName);

        if (value) {
          builder(optionName.c_str(), value, description.c_str());
        } else {
          builder(optionName.c_str(), description.c_str());
        }
      }

      // Create a command line arg value object for the option
      CommandLineArgValue arg(const options::variables_map &vm) const {
        return CommandLineArgValue(&name, &vm);
      }
    };
  } // namespace cmd
} // namespace util
