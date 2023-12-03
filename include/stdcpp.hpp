#ifndef _STDCPP_H_
#define _STDCPP_H_

#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include <algorithm>
#include <vector>
#include <functional>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

#if defined USE_WINAPI
#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <shellapi.h>

// TODO: Make a loggin system

// Option --------------------------------------------------

template <typename T>
struct Option{
  T value;
  bool _has_value{false};

  Option(){ }

  Option(T _value){
    value = _value;
    _has_value = true;
  }

  operator bool() { return has_value(); }
  bool has_value() const { return _has_value; }
  bool operator!() { return !has_value(); }

  T operator=(const T& _v){
    value = _v;
    _has_value = true;
    return value;
  }

  T operator=(T& _v){
     value = _v;
    _has_value = true;
    return value;
  }

  T& unwrap() { return value; }

};



namespace win {

  typedef PROCESS_INFORMATION Proc;

  int run_sync(const std::string& program, const std::string& cmd);
  Option<Proc> run_async(const std::string& program, const std::string& cmd);
  int close_proc(const Proc& proc);
  HINSTANCE open_dir(const std::string& dir);
  HINSTANCE open_file(const std::string& file);
  std::string last_error_str();
  std::vector<WIN32_FIND_DATAA> get_entries_in_dir(std::string dir);
  std::vector<std::string> get_files_in_dir(std::string dir);
  std::vector<std::string> get_dirs_in_dir(std::string dir);
  std::string get_current_dir();
  std::string change_dir(std::string dir);
} // namespace win
#endif

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
#define ERR(str, ...) PANIC("{}: "str, "ERROR", __VA_ARGS__)
#define FMT(str, ...) std::format((str), __VA_ARGS__)
#define PANIC(str, ...) panic(FMT("{}:{}: "str, __FILE__, __LINE__,  __VA_ARGS__))
void panic();
template <typename T, typename... Types> void panic(T arg, Types... args) {
  std::cerr << arg;
  panic(args...);
}
#define LOG(str, ...) log(FMT(str, __VA_ARGS__))
void log();
template <typename T, typename... Types> void log(T arg, Types... args) {
  std::cout << arg;
  log(args...);
}
#define UNREACHABLE() PANIC("Unreachable\n")
#define UNIMPLEMENTED() PANIC("{}() is unimplemented\n", __func__)
// JUST USE print("WARNING: ", ...) if you want to warn
#define FMT(str, ...) std::format((str), __VA_ARGS__)
#define fprint(file, str, ...) __print((file), FMT((str), __VA_ARGS__))
#define print(str, ...) fprint(std::cout, str, __VA_ARGS__)
#define TODO() PANIC("Todo reached!\n")

void __print(std::ostream &file);
template <typename T, typename... Types>
void __print(std::ostream &file, T arg, Types... args) {
  file << arg;
  __print(file, args...);
}

#define MAX_ENV_SIZE (1024*2)
static std::string __env_buf(MAX_ENV_SIZE, '_');
static size_t __env_size{0};
std::string get_env(const std::string& value);

#define ARG() Arg arg(argc, argv)

struct Arg {
  int *argc{nullptr};
  char ***argv{nullptr};
  bool evaluating_quote{false};

  Arg(int &_argc, char **&_argv);

  bool empty() const;
  operator bool();
  bool operator!();
  std::string pop();
};

// ch --------------------------------------------------
namespace ch {
  bool isspace(const char& ch);
  bool isalpha(const char& ch);
  bool isalphanum(const char& ch);
  bool isdigit(const char& ch);
} // namespace ch

// str --------------------------------------------------
namespace str {

  std::string tolower(std::string s);
  std::string toupper(std::string s);
  std::string rtrim(std::string str);
  std::string ltrim(std::string str);
  std::string trim(std::string str);
  std::vector<std::string> split_by(std::string str, char delim='\n', bool add_empty=false);
  std::string lremove(std::string str, size_t n=1);
  std::string rremove(std::string str, size_t n=1);
  std::string lremove_until(std::string str, std::function<bool(const char&)> predecate);
  std::string rremove_until(std::string str, std::function<bool(const char&)> predecate);
  std::string lremove_until(std::string str, const std::string& checker);
  std::string remove_char(std::string str, const char& ch);
  std::string replace(std::string str, const std::string& thing, const std::string& with);
  std::string lpop(std::string str, size_t n=1);
  std::string rpop(std::string str, size_t n=1);
  std::string lpop_until(std::string str, const char& ch);
  std::string lpop_until(std::string str, std::function<bool(const char&)> predacate);
  std::string rpop_until(std::string str, const char& ch);
  std::string rpop_until(std::string str, std::function<bool(const char&)> predacate);
  std::string trim_quote(std::string str);
} // namespace str


// sv --------------------------------------------------
namespace sv {

  std::string_view& rtrim(std::string_view& sv);
  std::string_view& ltrim(std::string_view& sv);
  std::string_view& trim(std::string_view& sv);
  std::string_view& lremove(std::string_view& sv, size_t size=1);
  std::string_view& rremove(std::string_view& sv, size_t size=1);
  std::string_view& lremove_until(std::string_view& sv, std::function<bool(const char&)> predecate);
  std::string_view& rremove_until(std::string_view& sv, std::function<bool(const char&)> predecate);
  std::string_view& lremove_until(std::string_view& sv, const std::string_view& checker);
  std::string_view& rremove_until(std::string_view& sv, const std::string_view& checker);


} // namespace sv

// math --------------------------------------------------
namespace math {
#define PI 3.14159265359

  float randomf(const float min, const float max);
  // TODO: randomi doesn't give that much random numbers.
  // maybe don't use rand() and use a more modern c++ random number generator?
  int randomi(const int min, const int max);
  float rad2deg(const float rad);
  float deg2rad(const float deg);
  float map(float val, float min, float max, float from, float to);
  bool chance(float percent);
  bool rect_intersects_rect(const float x1, const float y1, const float w1, const float h1,
			    const float x2, const float y2, const float w2, const float h2);
  bool rect_contains_rect(const float x1, const float y1, const float w1, const float h1,
			  const float x2, const float y2, const float w2, const float h2);
} // namespace math

namespace file {
  std::string slurp_file(const std::string& filename);

  // database format
  void save_data_to_file(const std::string name, const std::string& value, std::string filename, bool overwrite=false);

} // namespace file


#endif /* _STDCPP_H_ */
//////////////////////////////////////////////////
#if defined STDCPP_IMPLEMENTATION || STDCPP_IMPL

#if defined USE_WINAPI

namespace win {

  int run_sync(const std::string& program, const std::string& cmd){
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    Proc child_proc;
    
    std::string full_cmd = FMT("{} {}", program, cmd).c_str();

    DWORD creation_flags = NORMAL_PRIORITY_CLASS;

    if (!CreateProcessA(NULL,
			LPSTR(full_cmd.c_str()),
			NULL,
			NULL,
			TRUE,
			creation_flags,
			NULL,
			NULL,
			&si,
			&child_proc)){
      fprint(std::cerr, "ERROR: CreateProcessA() -> {}\n",  last_error_str());
      return 1;
    }

    if (WaitForSingleObject(child_proc.hProcess, INFINITE) == WAIT_FAILED){
      fprint(std::cerr, "ERROR: WaitForSingleObject() -> {}\n", last_error_str());
      return 1;
    }
    
    DWORD child_proc_exit_code{};
    GetExitCodeProcess(child_proc.hProcess, &child_proc_exit_code);
    if (child_proc_exit_code != 0){
      fprint(std::cerr, "ERROR: Process exited with code: {}\n", child_proc_exit_code);
      return child_proc_exit_code;
    }

    CloseHandle(child_proc.hProcess);
    CloseHandle(child_proc.hThread);
    return 0;
  }

  Option<Proc> run_async(const std::string& program, const std::string& cmd){

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    Proc child_proc;
    
    std::string full_cmd = FMT("{} {}", program, cmd).c_str();

    DWORD creation_flags = NORMAL_PRIORITY_CLASS;

    if (!CreateProcessA(NULL,
			LPSTR(full_cmd.c_str()),
			NULL,
			NULL,
			TRUE,
			creation_flags,
			NULL,
			NULL,
			&si,
			&child_proc)){
      fprint(std::cerr, "ERROR: CreateProcessA() -> {}\n",  last_error_str());
      return Option<Proc>{};
    }
    
    return Option<Proc>(child_proc);
  }

  int close_proc(const Proc& proc){
    if (WaitForSingleObject(proc.hProcess, INFINITE) == WAIT_FAILED){
      fprint(std::cerr, "ERROR: WaitForSingleObject() -> {}\n", last_error_str());
      return 1;
    }
    
    DWORD proc_exit_code{};
    GetExitCodeProcess(proc.hProcess, &proc_exit_code);
    if (proc_exit_code != 0){
      fprint(std::cerr, "ERROR: Process exited with code: {}\n", proc_exit_code);
      return proc_exit_code;
    }

    CloseHandle(proc.hProcess);
    CloseHandle(proc.hThread);
    
    return 0;
  }

  HINSTANCE open_dir(const std::string& dir){
    return open_file(dir);
  }

  HINSTANCE open_file(const std::string& file){
    HINSTANCE res = ShellExecuteA(NULL, "open", file.c_str(), NULL, NULL, SW_MAXIMIZE);
    if (INT_PTR(res) < 32) {
      ERR("Could not execute shell command {}\n", INT_PTR(res));
    }
    return res;
  }

  std::string last_error_str(){
    DWORD err = GetLastError();
    if (err==0) return {};
    LPSTR message_buffer{nullptr};
    DWORD size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,	// dwFlags
				NULL,												// lpSource
				err,												// dwMessageId
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),							// dwLanguageId
				LPSTR(&message_buffer),										//lpBuffer
				0,												// nSize
				NULL												// va_lists *Arguments
				);
    if (size==0){
      ERR("{} Failed\n", __func__);
    }
    std::string message(message_buffer, size_t(size));
    return message;
  }

  std::vector<WIN32_FIND_DATAA> get_entries_in_dir(std::string dir) {
    std::vector<WIN32_FIND_DATAA> res{};
    WIN32_FIND_DATAA fd{};
    dir += "\\*";
    HANDLE h = FindFirstFileA(dir.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE){
      fprint(std::cerr, "ERROR: {}({}) -> {}\n", __func__, dir, last_error_str());
      return res;
    }
    do {
      res.push_back(fd);
    } while (FindNextFileA(h, &fd) != FALSE);

    FindClose(h);
    return res;
  }

  std::vector<std::string> get_files_in_dir(std::string dir){
    std::vector<std::string> res{};
    for (auto& e : get_entries_in_dir(dir)){
      if (!(e.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
	res.push_back(e.cFileName);
      }
    }
    return res;
  }

  std::vector<std::string> get_dirs_in_dir(std::string dir){
    std::vector<std::string> res{};
    for (auto& e : get_entries_in_dir(dir)){
      if (e.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
	res.push_back(e.cFileName);
      }
    }
    return res;
  }

  std::string get_current_dir(){
    char cd[MAX_PATH];
    size_t n = GetCurrentDirectoryA(MAX_PATH, cd);
    if (n == 0) {
      fprint(std::cerr, "ERROR: {} -> {}\n", __func__, win::last_error_str());
      exit(1);
    }
    return std::string(cd);
  }

  std::string change_dir(std::string dir){
    std::string current_dir = get_current_dir();

    if (dir.empty()){
      return current_dir;
    }

    ASSERT(dir.size() > 0);

    if (dir[dir.size()-1] != '\\') dir += '\\';

    if (!fs::is_directory(fs::path(dir))){
      fprint(std::cerr, "ERROR: `{}` is not a valid directory\n", dir);
      exit(1);
    }

    if (!SetCurrentDirectoryA(dir.c_str())){
      fprint(std::cerr, "ERROR: {} -> {}\n", __func__, last_error_str());
      exit(1);
    }

    return current_dir;
  }

} // namespace win

#endif

void __print(std::ostream &file){};

void log(){};

void panic() { exit(1); };

std::string get_env(const std::string& value){
  getenv_s(&__env_size, (char*)__env_buf.c_str(), MAX_ENV_SIZE, value.c_str());
  if (__env_size > 0){
    __env_buf.resize(__env_size-1);
  } else {
    return {};
  }
  std::string res = __env_buf;
  return res;
}

// Arg --------------------------------------------------
Arg::Arg(int &_argc, char **&_argv) {
  argc = &_argc;
  argv = &_argv;
}

bool Arg::empty() const { return (*argc) <= 0; }

Arg::operator bool() { return !empty(); }
bool Arg::operator!() { return empty(); }

std::string Arg::pop() {
  if (empty()) { // return empty string if no args are available
    return {};
  };

  std::string arg = *argv[0];

  if (arg[0] == '\''){
    evaluating_quote = true;
    str::lremove(arg);
  }
  if (arg.back() == '\''){
    evaluating_quote = false;
    str::rremove(arg);
  }

  *argc = *argc - 1;
  *argv = *argv + 1;

  if (evaluating_quote){
    arg += " ";
    arg += pop();
  }

  return arg;
}


// ch --------------------------------------------------
namespace ch {
  bool isspace(const char& ch)	  { return std::isspace(ch);    }
  bool isalpha(const char& ch)	  { return std::isalpha(ch);    }
  bool isalphanum(const char& ch) { return (isalpha(ch) || isdigit(ch)); }
  bool isdigit(const char& ch)	  { return std::isdigit(ch);    }
} // namespace ch
// str --------------------------------------------------
namespace str {
  std::string tolower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
  }

  std::string toupper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::toupper(c); });
    return s;
  }

  std::string rtrim(std::string str){
    while (!str.empty() && (std::isspace(str[str.size()-1]) || str[str.size()-1] == '\0')){
      str.pop_back();
    }
    return str;
  }

  std::string ltrim(std::string str){
    while (!str.empty() && (std::isspace(str[0]) || str[0] == '\0')){
      str = str.substr(1);
    }
    return str;
  }

  std::string trim(std::string str){return rtrim(ltrim(str)); }

  std::vector<std::string> split_by(std::string str, char delim, bool add_empty){
    std::string elm{};
    std::vector<std::string> res{};

    std::string::size_type nl_pos{0};
    while (nl_pos != std::string::npos){
      nl_pos = str.find(delim);
      elm = str.substr(0, nl_pos);
      str = str.substr(nl_pos+1);
      if (!add_empty){
	if (!str.empty()){
	  res.push_back(elm);
	}
      } else {
	res.push_back(elm);
      }
    }

    return res;
  }

  std::string lremove(std::string str, size_t n){
    if (str.empty()) return str;
    if (n > str.size()) n = str.size();
    while (n > 0){
      str = str.substr(1);
      n--;
    }
    return str;
  }

  std::string rremove(std::string str, size_t n){
    if (str.empty()) return str;
    if (n > str.size()) n = str.size();
    while (n > 0){
      str.pop_back();
      n--;
    }
    return str;
  }

  std::string lremove_until(std::string str, std::function<bool(const char&)> predecate){
    while (predecate(str[0]) && !str.empty()){
      lremove(str);
    }
    return str;
  }

  std::string rremove_until(std::string str, std::function<bool(const char&)> predecate){
    while (predecate(str[str.size()-1]) && !str.empty()){
      rremove(str);
    }
    return str;
  }

  std::string lremove_until(std::string str, const std::string& checker){
    if (checker.size() > str.size() || str.empty() || checker.empty()) return str;

    std::string s{};
    size_t s_i{0};
    while (s_i + checker.size() < str.size()){
      s = str.substr(s_i, checker.size());
      if (s == checker) break;
      s_i++;
    }
    str = str.substr(s_i);
    return str;
  }

  std::string rremove_until(std::string str, const std::string& checker){
    if (checker.size() > str.size() || str.empty() || checker.empty()) return str;

    std::string s{};
    size_t s_i{str.size()};
    while (s_i - checker.size() > 0){
      s = str.substr(s_i - checker.size(), checker.size());
      if (s == checker) break;
      s_i--;
    }
    str = str.substr(0, s_i);
    return str;
  }

  std::string remove_char(std::string str, const char& ch){
    auto ch_pos = str.find(ch);
    while (ch_pos != std::string::npos){
      str.erase(ch_pos, 1);
      ch_pos = str.find(ch);
    }

    return str;
  }

  std::string replace(std::string str, const std::string& thing, const std::string& with){
    auto thing_pos{str.find(thing)};

    while (thing_pos != std::string::npos){
      std::string before_thing{str.substr(0, thing_pos)};
      std::string after_thing{str.substr(thing_pos)};
      str::lremove(after_thing, thing.size());
      str = before_thing + with + after_thing;
      thing_pos = str.find(thing, before_thing.size() + with.size());
    }

    return str;
  }

  // TODO: Maybe pass the str by reference? because if it's passed by value we can essentially just use std::string::substr() instead.
  std::string lpop(std::string str, size_t n){
    return str.substr(0, n);
  }
  std::string rpop(std::string str, size_t n){
    return str.substr(str.size() - n);
  }

  std::string lpop_until(std::string str, const char& ch){
    auto ch_idx = str.find_first_of(ch);
    if (ch_idx == std::string::npos){
      return str;
    }

    return str.substr(0, ch_idx);
  }

  std::string lpop_until(std::string str, std::function<bool(const char&)> predacate){
    size_t i = 0;
    while (i < str.size()){
      if (!predacate(str[i])){
	i++;
      } else {
	return str.substr(0, i);
      }
    }
    return str;
  }

  std::string rpop_until(std::string str, const char& ch){
    auto ch_idx = str.find_last_of(ch);
    if (ch_idx == std::string::npos){
      return str;
    }

    return str.substr(ch_idx+1);
  }

  std::string trim_quote(std::string str){
    if (str.empty()) return {};
    if (str[0] == '\'') str = lremove(str);
    if (str[str.size()-1] == '\'') str = rremove(str);
    return str;
  }

  std::string rpop_until(std::string str, std::function<bool(const char&)> predacate){
    size_t i = str.size()-1;
    while (i >= 0){
      if (!predacate(str[i])){
	i--;
      } else {
	return str.substr(i+1);
      }
    }
    return str;
  }

} // namespace str

// sv --------------------------------------------------
namespace sv{

std::string_view& rtrim(std::string_view& sv){
  while (!sv.empty() && (std::isspace(sv[sv.size()-1]) || sv[sv.size()-1] == '\0')){
    sv.remove_suffix(1);
  }
  return sv;
}

std::string_view& ltrim(std::string_view& sv){
  while (!sv.empty() && (std::isspace(sv[0]) || sv[0] == '\0')){
    sv.remove_prefix(1);
  }
  return sv;
}

std::string_view& trim(std::string_view& sv){return rtrim(ltrim(sv)); }

std::string_view& lremove(std::string_view& sv, size_t size){
  if (sv.empty()) return sv;
  if (size > sv.size()) size = sv.size();
  sv.remove_prefix(size);
  return sv;
}

std::string_view& rremove(std::string_view& sv, size_t size){
  if (sv.empty()) return sv;
  if (size > sv.size()) size = sv.size();
  sv.remove_suffix(size);
  return sv;
}

std::string_view& lremove_until(std::string_view& sv, std::function<bool(const char&)> predecate){
  while (predecate(sv[0])){
    sv::lremove(sv);
  }
  return sv;
}

std::string_view& rremove_until(std::string_view& sv, std::function<bool(const char&)> predecate){
  while (predecate(sv[sv.size()-1])){
    sv::rremove(sv);
  }
  return sv;
}

std::string_view& lremove_until(std::string_view& sv, const std::string_view& checker){
  if (checker.size() > sv.size() || sv.empty() || checker.empty()) return sv;

  std::string_view s{};
  size_t s_i{0};
  while (s_i + checker.size() < sv.size() && s != checker){
    s = sv.substr(s_i, checker.size());
    s_i++;
  }
  sv = sv.substr(s_i-1);
  return sv;
}

std::string_view& rremove_until(std::string_view& sv, const std::string_view& checker){
  if (checker.size() > sv.size() || sv.empty() || checker.empty()) return sv;

  std::string_view s{};
  size_t s_i{sv.size()};

  while (s_i - checker.size() >= 0){
    s = sv.substr(s_i - checker.size(), checker.size());
    if (s == checker) break;
    s_i--;
  }
  sv = sv.substr(0, s_i);
  return sv;
}


} // namespace sv

// math -------------------------
namespace math {

float randomf(const float min, const float max) {
  return (float(rand()) / float(RAND_MAX) * (max - min)) + min;
}

int randomi(const int min, const int max) {
  return int((float(rand()) / float(RAND_MAX) * (max - min)) + min);
}

float rad2deg(const float rad) { return float((rad / PI) * 180.f); }

float deg2rad(const float deg) { return float((deg / 180) * PI); }

float map(float val, float min, float max, float from, float to) {
  float normalized = val / (max - min);
  return normalized * (to - from) + from;
}

bool chance(float percent) {
  //
  ASSERT_MSG(0.f <= percent && percent <= 100.f,
             "percent should be between 0.f and 100.f");

  return math::randomf(0.f, 100.f) <= percent;
}

bool rect_intersects_rect(const float x1, const float y1, const float w1, const float h1,
			  const float x2, const float y2, const float w2, const float h2) {
  ///
  float r1_left = x1;
  float r1_top =  y1;
  float r2_left = x2;
  float r2_top =  y2;
  float r1_right = x1 + w2;
  float r2_right = x2 + w2;
  float r1_bottom = y1 + h1;
  float r2_bottom = y2 + h2;
  return (r1_left <= r2_right && r1_right >= r2_left && r1_top <= r2_bottom &&
          r1_bottom >= r2_top);
}

bool rect_contains_rect(const float x1, const float y1, const float w1, const float h1,
			const float x2, const float y2, const float w2, const float h2) {
  float r1_left = x1;
  float r1_top =  y1;
  float r2_left = x2;
  float r2_top =  y2;
  float r1_right = x1 + w2;
  float r2_right = x2 + w2;
  float r1_bottom = y1 + h1;
  float r2_bottom = y2 + h2;
  return r2_left >= r1_left && r2_right <= r1_right && r2_top >= r1_top &&
         r2_bottom <= r1_bottom;
}

} // namespace math

namespace file {
  std::string slurp_file(const std::string& filename){
    std::ifstream ifs;
    ifs.open(filename, std::ios::binary);
    if (!ifs.is_open()){
      fprint(std::cerr, "ERROR: Could not open file `{}` for reading...\n", filename);
      return {};
    }

    std::string res{};
    ifs.seekg(0, std::ios::end);
    res.resize(ifs.tellg());
    ifs.seekg(0, std::ios::beg);

    ifs.read((char*)res.data(), res.size());

    ifs.close();
    return res;
  }

  void save_data_to_file(const std::string name, const std::string& value, std::string filename, bool overwrite){
    // check if the value already exists in file
    std::string file = slurp_file(filename);
    if (file.find(name) != std::string::npos){
      if (overwrite){
	// overwrite file
	auto name_pos = file.find(name);
        size_t value_pos = name_pos + name.size() + 1;
        if (std::isspace(file[value_pos])){
	  value_pos++;
	}


	// print("before_value: |{}|\n", file.substr(0, value_pos));
	// value.size() here have to be the previous value size
	print("after_value: |{}|\n", file.substr(value_pos+value.size()));

	file = file.substr(0, value_pos) + value + file.substr(value_pos+value.size());

	print("file: |{}|\n", file);

      } else {
	print("WARNING: Ignored writing ({}: {}) because it already exists\n", name, value);
      }
    } else {
      file += FMT("{}: {}\n", name, value);
      std::ofstream ofs;
      ofs.open(filename, std::ios::binary);
      ofs << file;
    }
  }
} // namespace file

#endif
