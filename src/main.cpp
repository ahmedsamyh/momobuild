#define STDCPP_IMPLEMENTATION
#define USE_WIN32
#include <functional>
#include <stdcpp.hpp>
#include <vector>
#include <filesystem>
#include <fstream>
#include <shellapi.h>
#include <cstdlib>
namespace fs = std::filesystem;

// TODO: Implement `reset` subcommand.

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

typedef Subcmd Flag;

#define ROOT_IDENTIFIER ".topdir"
#define MSBUILD_PATH "D:\\bin\\Microsoft Visual Studio\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe"
#define PREMAKE5_PATH "D:\\bin\\premake 5.0 beta2\\premake5.exe"
#define VCREDIST_PATH "D:\\bin\\Microsoft Visual Studio\\Community\\VC\\Redist\\MSVC\\14.36.32532\\"
#define VCREDIST_EXE "vc_redist."

#define VERSION ("1.0.3")

// HOME is a environment variable defined to `C:\Users\<username>\`
#define HOME std::getenv("HOME")
#define PREMAKE5_TEMPLATE_PATH FMT("{}\\.emacs.d\\snippets\\lua-mode\\premake5", HOME)


int main(int argc, char *argv[]) {
  ARG();
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  std::string program{arg.pop_arg()};

  // flags
  bool quiet{false};
  bool not_build{false};
  bool will_copy_redist{false};
  bool will_help{false};
  bool will_clean{false};
  bool will_init{false};
  bool will_show_version{false};
  bool open_sln{false};
  bool force{false};
  bool executable_name_provided{false};
  std::string executable_name{};
  bool will_run{false};
  bool will_srun{false};
  std::string og_dir{fs::current_path().string()};
  std::string root_dir{};  
  std::string config{""};
  std::vector<std::string> valid_configs = {"Debug", "Release", "All"};
  std::string executable_args{};
  std::string project_name{};

  auto help = [&](){							
    print("Usage: {} [flags] [config] [subcmd] {{executable_args...}}\n", program);
    print("\nConfigs: \n"
          "    Debug (default)\n"
          "    Release\n"
	  "    All\n"	 
	  
	  "\nFlags: \n"
	  "    /Q    - Quiet mode; do not output anything\n"
	  "    /Rdst - Copies vcredist files to .\\redist\n"	
	  "    /h,/? - Same as the help subcommand.\n"		
	  "    /nb   - Do not build and run .\n"		
	  "    /ex   - If this flag is present, the argument after the "
	  "run subcommand is treated as the executable_name to run.\n"
	  "    /v    - Prints the version of momobuild.\n"
	  "    /Y    - Will answer `yes` on all confirmations.\n"
     
	  "\nSubcommands: \n"
	  "    help                     - Displays how to use this script.\n"
	  "    init [project_name]      - Initializes the project folder. optional arg [project_name] can be passed.\n"
	  "    reset                    - Resets the project folder.\n"
	  "    run                      - Runs the builded program.\n"
	  "    srun                     - Runs the builded program as a new process.\n"
	  "    dir                      - Opens the directory of the builded program.\n"
	  "    clean                    - Cleans the left-over things from the last build.\n"
	  "    sln                      - Opens the .sln file of the project.\n");
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
    root_dir = fs::current_path().string();
    if (fs::current_path().stem().string().empty()){
      // ERR("Could not find ROOT_IDENTIFIER `{}`\n", ROOT_IDENTIFIER);
      fprint(std::cerr, "ERROR: You are not inside a project folder. Please use `init` subcommand to initialize a project folder.\n");
      exit(1);
    }
  };


  /* asks for confirmation [y/n] */
  auto confirmation = [&](const std::string question, bool _default=true){
    if (force) return true;
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

  auto delete_file = [&](const std::string& file){
    if (fs::exists(file)){
      return fs::remove(file);
    }
    return false;
  };

  auto delete_dir = [&](const std::string& dir){
    if (fs::exists(dir)){
      return int(fs::remove_all(dir));
    }
    return 0;
  };

  std::vector<Flag> flags = {
    {false, "/Q",    [&]() { quiet = true; }},
    {false, "/Rdst", [&]() { will_copy_redist=true; }},
    {false, "/h",    [&]() { will_help = true; }},
    {false, "/?",    [&]() { will_help = true; }},
    {false, "/nb",   [&]() { not_build = true; }},
    {false, "/ex",   [&]() { executable_name_provided = true; }},
    {false, "/v",    [&]() { will_show_version = true; }},
    {false, "/Y",    [&]() { force=true; }}
  };
  
  std::vector<Subcmd> subcommands = {
    {false, "help",  [&]() { will_help = true; }},
    {false, "init",  [&]() { will_init = true; }},
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
      win::open_dir(FMT("bin\\{}\\", (config.empty() ? "Debug" : config)));
    }},
    {false, "clean",	[&]() { will_clean = true; }},
    {false, "sln",	[&]() {
      open_sln=true;
    }},
  };

  auto run_msbuild = [&](std::string config="Debug") {
    if (!quiet) print("\n{}: Running MSBuild [{}]...\n", "momobuild", config);
    if (config=="All") {
      win::wait_and_close_process(win::run_process(MSBUILD_PATH, FMT("-p:configuration={} build\\{}.sln -v:m -m", "Debug", project_name), quiet));
      win::wait_and_close_process(win::run_process(MSBUILD_PATH, FMT("-p:configuration={} build\\{}.sln -v:m -m", "Release", project_name), quiet));
    } else {
      auto msbuild = win::run_process(MSBUILD_PATH, FMT("-p:configuration={} build\\{}.sln -v:m -m", config, project_name));
      win::wait_and_close_process(msbuild);
    }
  };
  
  auto run_premake = [&]() {
    if (!quiet) print("\n{}: Running Premake5...\n", "momobuild");
    auto premake = win::run_process(PREMAKE5_PATH, "vs2022", quiet);
    win::wait_and_close_process(premake);
  };

  auto run = [&](){
    if (!quiet) {
      print("\n{}: Running {}.exe[{}]...\n", "momobuild", (!executable_name.empty() ? executable_name : project_name), config);
      print("--------------------------------------------------\n");
    }
    SetCurrentDirectoryA(FMT("bin\\{}\\", config).c_str());
    auto child = win::run_process(FMT("{}.exe", (!executable_name.empty() ? executable_name : project_name)), executable_args);
    win::wait_and_close_process(child);
    SetCurrentDirectoryA(root_dir.c_str());
  };

  auto srun = [&](){
    if (!quiet) {
      print("\n{}: Running {}.exe[{}] as a new process...\n", "momobuild", (!executable_name.empty() ? executable_name : project_name), config);
      print("--------------------------------------------------\n");
    }
    SetCurrentDirectoryA(FMT("bin\\{}\\", config).c_str());
    auto child = win::run_process(FMT("{}.exe", (!executable_name.empty() ? executable_name : project_name)), executable_args, false, true);
    win::wait_and_close_process(child);
    SetCurrentDirectoryA(root_dir.c_str());
  };

  // validators
  auto is_valid_config = [&](const std::string& a){
    for (auto& c : valid_configs){
      if (a==c){
	return true;
      }
    }
    return false;
  };

  auto is_valid_flag = [&](const std::string& a){
    for (auto& f : flags){
      if (f.name == a){
	return true;
      }
    }
    return false;
  };

  auto is_valid_subcommand = [&](const std::string& a){
    for (auto& s : subcommands){
      if (s.name == a){
	return true;
      }
    }
    return false;
  };


  // parse command line arguments
  // Usage: momobuild.exe [Config] [Flag] [Subcommand]
  bool config_handled{false};
  bool flag_handled{false};
  bool subcommand_handled{false};
    
  parse_arg:
  while (arg) {
    std::string a = arg.pop_arg();

    // try to parse as executable name or args
    if (subcommand_handled && (will_run || will_srun)){
      // if the `ex` arg is provided handle that
      if (executable_name.empty() && executable_name_provided){
	executable_name = a;
	goto parse_arg;
      }
      // append the arg to the executable args
      executable_args += (executable_args.empty() ? "" : " ");
      executable_args += a;
      goto parse_arg;
    }
    
    // check if the argument starts with `/`
    if (a[0] == '/'){ // this is a flag
      // check if the flag is valid
      if (flag_handled){
	fprint(std::cerr, "ERROR: Can only provide one flag at a time\n");
	exit(1);
      }
      for (auto& f : flags){
        if (f.handle(a)){
	  flag_handled = true;
	  break;
	}
      }
      if (!flag_handled){
	fprint(std::cerr, "ERROR: Invalid flag `{}`\n", a);
	exit(1);
      } else {
	goto parse_arg;
      }
    } else {
      // check if it's a config
      if (!config_handled) {
        if (is_valid_config(a)){
	  config = a;
	  config_handled = true;
	  goto parse_arg;
	}

	/// no config, subcomand

	// if the subcommand is already handled
	if (subcommand_handled){ 
	  if (is_valid_subcommand(a)){
	    fprint(std::cerr, "ERROR: Can only provide one subcommand at a time\n");
	    exit(1);
	  } else{
	    fprint(std::cerr, "ERROR: Invalid subcommand `{}`\n", a);
	    exit(1);
	  }
	}
	for (auto& s : subcommands){
	  if (s.handle(a)){
	    subcommand_handled=true;
	    if (s.name == "init"){
	      project_name = arg.pop_arg();
	    }
	    break;
	  }
	}
	if (!subcommand_handled){
	  fprint(std::cerr, "ERROR: Invalid config/subcommand `{}`\n", a);
	  exit(1);
	} else {
	  goto parse_arg;
	}
      } else {
	/// config provided, so try to parse as subcommand

        // if the subcommand is already handled
	if (subcommand_handled){ 
	  if (is_valid_subcommand(a)){
	    fprint(std::cerr, "ERROR: Can only provide one subcommand at a time\n");
	    exit(1);
	  } else{
	    fprint(std::cerr, "ERROR: Invalid subcommand `{}`\n", a);
	    exit(1);
	  }
	}
	for (auto& s : subcommands){
	  if (s.handle(a)){
	    subcommand_handled=true;
	    break;
	  }
	}
	if (!subcommand_handled){
	  fprint(std::cerr, "ERROR: Invalid subcommand `{}`\n", a);
	  exit(1);
	} else {
	  goto parse_arg;
	}
      }
    }
  }

  // subcommands/flags that doesn't need to be in root_dir
  
  if (will_help){
    help();
  }

  if (will_show_version){
    print("Momobuild Version {}\n", VERSION);
    exit(0);
  }
  
  if (will_init){
    std::string f{};
    if (!project_name.empty()){ // read the premake5 template if the project name is given
      std::ifstream ifs;
      ifs.open(PREMAKE5_TEMPLATE_PATH);
      if (!ifs.is_open()){
	ERR("Could not open {} for reading...\n", PREMAKE5_TEMPLATE_PATH);
      }
  
      ifs.seekg(0, std::ios::end);
      f.resize(ifs.tellg());
      ifs.seekg(0, std::ios::beg);
  
      ifs.read((char*)f.c_str(), f.size());
  
      ifs.close();
  
      str::lremove_until(f, "# --");
      str::lremove(f, 4);
    
      str::trim(f);

      str::replace(f, "$1", project_name);
    }
    
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
    create_file("premake5.lua", f);
    create_file(".topdir");
    create_file(".gitignore");
    create_file("src\\main.cpp");
    
    exit(0);
  }

  change_to_root_dir();

  if (will_clean){
    if (!quiet) print("{}: Cleaning project...\n", "momobuild");
    if (!confirmation("This will clean the previous build, continue?")) exit(0);
    size_t cleaned{0};
    if (delete_dir("build") && !quiet){
      print("INFO: Removed build\\...\n");
      cleaned++;
    }
    if (cleaned==0 && !quiet){
      print("INFO: Nothing to remove...\n");
    }
    
    exit(0);
  }

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
    win::open_file(FMT("build\\{}.sln", project_name));
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
