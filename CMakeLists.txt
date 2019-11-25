cmake_minimum_required (VERSION 3.14)
project(CD CXX)
include(Config.cmake)

set(CMAKE_CXX_COMPILER "g++")
set(EXECUTABLE_OUTPUT_PATH /usr/bin)
# set llvm options
set(LLVMConfig llvm-config-6.0)
llvm_config(GET_INCDIR LLVMIncDir GET_LIBDIR LLVMLibDir)

# common compile options
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${LLVMIncDir})
link_directories(${LLVMLibDir})

add_clang_target(
  codesim FILES "src/*.cpp" DEPENDS
  clangTooling
  clangToolingASTDiff
  clangFrontend
  clangDriver
  clangSerialization
  clangParse
  clangSema
  clangAnalysis
  clangEdit
  clangAST
  clangLex
  clangBasic
  clang
  LLVM
)
