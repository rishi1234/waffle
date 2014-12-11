
project(waffle CXX)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-std=c++11")
find_package(Boost 1.55.0 COMPONENTS filesystem system REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
# For my mac...
link_directories(/opt/local/lib)

add_subdirectory(lang)

add_executable(waffle
  main.cpp
  token.cpp 
  ast.cpp 
  scope.cpp
  language.cpp
  lexer.cpp
  syntax.cpp
  parser.cpp
  elab.cpp
  type.cpp
  value.cpp
  subst.cpp
  eval.cpp
  same.cpp
  less.cpp
  size.cpp)
target_link_libraries(waffle waffle-support)
target_link_libraries(waffle ${Boost_LIBRARIES})
