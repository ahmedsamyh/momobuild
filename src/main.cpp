#define STDCPP_IMPLEMENTATION
#define USE_WIN32
#include <functional>
#include <stdcpp.hpp>
#include <vector>
#include <filesystem>
#include <fstream>
#include <shellapi.h>
namespace fs = std::filesystem;

struct Subcmd {
  bool handled{false};
  std::string name{};
  std::function<void()> func{nullptr};

  bool handle(const std::string& a){
    if (!handled && a==name && func) {
      func();
      handled = true;
      return true;
    }
    return false;
  }
};

#define ROOT_IDENTIFIER ".topdir"
#define MSBUILD_PATH "D:\\bin\\Microsoft Visual Studio\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe"
#define PREMAKE5_PATH "D:\\bin\\premake 5.0 beta2\\premake5.exe"
#define VCREDIST_PATH "D:\\bin\\Microsoft Visual Studio\\Community\\VC\\Redist\\MSVC\\14.36.32532\\"
#define VCREDIST_EXE "vc_redist."

int main(int argc, char *argv[]) {
  ARG();
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  std::string program{arg.pop_arg()};

  // flags
  bool quiet{false};
  bool not_build{false};
  bool will_copy_redist{false};
  bool open_sln{false};
  bool executable_name_provided{false};
  std::string executable_name{};
  bool will_run{false};
  bool will_srun{false};
  std::string og_dir{fs::current_path().string()};
  std::string project_name{};
  std::string config{""};
  std::vector<std::string> valid_configs = {"Debug", "Release", "All"};
  std::string executable_args{};

  auto help = [&](){							
    print("Usage: {} [flags] [config] [subcmd] {{executable_args...}}\n", program);
    print("\nConfigs: \n"
          "    Debug (default)\n"
          "    Release\n"
	  "    All\n"	 
	  
	  "\nFlags: \n"
	  "    /Q    - Quite mode; do not output anything\n"	
	  "    /Rdst - Copies vcredist files to .\\redist\n"	
	  "    /h,/? - Same as the help subcommand.\n"		
	  "    /nb   - Do not build and run .\n"		
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
    exit(0);
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

  /* asks for confirmation [y/n] */
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

  // get the project name from the `.sln` file in `.\build`
  auto get_project_name = [&]() {
    SetCurrentDirectoryA("build");
      WIN32_FIND_DATAA file_data{};
      if (FindFirstFileA("*.sln", &file_data) == INVALID_HANDLE_VALUE){
        ERR("Could not find .sln file in `build\\`\n\nNOTE: Please build the project first\n");
      }
      project_name = file_data.cFileName;
      // remove `.sln` from the filename
      for (size_t i = 0; i < 4; ++i) project_name.pop_back();
    SetCurrentDirectoryA("..\\");
  };

  std::vector<Subcmd> subcommands = {
    {false, "/Q",	[&]() { quiet = true; }},
    {false, "/Rdst", [&]() {
      will_copy_redist=true;
    }},
    {false, "/h",    help},
    {false, "/?",    help},
    {false, "/nb",	[&]() { not_build = true; }},
    {false, "/ex",   [&]() { executable_name_provided = true; }},
    {false, "help",  help},
    {false, "init",	[&]() {
      if (!confirmation("This will create a project structure at the current dir, proceed?")) exit(0);
      if (!quiet) print("{}: Initializing Project...\n", "momobuild");
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
          if (!quiet) print("INFO: Created {}...\n", file);
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
      
      exit(0);
    }},
    {false, "run",	[&]() {
      if (will_srun) ERR("`srun` and `run` cannot be called simultaneously\n");
      will_run=true;
    }},
    {false, "srun",	[&]() {
      if (will_run) ERR("`srun` and `run` cannot be called simultaneously\n");
      will_srun=true;
    }},
    {false, "dir",	[&]() {
      change_to_root_dir();
      if (config == "All") config="Debug";
      open_dir(FMT("bin\\{}\\", (config.empty() ? "Debug" : config)));
    }},
    {false, "clean",	[&]() { UNIMPLEMENTED(); }},
    {false, "sln",	[&]() {
      open_sln=true;
    }},
  };

  auto run_msbuild = [&](std::string config="Debug") {
    if (!quiet) print("\n{}: Running MSBuild [{}]...\n", "momobuild", config);
    if (config=="All") {
      wait_and_close_process(run_process(MSBUILD_PATH, FMT("-p:configuration={} build\\{}.sln -v:m -m", "Debug", project_name)));
      wait_and_close_process(run_process(MSBUILD_PATH, FMT("-p:configuration={} build\\{}.sln -v:m -m", "Release", project_name)));
    } else {
      auto msbuild = run_process(MSBUILD_PATH, FMT("-p:configuration={} build\\{}.sln -v:m -m", config, project_name));
      wait_and_close_process(msbuild);
    }
  };
  
  auto run_premake = [&]() {
    if (!quiet) print("\n{}: Running Premake5...\n", "momobuild");
    auto premake = run_process(PREMAKE5_PATH, "vs2022", quiet);
    wait_and_close_process(premake);
  };

  auto run = [&](){
    if (!quiet) {
      print("\n{}: Running {}.exe[{}]...\n", "momobuild", (!executable_name.empty() ? executable_name : project_name), config);
      print("--------------------------------------------------\n");
    }
    auto child = run_process(FMT("bin\\{}\\{}.exe", config, (!executable_name.empty() ? executable_name : project_name)), executable_args);
    wait_and_close_process(child);
  };

  auto srun = [&](){
    if (!quiet) {
      print("\n{}: Running {}.exe[{}] as a new process...\n", "momobuild", (!executable_name.empty() ? executable_name : project_name), config);
      print("--------------------------------------------------\n");
    }
    auto child = run_process(FMT("bin\\{}\\{}.exe", config, (!executable_name.empty() ? executable_name : project_name)), executable_args, false, true);
    wait_and_close_process(child);
  };

  // parse command line arguments
  while (arg) {
    std::string a{arg.pop_arg()};
    bool matched{false};
    for (auto &subcmd : subcommands) {
      if (subcmd.handle(a)) {
    	matched = true;
      }
    }
    if (!matched){
      if (config.empty()){
        config = a;
	// check if the config is valid
	bool valid{false};
	for (auto& c : valid_configs) {
	  if (config == c){
	    valid = true;
	    break;
	  }
	}
	if (!valid) {
	  ERR("`{}` is not a valid config!\n", config);
	}
      } else if (executable_name.empty() && executable_name_provided && (will_run || will_srun)) {
	executable_name = a;
      } else {
	executable_args += (!executable_args.empty() ? " " : "");
	executable_args += a;
      }
    }
  };

  change_to_root_dir();

  if (config.empty()) {
    config="Debug";
    if (will_run || will_srun)
      not_build=true;
  }

  if (will_copy_redist){
    get_project_name();
    if (!fs::exists("redist")) {
      fs::create_directory("redist");
      if (!quiet) print("INFO: Created {}\\...\n", "redist");
    }
    fs::copy(FMT("{}{}{}", VCREDIST_PATH, VCREDIST_EXE, "x64.exe"), "redist\\", fs::copy_options::update_existing | fs::copy_options::recursive);
    if (!quiet) print("INFO: Copied {}{}...\n", VCREDIST_EXE, "x64.exe");
    fs::copy(FMT("{}{}{}", VCREDIST_PATH, VCREDIST_EXE, "x86.exe"), "redist\\", fs::copy_options::update_existing | fs::copy_options::recursive);
    if (!quiet) print("INFO: Copied {}{}...\n", VCREDIST_EXE, "x86.exe");

    exit(0);
  }

  if (open_sln) {
    get_project_name();
    if (!quiet) print("{}: Opening {}.sln...\n", "momobuild", project_name);
    open_file(FMT("build\\{}.sln", project_name));
    exit(0);
  }

  
  if (!not_build){
    run_premake();
    ASSERT(fs::exists("build"));
    get_project_name();
    run_msbuild(config);
  }

  if (will_run){
    get_project_name();
    run();
  } else if (will_srun){
    get_project_name();
    srun();
  }
  
  return 0;
}
