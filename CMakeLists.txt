cmake_minimum_required(VERSION 3.11)
set(PROJECT_NAME project_base)
project(${PROJECT_NAME})

function(watch)
    set_property(
            DIRECTORY
            APPEND
            PROPERTY CMAKE_CONFIGURE_DEPENDS ${ARGV}
    )
endfunction()

set(CMAKE_POLICY_DEFAULT_CMP0012 NEW)
set(CMAKE_CXX_STANDARD 14)

list(APPEND CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -O3")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

file(GLOB SOURCES "src/*.cpp" "src/*.c" src/main.cpp)
file(GLOB HEADERS "include/*.h" "include/*.hpp")

find_package(OpenGL REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(ASSIMP REQUIRED)

add_subdirectory(libs/glad)
add_subdirectory(libs/imgui)

add_definitions(${OPENGL_DEFINITIONS})

add_library(STB_IMAGE libs/stb_image.cpp)
set_source_files_properties(libs/stb_image.cpp include/stb_image.h
        PROPERTIES
        COMPILE_FLAGS
        "-Wno-shift-negative-value -Wno-implicit-fallthrough")

set(LIBS glfw glad OpenGL::GL X11 Xrandr Xinerama Xi Xxf86vm Xcursor dl pthread freetype ${ASSIMP_LIBRARIES} STB_IMAGE imgui)


configure_file(configuration/root_directory.h.in configuration/root_directory.h)
include_directories(${CMAKE_BINARY_DIR}/configuration)

find_program(CLANG_TIDY_EXE NAMES clang-tidy)
if(CLANG_TIDY_EXE)
    message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-fix;-extra-arg=--std=c++14")
endif()

include_directories(include/)
add_executable(${PROJECT_NAME}
        ${SOURCES}
        ${HEADERS})

find_program(LCOV lcov)
if(LCOV AND CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "Found lcov: ${LCOV}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    include(CodeCoverage)
    SETUP_TARGET_FOR_COVERAGE_LCOV(NAME coverage EXECUTABLE ${PROJECT_NAME})
endif()

target_link_libraries(${PROJECT_NAME} ${LIBS})

# set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${PROJECT_NAME}")
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")
file(GLOB SHADERS "resources/shaders/*.vert"
        "resources/shaders/*.frag")
foreach(SHADER ${SHADERS})
    # file(COPY ${SHADER} DESTINATION ${CMAKE_SOURCE_DIR}/bin/${PROJECT_NAME}/shaders)
    watch(${SHADER})
endforeach()

find_program(GLSLANG_VALIDATOR glslangValidator)
if(GLSLANG_VALIDATOR)
    message(STATUS "Found glslangValidator: ${GLSLANG_VALIDATOR}")

    add_custom_target(compile_shaders ALL DEPENDS ${SHADERS})

    foreach(SHADER ${SHADERS})
        add_custom_command(
                TARGET compile_shaders
                COMMAND ${GLSLANG_VALIDATOR} ${SHADER}
                COMMENT "Validating ${SHADER}"
        )
    endforeach()
endif()
