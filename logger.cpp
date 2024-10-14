#include "logger.h"

#include <iostream>
#include <string>

#include "boost/date_time/posix_time/ptime.hpp"
#include "boost/log/core/core.hpp"
#include "boost/log/detail/trivial_keyword.hpp"
#include "boost/log/keywords/format.hpp"
#include "boost/log/trivial.hpp"
#include "boost/log/utility/setup/common_attributes.hpp"
#include "boost/log/utility/setup/console.hpp"
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>

namespace logging = boost::log;

void
setupLogger(const unsigned level)
{
  auto logLevel = logging::trivial::info;
  if (level == 1) {
    logLevel = logging::trivial::debug;
  } else if (level >= 2) {
    logLevel = logging::trivial::trace;
  }

  boost::log::add_common_attributes();

  // clang-format off
  auto syslogFormat(
    logging::expressions::stream
    << "["
    << logging::expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S")
    << "] ["
    << std::left << std::setw(7) << std::setfill(' ')
    << logging::trivial::severity 
    << "] " << logging::expressions::smessage
    << " (" 
    << logging::expressions::attr<std::string>("Filename") 
    << ":"
    << logging::expressions::attr<int>("Line") 
    << ":"
    << logging::expressions::attr<std::string>("Function") << ")");

  logging::add_console_log(std::cout, logging::keywords::format = syslogFormat);
  logging::core::get()->set_filter
  (
      logging::trivial::severity >= logLevel
  );
  // clang-format on
}
