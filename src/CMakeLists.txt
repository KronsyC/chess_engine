cmake_minimum_required(VERSION 3.23)


project(chess )


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

if(X32)
  set(CMAKE_CXX_FLAGS -m32)
endif()

add_compile_options(-O3)
if(EXE)
  add_executable(chess ./main.cpp  ./game.h game.tcc ./pseudolegal_move_calculator.h ./pseudolegal_move_calculator.cc ./error.h  ./magic/moves.cc ./magic/moves.h ./bitboard.cc ./bitboard.h ./api.h ./api.cc agents/weighted.h agents/random.h ./agent.h game.cpp)
else()
  add_library(chess SHARED  ./game.h game.tcc ./pseudolegal_move_calculator.h ./pseudolegal_move_calculator.cc ./error.h  ./magic/moves.cc ./magic/moves.h ./bitboard.cc ./bitboard.h ./api.h ./api.cc agents/weighted.h agents/random.h ./agent.h game.cpp)

endif()

if(WIN32)
  message(STATUS "Compiling for windows")
  target_link_libraries(chess -static)
elseif(APPLE)
  message(STATUS "Compiling for MacOS")
  target_link_libraries(chess -static)

else()
  message(STATUS "Compiling for non-macos unix based system")

endif()

