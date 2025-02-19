/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include <malloc.h>
#include <cstring>

#include "xenia/base/cvar.h"
#include "xenia/base/logging.h"
#include "xenia/base/main_win.h"
#include "xenia/base/platform_win.h"
#include "xenia/base/string.h"

// Autogenerated by `xb premake`.
#include "build/version.h"

// For RequestHighPerformance.
#include <winternl.h>

DEFINE_bool(win32_high_freq, true,
            "Requests high performance from the NT kernel", "Kernel");

namespace xe {

static void RequestHighPerformance() {
#if XE_PLATFORM_WIN32
  NTSTATUS(*NtQueryTimerResolution)
  (OUT PULONG MinimumResolution, OUT PULONG MaximumResolution,
   OUT PULONG CurrentResolution);

  NTSTATUS(*NtSetTimerResolution)
  (IN ULONG DesiredResolution, IN BOOLEAN SetResolution,
   OUT PULONG CurrentResolution);

  NtQueryTimerResolution = (decltype(NtQueryTimerResolution))GetProcAddress(
      GetModuleHandleW(L"ntdll.dll"), "NtQueryTimerResolution");
  NtSetTimerResolution = (decltype(NtSetTimerResolution))GetProcAddress(
      GetModuleHandleW(L"ntdll.dll"), "NtSetTimerResolution");
  if (!NtQueryTimerResolution || !NtSetTimerResolution) {
    return;
  }

  ULONG minimum_resolution, maximum_resolution, current_resolution;
  NtQueryTimerResolution(&minimum_resolution, &maximum_resolution,
                         &current_resolution);
  NtSetTimerResolution(maximum_resolution, TRUE, &current_resolution);
#endif
}

bool ParseWin32LaunchArguments(
    bool transparent_options, const std::string_view positional_usage,
    const std::vector<std::string>& positional_options,
    std::vector<std::string>* args_out) {
  auto command_line = GetCommandLineW();

  int wargc;
  wchar_t** wargv = CommandLineToArgvW(command_line, &wargc);
  if (!wargv) {
    return false;
  }

  // Convert all args to narrow, as cxxopts doesn't support wchar.
  int argc = wargc;
  char** argv = reinterpret_cast<char**>(alloca(sizeof(char*) * argc));
  for (int n = 0; n < argc; n++) {
    size_t len = std::wcstombs(nullptr, wargv[n], 0);
    argv[n] = reinterpret_cast<char*>(alloca(sizeof(char) * (len + 1)));
    std::wcstombs(argv[n], wargv[n], len + 1);
  }

  LocalFree(wargv);

  if (!transparent_options) {
    cvar::ParseLaunchArguments(argc, argv, positional_usage,
                               positional_options);
  }

  if (args_out) {
    args_out->clear();
    for (int n = 0; n < argc; n++) {
      args_out->push_back(std::string(argv[n]));
    }
  }

  return true;
}

int InitializeWin32App(const std::string_view app_name) {
  // Initialize logging. Needs parsed FLAGS.
  xe::InitializeLogging(app_name);

  // Print version info.
  XELOGI(
      "Build: "
#ifdef XE_BUILD_IS_PR
      "PR#" XE_BUILD_PR_NUMBER " " XE_BUILD_PR_REPO " " XE_BUILD_PR_BRANCH
      "@" XE_BUILD_PR_COMMIT_SHORT " against "
#endif
      XE_BUILD_BRANCH "@" XE_BUILD_COMMIT_SHORT " on " XE_BUILD_DATE);

  // Request high performance timing.
  if (cvars::win32_high_freq) {
    RequestHighPerformance();
  }

  return 0;
}

void ShutdownWin32App() { xe::ShutdownLogging(); }

}  // namespace xe
