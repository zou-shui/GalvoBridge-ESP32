# Software

本目录提供两个用于测试GalvoBridge-ESP32的python脚本。

## 1. send_circle.py (静态圆形生成器)

**功能**：在内存中预先生成一个由 $N$ 个点（默认 400 点）组成的标准圆轨迹，打包成一个数据包并单次发送。

## 2. send_mouse.py (实时鼠标追踪器)

**功能**：以 100Hz 的频率实时捕获 Windows 屏幕上的鼠标光标位置，并将其映射为 16 位坐标发送。

---

## 环境准备与安装

1. **确保 Python 环境已安装**（建议 Python 3.8 及以上版本）。

2. **安装依赖**：
   在脚本所在目录下打开终端，运行以下命令安装串口通信库：
   
   ```bash
   pip install -r requirements.txt
   ```

3. 运行
   
   ```bash
   python send_circle.py
   ```
   
   ```bash
   python send_mouse.py
   ```


