#include "debug.h"

#include "boost/json/kind.hpp"
#include "boost/json/serialize.hpp"
#include "boost/json/value.hpp"

#include <iosfwd>
#include <string>

namespace json = boost::json;

// Stolen from:
//   https://www.boost.org/doc/libs/1_86_0/libs/json/doc/html/json/examples.html

void
DebugTools::json_pretty_print(std::ostream& os,
                              json::value const& jv,
                              std::string* indent)
{
  std::string indent_;
  if (!indent)
    indent = &indent_;
  switch (jv.kind()) {
    case json::kind::object: {
      os << "{\n";
      indent->append(4, ' ');
      auto const& obj = jv.get_object();
      if (!obj.empty()) {
        auto it = obj.begin();
        for (;;) {
          os << *indent << json::serialize(it->key()) << " : ";
          json_pretty_print(os, it->value(), indent);
          if (++it == obj.end())
            break;
          os << ",\n";
        }
      }
      os << "\n";
      indent->resize(indent->size() - 4);
      os << *indent << "}";
      break;
    }

    case json::kind::array: {
      os << "[\n";
      indent->append(4, ' ');
      auto const& arr = jv.get_array();
      if (!arr.empty()) {
        auto it = arr.begin();
        for (;;) {
          os << *indent;
          json_pretty_print(os, *it, indent);
          if (++it == arr.end())
            break;
          os << ",\n";
        }
      }
      os << "\n";
      indent->resize(indent->size() - 4);
      os << *indent << "]";
      break;
    }

    case json::kind::string: {
      os << json::serialize(jv.get_string());
      break;
    }

    case json::kind::uint64:
      os << jv.get_uint64();
      break;

    case json::kind::int64:
      os << jv.get_int64();
      break;

    case json::kind::double_:
      os << jv.get_double();
      break;

    case json::kind::bool_:
      if (jv.get_bool())
        os << "true";
      else
        os << "false";
      break;

    case json::kind::null:
      os << "null";
      break;
  }

  if (indent->empty())
    os << "\n";
}
