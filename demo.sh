#!/bin/bash
# demo.sh —— 一键演示 MiniDDS pub/sub
set -e
cd "$(dirname "$0")/build"
echo "=== 1. 启动服务器 ==="
./minidds_server 8000 &
SRV=$!
sleep 0.5
echo "=== 2. 启动订阅者（订阅 imu）==="
./subscriber 8000 imu &
SUB=$!
sleep 0.5
echo "=== 3. 发布 5 条消息 ==="
./publisher 8000 imu 5
sleep 0.5
echo "=== 4. 清理 ==="
kill $SRV $SUB 2>/dev/null
wait 2>/dev/null
echo "=== 演示结束 ==="