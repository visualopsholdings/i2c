#   CMakeLists.txt
#
#   Author: Paul Hamilton (paul@visualops.com)
#   Date: 5-May-2023
#
#   Input file for TinyLogo etc for cmake
#
#   This work is licensed under the Creative Commons Attribution 4.0 International License. 
#   To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/ or 
#   send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
#
#   https://github.com/visualopsholdings/i2c

cmake_minimum_required(VERSION 3.5)

project (flashcmd)
  find_package(Boost COMPONENTS unit_test_framework REQUIRED)
  add_definitions(-DBOOST_ALL_DYN_LINK) 
  enable_testing()

add_executable(RBTest rbtest.cpp ../ringbuffer.cpp)
  add_definitions(-DRINGBUF_SIZE_MINUS_1=8) 
  target_link_libraries(RBTest ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_executable(CmdTest cmdtest.cpp ../ringbuffer.cpp ../cmd.cpp)
  add_definitions(-DRINGBUF_SIZE_MINUS_1=8 -DCMD_SIZE=20) 
  target_link_libraries(CmdTest ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_executable(LGTest lgtest.cpp ../logo.cpp)
  target_link_libraries(LGTest ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_executable(LGTestErr lgtesterr.cpp ../logo.cpp)
  target_link_libraries(LGTestErr ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_executable(LGTestWords lgtestwords.cpp ../logo.cpp)
  target_link_libraries(LGTestWords ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_executable(LGTestSketch lgtestsketch.cpp ../logo.cpp)
  target_link_libraries(LGTestSketch ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_test(CmdTest CmdTest)
add_test(RBTest RBTest)
add_test(LGTest LGTest)
add_test(LGTestErr LGTestErr)
add_test(LGTestWords LGTestWords)
add_test(LGTestSketch LGTestSketch)
