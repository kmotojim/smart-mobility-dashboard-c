cmake_minimum_required(VERSION 3.15)
project(smart-mobility-backend VERSION 1.0.0 LANGUAGES CXX)

# C++17標準を使用
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
