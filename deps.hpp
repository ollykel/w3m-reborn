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
#include <utility>
#include <stdexcept>
#include <exception>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>

#include "string_exception.hpp"
#include "container.hpp"

using std::string;
using std::wstring;

template    <typename T>
using       u_ptr       = std::unique_ptr<T>;

template    <typename T>
using       s_ptr       = std::shared_ptr<T>;

#endif
