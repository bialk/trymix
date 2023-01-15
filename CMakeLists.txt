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


#set(glm_DIR <installation prefix>/lib/cmake/glm) # if necessary
find_path_by_regex(glm_DIR "glm/install/lib/cmake/glm" PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} )

find_package(glm REQUIRED)

set(QT_VERSION_MAJOR 6)

#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS OpenGL Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS OpenGL Widgets OpenGLWidgets  LinguistTools)

find_path_by_regex(BOOST_ROOT "boost_*[0-9]_*[0-9]_*[0-9]" PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} )

################### Boost #######################

find_package(Boost REQUIRED)

################### OpenGL ######################

find_package(OpenGL REQUIRED)

##################### CGAL ######################

set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)
find_package(CGAL REQUIRED PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} PATH_SUFFIXES "CGAL-5.5.1" COMPONENTS Core)
string(REPLACE ".lib" ".dll" GMP_DLL "${GMP_LIBRARIES}")
list(APPEND FOR_COPY_TO_BINARY_DIR ${GMP_DLL})

################# FREEIMAGE_LIB #################

IF(UNIX OR MINGW)
  FIND_PATH(FREEIMAGE_INCLUDE_DIR FreeImage.h )
  FIND_LIBRARY(FREEIMAGE_LIB NAMES freeimage FreeImage)
ELSEIF(WIN32)
#  FIND_PATH(FREEIMAGE_INCLUDE_DIR FreeImage.h PATH_SUFFIXES FreeImage/Dist)
#  FIND_LIBRARY(FREEIMAGE_LIB FreeImage HINTS ${FREEIMAGE_INCLUDE_DIR} )
#  FIND_FILE(FREEIMAGE_DLL FreeImage.dll HINTS ${FREEIMAGE_INCLUDE_DIR} )
#  list(APPEND FOR_COPY_TO_BINARY_DIR ${FREEIMAGE_DLL})
    FIND_PATH(FREEIMAGE_INCLUDE_DIR FreeImage.h REQUIRED
        PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} PATH_SUFFIXES "FreeImage/Dist/x64")
    FIND_LIBRARY(FREEIMAGE_LIB REQUIRED NAMES FreeImage PATHS ${FREEIMAGE_INCLUDE_DIR})
    FIND_FILE(FREEIMAGE_DLL FreeImage.dll HINTS ${FREEIMAGE_INCLUDE_DIR} )
    list(APPEND FOR_COPY_TO_BINARY_DIR ${FREEIMAGE_DLL})
ENDIF(UNIX OR MINGW)


####################### spooles ##################

if(WIN32)
  FIND_LIBRARY(SPOOLES_LIB spooles REQUIRED PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} PATH_SUFFIXES spooles-debug/src)
  FIND_PATH(SPOOLES_INCLUDE_DIR InpMtx.h PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} PATH_SUFFIXES spooles/include NO_CMAKE_PATH)
else(UNIX)
  FIND_LIBRARY(SPOOLES_LIB spooles)
  FIND_PATH(SPOOLES_INCLUDE_DIR InpMtx.h PATH_SUFFIXES spooles/include)
endif(WIN32)

####################### Eigen ##################
#find_path(EIGEN_INCLUDE_DIRS NAMES Sparse PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} PATH_SUFFIXES ceres/eigen-eigen-5a0156e40feb/Eigen)
#find_package(Eigen REQUIRED)
#if(EIGEN_FOUND)
#   list(APPEND CERES_INCLUDE_DIRS "${EIGEN_INCLUDE_DIRS}")
#endif()

################# pthread #########################

if(MINGW)
elseif(WIN32)
    find_path_by_regex(pthread_DIR "pthreads-w32-*[0-9]-*[0-9]-*[0-9]" PATHS ${WINDOWS_EXTERNAL_LIBS_PATH})

    FIND_PATH(pthread_INCLUDE_DIR pthread.h
      HINTS ${pthread_DIR} PATH_SUFFIXES Pre-built.2/include
      REQUIRED
    )

    FIND_LIBRARY(pthread_LIB pthreadVC2 REQUIRED
      HINTS ${pthread_DIR}/Pre-built.2/lib/x64
      REQUIRED
    )

    FIND_FILE(pthread_DLL Pre-built.2/dll/x64/pthreadVC2.dll
      HINTS ${pthread_DIR}
      REQUIRED
    )

    list(APPEND FOR_COPY_TO_BINARY_DIR ${pthread_DLL})    
endif(MINGW)

################# LAPACK #########################
if(UNIX OR MINGW)
  FIND_LIBRARY(LAPACK_LIB lapack)
  FIND_LIBRARY(BLAS_LIB blas)
elseif(WIN32)
  FIND_LIBRARY(LAPACK_LIB lapack REQUIRED PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} PATH_SUFFIXES lapack)
  FIND_LIBRARY(BLAS_LIB blas REQUIRED PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} PATH_SUFFIXES lapack)
  find_path_by_regex(LAPACK_DLLS "lapack/*.dll" PATHS ${WINDOWS_EXTERNAL_LIBS_PATH})
  list(APPEND FOR_COPY_TO_BINARY_DIR ${LAPACK_DLLS})
endif()
  list(APPEND FOR_COPY_TO_BINARY_DIR ${CMAKE_SOURCE_DIR}/torch.ico)
  list(APPEND FOR_COPY_TO_BINARY_DIR ${CMAKE_SOURCE_DIR}/omi_triang.ico)


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
    SFSBuilder/mathlib/lapackcpp/lapackcpp.h
    SFSBuilder/mathlib/lapackcpp/invert.cpp
    SFSBuilder/mathlib/lapackcpp/eigenval.cpp
    SFSBuilder/mathlib/lapackcpp/leastsquare.cpp
    SFSBuilder/imageplane.h SFSBuilder/imageplane.cpp
    SFSBuilder/editviewobj.h SFSBuilder/editviewobj.cpp
    SFSBuilder/ImageTile.h SFSBuilder/ImageTile.cpp
    SFSBuilder/editviewobj.h SFSBuilder/editviewobj.cpp
    SFSBuilder/glhelper.h SFSBuilder/glhelper.cpp
#    SFSBuilder/eventhnd.h SFSBuilder/eventhnd.cpp

    SFSBuilder/lights.h SFSBuilder/lights.cpp
    #SFSBuilder/dispview.h SFSBuilder/dispview.cpp
    SFSBuilder/toolspanel.h SFSBuilder/toolspanel.cpp
    SFSBuilder/savejpeg.h SFSBuilder/savejpeg.cpp
    SFSBuilder/mousectrl.h SFSBuilder/mousectrl.cpp
    SFSBuilder/viewctrl.h SFSBuilder/viewctrl.cpp
    SFSBuilder/mathstruct.h SFSBuilder/mathstruct.cpp

    SFSBuilder/apputil/AppSysSet.h SFSBuilder/apputil/AppSysSet.cpp

    EventHandling.h EventHandling.cpp
    EventParser.h EventParser.cpp

    omi_triang.qrc

    ${TS_FILES}
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    enable_language("RC")
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/omi_triang.rc")
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(omi_triang
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${APP_ICON_RESOURCE_WINDOWS}
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
            ${APP_ICON_RESOURCE_WINDOWS}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_include_directories(omi_triang PRIVATE
    ${SPOOLES_INCLUDE_DIR}
    ${FREEIMAGE_INCLUDE_DIR}
    ${pthread_INCLUDE_DIR}
    ${EIGEN_INCLUDE_DIRS}
)

target_compile_options(omi_triang PRIVATE "-D_TIMESPEC_DEFINED")

#left example for debugging checking properties on target
#print_target_properties(CGAL::CGAL)
#print_target_properties(Qt5::windeployqt)

# copy qt dlls in the buld binary directory (for windows only)
if(CMAKE_HOST_WIN32)
    #  add_target_dll("Qt5::Widgets")
    #  add_target_dll("Qt5::Core")
    #  add_target_dll("Qt5::Gui")

    # constructing path to windeployqt to copy dlls
    string(REPLACE qmake.exe "windeployqt" windeployqt ${QT_QMAKE_EXECUTABLE} )
    if(windeployqt)
        message(STATUS "Found: ${windeployqt} distributing qt dlls")
        add_custom_command(TARGET omi_triang POST_BUILD
            COMMAND ${windeployqt} --list mapping  $<TARGET_FILE:omi_triang>
        )
    endif()

    #copy dlls from other packages
    foreach(PATHITEM ${FOR_COPY_TO_BINARY_DIR})
      add_dll_by_path("${PATHITEM}")
    endforeach()
endif()

target_link_libraries(omi_triang PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets

    CGAL::CGAL CGAL::CGAL_Core
    ${SPOOLES_LIB}
    ${pthread_LIB}
    ${LAPACK_LIB}
    ${BLAS_LIB}
    ${FREEIMAGE_LIB}
    OpenGL::GL    
    glm::glm
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
