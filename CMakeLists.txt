cmake_minimum_required(VERSION 4.3)
project(Pong)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(vendor/SDL3 EXCLUDE_FROM_ALL)
add_subdirectory(vendor/glm)

add_executable(Pong main.cpp)

target_link_libraries(Pong PRIVATE SDL3::SDL3 glm::glm)

add_custom_command(TARGET Pong POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE_DIR:Pong>/vendor/SDL3/SDL3.dll"
        $<TARGET_FILE_DIR:Pong>
)