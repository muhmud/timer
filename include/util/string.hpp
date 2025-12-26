#pragma once

#define FORMAT_STRING(...)                                                                         \
  [&]() -> std::string {                                                                           \
    std::stringstream s;                                                                           \
    s << __VA_ARGS__;                                                                              \
    return s.str();                                                                                \
  }()
