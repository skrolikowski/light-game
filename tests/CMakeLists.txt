include(FetchContent)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz
)

# Keep gtest's runtime settings consistent with the rest of the build.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

add_executable(piece_test "PieceTest.cpp")
target_link_libraries(piece_test PRIVATE src GTest::gtest_main)
gtest_discover_tests(piece_test)

add_executable(bleed_test "BleedTest.cpp")
target_link_libraries(bleed_test PRIVATE src GTest::gtest_main)
gtest_discover_tests(bleed_test)

add_executable(input_test "InputTest.cpp")
target_link_libraries(input_test PRIVATE src GTest::gtest_main)

# Level paths like "res/levels/..." are relative to the project root
gtest_discover_tests(input_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_executable(level_test "LevelTest.cpp")
target_link_libraries(level_test PRIVATE src GTest::gtest_main nlohmann_json_schema_validator)

# Level paths like "res/levels/..." are relative to the project root
gtest_discover_tests(level_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})