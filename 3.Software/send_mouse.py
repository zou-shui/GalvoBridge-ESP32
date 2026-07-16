"""
实时获取鼠标位置并通过串口发送。

数据格式 (大端序):
  FF FF | N(2B) | X1 Y1(4B) | ... | Xn Yn(4B)

100fps 采样，每帧 N=1。屏幕完整映射到 0x0000~0xFFFF，屏幕中心 = 原点 0x7FFF。
"""

import struct
import time
import ctypes
import serial

# ====== 可配置参数 ======
COM_PORT = "COM9"
BAUDRATE = 921600
FPS = 100                    # 采样帧率
ORIGIN = 0x7FFF              # 16 位原点
INVERT_X = True             # True = 鼠标左右反向
INVERT_Y = True             # True = 鼠标上下反向
# =======================

# ── Windows API：鼠标位置 & 屏幕尺寸 ──
user32 = ctypes.windll.user32
SCREEN_W = user32.GetSystemMetrics(0)
SCREEN_H = user32.GetSystemMetrics(1)


class POINT(ctypes.Structure):
    _fields_ = [("x", ctypes.c_long), ("y", ctypes.c_long)]


def get_mouse_pos() -> tuple[int, int]:
    """通过 Win32 API 获取当前鼠标屏幕坐标。"""
    pt = POINT()
    user32.GetCursorPos(ctypes.byref(pt))
    return pt.x, pt.y


def screen_to_16bit(mx: int, my: int) -> tuple[int, int]:
    """将屏幕像素坐标映射到 16 位坐标空间 [0x0000, 0xFFFF]。"""
    x = int(mx / SCREEN_W * 0xFFFF)
    y = int(my / SCREEN_H * 0xFFFF)
    if x < 0:
        x = 0
    elif x > 0xFFFF:
        x = 0xFFFF
    if y < 0:
        y = 0
    elif y > 0xFFFF:
        y = 0xFFFF
    # 方向反转 (以 0x7FFF 为中心镜像)
    if INVERT_X:
        x = 0xFFFF - x
    if INVERT_Y:
        y = 0xFFFF - y
    return x, y


def build_packet(points: list[tuple[int, int]]) -> bytes:
    """打包数据帧 (复用与 send_circle 相同的格式)。"""
    n = len(points)
    data = struct.pack(">HH", 0xFFFF, n)
    for x, y in points:
        data += struct.pack(">HH", x, y)
    return data


def main():
    print(f"屏幕分辨率: {SCREEN_W}x{SCREEN_H}")
    print(f"屏幕中心 → 0x{ORIGIN:04X} (约 {int(SCREEN_W/2*0xFFFF/SCREEN_W):04X})")
    print(f"采样率: {FPS} fps, 串口: {COM_PORT} @ {BAUDRATE}")
    print("按 Ctrl+C 停止...\n")

    interval = 1.0 / FPS
    frame = 0

    try:
        ser = serial.Serial(COM_PORT, BAUDRATE, timeout=0.01)
    except serial.SerialException as e:
        print(f"无法打开串口 {COM_PORT}: {e}")
        return

    try:
        while True:
            t0 = time.perf_counter()

            mx, my = get_mouse_pos()
            x16, y16 = screen_to_16bit(mx, my)
            packet = build_packet([(x16, y16)])
            ser.write(packet)
            frame += 1

            # 精准帧率控制
            elapsed = time.perf_counter() - t0
            sleep_t = interval - elapsed
            if sleep_t > 0:
                time.sleep(sleep_t)

            # 每秒输出一次状态
            if frame % FPS == 0:
                print(
                    f"  [{frame:6d}] 鼠标({mx:4d},{my:4d}) → (0x{x16:04X}, 0x{y16:04X})", end="\r")

    except KeyboardInterrupt:
        print(f"\n\n用户中断，共发送 {frame} 帧。")
    finally:
        ser.flush()
        ser.close()
        print("串口已关闭。")


if __name__ == "__main__":
    main()
