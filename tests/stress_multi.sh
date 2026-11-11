#!/bin/bash
# 多订阅多发布压测，对账 + 看订阅表清理
# ./stress_multi.sh [订阅数] [发布数] [每人条数]

set -u

NSUB=${1:-10}
NPUB=${2:-5}
NMSG=${3:-20}
PORT=${PORT:-8000}
WORK=/tmp/minidds_stress

cd "$(dirname "$0")/../build" 2>/dev/null || { echo "no build/, run cmake && make first"; exit 1; }
for exe in minidds_server subscriber publisher; do
    [ -x "./$exe" ] || { echo "missing ./$exe"; exit 1; }
done

cleanup(){
    [ -n "${SRV:-}" ] && kill "$SRV" 2>/dev/null
    for p in ${SUBPIDS:-}; do kill "$p" 2>/dev/null; done
    pkill -f minidds_server 2>/dev/null
}
trap cleanup EXIT

rm -rf "$WORK"; mkdir -p "$WORK"
pkill -f minidds_server 2>/dev/null
sleep 0.3

# server
./minidds_server "$PORT" > "$WORK/server.log" 2>&1 &
SRV=$!
sleep 0.5
if ! kill -0 "$SRV" 2>/dev/null; then
    echo "server died:"; cat "$WORK/server.log"; exit 1
fi
echo "server up, pid $SRV"

# 订阅者先起来，MiniDDS 不存历史消息
SUBPIDS=""
for i in $(seq 1 "$NSUB"); do
    ./subscriber "$PORT" imu > "$WORK/sub_$i.log" 2>&1 &
    SUBPIDS="$SUBPIDS $!"
done
sleep 1.5    # 试出来的，1s 偶尔会漏

# publisher 并发发
PUBPIDS=""
for i in $(seq 1 "$NPUB"); do
    ./publisher "$PORT" imu "$NMSG" > "$WORK/pub_$i.log" 2>&1 &
    PUBPIDS="$PUBPIDS $!"
done
for p in $PUBPIDS; do wait "$p"; done
sleep 2

# 对账
EXPECT=$(( NPUB * NMSG ))
echo "expect $EXPECT per sub ($NPUB x $NMSG)"
PASS=0; FAIL=0
for i in $(seq 1 "$NSUB"); do
    got=$(grep -c 'recv topic' "$WORK/sub_$i.log" 2>/dev/null)
    got=${got:-0}
    if [ "$got" -eq "$EXPECT" ]; then
        PASS=$((PASS+1)); echo "sub_$i got $got OK"
    else
        FAIL=$((FAIL+1)); echo "sub_$i got $got, FAIL want $EXPECT"
    fi
done
echo "$PASS ok / $FAIL bad"

# 干掉一半，看订阅表有没有残留
KILL=$(( NSUB / 2 ))
n=0
for p in $SUBPIDS; do
    [ "$n" -ge "$KILL" ] && break
    kill "$p" 2>/dev/null
    n=$((n+1))
done
sleep 1

# 探针，再发一条看 server 崩不崩
./publisher "$PORT" imu 1 > "$WORK/pub_probe.log" 2>&1
sleep 0.8

alive=$(grep -o -- '-> [0-9]* 个订阅者' "$WORK/server.log" | tail -1 | grep -o '[0-9]*')
echo "server says ${alive:-?} subs, want $((NSUB - KILL))"

SERVER_OK=1
kill -0 "$SRV" 2>/dev/null || SERVER_OK=0

echo ""
[ "$FAIL" -eq 0 ] && echo "recv: PASS ($NSUB x $EXPECT)" || echo "recv: FAIL ($FAIL subs bad)"
[ "${alive:-0}" = "$((NSUB - KILL))" ] && echo "cleanup: PASS (left $((NSUB-KILL)))" || echo "cleanup: FAIL (want $((NSUB-KILL)), got ${alive:-?})"
[ "$SERVER_OK" -eq 1 ] && echo "server: PASS" || echo "server: FAIL, see $WORK/server.log"
echo "logs: $WORK"

# TODO 只验条数，验不了内容有没有串
