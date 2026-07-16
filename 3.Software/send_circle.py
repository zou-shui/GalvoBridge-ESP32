"""
向串口发送标准圆的坐标点序列。

数据格式 (大端序):
  FF FF | N(2B) | X1 Y1(4B) | ... | Xn Yn(4B)

每个坐标轴 16 位无符号，原点 0x7FFF。
"""

import math
import struct
import time
import serial

# ====== 可配置参数 ======
COM_PORT = "COM9"
BAUDRATE = 921600
POINT_COUNT = 400          # 点数量
ORIGIN = 0x7FFF              # 原点 (32767)
RADIUS = 0x6000              # 圆半径
# =======================


def generate_circle_points(n: int, cx: int, cy: int, r: float) -> list[tuple[int, int]]:
    """生成标准圆上的 n 个点，均匀分布。"""
    points = []
    for i in range(n):
        angle = 2.0 * math.pi * i / n
        x = int(cx + r * math.cos(angle))
        y = int(cy + r * math.sin(angle))
        if x < 0:
            x = 0
        elif x > 0xFFFF:
            x = 0xFFFF
        if y < 0:
            y = 0
        elif y > 0xFFFF:
            y = 0xFFFF
        points.append((x, y))
    return points


def build_packet(points: list[tuple[int, int]]) -> bytes:
    """按照协议格式打包数据。"""
    n = len(points)
    # 大端序打包: header(2B) + N(2B) + points(4B each)
    data = struct.pack(">HH", 0xFFFF, n)            # header + count
    for x, y in points:
        data += struct.pack(">HH", x, y)            # x + y
    return data


def main():
    print(f"生成 {POINT_COUNT} 个圆坐标点 (原点=0x{ORIGIN:04X}, 半径=0x{RADIUS:04X})...")
    points = generate_circle_points(POINT_COUNT, ORIGIN, ORIGIN, RADIUS)
    print(f"坐标生成完成，首点: (0x{points[0][0]:04X}, 0x{points[0][1]:04X})")

    print("打包数据帧...")
    packet = build_packet(points)
    print(f"数据帧大小: {len(packet)} 字节 (预期: {2+2+POINT_COUNT*4})")

    print(f"打开串口 {COM_PORT} @ {BAUDRATE} bps...")
    ser = serial.Serial(COM_PORT, BAUDRATE, timeout=1)

    print("发送数据...")
    start = time.time()
    ser.write(packet)
    ser.flush()                     # 等待硬件 FIFO 排空
    elapsed = time.time() - start

    print(f"发送完成，耗时 {elapsed:.2f}s, 速率 {len(packet)*8/elapsed/1000:.1f} kbps")
    ser.close()
    print("串口已关闭。")


if __name__ == "__main__":
    main()
