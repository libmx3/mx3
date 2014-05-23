// all of the standard c++11 types included
#pragma once
#include "stl_util.hpp"

#include <string>
using std::string;

#include <memory>
using std::unique_ptr;
using std::shared_ptr;
using std::make_shared;
using std::make_unique;

#include <vector>
using std::vector;

#include <functional>
using std::function;

// less common stuff, don't "use" them
#include <mutex>
#include <condition_variable>
