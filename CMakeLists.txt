cmake_minimum_required(VERSION 3.20)

project(
    PerfectValueCubeHash
    VERSION 0.6.0
    DESCRIPTION "Perfect Value Cube hash and foldback separation cryptanalysis framework"
    LANGUAGES CXX
)

option(PVC_BUILD_TESTS "Build the test suite" ON)
option(PVC_BUILD_TOOLS "Build cryptanalysis tools" ON)
option(PVC_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(PVC_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

find_package(Threads REQUIRED)

add_library(pvc_project_options INTERFACE)
target_compile_features(pvc_project_options INTERFACE cxx_std_20)

if(MSVC)
    target_compile_options(pvc_project_options INTERFACE /W4 /permissive-)
    if(PVC_WARNINGS_AS_ERRORS)
        target_compile_options(pvc_project_options INTERFACE /WX)
    endif()
else()
    target_compile_options(
        pvc_project_options
        INTERFACE -Wall -Wextra -Wpedantic -Wconversion -Wshadow
    )
    if(PVC_WARNINGS_AS_ERRORS)
        target_compile_options(pvc_project_options INTERFACE -Werror)
    endif()
endif()

if(PVC_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(
        pvc_project_options
        INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer
    )
    target_link_options(
        pvc_project_options
        INTERFACE -fsanitize=address,undefined
    )
endif()

add_library(pvc_hash
    src/cube.cpp
    src/engine.cpp
    src/hash.cpp
    src/hex.cpp
    src/research.cpp
)
add_library(PVC::Hash ALIAS pvc_hash)

target_include_directories(pvc_hash
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(pvc_hash PUBLIC cxx_std_20)
target_link_libraries(pvc_hash PRIVATE pvc_project_options)

function(pvc_add_executable target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE PVC::Hash pvc_project_options)
endfunction()

pvc_add_executable(pvc-hash app/main.cpp)

if(PVC_BUILD_TOOLS)
    pvc_add_executable(pvc-avalanche tools/avalanche.cpp)
    pvc_add_executable(pvc-avalanche-sweep tools/avalanche_sweep.cpp)
    pvc_add_executable(pvc-collision-probe tools/collision_probe.cpp)
    pvc_add_executable(pvc-distribution-probe tools/distribution_probe.cpp)
    pvc_add_executable(pvc-state-bias-probe tools/state_bias_probe.cpp)
    pvc_add_executable(pvc-structure-probe tools/structure_probe.cpp)
    pvc_add_executable(pvc-reduced-round-probe tools/reduced_round_probe.cpp)
    pvc_add_executable(pvc-transition-collision tools/transition_collision.cpp)
    pvc_add_executable(pvc-truncated-collision tools/truncated_collision.cpp)
    pvc_add_executable(pvc-differential-search tools/differential_search.cpp)
    pvc_add_executable(pvc-predecessor-enumerator tools/predecessor_enumerator.cpp)
    pvc_add_executable(pvc-related-input-probe tools/related_input_probe.cpp)
    pvc_add_executable(pvc-phase-collision tools/phase_collision.cpp)
    pvc_add_executable(pvc-foldback-merge-search tools/foldback_merge_search.cpp)
    pvc_add_executable(pvc-three-byte-collision tools/three_byte_collision.cpp)
    target_link_libraries(pvc-three-byte-collision PRIVATE Threads::Threads)
    pvc_add_executable(pvc-alignment-probe tools/alignment_probe.cpp)
    pvc_add_executable(pvc-constrained-merge-search tools/constrained_merge_search.cpp)
    pvc_add_executable(pvc-foldback-aware-alias tools/foldback_aware_alias.cpp)
    target_link_libraries(pvc-foldback-aware-alias PRIVATE Threads::Threads)
    pvc_add_executable(pvc-multicollision-probe tools/multicollision_probe.cpp)
    target_link_libraries(pvc-multicollision-probe PRIVATE Threads::Threads)
    pvc_add_executable(pvc-foldback-separation-profile tools/foldback_separation_profile.cpp)
    target_link_libraries(pvc-foldback-separation-profile PRIVATE Threads::Threads)
    pvc_add_executable(pvc-independent-suffix-catalog tools/independent_suffix_catalog.cpp)
    target_link_libraries(pvc-independent-suffix-catalog PRIVATE Threads::Threads)
    pvc_add_executable(pvc-bridged-multicollision tools/bridged_multicollision.cpp)
    target_link_libraries(pvc-bridged-multicollision PRIVATE Threads::Threads)
endif()

if(PVC_BUILD_TESTS)
    include(CTest)
    enable_testing()

    pvc_add_executable(pvc-tests tests/test_main.cpp)
    add_test(NAME pvc-tests COMMAND pvc-tests)
endif()
