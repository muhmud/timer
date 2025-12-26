#pragma once

// Enum Helpers
#define CHECK_STRING_TO_ENUM_VALUE(val, strval)                                                    \
  {                                                                                                \
    if (value == strval) {                                                                         \
      return Value::val;                                                                           \
    }                                                                                              \
  }
#define CHECK_STRING_TO_ENUM(val) CHECK_STRING_TO_ENUM_VALUE(val, #val)

#define CHECK_ENUM_TO_STRING(val)                                                                  \
  case Value::val:                                                                                 \
    return #val;
#define CHECK_ENUM_TO_VALUE(val, returnVal)                                                        \
  case Value::val:                                                                                 \
    return returnVal;
