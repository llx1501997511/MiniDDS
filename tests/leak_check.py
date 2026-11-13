#!/usr/bin/env python3
# 连断 N 次，每 100 次采一次 RSS 和 fd
import os
import socket
import sys
import time

times = int(sys.argv[1]) if len(sys.argv) > 1 else 300
pid = os.popen("pgrep -x minidds_server").read().strip()
if not pid:
    sys.exit("找不到 minidds_server，先起服务器")

def snap():
    rss = "?"
    for line in open("/proc/%s/status" % pid):
        if line.startswith("VmRSS"):
            rss = line.split()[1]
    return rss, len(os.listdir("/proc/%s/fd" % pid))

t0 = time.time()
print("起点      RSS=%s kB  fd=%d" % snap())
for i in range(1, times + 1):
    s = socket.create_connection(("127.0.0.1", 8000))
    s.close()
    if i % 100 == 0:
        time.sleep(0.1)          # 先让服务器把关闭事件消化完
        rss, fd = snap()
        print("%4d 次   RSS=%s kB  fd=%d   %.1fs" % (i, rss, fd, time.time() - t0))

time.sleep(2)
print("静置 2s  RSS=%s kB  fd=%d" % snap())
