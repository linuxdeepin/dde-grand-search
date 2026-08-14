# 计算器搜索插件示例

一个 DDE 全局搜索插件，提供数学表达式实时计算功能——类似 macOS 聚焦搜索的计算器。

## 工作原理

用户在全局搜索栏输入数学表达式（如 `1+2*3`、`sin(0.5)`、`(3+4)^2`），插件实时计算表达式并返回结果作为搜索项。点击结果可将计算值复制到系统剪贴板。

```
┌──────────────────────────────────────────────────┐
│  🔍  (3+4)*2                                      │
├──────────────────────────────────────────────────┤
│                                                    │
│  ┌────────────────────────────────────────────┐   │
│  │  🧮  计算器                                   │   │
│  │                                              │   │
│  │  (3+4)*2 = 14                               │   │
│  │  ↳ 点击复制到剪贴板                            │   │
│  └────────────────────────────────────────────┘   │
│                                                    │
└──────────────────────────────────────────────────┘
```

### 功能特性

- **实时计算** —— 用户输入表达式时即时解析并返回结果
- **ExprTk 引擎** —— 基于 [ExprTk](https://github.com/ArashPartow/exprtk) header-only 库，支持算术运算、三角函数、对数等
- **点击复制** —— 点击搜索结果将计算值复制到剪贴板
- **自动模式** —— 由 daemon 管理插件进程的完整生命周期
- **Qt5/Qt6 兼容** —— CMake 自动检测系统已安装的 Qt 版本

## 项目结构

```
calculator-search-plugin/
├── CMakeLists.txt                          # 构建配置
├── README.md                               # 本文档
├── main.cpp                                # 入口，DBus 服务注册
├── calculatorsearch.h / .cpp               # 插件核心逻辑
├── searchpluginadaptor.h / .cpp            # DBus Adaptor
├── calculator-search.conf.in               # 插件配置模板（CMake 生成）
├── 3rdparty/
│   └── exprtk/
│       └── exprtk.hpp                      # ExprTk header-only 库
└── debian/
    ├── changelog                           # 版本历史
    ├── compat                              # Debhelper 兼容级别
    ├── control                             # 包元数据与依赖
    ├── copyright                           # 许可证信息
    ├── rules                               # 构建规则（cmake）
    └── source/
        └── format                          # 源码格式
```

## 依赖

### 编译依赖

- CMake >= 3.10
- Qt5 或 Qt6（Core、Gui、DBus 模块）
- C++17 编译器

### 运行依赖

- `dde-grand-search`

在 Deepin / UOS 上安装编译依赖：

```bash
sudo apt install cmake qtbase5-dev qtbase5-dev-tools
# 或 Qt6：
sudo apt install cmake qt6-base-dev qt6-base-dev-tools
```

## 构建与安装

### CMake 开发构建

```bash
# 配置
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr

# 编译
cmake --build build -j$(nproc)

# 安装（需要 root 权限）
sudo cmake --build build --target install
```

安装后，插件二进制和配置文件位于：

```
/usr/lib/<arch>/dde-grand-search-daemon/plugins/searcher/
```

例如在 x86_64 上：

```
/usr/lib/x86_64-linux-gnu/dde-grand-search-daemon/plugins/searcher/
├── calculator-search-plugin          # 插件可执行文件
└── calculator-search.conf            # 插件配置文件
```

> `calculator-search.conf` 由 CMake 从 `calculator-search.conf.in` 模板动态生成，`Exec` 路径根据目标平台架构自动填充，无需手动修改。

### Debian 打包

```bash
# 安装编译依赖
sudo apt build-dep .

# 构建二进制包
dpkg-buildpackage -uc -us -nc
```

生成的 `.deb` 包安装：

```bash
sudo dpkg -i calculator-search-plugin_1.0.0_*.deb
```

### 安装后：重启 daemon

安装插件后（无论通过 CMake 还是 .deb），需重启全局搜索后端以加载新插件：

```bash
sudo systemctl restart dde-grand-search-daemon
# 或：
killall dde-grand-search-daemon && dde-grand-search-daemon &
```

## 配置文件

插件配置文件（`calculator-search.conf`）定义了 daemon 如何发现和管理插件：

| 字段 | 值 | 说明 |
|------|-----|------|
| `Name` | `com.example.CalculatorSearch` | 插件唯一标识 |
| `Mode` | `Auto` | daemon 管理进程生命周期 |
| `Priority` | `1`（Middle） | 用户搜索时启动，常驻后台 |
| `DBusService` | `com.example.CalculatorSearch` | DBus 服务名 |
| `DBusAddress` | `/com/example/CalculatorSearch` | DBus 对象路径 |
| `DBusInterface` | `com.example.CalculatorSearch.SearchPlugin` | DBus 接口名 |
| `InterfaceVersion` | `1.0` | 协议版本 |
| `Exec` | `/usr/lib/.../calculator-search-plugin` | 插件可执行文件路径（CMake 自动生成） |

## DBus 接口

插件按 V1.0 插件规范实现三个 DBus 方法：

### Search

```
Search(String json) -> String json
```

输入：
```json
{"ver": "1.0", "mID": "task-001", "cont": "(3+4)*2"}
```

输出（有结果时）：
```json
{
    "ver": "1.0",
    "mID": "task-001",
    "cont": [{
        "group": "计算器",
        "items": [{
            "item": "calc-(3+4)*2",
            "name": "(3+4)*2 = 14",
            "icon": "accessories-calculator",
            "type": "calculator/result"
        }]
    }]
}
```

输出（无结果时）：
```json
{"ver": "1.0", "mID": "task-001", "cont": []}
```

### Stop

```
Stop(String json) -> Boolean
```

### Action

```
Action(String json) -> Boolean
```

当 `action` 为 `"openitem"` 时，插件将计算结果复制到系统剪贴板。

## 支持的表达式

ExprTk 支持广泛的数学表达式：

| 表达式 | 结果 |
|--------|------|
| `1+2*3` | `7` |
| `(3+4)*2` | `14` |
| `2^10` | `1024` |
| `sin(0)` | `0` |
| `cos(3.14159265358979)` | `-1` |
| `sqrt(144)` | `12` |
| `log(100)` | `2` |
| `abs(-42)` | `42` |
| `floor(3.7)` | `3` |
| `ceil(3.2)` | `4` |
| `max(3, 7)` | `7` |
| `avg(1, 2, 3, 4, 5)` | `3` |
| `hypot(3, 4)` | `5` |

完整函数列表请参考 [ExprTk 文档](https://github.com/ArashPartow/exprtk)。

## 关键实现说明

| 主题 | 说明 |
|------|------|
| **表达式检测** | 基于正则表达式预过滤：必须包含数字和至少一个运算符（+, -, *, /, ^, %, !） |
| **计算引擎** | ExprTk header-only 库，使用 C++17 编译 |
| **结果格式化** | 整数不显示小数点；浮点数最多保留 12 位有效数字 |
| **Action 行为** | `"openitem"` 将计算结果复制到 `QGuiApplication::clipboard()` |
| **DBus Adaptor** | 手写 `QDBusAbstractAdaptor` 子类，通过 `Q_CLASSINFO` 声明接口元数据 |
| **线程安全** | `QMutex` 保护 `Action` 接口访问的结果缓存 |
| **进程模式** | Auto + Priority=1：首次搜索时由 daemon 启动，之后常驻后台 |
| **配置文件生成** | `calculator-search.conf.in` 模板由 CMake `configure_file` 动态生成，`Exec` 路径适配目标架构 |

## 扩展此示例

- **单位转换** —— 利用 ExprTk 的自定义函数支持，添加 `100 km to miles` 类表达式
- **历史记录** —— 缓存最近的计算，作为额外搜索结果返回
- **变量定义** —— 允许用户定义变量（如 `x = 5; x * 3`）
- **Trigger 模式** —— 切换为 Trigger 模式，通过 DBus service 自启动按需激活，避免常驻进程
