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

