cmake_minimum_required(VERSION 3.24)
project(trymix VERSION 0.1 LANGUAGES CXX)

if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()


if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

###################################################################################
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/config.cmake")
  include("${CMAKE_CURRENT_LIST_DIR}/config.cmake")
else()
  message(FATAL_ERROR
    "You have to prepare config.cmake file for your project. Exitting now...
    ###################################################################################
    # consider to specify cmake variables in cmake configuration file
    # config.cmake. I.e.:
    set(WINDOWS_EXTERNAL_LIBS_PATH D:/trymix/test_omi;D:/zero-devel)
    # this is directories where <PackageName>Config.cmake file placed
    # you may search it with command, i.e.
    # find /d/zero-devel -name BoostConfig.cmake
    set(CUDAToolkit_ROOT C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.5/bin)
    set(Boost_ROOT D:/zero-devel/boost_1_87_0/lib64-msvc-14.3/lib/cmake/Boost-1.87.0)
    set(CGAL_ROOT D:/zero-devel/CGAL-6.0.1/lib/cmake/CGAL)
    set(glm_ROOT D:/zero-devel/glm/install/lib/cmake/glm)
    set(Eigen3_ROOT D:/zero-devel/eigen3/eigen-3.4.0_build)
    "
  )
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(this_project_utils)

# path to thirdparty dirs (specify at at command line see top of the file)
if(DEFINED WINDOWS_EXTERNAL_LIBS_PATH)
  message(STATUS "WINDOWS_EXTERNAL_LIBS_PATH = ${WINDOWS_EXTERNAL_LIBS_PATH}" )
else()
  message(FATAL_ERROR "Please, define WINDOWS_EXTERNAL_LIBS_PATH varable")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(-Wno-overriding-option)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # options for debugging in RelWithDebInfo (it seems same as RelWithDebInfo)
    #set(RELEASE_FOR_DEBUG_CXX_OPTIONS /JMC /Zi /Od /MD)

    # removing O2 option it will be replaced with /Od
    string(REPLACE "/O2" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
    # message(FATAL_ERROR ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
    set(RELEASE_FOR_DEBUG_CXX_OPTIONS /Zi /Od /MD)
    set(RELEASE_FOR_DEBUG_LINKER_OPTIONS /DEBUG:FULL /OPT:REF /OPT:ICF /INCREMENTAL:NO /IGNORE:4099)
endif()


find_package(glm REQUIRED)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
message(STATUS "found Qt${QT_VERSION_MAJOR}")
set(QtVer Qt${QT_VERSION_MAJOR})

find_package(${QtVer} REQUIRED COMPONENTS OpenGL Widgets OpenGLWidgets  LinguistTools)

################### Boost #######################

find_package(Boost 1.86 REQUIRED)

################### OpenGL ######################

find_package(OpenGL REQUIRED)

################### OpenGL ######################
# example to specify version to locate: CUDAToolkit_ROOT = "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.1"

find_package(CUDAToolkit 12)
if(${CUDAToolkit_FOUND})
  message(STATUS CUDAToolkit_FOUND=${CUDAToolkit_FOUND})
  add_compile_definitions(OpenCL_FOUND CUDA_FOUND CL_TARGET_OPENCL_VERSION=300)
else()
  find_package(OpenCL 3.0 REQUIRED)
  if(OpenCL_FOUND)
    add_compile_definitions(OpenCL_FOUND CL_TARGET_OPENCL_VERSION=300)
  endif()
endif()


##################### CGAL ######################

#set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)
find_package(CGAL REQUIRED COMPONENTS Core)
# example to copy dlls if necessary
if(WIN32)
file(GLOB_RECURSE CGAL_DLLS "${CGAL_ROOT}/*.dll")
list(APPEND FOR_COPY_TO_BINARY_DIR ${CGAL_DLLS})
endif()

################# FREEIMAGE_LIB #################

IF(UNIX OR MINGW)
  FIND_PATH(FREEIMAGE_INCLUDE_DIR FreeImage.h )
  FIND_LIBRARY(FREEIMAGE_LIB NAMES freeimage FreeImage)
ELSEIF(WIN32)
    FIND_PATH(FREEIMAGE_INCLUDE_DIR FreeImage.h REQUIRED
        PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} PATH_SUFFIXES "FreeImage/Dist/x64")
    FIND_LIBRARY(FREEIMAGE_LIB REQUIRED NAMES FreeImage PATHS ${FREEIMAGE_INCLUDE_DIR})
    FIND_FILE(FREEIMAGE_DLL FreeImage.dll HINTS ${FREEIMAGE_INCLUDE_DIR} )
    list(APPEND FOR_COPY_TO_BINARY_DIR ${FREEIMAGE_DLL})
ENDIF(UNIX OR MINGW)


####################### spooles ##################

if(WIN32)
  FIND_LIBRARY(SPOOLES_LIB spooles REQUIRED PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} PATH_SUFFIXES spooles_llvm_build/RelWithDebInfo/src)
  FIND_PATH(SPOOLES_INCLUDE_DIR InpMtx.h PATHS ${WINDOWS_EXTERNAL_LIBS_PATH} PATH_SUFFIXES spooles/include NO_CMAKE_PATH)
else(UNIX)
  FIND_LIBRARY(SPOOLES_LIB spooles)
  FIND_PATH(SPOOLES_INCLUDE_DIR InpMtx.h PATH_SUFFIXES spooles/include)
endif(WIN32)

####################### Eigen ##################

find_package(Eigen3 3.4 REQUIRED NO_MODULE)

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

list(APPEND FOR_COPY_TO_BINARY_DIR ${CMAKE_SOURCE_DIR}/torch.3dIco)
list(APPEND FOR_COPY_TO_BINARY_DIR ${CMAKE_SOURCE_DIR}/trymix.ico)


set(TS_FILES trymix_en_GB.ts)

set(PROJECT_SOURCES
    main.cpp
    CommonComponents/mainwindow.cpp
    CommonComponents/mainwindow.h
    CommonComponents/mainwindow.ui
    CommonComponents/CentralWidget.cpp
    CommonComponents/CentralWidget.h
    CommonComponents/CentralWidget.ui

    CommonComponents/ProjectTree.h CommonComponents/ProjectTree.cpp
    CommonComponents/Projects_TreeItem.h CommonComponents/Projects_TreeItem.cpp

    CommonComponents/EventHandling.h CommonComponents/EventHandling.cpp
    CommonComponents/EventParser.h CommonComponents/EventParser.cpp

    CommonComponents/testScene.cpp CommonComponents/testScene.h

    CommonComponents/drawContext.h CommonComponents/drawContext.cpp

    CommonComponents/viewctrl.h CommonComponents/viewctrl.cpp

    #2023-12-12:16:45:26 (obsolete left for reference) apputil/serializer.cpp apputil/serializer.h
    apputil/serializerV2.cpp apputil/serializerV2.h
    apputil/storagestreamjson.h apputil/storagestreamjson.cpp
    apputil/storagestreamsimplexml.h apputil/storagestreamsimplexml.cpp
    apputil/storagestreamindexedbinary.h apputil/storagestreamindexedbinary.cpp
    apputil/base64encoder.cpp apputil/base64encoder.h
    apputil/compatibility.cpp apputil/compatibility.h

    mathlib/lapackcpp/lapackcpp.h
    mathlib/lapackcpp/invert.cpp
    mathlib/lapackcpp/eigenval.cpp
    mathlib/lapackcpp/leastsquare.cpp

    PolygonTests/PolygonTests_TreeItem.cpp PolygonTests/PolygonTests_TreeItem.h
    PolygonTests/PolygonTests_panel.ui
    PolygonTests/PolygonTestConvexPartitioning.h   PolygonTests/PolygonTestConvexPartitioning.cpp
    PolygonTests/PolygonTestMonotonePartitioning.h PolygonTests/PolygonTestMonotonePartitioning.cpp
    PolygonTests/PolygonTestConformingDelanay.h     PolygonTests/PolygonTestConformingDelanay.cpp

    SFSBuilder/SFSBuilder_TreeItem.h SFSBuilder/SFSBuilder_TreeItem.cpp
    SFSBuilder/SFSBuilder_panel.ui
    SFSBuilder/shapefromshade.cpp SFSBuilder/shapefromshade.h
    SFSBuilder/lineqsol.h SFSBuilder/lineqsol.cpp
    SFSBuilder/lineqsol2.h SFSBuilder/lineqsol2.cpp

    SFSBuilder/imageplane.h SFSBuilder/imageplane.cpp
    CommonComponents/ImageTile.h CommonComponents/ImageTile.cpp
    CommonComponents/glhelper.h CommonComponents/glhelper.cpp

    CommonComponents/lights.h CommonComponents/lights.cpp
    CommonComponents/toolspanel.h CommonComponents/toolspanel.cpp
    CommonComponents/mousectrl.h CommonComponents/mousectrl.cpp
    CommonComponents/mathstruct.h CommonComponents/mathstruct.cpp


    CameraControl/CameraControl_TreeItem.h CameraControl/CameraControl_TreeItem.cpp
    CameraControl/CameraControl_panel.ui

    ShaderMix/ShaderMix_TreeItem.h ShaderMix/ShaderMix_TreeItem.cpp
    ShaderMix/ShaderMix_panel.ui

    BlurTests/BlurTests_TreeItem.cpp BlurTests/BlurTests_TreeItem.h
    BlurTests/BlurTests_Panel.ui
    BlurTests/BlurTests.cpp BlurTests/BlurTests.cpp
    BlurTests/BoxBlur.cpp BlurTests/BoxBlur.h
    BlurTests/GaussBlurEngine.cpp BlurTests/GaussBlurEngine.h

    trymix.qrc

    ${TS_FILES}
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    enable_language("RC")
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/trymix.rc")
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(trymix
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${APP_ICON_RESOURCE_WINDOWS}
        history.md
    )

# Define target properties for Android with Qt 6 as:
#    set_property(TARGET trymix APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(trymix SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(trymix
            ${PROJECT_SOURCES}
            ${APP_ICON_RESOURCE_WINDOWS}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_include_directories(trymix PRIVATE
    ${SPOOLES_INCLUDE_DIR}
    ${FREEIMAGE_INCLUDE_DIR}
    ${pthread_INCLUDE_DIR}
)

target_compile_definitions(trymix PRIVATE "-D_TIMESPEC_DEFINED ")
target_compile_options(trymix PRIVATE
  $<$<CONFIG:RelWithDebInfo>:${RELEASE_FOR_DEBUG_CXX_OPTIONS}>
)

target_link_options(trymix PRIVATE
  $<$<CONFIG:RelWithDebInfo>:${RELEASE_FOR_DEBUG_LINKER_OPTIONS}>
)

#left example for debugging checking properties on target
#print_target_properties(CGAL::CGAL)
#print_target_properties(Qt5::windeployqt)

# copy qt dlls in the buld binary directory (for windows only)
if(CMAKE_HOST_WIN32)
    # constructing path to windeployqt to copy dlls
    string(REPLACE qmake.exe "windeployqt" windeployqt ${QT_QMAKE_EXECUTABLE} )
    if(windeployqt)
        message(STATUS "Found: ${windeployqt} distributing qt dlls")
        add_custom_command(TARGET trymix POST_BUILD
            COMMAND ${windeployqt} --list mapping  $<TARGET_FILE:trymix>
        )
    endif()

    #copy dlls from other packages
    foreach(PATHITEM ${FOR_COPY_TO_BINARY_DIR})
      add_dll_by_path("${PATHITEM}")
    endforeach()
endif()

target_link_libraries(trymix PRIVATE
    ${QtVer}::Widgets
    ${QtVer}::OpenGL
    ${QtVer}::OpenGLWidgets

    CGAL::CGAL CGAL::CGAL_Core
    ${SPOOLES_LIB}
    ${pthread_LIB}
    ${LAPACK_LIB}
    ${BLAS_LIB}
    ${FREEIMAGE_LIB}
    OpenGL::GL    
    glm::glm
    CUDA::OpenCL
    Eigen3::Eigen
)

set_target_properties(trymix PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(trymix)
endif()
