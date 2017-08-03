# cis
(Mostly) **C**onstexpr **I**nterpolated **S**trings for c++14 inspired by es6 template literals

> It works quite well for our purposes, so why not open source it?

## What it does
Give cis a string literal, and it will parse it during compilation for patterns like `${<identifier>}`.
In return, it will give you an object that has a `::compile` method. Just pass it a map that specifies a value for each `<identifier>`
and you shall receive a string where all patterns have been replaced - lightning fast, because the template literal has already been parsed during compilation!

See it in action:
```c++
#include "cis.hpp"
#include <iostream>
int main() {
  const auto result = CIS(R"delim(
    This is a ${test}!
  )delim").compile({
    {"test", "test which passes"}
  });
  std::cout << result << "\n"; // Will output "This is a test which passes!"
  return 0;
}
```

## Limitations
* You can't have arbitrary c++ expressions inside `${...}`
* As much as I hate it, template literals have to be wrapped by a preprocessor macro (`CIS`)
* There is no support (yet) for having `::compile` `constexpr`, i.e. in case that all your interpolated variables are known during comilation,
  the final string still has to be concatenated during runtime
* Behavior on illegal input hasn't been tested at all yet. Error handling is a task kept for later

## About
Working on a [sibling project](github.com/luckyxxl/hfg-webcam-particles) of our webapp [hfg-particles](https://github.com/suluke/hfg-particles) 
required us to rewrite a lot of es6 code in c++. Since we relied heavily on [es6 template literals ](https://developer.mozilla.org/en/docs/Web/JavaScript/Reference/Template_literals)
in our web project, we found it desirable to have a way of making the same template strings usable in c++.
Unfortunately, c++ doesn't have any built-in features for string interpolation, and performing string replacement dynamically at runtime
seemed like unnecessary overhead when compared to manual string concatenation inside our code. 
Therefore I wondered whether it would be possible to write a template string preprocessor based on c++'s `constexpr`.
One long night and an inspirational morning later, `cis` was born.

## Similar Projects
* [tstrings](https://github.com/rayglover/tstrings): Similar goals, but no focus on compile-time template parsing
