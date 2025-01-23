#!/bin/bash

# can_setup.sh
sudo ip link set can0 up type can bitrate 500000

# 설정이 성공적으로 됐는지 확인
if [ $? -eq 0 ]; then
    echo "CAN 설정 완료"
    ip link show can0
else
    echo "CAN 설정 실패"
    exit 1
fi