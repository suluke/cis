#include <cassert>
#include "cis.hpp"

int main() {
  auto res = CIS(R"delim(
    Print the ${var} of var
  )delim").compile({
    {"var", "value"}
  });
  assert(res == R"delim(
    Print the value of var
  )delim");

  res = CIS("I like ${PI}").compile({
    {"PI", 3.141f}
  });
  assert(res == "I like " + std::to_string(3.141f));
}
