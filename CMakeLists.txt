cmake_minimum_required(VERSION 3.5)

project(omi_triang VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(this_project_utils)

# temporarily variables need to be set as external parameters
# path to thirdparty dirs
set(WINDOWS_EXTERNAL_LIBS_PATH  "D:/trymix/test_omi" "D:/zero-devel")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)


function(find_path_by_regex var_out regex_in)
    foreach(item ${ARGN})
        if(item STREQUAL "PATHS")
            set(itemmode "PATHS")
            continue()
        endif()
        if(itemmode STREQUAL "PATHS")
           FILE(GLOB found_path "${item}/${regex_in}")
           message(STATUS found_path:${found_path})
           if(found_path)
               set(${var_out} ${found_path} PARENT_SCOPE)
               return()
           endif()
        endif()
    endforeach()
endfunction()

find_path_by_regex(BOOST_ROOT "boost_*[0-9]_*[0-9]_*[0-9]" PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} )

find_package(Boost REQUIRED)

set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)
find_package(CGAL PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} PATH_SUFFIXES "CGAL-5.5.1" COMPONENTS Core)

####################### spooles ##################

if(WIN32)
  FIND_LIBRARY(SPOOLES_LIB spooles PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} PATH_SUFFIXES spooles-debug/src)
  FIND_PATH(SPOOLES_INCLUDE_DIR InpMtx.h PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} PATH_SUFFIXES spooles/include NO_CMAKE_PATH)
else(UNIX)
  FIND_LIBRARY(SPOOLES_LIB spooles)
  FIND_PATH(SPOOLES_INCLUDE_DIR InpMtx.h PATH_SUFFIXES spooles/include)
endif(WIN32)

################# pthread #########################

if(MINGW)
elseif(WIN32)
    find_path_by_regex(pthread_DIR "pthreads-w32-*[0-9]-*[0-9]-*[0-9]" PATHS ${WINDOWS_EXTERNAL_LIBS_PATH})

    FIND_PATH(pthread_INCLUDE_DIR pthread.h
      HINTS ${pthread_DIR} PATH_SUFFIXES Pre-built.2/include
      REQUIRED
    )

    FIND_LIBRARY(pthread_LIB pthreadVC2
      HINTS ${pthread_DIR}/Pre-built.2/lib/x64
      REQUIRED
    )

    FIND_FILE(pthread_DLL Pre-built.2/dll/x64/pthreadVC2.dll
      HINTS ${pthread_DIR}
      REQUIRED
    )

    list(APPEND FOR_COPY_TO_BINARY_DIR ${pthread_DLL})

    INCLUDE_DIRECTORIES( ${pthread_INCLUDE_DIR} )
endif(MINGW)

################# LAPACK #########################
if(UNIX OR MINGW)
  FIND_LIBRARY(LAPACK_LIB lapack)
  FIND_LIBRARY(BLAS_LIB blas)
elseif(WIN32)
  FIND_LIBRARY(LAPACK_LIB lapack PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} PATH_SUFFIXES lapack)
  FIND_LIBRARY(BLAS_LIB blas PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} PATH_SUFFIXES lapack)
  FIND_FILE(LAPACK_DLL liblapack.dll PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} PATH_SUFFIXES lapack)
  FIND_FILE(BLAS_DLL libblas.dll PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} PATH_SUFFIXES lapack)
  list(APPEND FOR_COPY_TO_BINARY_DIR ${LAPACK_DLL} ${BLAS_DLL})
endif()

if(NOT ${CGAL_FOUND})
   message(SEND_ERROR "CGAL is not found, try to add -DCGAL_DIR=<path to CGAL dir> variable")
endif()

if(NOT ${BOOST_FOUND})
   message(SEND_ERROR "BOOST is not found, try to add -DBOOST_ROOT=<path to BOOST dir> variable")
endif()

set(TS_FILES omi_triang_en_GB.ts)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    CentralWidget.cpp  CentralWidget.h CentralWidget.ui

    ProjectTree.h ProjectTree.cpp
    Projects_TreeItem.h Projects_TreeItem.cpp

    PolygonTests/PolygonTests_TreeItem.cpp PolygonTests/PolygonTests_TreeItem.h
    PolygonTests/PolygonTests_panel.ui
    PolygonTests/PolygonTestConvexPartitioning.h   PolygonTests/PolygonTestConvexPartitioning.cpp
    PolygonTests/PolygonTestMonotonePartitioning.h PolygonTests/PolygonTestMonotonePartitioning.cpp
    PolygonTests/PolygonTestConformingDelanay.h     PolygonTests/PolygonTestConformingDelanay.cpp

    SFSBuilder/SFSBuilder_TreeItem.h SFSBuilder/SFSBuilder_TreeItem.cpp
    SFSBuilder/SFSBuilder_panel.ui
    SFSBuilder/shapefromshade.cpp SFSBuilder/shapefromshade.h
    SFSBuilder/lineqsol.h SFSBuilder/lineqsol.cpp
    SFSBuilder/lapackcpp.h SFSBuilder/eigenval.cpp SFSBuilder/leastsquare.cpp

    ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(omi_triang
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET omi_triang APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(omi_triang SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(omi_triang
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_include_directories(omi_triang PRIVATE ${SPOOLES_INCLUDE_DIR})
target_compile_options(omi_triang PRIVATE "-D_TIMESPEC_DEFINED")

#left for debugging
#print_target_properties(CGAL::CGAL)
#print_target_properties(Qt5::windeployqt)

# copy qt dlls in the buld binary directory (for windows only)
if(CMAKE_HOST_WIN32)
    #  add_target_dll("Qt5::Widgets")
    #  add_target_dll("Qt5::Core")
    #  add_target_dll("Qt5::Gui")

    # constructing path to windeployqt to copy dlls
    string(REPLACE qmake.exe "windeployqt" windeployqt ${QT_QMAKE_EXECUTABLE} )
    message(STATUS "Found: ${windeployqt}")
    add_custom_command(TARGET omi_triang POST_BUILD
        COMMAND ${windeployqt} --list mapping  $<TARGET_FILE:omi_triang>
    )

    #copy some dlls from CGAL package
    string(REPLACE ".lib" ".dll" GMP_DLL "${GMP_LIBRARIES}")
    add_dll_by_path("${GMP_DLL}")
    foreach(PATHITEM ${FOR_COPY_TO_BINARY_DIR})
      add_dll_by_path("${PATHITEM}")
    endforeach()
endif()

target_link_libraries(omi_triang PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets CGAL::CGAL CGAL::CGAL_Core
    ${SPOOLES_LIB}
    ${pthread_LIB}
    ${LAPACK_LIB}
    ${BLAS_LIB}
)

set_target_properties(omi_triang PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(omi_triang)
endif()
