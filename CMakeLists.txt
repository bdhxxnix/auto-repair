
cmake_minimum_required(VERSION 3.12)
project(AutoRepairPlantManagementSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(auto_repair
    src/main.cpp
    src/domain/work_order.cpp
    src/ui/cli.cpp
    src/ui/gui.cpp
)

target_include_directories(auto_repair PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(auto_repair PRIVATE Qt6::Widgets)
