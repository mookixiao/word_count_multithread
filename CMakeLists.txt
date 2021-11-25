cmake_minimum_required(VERSION 3.16)

project(word_count_multithread)

add_executable(word_count_mt main.c)
add_executable(test test.c)

target_link_libraries(
        word_count_mt
        pthread
)

target_link_libraries(
        test
        pthread
)