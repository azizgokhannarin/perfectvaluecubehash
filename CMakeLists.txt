cmake_minimum_required(VERSION 3.20)

project(
    PerfectValueCubeHash
    VERSION 0.1.0
    DESCRIPTION "Experimental rotation-only hash construction based on the Perfect Value Cube"
    LANGUAGES CXX
)

option(PVC_BUILD_TESTS "Build the test suite" ON)
option(PVC_BUILD_TOOLS "Build cryptanalysis tools" ON)
option(PVC_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(PVC_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

add_library(pvc_hash
    src/cube.cpp
    src/hash.cpp
    src/hex.cpp
)

add_library(PVC::Hash ALIAS pvc_hash)

target_include_directories(pvc_hash
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(pvc_hash PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(pvc_hash PRIVATE /W4 /permissive-)
    if(PVC_WARNINGS_AS_ERRORS)
        target_compile_options(pvc_hash PRIVATE /WX)
    endif()
else()
    target_compile_options(pvc_hash PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    if(PVC_WARNINGS_AS_ERRORS)
        target_compile_options(pvc_hash PRIVATE -Werror)
    endif()
endif()

if(PVC_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(pvc_hash PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(pvc_hash PUBLIC -fsanitize=address,undefined)
endif()

add_executable(pvc-hash app/main.cpp)
target_link_libraries(pvc-hash PRIVATE PVC::Hash)

if(PVC_BUILD_TOOLS)
    add_executable(pvc-avalanche tools/avalanche.cpp)
    target_link_libraries(pvc-avalanche PRIVATE PVC::Hash)

    add_executable(pvc-collision-probe tools/collision_probe.cpp)
    target_link_libraries(pvc-collision-probe PRIVATE PVC::Hash)

    add_executable(pvc-structure-probe tools/structure_probe.cpp)
    target_link_libraries(pvc-structure-probe PRIVATE PVC::Hash)
endif()

if(PVC_BUILD_TESTS)
    include(CTest)
    enable_testing()

    add_executable(pvc-tests tests/test_main.cpp)
    target_link_libraries(pvc-tests PRIVATE PVC::Hash)
    add_test(NAME pvc-tests COMMAND pvc-tests)
endif()
