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
}
