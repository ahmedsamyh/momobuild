#ifndef _STDCPP_H_
#define _STDCPP_H_

#include <format>
#include <iostream>
#include <string>

#define VAR(name) print("{}: {}\n", #name, name)
#define VAR_STR(name) std::format("{}: {}", #name, name)
#define NL() print("\n")
#define ASSERT(condition)                                                      \
  if (!(condition)) {                                                          \
    PANIC(#condition);                                                         \
  }
#define ASSERT_MSG(condition, msg)                                             \
  if (!(condition)) {                                                          \
    PANIC(msg);                                                                \
  }
#define ERROR(...) PANIC("ERROR: ", __VA_ARGS__)
#define FMT(str, ...) std::format((str), __VA_ARGS__)
#define PANIC(...) panic(__FILE__, ":", __LINE__, ":", __VA_ARGS__)
void panic();
template <typename T, typename... Types> void panic(T arg, Types... args) {
  std::cerr << arg;
  panic(args...);
}
#define LOG(...) log(__VA_ARGS__)
void log();
template <typename T, typename... Types> void log(T arg, Types... args) {
  std::cout << arg;
  log(args...);
}
#define UNREACHABLE() PANIC("Uncreachable\n")
#define UNIMPLEMENTED() PANIC(__func__, "() is unimplemented\n")
#define WARNING(...) LOG("WARNING: ", __VA_ARGS__)
#define FMT(str, ...) std::format((str), __VA_ARGS__)
#define fprint(file, str, ...) __print((file), FMT((str), __VA_ARGS__))
#define print(str, ...) fprint(std::cout, str, __VA_ARGS__)

void __print(std::ostream &file);
template <typename T, typename... Types>
void __print(std::ostream &file, T arg, Types... args) {
  file << arg;
  __print(file, args...);
}

#define ARG() Arg arg(argc, argv)

struct Arg {
  int *argc{nullptr};
  char ***argv{nullptr};

  Arg(int &_argc, char **&_argv);

  bool has_arg() const;
  operator bool();
  bool operator!();
  std::string pop_arg();
};

#endif /* _STDCPP_H_ */

//////////////////////////////////////////////////
#if defined STDCPP_IMPLEMENTATION || STDCPP_IMPL

void __print(std::ostream &file){};

void log(){};

void panic() { exit(1); };

// Arg --------------------------------------------------
Arg::Arg(int &_argc, char **&_argv) {
  argc = &_argc;
  argv = &_argv;
}

bool Arg::has_arg() const { return (*argc) > 0; }

Arg::operator bool() { return has_arg(); }
bool Arg::operator!() { return !has_arg(); }

std::string Arg::pop_arg() {
  if (!has_arg()) { // return empty string if no args are available
    return {};
  };

  std::string arg = *argv[0];

  *argc = *argc - 1;
  *argv = *argv + 1;
  return arg;
}

#endif
