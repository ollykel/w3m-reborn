#ifndef __STD_DEPENDENCIES_HPP__
#define __STD_DEPENDENCIES_HPP__

// === Standard Dependencies ==============================================
//
// The headers included in this file are used in almost every file.
// Include this file by default for simplicity.
//
// ========================================================================

#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <cassert>
#include <limits>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>

#include "utils.hpp"
#include "string_exception.hpp"
#include "container.hpp"

using std::string;

template    <typename T>
using       u_ptr       = std::unique_ptr<T>;

template    <typename T>
using       s_ptr       = std::shared_ptr<T>;

#endif
