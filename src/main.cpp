#define STDCPP_IMPLEMENTATION
#include <functional>
#include <stdcpp.hpp>
#include <vector>
#include <filesystem>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
namespace fs = std::filesystem;

struct Subcmd {
  std::string name{};
  std::function<void()> func{nullptr};
};

#define ROOT_IDENTIFIER ".topdir"

int main(int argc, char *argv[]) {
  ARG();

  std::string program{arg.pop_arg()};

  // flags
  bool quiet{false};
  bool not_build{false};
  bool executable_name_provided{false};

  auto help = [&](){							
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
  };

  std::vector<Subcmd> subcommands = {
      {"/Q",	[&]() { quiet = true; }},
      {"/Rdst", [&]() { UNIMPLEMENTED(); }},
      {"/h",    help},
      {"/nb",	[&]() { not_build = true; }},
      {"/ex",   [&]() { executable_name_provided = true; }},
      {"help",  help},
      {"init",	[&]() { UNIMPLEMENTED(); }},
      {"run",	[&]() { UNIMPLEMENTED(); }},
      {"srun",	[&]() { UNIMPLEMENTED(); }},
      {"dir",	[&]() { UNIMPLEMENTED(); }},
      {"clean", [&]() { UNIMPLEMENTED(); }},
      {"sln",	[&]() { UNIMPLEMENTED(); }},
  };


  /* changes to the project root dir */
  while (!fs::exists(ROOT_IDENTIFIER) && !fs::current_path().stem().string().empty()) {
    if (!SetCurrentDirectoryA("..\\")){
      ERR("Could not change directory");
      return 0;
    }
  }

  if (fs::current_path().stem().string().empty()){
    ERR("Could not find {}", ROOT_IDENTIFIER);
  }

  // parse command line arguments
  if (arg){
    do {
      std::string a{arg.pop_arg()};
      bool matched{false};
      for (auto &subcmd : subcommands) {
        if (a == subcmd.name) {
    	matched = true;
          subcmd.func();
        }
      }
      if (!matched){
        ERR("Invalid subcommand or flag `{}`", a);
      }
    } while (arg);
  }
  return 0;
}
