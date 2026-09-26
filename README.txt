WINDOWS BUILD (configured for this workstation)

Run from Git Bash:
    ./build.sh

Select builds in build-config.sh by uncommenting one or more entries. build.sh
configures and builds every selected preset in order. Available choices are
Clang and MSVC, each with Debug, RelWithDebInfo, and Release. Every combination
uses a separate sibling build directory.

build.sh locates the current Visual Studio installation with vswhere.exe and
imports its x64 developer environment from vcvars64.bat. VS Code CMake Tools
does the equivalent through cmake.useVsDeveloperEnvironment in settings.json.

Ninja builds in parallel automatically. To limit every selected build to two
parallel jobs, for example, run:
    ./build.sh --parallel 2

To delete the complete build directories for every selected preset, including
CMake caches and generated files, run:
    ./build.sh cleanall

Tool and Qt locations are in CMakePresets.json; dependency locations are
in config.cmake. Adjust these paths when moving to another workstation.
From an initialized developer terminal with CMake on PATH, you can also run:
    cmake --preset windows-clang-relwithdebinfo
    cmake --build --preset windows-clang-relwithdebinfo

The same six choices appear in the IDE's configure/build preset selectors.

LEGACY BUILD NOTES (older dependency versions)
PREREQUISITES
1) install boost (tested with version boost_1_80_0) - https://www.boost.org/users/download/
2) install qt5 (tested with version 5.12.9) - https://doc.qt.io/qt-5/gettingstarted.html#online-installation
3) install CGAL with GMP and MPFR (tested with version 5.5.1) - https://doc.cgal.org/latest/Manual/windows.html
4) install ninja - https://github.com/ninja-build/ninja/releases
5) install cmake

To build application run cmake command from the root of the source folder. You have to specify path
to the installed libraries as shown below. Optionally you may want change your build system to VC (I did not test it).

"C:/Program Files/CMake/bin/cmake.exe" \
-S . \
-B ../build-omi-triang-RelWithDebInfo "-DCMAKE_GENERATOR:STRING=Ninja" \
"-DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo" \
"-DCMAKE_PREFIX_PATH:PATH=C:/Qt/5.12.9/msvc2017_64" \
"-DCGAL_DIR:PATH=D:/test_omi/CGAL-5.5.1" \
"-DBOOST_ROOT:PATH=D:/test_omi/boost_1_80_0"

