#pragma once

#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/setup/console.hpp>

#if defined(LOG_LOC)
#error Conflict with LOG_LOC symbol
#endif

#define LOG_LOC(LEVEL)                                                         \
  BOOST_LOG_SEV(boost::log::trivial::logger::get(), LEVEL)                     \
    << boost::log::add_value("Line", __LINE__)                                 \
    << boost::log::add_value("File", __FILE__)                                 \
    << boost::log::add_value("Function", __FUNCTION__)

using ::boost::log::trivial::debug;
using ::boost::log::trivial::error;
using ::boost::log::trivial::fatal;
using ::boost::log::trivial::info;
using ::boost::log::trivial::trace;
using ::boost::log::trivial::warning;

void
setupLogger(const unsigned level);
