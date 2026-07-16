# Firmware

本目录存放GalvoBridge-ESP32的固件及源码，硬件平台：ESP32-S3（Arduino 框架）。构建系统：PlatformIO。如要自行修改固件，请先安装PlatformIO。

由于PlatformIO官方尚未提供 ESP32-S3FH4R2 的板卡定义文件，
请手动将 `esp32-s3-fh4r2.json` 放入 PlatformIO 的 boards 目录：

Windows:
C:/Users/<用户名>/.platformio/platforms/espressif32/boards/

Linux / macOS:
~/.platformio/platforms/espressif32/boards/

之后即可正常打开工程。

## 如果想直接烧录固件

使用`5.Others/Tool`下的`flash_download_tool`工具烧录`2.Firmware/firmware`下的三个`bin`文件，具体配置截图可在`Tool`文件夹下找到。另外可参考官方的烧录步骤[Flash 下载工具用户指南 - ESP32 - — ESP 测试工具 latest 文档](https://docs.espressif.com/projects/esp-test-tools/zh_CN/latest/esp32/production_stage/tools/flash_download_tool.html#id4)


