#pragma once

#include <iomanip>
#include <iosfwd>
#include <iostream>
#include <string>

#include "boost/json/value.hpp"
#include <boost/json.hpp>

namespace DebugTools {

void
json_pretty_print(std::ostream& os,
                  boost::json::value const& jv,
                  std::string* indent = nullptr);
}
