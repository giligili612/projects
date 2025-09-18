#!/bin/bash

# 简单循环启动10个客户端
for i in {1..100}
do
    echo "启动客户端 $i"
    ./build/client 127.0.0.1 5085 &
    sleep 0.1  # 稍微延迟一下，避免同时启动
done

