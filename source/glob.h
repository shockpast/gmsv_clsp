#pragma once

#include <string>

namespace clsp {
  namespace glob {
    bool Match(const std::string& pattern, const std::string& value);
  } // namespace glob
} // namespace clsp