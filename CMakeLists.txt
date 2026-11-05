cmake_minimum_required(VERSION 3.16)
project(minidds CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Protobuf REQUIRED)
# 生成 pb 文件
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS proto/message.proto)

# 核心库（现在还空着，后面每天往 src/ 加 .cc）
file(GLOB CORE_SRCS src/*.cc)
add_library(minidds_core STATIC ${CORE_SRCS} ${PROTO_SRCS})
target_include_directories(minidds_core PUBLIC
    include
    ${CMAKE_CURRENT_BINARY_DIR}      # 让 #include "message.pb.h" 找得到生成文件
    ${Protobuf_INCLUDE_DIRS})
target_link_libraries(minidds_core PUBLIC ${Protobuf_LIBRARIES} pthread)

# 服务器
add_executable(minidds_server src/main.cc)
target_link_libraries(minidds_server PRIVATE minidds_core)



# 可执行文件：echo 验证服务器
add_executable(echo examples/echo.cc)
target_link_libraries(echo PRIVATE minidds_core)
# Day4 验证 protobuf
add_executable(proto_test tests/proto_test.cc)
target_link_libraries(proto_test PRIVATE minidds_core)
