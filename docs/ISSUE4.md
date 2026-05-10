# export.bat 依旧报错：Python 虚拟环境版本不匹配（3.12 vs 3.13）

## 问题背景

用户按照 `ISSUE2.md` 的方案一，设置了 `IDF_TOOLS_PATH` 并尝试运行 `export.bat`。

## 问题现象

```cmd
D:\.espressif\v6.0\esp-idf>set IDF_TOOLS_PATH=D:\.espressif

D:\.espressif\v6.0\esp-idf>export.bat

ERROR: ESP-IDF Python virtual environment "D:\.espressif\python_env\idf6.0_py3.12_env\Scripts\python.exe" not found.
Please run the install script to set it up before proceeding.
```

即使设置了 `IDF_TOOLS_PATH`（路径已正确指向 `D:\.espressif`），仍然报错。

---

## 问题排查

### 关键线索

错误信息显示查找的是 `idf6.0_py3.12_env`，而不是正确的 `idf6.0_py3.13_env`。

这说明 **`python` 命令仍然指向 conda base 环境的 Python 3.12**，而不是系统 Python 3.13。

### 根本原因

`activate.py` 通过 `sys.version_info` 获取当前运行的 Python 版本来构造虚拟环境路径。虽然用户已设置了 `IDF_TOOLS_PATH`，但 `export.bat` 内部调用的是：

```bat
python "%IDF_PATH%\tools\activate.py" --export
```

这里的 `python` 仍然解析为 conda base 的 Python 3.12（因为 conda 会将其 Python 注入到 PATH 最前面），所以构造出的路径是 `idf6.0_py3.12_env`，而实际存在的虚拟环境是 `idf6.0_py3.13_env`。

### conda deactivate 为何无效

即使运行了 `conda deactivate`，某些情况下 conda 仍会影响 PATH：

- conda 配置了 `auto_activate_base: true`，新终端自动进入 base
- conda 的 `condabin` 目录在系统 PATH 中，`deactivate` 后仍可能残留
- 不同终端（CMD vs PowerShell）对 conda 的处理方式不同

---

## 解决方案

### 方案一：使用系统 Python 绝对路径直接运行（最可靠）

完全绕过 conda，直接指定系统 Python 路径：

```cmd
set IDF_TOOLS_PATH=D:\.espressif
cd /d D:\.espressif\v6.0\esp-idf
C:\Users\<username>\scoop\apps\python313\current\python.exe tools\activate.py --export
```

运行后会输出类似以下内容：

```
CALL "D:\.espressif\python_env\idf6.0_py3.13_env\Scripts\activate.bat"
```

复制这行输出，在终端中粘贴执行即可激活环境。

> **注意**：将 `<username>` 替换为你的实际 Windows 用户名。如果不确定 Python 路径，可以运行 `where python` 查看所有 Python 安装位置。

### 方案二：在 PowerShell 中运行（推荐）

PowerShell 中 conda 的影响较小，可以直接使用 `export.ps1`：

```powershell
$env:IDF_TOOLS_PATH = "D:\.espressif"
cd D:\.espressif\v6.0\esp-idf
. .\export.ps1
idf.py --version
```

### 方案三：禁用 conda 自动激活

永久禁用 conda base 环境的自动激活：

```cmd
conda config --set auto_activate_base false
```

然后**重新打开终端**，此时 `python` 将指向系统 Python 3.13，之后即可正常使用：

```cmd
set IDF_TOOLS_PATH=D:\.espressif
cd /d D:\.espressif\v6.0\esp-idf
export.bat
```

> 注意：禁用后如需使用 conda，需手动运行 `conda activate base` 或 `conda activate <环境名>`。

---

## 问题总结

| 问题 | 原因 | 解决 |
|------|------|------|
| 设置了 `IDF_TOOLS_PATH` 仍报 `py3.12_env not found` | `python` 命令仍指向 conda base 的 Python 3.12 | 使用系统 Python 绝对路径或在 PowerShell 中运行 |
| `conda deactivate` 无效 | conda 的 `condabin` 在 PATH 中残留，或配置了 `auto_activate_base` | 禁用 `auto_activate_base` 或直接指定 Python 路径 |

---

## 注意事项

1. **判断当前 Python 版本**：在终端中运行 `python --version`，如果不是 `3.13.x`，说明仍在 conda 环境中。

2. **查找系统 Python 路径**：
   ```cmd
   where python
   ```
   会列出所有 Python 路径，选择非 conda 路径的那个（通常是 `C:\Users\<username>\scoop\apps\python313\current\python.exe`）。

3. **永久解决方案**：运行 `conda config --set auto_activate_base false` 可以从根本上避免此问题，推荐所有使用 conda + ESP-IDF 的用户设置。
