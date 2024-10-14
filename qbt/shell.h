#pragma once

// Shell! Thank you, Dr. Kim Buckner.
// RIP Flex and Bison :(

#include <boost/phoenix/core.hpp>
#include <boost/phoenix/operator.hpp>
#include <boost/phoenix/stl.hpp>
#include <boost/spirit/include/qi.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace Shell {

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

template<typename Iterator>
bool
parseNumbers(Iterator first, Iterator last, std::vector<double>& v)
{
  using ascii::space;
  using phoenix::push_back;
  using qi::_1;
  using qi::double_;
  using qi::phrase_parse;

  bool r = phrase_parse(first,
                        last,

                        //  Begin grammar
                        (double_[push_back(phoenix::ref(v), _1)] %
                         *(double_[push_back(phoenix::ref(v), _1)])),
                        //  End grammar

                        space);

  if (first != last) // fail if we did not get a full match
    return false;
  return r;
}

};
