# ESP32-S3 连接与烧录问题解决记录

## 问题背景

用户需要检测 ESP32-S3 设备（COM8 端口）的连接状态，并烧录 `sg90_servo_control` 程序。

---

## 问题一：esptool 不可用

### 现象
运行 `python -m esptool --port COM8 chip_id` 时报错：
```
No module named esptool
```

### 原因
esptool 未安装在系统 Python 环境中。

### 解决方案
使用 pip 安装 esptool：
```powershell
pip install esptool
```

---

## 问题二：ESP-IDF Python 虚拟环境未创建

### 现象
尝试激活 ESP-IDF 环境时报错：
```
ERROR: ESP-IDF Python virtual environment "C:\Users\<username>\.espressif\python_env\idf6.0_py3.13_env\Scripts\python.exe" not found.
```

### 原因
用户将 `.espressif` 目录从 `C:\Users\<username>\` 迁移到了 `D:\`，但 ESP-IDF 工具路径配置仍指向原位置。

### 解决方案
设置 `IDF_TOOLS_PATH` 环境变量指向新位置：
```powershell
$env:IDF_TOOLS_PATH = "D:\.espressif"
```

---

## 问题三：工具下载失败

### 现象
运行 `install.ps1 esp32s3` 时报错：
```
WARNING: Download failure: <urlopen error [WinError 10060]>
ERROR: Failed to download, and retry count has expired
```

### 原因
网络连接问题，无法从 GitHub 下载工具。

### 解决方案
由于项目目录中已存在编译好的文件（`build/sg90_servo_control.bin` 等），可以直接使用 esptool 进行烧录，无需重新下载工具。

---

## 最终解决方案

### 1. 安装 esptool
```powershell
pip install esptool
```

### 2. 检测设备连接
```powershell
python -m esptool --port COM8 chip_id
```

### 3. 烧录程序
```powershell
python -m esptool --port COM8 --baud 115200 --before default-reset --after hard-reset --chip esp32s3 write_flash --flash-mode dio --flash-size 2MB --flash-freq 80m 0x0 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x10000 build/sg90_servo_control.bin
```

---

## 检测结果

✅ **ESP32-S3 连接正常**

| 属性 | 值 |
|------|-----|
| 芯片类型 | ESP32-S3 (QFN56) (revision v0.2) |
| 特性 | Wi-Fi, BT 5 (LE), Dual Core + LP Core, 240MHz, Embedded PSRAM 8MB |
| 晶振频率 | 40MHz |
| USB 模式 | USB-Serial/JTAG |
| MAC 地址 | `<your-mac-address>` |

## 烧录结果

✅ **程序烧录成功**

| 文件 | 地址 | 大小 |
|------|------|------|
| bootloader.bin | 0x0 | 21,056 bytes |
| partition-table.bin | 0x8000 | 3,072 bytes |
| sg90_servo_control.bin | 0x10000 | 175,296 bytes |

---

## 注意事项

1. **ESP-IDF 环境路径配置**：如果将 `.espressif` 迁移到其他位置，需要设置 `IDF_TOOLS_PATH` 环境变量。

2. **网络问题**：如果遇到工具下载失败，可以考虑使用国内镜像源或离线安装包。

3. **串口监视**：如需查看程序运行日志，需要先激活 ESP-IDF 环境：
   ```powershell
   cd D:\.espressif\v6.0\esp-idf
   . .\export.ps1
   idf.py -p COM8 monitor
   ```

4. **程序控制**：
   - 停止程序：拔掉 USB 线
   - 重启程序：按 RESET 键
   - 暂停运行：按住 BOOT 键不放
