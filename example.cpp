#include "cis.hpp"
#include <iostream>
int main() {
  const auto result = CIS(R"delim(
    This is a ${test}!
  )delim").compile({
    {"test", "test which passes"}
  });
  std::cout << result << "\n";
  return 0;
}
