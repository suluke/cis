#ifndef CIS_HPP
#define CIS_HPP
#pragma once
#include <array>
#include <experimental/string_view>
#include <map>
#include <sstream>
#include <string>

namespace cis {

/**
 * Constructs strings from many built-in types so to-string-conversion
 * can happen implicitly.
 */
class StringCast {
  using string = std::string;
  string storage;
  std::experimental::string_view str;

public:
  StringCast(string str)
      : storage(std::move(str)), str(storage.data(), storage.size()) {}
  template <size_t N>
  StringCast(const char (&L)[N]) : storage(), str(L, N - 1) {}
  template <typename T>
  StringCast(T t)
      : storage(std::to_string(t)), str(storage.data(), storage.size()) {}

  StringCast() = default;
  StringCast(const StringCast &) = default;
  StringCast(StringCast &&) = default;
  StringCast &operator=(const StringCast &) = default;
  StringCast &operator=(StringCast &&) = default;

  operator std::experimental::string_view const &() const { return str; }
};

std::ostream &operator<<(std::ostream &os, const StringCast &S) {
  return os << static_cast<std::experimental::string_view>(S);
}

/**
 * A template implementation which does almost all work during compile
 * time. Requires to know the template during compilation, of course.
 */
template <unsigned shard_count> class ConstexprTemplate {
  friend class TemplateLiteralParser;
  using str_view_t = std::experimental::string_view;
  using subst_t = std::map<str_view_t, StringCast>;
  using shards_t = std::array<str_view_t, shard_count>;
  using identifiers_t = std::array<str_view_t, shard_count - 1>;

  shards_t shards;
  identifiers_t identifiers;

public:
  constexpr ConstexprTemplate() = default;
  constexpr ConstexprTemplate(shards_t s, identifiers_t i)
      : shards(s), identifiers(i){};
  constexpr ConstexprTemplate(ConstexprTemplate &&) = default;
  constexpr ConstexprTemplate(const ConstexprTemplate &) = default;

  std::string compile(const subst_t &map) {
    std::stringstream builder;
    auto idIt = identifiers.begin();
    auto idEnd = identifiers.end();
    for (auto &shard : shards) {
      builder << shard;
      if (idIt != idEnd) {
        builder << map.at(*idIt);
        ++idIt;
      }
    }

    return builder.str();
  }
};

/**
 * A small wrapper around string literals which reduces the amount of
 * templating needed for parsing ConstexprTemplate by doing most of the
 * work inside constexpr functions.
 */
class TemplateLiteralParser {
  template <size_t shard_count>
  using shards_t = typename ConstexprTemplate<shard_count>::shards_t;
  template <size_t shard_count>
  using identifiers_t = typename ConstexprTemplate<shard_count>::identifiers_t;

  const char *const str;
  const size_t size;

  constexpr void checkInBounds(size_t i) const {
    if (i >= size)
      throw "Out of range";
  }
  constexpr const char *begin() const { return str; }
  constexpr const char *end() const { return str + size; }

public:
  template <class T> struct tag { using type = T; };
  template <typename L>
  constexpr TemplateLiteralParser(tag<L>) : str(L::str()), size(L::size()) {}

  constexpr size_t count_shards() const {
    size_t shards = 1;
    bool insidePlaceholder = false;
    char prev = '\0';
    for (char c : *this) {
      if (!insidePlaceholder && prev == '$' && c == '{') {
        insidePlaceholder = true;
        ++shards;
      } else if (insidePlaceholder && c == '}') {
        insidePlaceholder = false;
      }
      prev = c;
    }
    return shards;
  }

  template <size_t shard_count>
  constexpr ConstexprTemplate<shard_count> parse() const {
    auto shards = create_shards<shard_count>();
    auto ids = create_identifiers<shard_count>();
    auto T = ConstexprTemplate<shard_count>(shards, ids);
    return T;
  }

private:
  template <size_t shard_count> constexpr auto create_shards() const {
    using ret_t = shards_t<shard_count>;
    using str_view_t = typename ConstexprTemplate<shard_count>::str_view_t;
    ret_t result{};
    size_t idx = 0;
    size_t begin = 0;
    size_t pos = 0;
    bool insidePlaceholder = false;
    char prev = '\0';
    for (char c : *this) {
      if (!insidePlaceholder && prev == '$' && c == '{') {
        // https://stackoverflow.com/questions/34338241/filling-a-stdarray-at-compile-time-and-possible-undefined-behaviour-with-const/34341498#34341498
        const_cast<str_view_t &>(static_cast<const ret_t &>(result)[idx]) = {
            str + begin, pos - begin - 1};
        ++idx;
        insidePlaceholder = true;
      } else if (insidePlaceholder && c == '}') {
        begin = pos + 1;
        insidePlaceholder = false;
      }
      prev = c;
      ++pos;
    }
    const_cast<str_view_t &>(static_cast<const ret_t &>(result)[idx]) = {
        str + begin, pos - begin};
    ++idx;
    if (insidePlaceholder || idx != shard_count)
      throw "Placeholder not correctly closed";
    return result;
  }
  template <size_t shard_count> constexpr auto create_identifiers() const {
    using ret_t = identifiers_t<shard_count>;
    using str_view_t = typename ConstexprTemplate<shard_count>::str_view_t;
    ret_t result{};
    size_t idx = 0;
    size_t begin = 0;
    size_t pos = 0;
    bool insidePlaceholder = false;
    char prev = '\0';
    for (char c : *this) {
      if (!insidePlaceholder && prev == '$' && c == '{') {
        begin = pos + 1;
        insidePlaceholder = true;
      } else if (insidePlaceholder && c == '}') {
        // See above for const_cast explanation
        const_cast<str_view_t &>(static_cast<const ret_t &>(result)[idx]) = {
            str + begin, pos - begin};
        ++idx;
        insidePlaceholder = false;
      }
      prev = c;
      ++pos;
    }
    if (insidePlaceholder || idx != shard_count - 1)
      throw "Placeholder not correctly closed";
    return result;
  }
};
} // namespace cis

#define CIS(STR)                                                               \
  []() {                                                                       \
    struct L {                                                                 \
      constexpr static const char *str() { return STR; }                       \
      constexpr static size_t size() {                                         \
        constexpr auto str = STR;                                              \
        for (size_t i = 0;; ++i)                                               \
          if (str[i] == '\0')                                                  \
            return i;                                                          \
      }                                                                        \
    };                                                                         \
    using tag = cis::TemplateLiteralParser::tag<L>;                            \
    constexpr auto P = cis::TemplateLiteralParser{tag{}};                      \
    constexpr auto shard_count = P.count_shards();                             \
    constexpr auto T = P.parse<shard_count>();                                 \
    return T;                                                                  \
  }()

#endif // CIS_HPP
