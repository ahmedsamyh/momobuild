#ifndef _STDCPP_H_
#define _STDCPP_H_

#include <format>
#include <iostream>
#include <string>
#include <algorithm>

#if defined USE_WIN32
#define WIN32_MEAN_AND_LEAN
#include <windows.h>

namespace win {

PROCESS_INFORMATION run_process(const std::string& program, const std::string& cmd, bool no_stdout=false, bool new_console=false);
void wait_and_close_process(PROCESS_INFORMATION proc);
HINSTANCE open_dir(const std::string& dir);
HINSTANCE open_file(const std::string& file);
std::string last_error_str();
  
}; // namespace win
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
#define LOG(...) log(__VA_ARGS__)
void log();
template <typename T, typename... Types> void log(T arg, Types... args) {
  std::cout << arg;
  log(args...);
}
#define UNREACHABLE() PANIC("Uncreachable\n")
#define UNIMPLEMENTED() PANIC("{}() is unimplemented\n", __func__)
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

namespace str {
std::string& tolower(std::string& s);
std::string& toupper(std::string& s);
} // namespace str

#endif /* _STDCPP_H_ */
//////////////////////////////////////////////////
#if defined STDCPP_IMPLEMENTATION || STDCPP_IMPL

#if defined USE_WIN32

namespace win {
  
PROCESS_INFORMATION run_process(const std::string& program, const std::string& cmd, bool no_stdout, bool new_console) {
  STARTUPINFOA startupinfo{};
  if (no_stdout){
    startupinfo.dwFlags |= STARTF_USESTDHANDLES;
    startupinfo.hStdOutput = NULL;
  }
  startupinfo.cb = sizeof(startupinfo);
  PROCESS_INFORMATION child_process_info{};

  std::string full_cmd = FMT("{} {}", program, cmd).c_str();

  DWORD creation_flags = NORMAL_PRIORITY_CLASS;
  if (new_console) creation_flags |= CREATE_NEW_CONSOLE;
  
  if (!CreateProcessA(NULL,
		      LPSTR(full_cmd.c_str()),
  	              NULL,
                      NULL,
                      TRUE,
                      creation_flags,
                      NULL,
                      NULL,
                      &startupinfo,&child_process_info)) {
    fprint(std::cerr, "ERROR: {}(\"{}\", \"{}\", {}, {}) -> {}\n", __func__, program, cmd, no_stdout, new_console, last_error_str());
    exit(1);
  };
  return child_process_info;
}

void wait_and_close_process(PROCESS_INFORMATION proc){
  if (WaitForSingleObject(proc.hProcess, INFINITE) == WAIT_FAILED){
    fprint(std::cerr, "ERROR: {} -> {}\n", __func__, last_error_str());
    exit(1);
  }

  DWORD proc_exit_code{};
  GetExitCodeProcess(proc.hProcess, &proc_exit_code);
  if (proc_exit_code != 0){
    fprint(std::cerr, "ERROR: Process exited with code: {}\n", proc_exit_code);
    exit(proc_exit_code);
  }
  
  CloseHandle(proc.hProcess);
  CloseHandle(proc.hThread);
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

}; // namespace win

#endif

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

namespace str {
std::string& tolower(std::string& s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
  return s;
}

std::string& toupper(std::string& s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::toupper(c); });
  return s;
}
} // namespace str
#endif
