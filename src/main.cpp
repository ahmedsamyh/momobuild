#define STDCPP_IMPLEMENTATION
#include <functional>
#include <stdcpp.hpp>
#include <vector>
#include <filesystem>
#include <fstream>
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
  std::string og_dir{fs::current_path().string()};

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

  /* changes to the project root dir */
  auto change_to_root_dir = [&]() {
    while (!fs::exists(ROOT_IDENTIFIER) && !fs::current_path().stem().string().empty()) {
      if (!SetCurrentDirectoryA("..\\")){
        ERR("Could not change directory\n");
        exit(1);
      }
    }
    std::string root_dir{fs::current_path().string()};  
    if (fs::current_path().stem().string().empty()){
      ERR("Could not find ROOT_IDENTIFIER `{}`\n", ROOT_IDENTIFIER);
    }
  };

  auto confirmation = [&](const std::string question, bool _default=true){
    std::string response{"AAA"};
    str::tolower(response);
    auto valid = [&](const std::string& str){ return str=="" || str=="y" || str=="yes" || str=="n" || str=="no"; };
    
    do {
      if (!valid(response)){
	print("Please enter yes or no\n");
      }

      print("{} [yes/no]{{default: {}}}\n", question, _default  ? "yes" : "no");
      
      std::getline(std::cin, response);
    } while (!valid(response));
    return ((_default && response=="") || response=="y" || response=="yes") ? true : false;
  };

  std::vector<Subcmd> subcommands = {
      {"/Q",	[&]() { quiet = true; }},
      {"/Rdst", [&]() { UNIMPLEMENTED(); }},
      {"/h",    help},
      {"/nb",	[&]() { not_build = true; }},
      {"/ex",   [&]() { executable_name_provided = true; }},
      {"help",  help},
      {"init",	[&]() {
	if (!confirmation("This will create a project structure at the current dir, proceed?")) exit(0);
	if (!quiet) print(" + Initializing Project...\n");
	// create folders
	auto create_dir = [&](const std::string& dir) {
	  if (!fs::exists(dir)) {
	    fs::create_directory(dir);
	    if (!quiet) print("INFO: Created {}\\...\n", dir);
	  }
	};
	auto create_file = [&](const std::string& file, const std::string& content={}){
	  if (!fs::exists(file)) {
	    std::ofstream ofs;
	    ofs.open(file, std::ios::binary | std::ios::out);
	    if (!ofs.is_open()) {
	      ERR("Could not open file {} for writing.\n", file);
	    }
	    if (!content.empty()){
	      ofs.write((char*)content.c_str(), content.size());
	    }
	    ofs.close();
	    if (!quiet) print("Created {}...\n", file);
	  }
	};
	
	create_dir("src");
	create_dir("bin");
	create_dir("bin\\Release"); // TODO: Maybe have all Configs in an array and iterator over to create the folders?
	create_dir("bin\\Debug");
	create_dir("build");
	create_dir("lib");
	create_dir("include");

	// create files
	create_file("premake5.lua");
	create_file(".topdir");
	create_file(".gitignore");
	create_file("src\\main.cpp");
      }},
      {"run",	[&]() { UNIMPLEMENTED(); }},
      {"srun",	[&]() { UNIMPLEMENTED(); }},
      {"dir",	[&]() { UNIMPLEMENTED(); }},
      {"clean", [&]() { UNIMPLEMENTED(); }},
      {"sln",	[&]() { UNIMPLEMENTED(); }},
  };


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
