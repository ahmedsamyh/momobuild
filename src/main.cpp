#define STDCPP_IMPLEMENTATION
#include <functional>
#include <stdcpp.hpp>
#include <vector>

struct Subcmd {
  std::string name{};
  std::function<void()> func{nullptr};
};

int main(int argc, char *argv[]) {
  ARG();

  std::string program{arg.pop_arg()};

  // flags
  bool quiet{false};

  std::vector<Subcmd> subcommands = {
      {"help",
       [&]() {
         print("Usage: {} [flags] [config] [subcmd]\n", program);
         print("\nConfigs: \n"
               "    Debug (default)\n"
               "    Release\n"
               "    All\n"

               "\nFlags: \n"
               "    /Q    - Quite mode; do not output anything\n"
               "    /Rdst - Copies vcredist files to .\\redist\n"
               "    /h    - Same as the help subcommand.\n"
               "    /nb   - Do not build and run premake.\n"
               "    /ex   - If this flag is present, the argument after the "
               "run subcommand is treated as the executable_name to run.\n"

               "\nSubcommands: \n"
               "    help  - Displays how to use this script.\n"
               "    init  - Initializes the project folder. echo     reset - "
               "Resets the project folder.\n"
               "    run   - Runs the builded program.\n"
               "    srun  - Runs the builded program as a new process.\n"
               "    dir   - Opens the directory of the builded program.\n"
               "    clean - Cleans the left-over things from the last build.\n"
               "    sln   - Opens the .sln file of the project.\n");
       }},
      {"init", [&]() { UNIMPLEMENTED(); }},
      {"run", [&]() { UNIMPLEMENTED(); }},
      {"srun", [&]() { UNIMPLEMENTED(); }},
      {"dir", [&]() { UNIMPLEMENTED(); }},
      {"clean", [&]() { UNIMPLEMENTED(); }},
      {"sln", [&]() { UNIMPLEMENTED(); }},
  };

  // parse command line arguments
  do {
    std::string a{arg.pop_arg()};
    for (auto &subcmd : subcommands) {
      if (a == subcmd.name) {
        subcmd.func();
      }
    }

  } while (arg);

  return 0;
}
