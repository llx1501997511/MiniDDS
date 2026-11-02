cmake_minimum_required(VERSION 3.16)
project(minidds CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 核心库（现在还空着，后面每天往 src/ 加 .cc）
file(GLOB CORE_SRCS src/*.cc)
add_library(minidds_core STATIC ${CORE_SRCS})
target_include_directories(minidds_core PUBLIC include)
