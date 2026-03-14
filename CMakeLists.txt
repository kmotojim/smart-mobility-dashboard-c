cmake_minimum_required(VERSION 3.15)
project(smart-mobility-backend VERSION 1.0.0 LANGUAGES CXX)

# C++17標準を使用
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -isystem /usr/include による #include_next 破壊を防止
set(CMAKE_NO_SYSTEM_FROM_IMPORTED ON)

# コンパイラ警告の設定
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# インクルードディレクトリ
include_directories(${PROJECT_SOURCE_DIR}/include)

# cpp-httplib と nlohmann-json のダウンロード (FetchContent)
include(FetchContent)

# cpp-httplib
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.18.3
)
FetchContent_MakeAvailable(httplib)

# nlohmann-json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# ソースファイル
set(SOURCES
    src/VehicleState.cpp
    src/VehicleService.cpp
    src/VehicleResource.cpp
    src/main.cpp
)

# 実行ファイル
add_executable(smart-mobility-backend ${SOURCES})

# リンク
target_link_libraries(smart-mobility-backend
    PRIVATE
    httplib::httplib
    nlohmann_json::nlohmann_json
)

# Pthread のリンク (Linux/macOS)
if(UNIX)
    target_link_libraries(smart-mobility-backend PRIVATE pthread)
endif()

# インストール設定
install(TARGETS smart-mobility-backend
    RUNTIME DESTINATION bin
)

# ===== カバレッジ設定 =====
option(ENABLE_COVERAGE "gcov カバレッジ計測を有効化する" OFF)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
        add_link_options(--coverage)
    endif()
endif()

# ===== テスト設定 =====
option(BUILD_TESTS "テストをビルドする" ON)

if(BUILD_TESTS)
    enable_testing()

    # Google Test
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # テスト用ソースファイル（main.cpp を除く）
    set(LIB_SOURCES
        src/VehicleState.cpp
        src/VehicleService.cpp
        src/VehicleResource.cpp
    )

    # VehicleState 単体テスト
    add_executable(VehicleStateTest test/VehicleStateTest.cpp ${LIB_SOURCES})
    target_link_libraries(VehicleStateTest
        PRIVATE GTest::gtest_main httplib::httplib nlohmann_json::nlohmann_json)
    if(UNIX)
        target_link_libraries(VehicleStateTest PRIVATE pthread)
    endif()

    # VehicleService 単体テスト
    add_executable(VehicleServiceTest test/VehicleServiceTest.cpp ${LIB_SOURCES})
    target_link_libraries(VehicleServiceTest
        PRIVATE GTest::gtest_main httplib::httplib nlohmann_json::nlohmann_json)
    if(UNIX)
        target_link_libraries(VehicleServiceTest PRIVATE pthread)
    endif()

    # VehicleResource REST API 統合テスト
    add_executable(VehicleResourceTest test/VehicleResourceTest.cpp ${LIB_SOURCES})
    target_link_libraries(VehicleResourceTest
        PRIVATE GTest::gtest_main httplib::httplib nlohmann_json::nlohmann_json)
    if(UNIX)
        target_link_libraries(VehicleResourceTest PRIVATE pthread)
    endif()

    include(GoogleTest)
    gtest_discover_tests(VehicleStateTest)
    gtest_discover_tests(VehicleServiceTest)
    gtest_discover_tests(VehicleResourceTest)
endif()
