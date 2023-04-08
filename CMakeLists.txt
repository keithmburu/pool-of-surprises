cmake_minimum_required(VERSION 3.8)
project(collision-physics)

if (WIN32) # Include win64 platforms

  find_package(OpenGL REQUIRED)
  find_library(GLEW NAMES glew32s PATHS ./external/lib/x64)
  find_library(GLFW NAMES glfw3 PATHS ./external/lib)

  set(INCLUDE_DIRS
     ./src/agl
     ./src
     ./external/include )

  set(LIBRARY_DIRS
    lib
    ./external/lib)

  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_FLAGS 
     "/wd4018 /wd4244 /wd4305 /wd4312
     /D_CRT_SECURE_NO_WARNINGS 
     /D_CRT_NONSTDC_NO_DEPRECATE 
     /D NOMINMAX /DGLEW_STATIC
     /EHsc")

  set(VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
  set(CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB:\"MSVCRT\" /NODEFAULTLIB:\"LIBCMT\"")

  set(CORE ${GLEW} ${GLFW} opengl32.lib)
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
  set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)


elseif (APPLE)

  set(CMAKE_MACOSX_RPATH 1)
  set(CMAKE_CXX_FLAGS "-Wall -Wno-deprecated-declarations -Wno-unused-function -Wno-reorder-ctor -Wno-unused-variable -g -stdlib=libc++ -std=c++14")
  set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
  find_library(GL_LIB OpenGL)
  find_library(GLFW glfw)
  add_definitions(-DAPPLE)

  set(INCLUDE_DIRS
    /System/Library/Frameworks
     ./src/agl
     ./src
    ./external/include
    /usr/local/include)

  set(CORE ${GLFW} ${GL_LIB})

elseif (UNIX)

  set(OpenGL_GL_PREFERENCE  "GLVND")
  set(CMAKE_CXX_FLAGS "-Wall -g -std=c++14 -Wno-comment -Wno-sign-compare -Wno-reorder -Wno-unused-function")
  set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

  FIND_PACKAGE(OpenGL REQUIRED) 
  FIND_PACKAGE(GLEW REQUIRED)

  set(INCLUDE_DIRS
    /usr/local/include
     ./src/agl
     ./src
    ./external/include)

  set(LIBRARY_DIRS
    ./external/lib
    /usr/X11R6/lib
    /usr/local/lib
    lib)

  add_definitions(-DUNIX)
  set(CORE GLEW glfw GL X11)

endif()

include_directories(${INCLUDE_DIRS})
link_directories(${LIBRARY_DIRS})

file(GLOB SOURCES "src/agl/*.h" "src/agl/*.cpp" "src/agl/mesh/*.h" "src/agl/mesh/*.cpp") 

set(SHADERS
    shaders/billboard-animated.fs
    shaders/billboard-animated.vs
    shaders/simple-texture.fs
    shaders/simple-texture.vs
    shaders/billboard.fs
    shaders/billboard.vs)

add_executable(collision-physics src/collision-physics.cpp ${SOURCES} ${SHADERS})
target_link_libraries(collision-physics ${CORE})

if (WIN32)
  source_group("shaders" FILES ${SHADERS})
  source_group("agl" FILES ${SOURCES})
endif()
