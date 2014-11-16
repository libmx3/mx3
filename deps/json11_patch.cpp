#ifdef _MSC_VER
    // MSVC has funny names for snprintf, we just use it instead
    #define snprintf(str, size, format, ...) _snprintf_s(str, size, _TRUNCATE, format, __VA_ARGS__)
#endif
#include "json11/json11.cpp"
