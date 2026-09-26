#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$script_dir"

config_file="$script_dir/build-config.sh"
if [[ ! -f "$config_file" ]]; then
  printf 'Build configuration file not found: %s\n' "$config_file" >&2
  exit 1
fi

# shellcheck source=build-config.sh
source "$config_file"

if ! declare -p BUILD_PRESETS &>/dev/null; then
  printf 'BUILD_PRESETS is not defined in %s\n' "$config_file" >&2
  exit 1
fi

if (( ${#BUILD_PRESETS[@]} == 0 )); then
  printf 'No builds are selected in %s\n' "$config_file" >&2
  exit 1
fi

source_parent="$(cd -- "$script_dir/.." && pwd -P)"
source_name="$(basename -- "$script_dir")"

build_directory_for_preset() {
  case "$1" in
    windows-clang-debug)
      printf '%s/%s-llvm-ninja-Debug\n' "$source_parent" "$source_name"
      ;;
    windows-clang-relwithdebinfo)
      printf '%s/%s-llvm-ninja-RelWithDebInfo\n' "$source_parent" "$source_name"
      ;;
    windows-clang-release)
      printf '%s/%s-llvm-ninja-Release\n' "$source_parent" "$source_name"
      ;;
    windows-msvc-debug)
      printf '%s/%s-msvc-ninja-Debug\n' "$source_parent" "$source_name"
      ;;
    windows-msvc-relwithdebinfo)
      printf '%s/%s-msvc-ninja-RelWithDebInfo\n' "$source_parent" "$source_name"
      ;;
    windows-msvc-release)
      printf '%s/%s-msvc-ninja-Release\n' "$source_parent" "$source_name"
      ;;
    *)
      printf 'Cannot determine the build directory for preset: %s\n' "$1" >&2
      return 1
      ;;
  esac
}

if [[ "${1:-}" == "cleanall" ]]; then
  if (( $# != 1 )); then
    printf 'Usage: %s cleanall\n' "$0" >&2
    exit 1
  fi

  for build_preset in "${BUILD_PRESETS[@]}"; do
    build_dir="$(build_directory_for_preset "$build_preset")"

    # Only sibling directories with this source tree's known build prefix may
    # be removed. Never allow the source directory or its parent as a target.
    expected_prefix="$source_parent/$source_name-"
    if [[ "$build_dir" != "$expected_prefix"* ||
          "$build_dir" == "$script_dir" ||
          "$build_dir" == "$source_parent" ||
          "$build_dir" == "/" ]]; then
      printf 'Refusing to remove unsafe path: %s\n' "$build_dir" >&2
      exit 1
    fi

    if [[ -e "$build_dir" ]]; then
      printf 'Removing build directory: %s\n' "$build_dir"
      rm -rf -- "$build_dir"
    else
      printf 'Build directory does not exist: %s\n' "$build_dir"
    fi
  done

  printf 'Cleaned all selected build directories.\n'
  exit 0
fi

initialize_vs_environment() {
  local vswhere="/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe"
  if [[ ! -x "$vswhere" ]]; then
    printf 'Visual Studio locator not found: %s\n' "$vswhere" >&2
    return 1
  fi

  local installation_path
  installation_path="$(
    "$vswhere" -latest -products '*' \
      -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 \
      -property installationPath | tr -d '\r'
  )"
  if [[ -z "$installation_path" ]]; then
    printf 'No Visual Studio installation with the C++ x64 tools was found.\n' >&2
    return 1
  fi

  local vcvars_windows="${installation_path}\\VC\\Auxiliary\\Build\\vcvars64.bat"
  local vcvars_unix
  vcvars_unix="$(cygpath -u "$vcvars_windows")"
  if [[ ! -f "$vcvars_unix" ]]; then
    printf 'Visual Studio environment script not found: %s\n' "$vcvars_windows" >&2
    return 1
  fi

  local vcvars_command
  vcvars_command="$(cygpath -w -s "$vcvars_unix")"

  local environment_file
  environment_file="$(mktemp)"
  if ! cmd.exe //d //c "call $vcvars_command >nul && set" \
      >"$environment_file"; then
    rm -f -- "$environment_file"
    printf 'Failed to initialize the Visual Studio x64 environment.\n' >&2
    return 1
  fi

  local name value
  while IFS='=' read -r name value; do
    value="${value%$'\r'}"
    [[ "$name" =~ ^[A-Za-z_][A-Za-z0-9_]*$ ]] || continue

    if [[ "${name^^}" == "PATH" ]]; then
      PATH="$(cygpath -u -p "$value")"
      export PATH
    else
      export "$name=$value"
    fi
  done <"$environment_file"
  rm -f -- "$environment_file"

  if [[ -z "${VSCMD_VER:-}" ]]; then
    printf 'vcvars64.bat completed without establishing a developer environment.\n' >&2
    return 1
  fi

  printf 'Using Visual Studio at: %s\n' "$installation_path"
}

initialize_vs_environment
export PATH="/d/Qt/Tools/CMake_64/bin:/d/Qt/Tools/Ninja:$PATH"

build_count=${#BUILD_PRESETS[@]}
build_number=0

for build_preset in "${BUILD_PRESETS[@]}"; do
  ((build_number += 1))
  printf '\n[%d/%d] Configuring preset: %s\n' \
    "$build_number" "$build_count" "$build_preset"
  cmake --preset "$build_preset"

  printf '[%d/%d] Building preset: %s\n' \
    "$build_number" "$build_count" "$build_preset"
  cmake --build --preset "$build_preset" "$@"
done

printf '\nCompleted %d selected build(s).\n' "$build_count"
