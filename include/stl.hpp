// include all of the standard stl files
//
// although it may be considered bad practice to "use" namespace items in a header file,
// these types are so intrinsic to c++ that it would be worse practice to copy these names
// I feel the readability this provides trumps the reasons to not
#pragma once
#include "stl_util.hpp"

#include <string>
using std::string;

#include <memory>
using std::unique_ptr;
using std::shared_ptr;
using std::make_shared;
using std_patch::make_unique;
using std_patch::to_string;

#include <vector>
using std::vector;

#include <functional>
using std::function;

// less common stuff, don't "use" them
#include <mutex>
#include <condition_variable>

#include <optional/optional.hpp>
using std::experimental::optional;
using std::experimental::nullopt;
using std::experimental::nullopt_t;

// noexcept was added to MSVC in Visua
#if defined(_MSC_VER) && _MSC_VER <= 1800
  #define noexcept throw()
#endif
#include <json11/json11.hpp>
#undef noexcept
