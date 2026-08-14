# 全局搜索插件开发指南

> 本文档基于 DDE Grand Search 项目源码编写，涵盖搜索插件（Searcher Plugin）的架构设计、接口规范、配置方法和开发流程。

## 目录

- [更新日志](#更新日志)
- [概述](#概述)
- [架构设计](#架构设计)
- [接入方式](#接入方式)
- [配置文件](#配置文件)
- [插件接口规范](#插件接口规范)
  - [V1.0 接口](#v10-接口)
    - [Search 接口](#search-接口)
    - [Stop 接口](#stop-接口)
    - [Action 接口](#action-接口)
- [数据格式详解](#数据格式详解)
- [进程管理](#进程管理)
- [内置搜索项](#内置搜索项)
- [调试与排查](#调试与排查)
- [完整开发流程](#完整开发流程)
- [DBBus 接口 XML 定义](#dbus-接口-xml-定义)

---

## 概述

搜索插件为全局搜索提供搜索更多内容的能力。插件采用 **DBus 服务**的形式接入，可以是只提供搜索服务的独立进程，也可以由主业务程序提供搜索服务。

全局搜索开始搜索后，调用搜索插件的 DBus 接口，传入搜索关键词，搜索插件执行搜索并返回搜索结果。搜索结果会展示在全局搜索界面中，用户可对结果进行预览、打开等操作。

### 核心特性

- **解耦设计**：插件作为独立 DBus 服务运行，与全局搜索后端（`dde-grand-search-daemon`）通过 DBus 通信
- **多版本协议**：支持接口版本管理，当前支持 V1.0
- **灵活的进程管理**：三种运行模式适应不同场景
- **国际化支持**：搜索结果中的组名和项目名称支持根据系统语言返回
- **扩展数据**：搜索结果支持携带额外属性（权重、匹配上下文、关键词等）

---

## 架构设计

### 整体架构

```
┌─────────────────────────────────────────────────────────────┐
│                    全局搜索前端 (dde-grand-search)            │
│                    ┌─────────────────────┐                   │
│                    │   搜索结果展示界面    │                   │
│                    └─────────┬───────────┘                   │
└──────────────────────────────┼──────────────────────────────┘
                               │ DBus
┌──────────────────────────────┼──────────────────────────────┐
│                全局搜索后端 (dde-grand-search-daemon)         │
│                    ┌─────────┴───────────┐                   │
│                    │   MainController     │                   │
│                    │   (搜索任务调度)     │                   │
│                    └─────────┬───────────┘                   │
│              ┌───────────────┼───────────────┐               │
│              ▼               ▼               ▼               │
│      ┌──────────────┐ ┌──────────────┐ ┌──────────────┐      │
│      │  内置搜索器   │ │  扩展搜索器   │ │  预览插件     │      │
│      │  (文件/应用   │ │ (ExtendSea-  │ │ (preview)    │      │
│      │   /设置/Web)  │ │   rcher)     │ │              │      │
│      └──────────────┘ └──────┬───────┘ └──────────────┘      │
│                             │                                │
│                    ┌────────┴──────────┐                     │
│                    │  PluginManager     │                     │
│                    │  (插件管理/加载)    │                     │
│                    └────────┬──────────┘                     │
│                             │                                │
│              ┌──────────────┼──────────────┐               │
│              ▼              ▼              ▼                  │
│      ┌──────────────┐┌──────────────┐┌──────────────┐       │
│      │PluginLoader  ││PluginProcess ││PluginLiaison │       │
│      │(读取conf配置) ││(进程管理)     ││(DBus通信)     │       │
│      └──────────────┘└──────────────┘└──────────────┘       │
└─────────────────────────────────────────────────────────────┘
                               │ DBus
┌──────────────────────────────┼──────────────────────────────┐
│                     第三方搜索插件                            │
│              ┌───────────────┴───────────────┐              │
│              │   插件进程 (独立 DBus 服务)      │              │
│              │   ┌─────────────────────────┐ │              │
│              │   │  Search()  搜索接口      │ │              │
│              │   │  Stop()    停止接口      │ │              │
│              │   │  Action()  操作接口      │ │              │
│              │   └─────────────────────────┘ │              │
│              └───────────────────────────────┘              │
└─────────────────────────────────────────────────────────────┘
```

## 接入方式

搜索插件以 **配置文件**的形式接入，提供插件的必要信息。

- **格式**：INI 格式
- **名称**：必须为 `.conf` 后缀，如：`plugin.conf`
- **存放路径**：`/usr/lib/$PLATFORM/dde-grand-search-daemon/plugins/searcher/`
  - `$PLATFORM` 为平台相关目录，如 x86_64 下为 `x86_64-linux-gnu`
  - 完整路径示例：`/usr/lib/x86_64-linux-gnu/dde-grand-search-daemon/plugins/searcher/`

将配置文件安装到存放路径后，**重启全局搜索后端**（`dde-grand-search-daemon`）即可生效。

> **注意**：配置文件中的插件名称不能与内置搜索项冲突，否则该配置文件将被忽略。

---

## 配置文件

配置文件使用 INI 格式，根节点为 `[Grand Search]`。各字段说明如下：

| 字段 | 名称 | 必须 | 说明 |
|------|------|------|------|
| `Name` | 插件名称 | 是 | 搜索插件的名称，作为唯一 ID 使用，需保证全局唯一。推荐使用 `com.company.app.xxx` 格式。搜索结果中会标识其出自的搜索项名称，用于后续的预览、打开等操作。 |
| `Mode` | 运行模式 | 否 | 可选值为 `Auto`、`Manual`、`Trigger`，默认值为 `Manual`。详见 [运行模式](#运行模式)。 |
| `Priority` | 优先级 | 仅 Auto 模式 | 值为 `0`、`1`、`2`。详见 [优先级](#优先级)。 |
| `InterfaceVersion` | 协议版本 | 是 | 接口协议版本，如：`1.0`。必须与插件通信数据包中的 `ver` 字段保持一致。全局搜索根据该值选择功能接口及数据格式化方式。 |
| `DBusService` | 服务名 | 是 | 插件提供的 DBus 服务名。 |
| `DBusAddress` | 服务地址 | 是 | 插件提供的 DBus 服务对象路径。 |
| `DBusInterface` | 服务接口 | 是 | 插件提供的 DBus 接口名。 |
| `Exec` | 可执行程序路径 | 仅 Auto 模式 | 插件的可执行程序路径，用于启动插件进程。支持带参数的命令行。 |

### 运行模式

| 模式 | 说明 | 适用场景 |
|------|------|----------|
| `Auto` | 由全局搜索后端（daemon）启动并控制运行时间。daemon 根据 `Priority` 决定启动时机，并守护进程运行。 | 独立的搜索服务进程，由 daemon 统一管理生命周期 |
| `Manual` | 由用户或其他方式控制搜索服务的启动，daemon 只调用其提供的 DBus 接口。 | 插件由其他系统服务管理，或插件本身是某个应用的一部分 |
| `Trigger` | 由 DBus 自启动机制控制。全局搜索直接访问插件的 DBus 服务，若插件未启动则 DBus 自动启动该服务（需插件注册 D-Bus service 文件到系统）。 | 插件需要按需启动，不需要常驻后台 |

> **注意**：`Trigger` 模式从 5.1.1 版本开始支持。

### 优先级

仅 `Auto` 模式下有效，决定 daemon 何时启动插件进程及是否守护运行：

| 优先级 | 值 | 启动时机 | 守护机制 |
|--------|-----|----------|----------|
| `High` | 0 | daemon 启动时常驻启动 | 守护运行，进程异常退出后自动重启（最多重启 3 次） |
| `Middle` | 1 | 用户发起搜索时启动 | 守护运行，进程异常退出后自动重启（最多重启 3 次） |
| `Low` | 2 | 用户发起搜索时启动 | 不守护，空闲后退出，进程异常退出后不自动重启 |

### 配置文件示例

**Auto 模式（daemon 管理进程）：**

```ini
[Grand Search]
Name=com.example.CalculatorSearch
Mode=Auto
Priority=1
DBusService=com.example.CalculatorSearch
DBusAddress=/com/example/CalculatorSearch
DBusInterface=com.example.CalculatorSearch.SearchPlugin
InterfaceVersion=1.0
Exec=/usr/lib/x86_64-linux-gnu/dde-grand-search-daemon/plugins/searcher/calculator-search
```

**Manual 模式（外部控制启动）：**

```ini
[Grand Search]
Name=com.example.CalculatorSearch
Mode=Manual
DBusService=com.example.CalculatorSearch
DBusAddress=/com/example/CalculatorSearch
DBusInterface=com.example.CalculatorSearch.SearchPlugin
InterfaceVersion=1.0
```

**Trigger 模式（DBus 自启动）：**

```ini
[Grand Search]
Name=com.example.CalculatorSearch
Mode=Trigger
DBusService=com.example.CalculatorSearch
DBusAddress=/com/example/CalculatorSearch
DBusInterface=com.example.CalculatorSearch.SearchPlugin
InterfaceVersion=1.0
```

> Trigger 模式还需要在 `/usr/share/dbus-1/services/` 下安装 `.service` 文件，指定 DBus 服务名与可执行文件路径的映射关系。

---

## 插件接口规范

搜索插件进程需注册 DBus 服务，DBus 服务信息与配置文件一致。按全局搜索插件接口规范提供 DBus 接口。

插件接口规范随着功能业务的更新迭代会有不同的版本，因此在开发搜索插件时需明确保证版本号的正确性：
- 配置文件中 `InterfaceVersion` 字段的版本号
- 插件提供的 DBus 接口实现
- 接口的输入输出数据格式

以上三者必须保持一致的版本。

### V1.0 接口

V1.0 为全局搜索的第一个插件接口版本，包含搜索与操作搜索结果的功能。

该版本下插件需提供 **三个 DBus 方法**：`Search`、`Stop`、`Action`。

#### DBus 接口签名

| 方法 | 输入 | 输出 | 说明 |
|------|------|------|------|
| `Search` | `String json` | `String json` | 执行搜索，返回搜索结果 |
| `Stop` | `String json` | `Boolean` | 中断搜索任务 |
| `Action` | `String json` | `Boolean` | 对搜索结果执行操作 |

#### Search 接口

Search 接口中插件需根据输入的关键词查找匹配的项目并返回搜索结果。该接口必须为 **可中断的**——即当 Stop 接口被调用时，正在执行的 Search 调用应尽快返回。

**接口定义：**

```
Search (String json) ↦ (String arg_0)
```

**输入参数：**

输入参数为 JSON 格式的字符串，包含接口版本号、任务号、搜索关键词：

```json
{
    "ver": "1.0",
    "mID": "202110000000",
    "cont": "dde"
}
```

| 字段 | 类型 | 说明 |
|------|------|------|
| `ver` | String | 版本号，固定值为 `"1.0"` |
| `mID` | String | 任务号，仅用作标识一次搜索，具有唯一性 |
| `cont` | String | 搜索关键词，需要搜索的内容 |

**返回值：**

返回值为 JSON 格式的字符串，包含搜索结果的必要信息：

```json
{
    "ver": "1.0",
    "mID": "202110000000",
    "cont": [
        {
            "group": "应用商店",
            "items": [
                {
                    "item": "dde-desktop",
                    "name": "桌面",
                    "icon": "/usr/share/icons/hicolor/48x48/apps/desktop.svg",
                    "type": "application/appstore-app"
                },
                {
                    "item": "dde-file-manager",
                    "name": "文件管理器",
                    "icon": "dde-file-manager",
                    "type": "application/appstore-app"
                }
            ]
        }
    ]
}
```

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `ver` | String | 是 | 版本号，固定值为 `"1.0"` |
| `mID` | String | 是 | 任务号，与输入参数中 `mID` 保持一致 |
| `cont` | Array | 是 | 搜索结果的分组列表，支持返回多个分组 |
| `cont[].group` | String | 是 | 分组名称，在全局搜索界面中展示。**需支持国际化，根据系统语言返回组名** |
| `cont[].items` | Array | 是 | 该组下的搜索结果列表 |
| `cont[].items[].item` | String | 是 | 项目标识，组内唯一。不用于界面展示，仅作为标识用于后续操作 |
| `cont[].items[].name` | String | 是 | 项目名称，在界面中展示。**需支持国际化** |
| `cont[].items[].icon` | String | 否 | 项目图标，支持图标文件路径和图标名称（通过主题获取） |
| `cont[].items[].type` | String | 是 | 项目类型，格式应遵循 MIME type 规范，可填写自定义值。**目前仅用作匹配预览界面** |

> **重要限制**：
> - 每组搜索结果最多解析 **100 个项目**，超出部分将被忽略
> - `item`、`name`、`type` 三个字段为必填，任一为空则该结果项被跳过
> - `icon` 为可选字段，为空时使用默认图标

#### Stop 接口

Stop 接口中插件需中断搜索任务，使正在执行的 Search 接口调用结束并返回。

**接口定义：**

```
Stop (String json) ↦ (Boolean arg_0)
```

**输入参数：**

```json
{
    "ver": "1.0",
    "mID": "202110000000"
}
```

| 字段 | 类型 | 说明 |
|------|------|------|
| `ver` | String | 接口版本号，固定值为 `"1.0"` |
| `mID` | String | 任务号，与 Search 接口输入的任务号一致，表示需停止该次搜索 |

**返回值：**

- `true`：停止成功
- `false`：停止失败

#### Action 接口

Action 接口中插件需根据输入的参数，对搜索结果执行操作，如打开某一项搜索结果。

**接口定义：**

```
Action (String json) ↦ (Boolean arg_0)
```

**输入参数：**

```json
{
    "ver": "1.0",
    "action": "openitem",
    "item": "dde-desktop"
}
```

| 字段 | 类型 | 说明 |
|------|------|------|
| `ver` | String | 接口版本号，固定值为 `"1.0"` |
| `action` | String | 操作类型。当前版本仅支持 `"openitem"`（打开操作） |
| `item` | String | 需操作的项目标识，与搜索结果中的 `item` 字段值一致 |

**返回值：**

- `true`：操作成功
- `false`：操作失败

---

## 数据格式详解

### 协议常量定义

以下常量定义在源码 `searchpluginprotocol.h` 中，插件开发者可参考：

```cpp
// 协议字段名
#define PLUGININTERFACE_PROTOCOL_VERSION     "ver"       // 版本号
#define PLUGININTERFACE_PROTOCOL_MISSIONID   "mID"       // 任务号
#define PLUGININTERFACE_PROTOCOL_CONTENT     "cont"      // 搜索内容/结果
#define PLUGININTERFACE_PROTOCOL_GROUP       "group"     // 分组名
#define PLUGININTERFACE_PROTOCOL_ITEMS       "items"     // 项目列表
#define PLUGININTERFACE_PROTOCOL_ITEM        "item"      // 项目标识
#define PLUGININTERFACE_PROTOCOL_NAME        "name"      // 项目名称
#define PLUGININTERFACE_PROTOCOL_TYPE        "type"      // 项目类型
#define PLUGININTERFACE_PROTOCOL_ICON        "icon"      // 项目图标
#define PLUGININTERFACE_PROTOCOL_ACTION      "action"    // 操作类型

// 操作类型值
#define PLUGININTERFACE_PROTOCOL_ACTION_OPEN "openitem"  // 打开操作
```

### 配置文件常量定义

以下常量定义在源码 `searchpluginconf.h` 中：

```cpp
#define PLUGININTERFACE_CONF_ROOT             "Grand Search"
#define PLUGININTERFACE_CONF_NAME             "Name"
#define PLUGININTERFACE_CONF_MODE             "Mode"
#define PLUGININTERFACE_CONF_MODE_AUTO        "auto"
#define PLUGININTERFACE_CONF_MODE_MANUAL      "manual"
#define PLUGININTERFACE_CONF_MODE_TRIGGER     "trigger"
#define PLUGININTERFACE_CONF_PRIORITY         "Priority"
#define PLUGININTERFACE_CONF_DBUSSERVICE      "DBusService"
#define PLUGININTERFACE_CONF_DBUSADDRESS      "DBusAddress"
#define PLUGININTERFACE_CONF_DBUSINTERFACE    "DBusInterface"
#define PLUGININTERFACE_CONF_INTERFACEVERSION "InterfaceVersion"
#define PLUGININTERFACE_CONF_EXEC             "Exec"
```

### 数据转换流程

全局搜索后端在调用插件接口时，会通过 `DataConvertor` 对数据进行 JSON 序列化/反序列化。插件开发者无需关心 daemon 端的实现，但需确保返回的 JSON 数据格式符合规范。

```
搜索请求流程:
  daemon → DataConvertor (search) → JSON → DBus → 插件 Search()

搜索结果流程:
  插件 Search() → JSON → DBus → DataConvertor (result) → MatchedItemMap → 界面展示

  结果解析限制:
  - 最多解析 100 个项目（跨所有分组累计）
  - item/name/type 字段为空的结果项会被跳过
  - 超出 100 个的项目不再解析
```

### 搜索结果扩展属性

搜索结果支持携带额外属性（定义在 `builtinsearch.h` 中），用于在界面中进行排序、展示等：

| 属性 | 字段名 | 说明 |
|------|--------|------|
| 显示等级 | `itemLevel` | 1（第一等级）、2、3，控制在组内的显示顺序 |
| 权重计算方法 | `weightMethod` | `localFileMethod`、`desktopAppMethod`、`settingMethod` |
| 权重值 | `itemWeight` | 数值，用于排序 |
| 拖尾信息 | `itemTailer` | 在搜索结果下方显示的附加信息 |
| 修改时间 | `itemModifiedTime` | 文件的修改时间 |
| 匹配上下文 | `itemMatchedContext` | 匹配到的具体信息 |
| 匹配关键词 | `itemKeywords` | 匹配的关键词列表 |

> 扩展属性通过 `MatchedItem.extra` 字段（`QVariantHash`）传递，仅适用于内置搜索器。第三方插件返回的数据通过 V1.0 协议传输，extra 字段在 daemon 端解析时自动设置。

---

## 进程管理

### Auto 模式进程管理

对于 `Auto` 模式的插件，daemon 通过 `PluginProcess` 进行进程管理：

1. **进程启动**：daemon 根据 `Priority` 在适当时候通过 `QProcess` 启动插件进程
2. **进程守护**：`High` 和 `Middle` 优先级的插件会被守护，进程异常退出后自动重启
3. **重启限制**：最多重启 3 次，超过后不再尝试启动
4. **稳定性检查**：进程启动后 1 分钟进行稳定性检查，如果进程仍在运行则清除重启计数
5. **进程终止**：daemon 退出时终止所有插件进程，先 `terminate()` 等待 1 秒，超时则 `kill()`

### 进程启动命令解析

`Exec` 字段支持带参数的命令行，daemon 会自动解析可执行程序路径和参数。例如：

```ini
Exec=/usr/lib/x86_64-linux-gnu/dde-grand-search-daemon/plugins/searcher/myplugin --option value
```

### Trigger 模式激活

`Trigger` 模式下，daemon 通过发送 DBus `Ping` 消息来激活插件服务。如果插件未运行，DBus 会根据 `.service` 文件自动启动插件进程。

---

## 内置搜索项

以下搜索项名称已被内置搜索器使用，插件名称不能与之冲突，否则配置文件将被忽略：

| 内置搜索项名称 | 说明 |
|----------------|------|
| `com.deepin.dde-grand-search.file-deepin` | 文件搜索 |
| `com.deepin.dde-grand-search.app-desktop` | 应用搜索 |
| `com.deepin.dde-grand-search.dde-control-center-setting` | 控制中心设置搜索 |
| `com.deepin.dde-grand-search.web-statictext` | Web 静态文本搜索 |
| `com.deepin.dde-grand-search.semantic` | 语义搜索 |
| `com.deepin.dde-grand-search.ocr-text` | OCR 文本搜索 |
| `com.deepin.dde-grand-search.file-fulltext` | 文件全文搜索 |

---

## 调试与排查

### 日志

全局搜索后端使用 `QLoggingCategory` 进行日志管理，调试时可通过环境变量开启详细日志：

```bash
# 开启 daemon 的调试日志
QT_LOGGING_RULES="logDaemon.debug=true" dde-grand-search-daemon
```

### 常见问题

| 问题 | 可能原因 | 解决方案 |
|------|----------|----------|
| 插件未被加载 | 配置文件路径不正确 | 确认文件位于 `/usr/lib/$PLATFORM/dde-grand-search-daemon/plugins/searcher/` 且后缀为 `.conf` |
| 插件未被加载 | 名称与内置搜索项冲突 | 修改 `Name` 字段，避免使用内置搜索项名称 |
| 插件未被加载 | `InterfaceVersion` 不被支持 | 确认版本号为 `1.0` |
| 插件未被加载 | 配置文件缺少必填字段 | 检查 `Name`、`DBusService`、`DBusAddress`、`DBusInterface`、`InterfaceVersion` 是否已填写 |
| Auto 插件未启动 | `Exec` 字段为空或路径不正确 | 检查可执行文件路径是否存在且有执行权限 |
| Auto 插件启动后立即退出 | 程序启动失败 | 检查程序依赖、参数是否正确；查看 daemon 日志 |
| Auto 插件反复重启 | 程序崩溃 | 查看插件进程日志；重启超过 3 次后 daemon 会放弃 |
| 搜索无结果 | DBus 服务未注册成功 | 使用 `dbus-send` 或 `d-feet` 检查 DBus 服务是否注册 |
| 搜索无结果 | 返回的 JSON 格式不正确 | 检查 JSON 结构是否符合 V1.0 规范，`item`/`name`/`type` 是否为空 |
| 搜索结果被截断 | 每组最多 100 个项目 | 这是 daemon 端的限制，无需修复 |
| 搜索超时 | 插件响应时间超过 25 秒 | daemon 设置了 25 秒超时，优化插件搜索性能 |

### DBus 调试

使用 `dbus-send` 手动测试插件接口：

```bash
# 测试 Search 接口
dbus-send --session --print-reply \
  --dest=com.example.CalculatorSearch \
  /com/example/CalculatorSearch \
  com.example.CalculatorSearch.SearchPlugin.Search \
  "string:{\"ver\":\"1.0\",\"mID\":\"test001\",\"cont\":\"1+2*3\"}"

# 测试 Stop 接口
dbus-send --session --print-reply \
  --dest=com.example.CalculatorSearch \
  /com/example/CalculatorSearch \
  com.example.CalculatorSearch.SearchPlugin.Stop \
  "string:{\"ver\":\"1.0\",\"mID\":\"test001\"}"

# 测试 Action 接口
dbus-send --session --print-reply \
  --dest=com.example.CalculatorSearch \
  /com/example/CalculatorSearch \
  com.example.CalculatorSearch.SearchPlugin.Action \
  "string:{\"ver\":\"1.0\",\"action\":\"openitem\",\"item\":\"result-0\"}"
```

使用 `d-feet` 图形化工具浏览 DBus 服务：

```bash
d-feet
```

---

## 完整开发流程

### 1. 确定 DBus 服务信息

为插件分配唯一的 DBus 服务名、地址和接口名：

```
DBusService:   com.example.MySearch
DBusAddress:   /com/example/MySearch
DBusInterface: com.example.MySearch.SearchPlugin
```

### 2. 实现插件 DBus 服务

使用 Qt DBus 实现三个接口方法。可参考项目中提供的 DBus 接口 XML 定义生成 Adaptor。

### 3. 编写配置文件

创建 `.conf` 文件，填写所有必填字段。

### 4. 构建和安装

编译插件程序，安装可执行文件和配置文件到系统路径。

### 5. 重启 daemon

```bash
sudo systemctl restart dde-grand-search-daemon
```

或手动重启：

```bash
killall dde-grand-search-daemon
dde-grand-search-daemon &
```

### 6. 验证

通过全局搜索界面搜索，确认插件结果是否正常展示。

---

## DBus 接口 XML 定义

项目提供了 DBus 接口的 XML 定义文件，位于：
`src/dde-grand-search-daemon/searchplugin/interface/abstract/com.deepin.dde.grandsearch.searchplugin.xml`

插件开发者可使用此 XML 文件通过 `qdbusxml2cpp` 生成 DBus Adaptor 代码：

```bash
qdbusxml2cpp -a searchpluginadaptor -c SearchPluginAdaptor \
  com.deepin.dde.grandsearch.searchplugin.xml
```

生成的 Adaptor 头文件和源文件可直接集成到插件项目中。

XML 文件内容：

```xml
<!DOCTYPE node PUBLIC "-//freedesktop//DTD D-BUS Object Introspection 1.0//EN"
  "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd">
<node>
  <interface name="com.deepin.dde.GrandSearch.SearchPlugin">
    <method name="Search">
      <arg type="s" direction="out"/>
      <arg name="json" type="s" direction="in"/>
    </method>
    <method name="Action">
      <arg type="b" direction="out"/>
      <arg name="json" type="s" direction="in"/>
    </method>
    <method name="Stop">
      <arg type="b" direction="out"/>
      <arg name="json" type="s" direction="in"/>
    </method>
  </interface>
</node>
```

> **注意**：XML 中的接口名 `com.deepin.dde.GrandSearch.SearchPlugin` 是默认接口名。插件可使用自定义接口名，只需在配置文件的 `DBusInterface` 字段中保持一致即可。

---

## 参考示例

项目中提供了完整的计算器搜索插件示例，位于：
`examples/calculator-search-plugin/`

该示例展示了如何开发一个数学表达式计算插件——用户在全局搜索中输入数学表达式（如 `1+2*3`、`sin(0.5)`），插件实时计算并返回结果，点击结果可复制到剪贴板。详见示例目录中的 `README.md`。
