cmake_minimum_required(VERSION 3.20)
project(shell C)

set(CMAKE_C_STANDARD 99)

add_executable(shell driver.c list.c str.c process.c parser.c token.c launcher.c)
