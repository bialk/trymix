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

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)
find_package(CGAL COMPONENTS Core)

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
        Projects_TreeItem.h Projects_TreeItem.cpp
        PolygonTests/PolygonTests_TreeItem.cpp PolygonTests/PolygonTests_TreeItem.h
        ProjectTree.h ProjectTree.cpp
        PolygonTests/PolygonTests_panel.ui
        PolygonTests/PolygonTestConvexPartitioning.h   PolygonTests/PolygonTestConvexPartitioning.cpp
        PolygonTests/PolygonTestMonotonePartitioning.h PolygonTests/PolygonTestMonotonePartitioning.cpp
        PolygonTests/PolygonTestConformingDelanay.h     PolygonTests/PolygonTestConformingDelanay.cpp
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

#left for debugging
#print_target_properties(CGAL::CGAL)
#print_target_properties(Qt5::windeployqt)

# copy qt dlls in the buld binary directory for windows only
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
endif()


target_link_libraries(omi_triang PRIVATE Qt${QT_VERSION_MAJOR}::Widgets CGAL::CGAL CGAL::CGAL_Core)

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
