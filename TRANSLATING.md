# XTPlayer 翻译指南

本文档面向希望为 XTPlayer 添加新语言或改进现有翻译的贡献者。XTPlayer 使用 Qt 官方的 Linguist 国际化框架（`tr()` + `.ts`/`.qm`），这是 Qt 生态中最标准、最易扩展的翻译方案。

## 目录

- [架构概述](#架构概述)
- [前置条件](#前置条件)
- [添加新语言（5 步快速开始）](#添加新语言5-步快速开始)
- [翻译规则与注意事项](#翻译规则与注意事项)
- [使用 Qt Linguist（推荐）](#使用-qt-linguist推荐)
- [手动编辑 .ts 文件](#手动编辑-ts-文件)
- [编译 .qm 文件](#编译-qm-文件)
- [测试翻译](#测试翻译)
- [提交贡献](#提交贡献)
- [常见问题](#常见问题)

---

## 架构概述

XTPlayer 的国际化（i18n）由以下部分组成：

| 组件 | 位置 | 作用 |
|---|---|---|
| `tr()` 调用 | `src/**/*.cpp`、`src/**/*.ui` | 标记需要翻译的字符串 |
| 翻译源文件 | `src/translations/xtplayer_<lang>.ts` | XML 格式的翻译源（人类可编辑） |
| 编译后翻译 | `src/translations/xtplayer_<lang>.qm`（构建时生成到 `build-*/release/translations/`） | 二进制格式，运行时加载 |
| 语言持久化 | `src/xtpsettings.h/.cpp` | `getLanguage()`/`setLanguage()`，存储在 settings 的 `language` 键 |
| 语言选择 UI | `src/settings.ui` + `src/settingsdialog.cpp` | System 标签页的语言下拉框 |
| 翻译加载 | `src/main.cpp` | 启动时根据语言加载 `.qm` 文件 |
| 构建规则 | `src/XTPlayer.pro` | `TRANSLATIONS` 声明 + `lrelease` 自动编译规则 |

**工作流程**：源码中的 `tr("English")` → `lupdate` 提取到 `.ts` → 翻译者填写译文 → `lrelease` 编译为 `.qm` → 运行时 `QTranslator` 加载。

## 前置条件

1. **Qt 6.11+**（含 Linguist 工具：`lupdate`、`lrelease`、Qt Linguist GUI）
   - 通过 Qt 在线安装器安装时，勾选 "Qt Linguist" 组件
   - 或从源码构建 Qt 时包含 qttools 模块
2. **XTPlayer 源码**（本仓库）
3. 文本编辑器（推荐 Qt Linguist GUI，也支持 VS Code + XML 插件）

验证工具可用：
```bash
lupdate -version
lrelease -version
```

---

## 添加新语言（5 步快速开始）

以添加法语（`fr`）为例：

### 步骤 1：在 `.pro` 文件声明新语言

编辑 [`src/XTPlayer.pro`](src/XTPlayer.pro)，找到 `TRANSLATIONS` 行，追加新语言：

```pro
TRANSLATIONS += translations/xtplayer_zh_CN.ts \
                translations/xtplayer_fr.ts
```

> **提示**：每行用 `\` 续行。语言代码遵循 [Qt 语言代码规范](https://doc.qt.io/qt-6/i18n-source-translation.html#language-codes)，如 `zh_CN`、`fr`、`de`、`ja`、`es` 等。

### 步骤 2：生成 `.ts` 文件

在 `src/` 目录下运行 `lupdate`，它会扫描所有 `tr()` 调用和 `.ui` 文件，生成或更新 `.ts` 文件：

```bash
cd src
lupdate XTPlayer.pro
```

这会为每个 `TRANSLATIONS` 中声明的语言生成 `translations/xtplayer_<lang>.ts`。已有翻译的条目会保留（标记为 `<translation type="unfinished">`），新增的字符串会自动加入。

### 步骤 3：翻译字符串

用 **Qt Linguist**（推荐）或文本编辑器打开 `src/translations/xtplayer_fr.ts`，逐条填写译文。详见 [翻译规则与注意事项](#翻译规则与注意事项)。

```bash
linguist src/translations/xtplayer_fr.ts
```

### 步骤 4：在设置对话框注册新语言

编辑 [`src/settingsdialog.cpp`](src/settingsdialog.cpp)，找到 `setupUi()` 函数中的语言下拉框填充代码（搜索 `languageComboBox`），追加新选项：

```cpp
ui.languageComboBox->blockSignals(true);
ui.languageComboBox->addItem(QStringLiteral("English"), QStringLiteral("en"));
ui.languageComboBox->addItem(QStringLiteral("简体中文"), QStringLiteral("zh_CN"));
ui.languageComboBox->addItem(QStringLiteral("Français"), QStringLiteral("fr"));  // 新增
{
    int langIdx = ui.languageComboBox->findData(XTPSettings::getLanguage());
    if (langIdx < 0)
        langIdx = 0; // default to English
    ui.languageComboBox->setCurrentIndex(langIdx);
}
ui.languageComboBox->blockSignals(false);
```

> **重要**：语言显示名（如 "Français"）**不要**用 `tr()` 包裹。语言名应始终以本国文字显示，这样用户无论当前界面是什么语言，都能找到自己的语言。

### 步骤 5：编译并测试

```bash
cd src
lrelease XTPlayer.pro
```

构建 XTPlayer，启动后在 **Settings → System → Language** 下拉框中选择新语言，按提示重启即可。详见 [测试翻译](#测试翻译)。

---

## 翻译规则与注意事项

### 1. 翻译上下文（context）

`.ts` 文件按 `<context>` 分块，每个 context 对应一个 `Q_OBJECT` 类。**`<source>` 字符串必须在正确的 context 下才能被翻译**。

| 来源 | context 名 | 说明 |
|---|---|---|
| `mainwindow.ui` | `MainWindow` | uic `<class>` 名 |
| `settings.ui` | `SettingsDialog` | uic `<class>` 名 |
| `welcomedialog.ui` | `welcomedialog`（**小写！**） | uic `<class>` 名 |
| `dlnascriptlinks.ui` | `DLNAScriptLinksDialog` | uic `<class>` 名 |
| `libraryexclusions.ui` | `LibraryExclusionsDialog` | uic `<class>` 名 |
| `*.cpp` 中的 `tr()` | 所在类名（如 `SettingsDialog`） | `Q_OBJECT` 类名 |
| `main.cpp` 自由函数 | `XTPlayerApp` | 用 `QCoreApplication::translate("XTPlayerApp", ...)` |

其他 `.cpp` context：`AddChannelDialog`、`PlaylistDialog`、`GetTextDialog`、`NoMatchingScriptDialog`、`PlayerControls`、`InputMapWidget`、`DialogHandler`、`LibraryItemMetadataDialog`、`ChannelTableViewModel`、`LibraryListViewModel`、`TimeLine`、`RangeSlider`、`XLibraryList`。

> **关键**：`lupdate` 会自动正确分类 context。如果你手动编辑 `.ts`，务必确保 context 名与上表一致，否则翻译会静默失效。

### 2. `<source>` 必须与源码逐字节一致

`.ts` 中的 `<source>` 必须与源码中 `tr("...")` 的字符串完全一致（含标点、空格、大小写）。`lupdate` 自动生成时保证一致；手动编辑时切勿修改 `<source>`，只改 `<translation>`。

XML 转义规则：
- `&` → `&amp;`
- `<` → `&lt;`
- `>` → `&gt;`
- `"` → `&quot;`（在属性值中）

### 3. 助记符（`&` 加速键）

菜单和按钮常用 `&` 标记加速键字母，如 `&File`（Alt+F）。翻译时保留拉丁字母加速键，格式为 `译文(&X)`：

```xml
<source>&File</source>
<translation>文件(&F)</translation>
```

规则：
- 同一对话框内避免两个控件共用相同字母
- 优先选择译文首字母或语义相关字母
- 如果译文不含拉丁字母（如纯中文），必须用 `(&X)` 形式追加

### 4. 占位符 `%1`、`%2`

含 `QString::arg()` 的字符串会有 `%1`、`%2` 等占位符。译文中**必须保留**相同数量和编号的占位符，但顺序可以调整：

```xml
<source>Found %1 files in %2 folders</source>
<translation>在 %2 个文件夹中找到 %1 个文件</translation>
```

### 5. HTML 内容

部分工具提示和标签使用 HTML。翻译时**只翻译文本节点，保留所有 HTML 标签**：

```xml
<source>Choose from the list (click to test or right click and copy link)<br>to enter into your devices browser. You can also test:<br>&lt;a href='http://localhost%1/'&gt;&lt;span&gt;http://localhost%1/&lt;/span&gt;&lt;/a&gt; ONLY on the local machine.</source>
<translation>从列表中选择（点击测试或右键复制链接）<br>输入到你的设备浏览器中。你也可以测试：<br>&lt;a href='http://localhost%1/'&gt;&lt;span&gt;http://localhost%1/&lt;/span&gt;&lt;/a&gt; 仅限本机。</translation>
```

### 6. 多行字符串 `\n`

源码中的 `\n`（换行）在 `.ts` 中可以：
- 保持为单行（`lrelease` 会处理）
- 或在 `<source>`/`<translation>` 中用字面换行

建议保持单行，用 `\n` 转义序列表示换行。

### 7. 不应翻译的内容

以下内容**不要**翻译：
- `LogHandler::Info/Debug/Warn/Error` 的消息（日志，非用户可见）
- `qDebug`/`qWarning`/`qCritical` 输出
- `QSettings` 的键名（如 `"language"`、`"selectedTheme"`）
- Widget 的 `objectName`
- 文件路径和资源名
- 语言名自身（如 "English"、"简体中文" 在下拉框中不翻译）

### 8. 翻译状态标记

`.ts` 文件中每条 `<translation>` 有状态：
- `type="unfinished"` — 未翻译或源串已变更，需处理
- `type="obsolete"` — 源串已从代码中删除，可安全移除
- 无 `type` 属性 — 已完成

`lupdate` 更新时会自动设置这些标记。翻译完成后，用 Qt Linguist 标记为 "finished"，或手动删除 `type="unfinished"` 属性。

---

## 使用 Qt Linguist（推荐）

Qt Linguist 是 Qt 官方的翻译 GUI 工具，比手动编辑 XML 高效得多。

### 基本流程

1. **打开 `.ts` 文件**：`linguist translations/xtplayer_fr.ts`
2. **选择 context**：左侧面板列出所有 context（类名），点击展开
3. **逐条翻译**：
   - 中间面板显示源串和当前译文
   - 下方输入译文
   - 工具栏的 ✓ 按钮标记为完成（删除 `unfinished`）
4. **验证**：菜单 `Validation → Validate` 检查占位符、加速键等
5. **保存**

### 实用快捷键

| 快捷键 | 功能 |
|---|---|
| `Ctrl+Enter` | 标记当前条目为完成并跳到下一条 |
| `Ctrl+Shift+Enter` | 标记为完成但不跳转 |
| `Ctrl+Z` | 撤销 |
| `F3` | 查找下一个未翻译条目 |

### 批量操作

- `Translation → Find Unfinished`：跳到所有未完成条目
- `Translation → Finish All`：批量标记完成（谨慎使用）

---

## 手动编辑 .ts 文件

如果没有 Qt Linguist，可以用任何文本编辑器（VS Code、Notepad++ 等）直接编辑 `.ts` 文件。它本质是 XML：

```xml
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="fr">
<context>
    <name>MainWindow</name>
    <message>
        <source>&amp;File</source>
        <translation>Fichier(&amp;F)</translation>
    </message>
    <message>
        <source>Found %1 files</source>
        <translation>%1 fichiers trouvés</translation>
    </message>
</context>
</TS>
```

注意事项：
- 文件编码必须是 **UTF-8**
- `<TS>` 标签的 `language` 属性设为目标语言代码
- 只修改 `<translation>` 内容，**不要**改 `<source>`
- 翻译完成后删除 `type="unfinished"` 属性（或用 Qt Linguist 标记）

---

## 编译 .qm 文件

`.ts` 是人类可编辑的源文件，`.qm` 是运行时加载的二进制文件。构建 XTPlayer 时，`XTPlayer.pro` 中的 `lrelease` 规则会自动编译所有 `.ts` 为 `.qm`，输出到构建目录的 `translations/` 子目录。

### 方式 1：随构建自动编译（推荐）

正常构建项目即可，`lrelease` 规则会自动执行：
```bash
cd src
qmake XTPlayer.pro
make   # 或 nmake / mingw32-make
```

输出：`build-release/release/translations/xtplayer_<lang>.qm`

### 方式 2：手动编译

只编译翻译文件，不构建整个项目：
```bash
cd src
lrelease XTPlayer.pro
```

或编译单个语言：
```bash
lrelease translations/xtplayer_fr.ts -qm translations/xtplayer_fr.qm
```

### 方式 3：Qt Linguist 内编译

在 Qt Linguist 中：`File → Release`，会生成同名 `.qm` 文件。

---

## 测试翻译

### 快速测试（不重新构建）

1. 编译 `.qm`：`lrelease src/translations/xtplayer_fr.ts -qm <deploy_dir>/translations/xtplayer_fr.qm`
2. 将 `.qm` 复制到 XTPlayer 可执行文件同级的 `translations/` 目录
3. 启动 XTPlayer → Settings → System → Language → 选择新语言
4. 弹出重启提示 → 点 Yes 重启
5. 界面应变为新语言

### 完整测试

1. 完整构建 XTPlayer（`qmake` + `make`）
2. 确认构建日志中没有 `lrelease not found` 警告
3. 检查 `build-release/release/translations/` 下有 `xtplayer_<lang>.qm`
4. 启动应用，验证：
   - 菜单栏（含 `&` 助记符和 Alt 快捷键）
   - 工具栏提示
   - 设置对话框所有标签页
   - 各子对话框（添加通道、播放列表、欢迎页等）
   - QMessageBox 消息
   - 含 `%1` 占位符的消息（如播放列表数量）显示正确数值
   - HTML 工具提示正常渲染
5. 切回 English 重启，验证可逆性
6. 重启后再次打开设置，确认下拉框停在所选语言（持久化生效）

### 标准对话框按钮（OK/Cancel 等）

Qt 自带的标准对话框按钮（OK、Cancel、Yes、No 等）由 `qtbase_<lang>.qm` 翻译，随 Qt 发行。`main.cpp` 会自动尝试加载它。如果你的系统 Qt 安装缺少对应语言的 `qtbase` 翻译，标准按钮会保持英文——这是可接受的，或可从 Qt 安装目录的 `translations/` 手动复制 `qtbase_<lang>.qm` 到部署目录。

---

## 提交贡献

### 文件清单

提交翻译贡献时，应包含以下文件：

| 文件 | 说明 | 必须 |
|---|---|---|
| `src/translations/xtplayer_<lang>.ts` | 翻译源文件 | ✅ |
| `src/XTPlayer.pro` | 添加了 `TRANSLATIONS` 条目 | ✅ |
| `src/settingsdialog.cpp` | 添加了下拉框选项 | ✅ |
| `src/translations/xtplayer_<lang>.qm` | 编译后的翻译 | ❌（`.gitignore` 忽略，构建时自动生成） |

### 提交信息格式

```
Add <Language Name> (<lang code>) translation

- Add src/translations/xtplayer_<lang>.ts with <N> translated strings
- Register language in settingsdialog.cpp language dropdown
- Update XTPlayer.pro TRANSLATIONS
```

### 注意事项

- **不要**提交 `.qm` 文件（`.gitignore` 已忽略，构建时自动生成）
- **不要**修改 `.ts` 中的 `<source>` 标签
- 确保所有条目都已标记为完成（无 `type="unfinished"`）
- 运行 `lupdate` 确保没有遗漏的字符串
- 如果改进现有翻译，只修改 `<translation>` 部分

---

## 常见问题

### Q: 翻译后界面没有变化？

**A:** 检查以下几点：
1. `.qm` 文件是否在 `<exe_dir>/translations/` 目录下
2. `.qm` 文件名是否为 `xtplayer_<lang>.qm`（与语言代码一致）
3. `.ts` 中的 `<source>` 是否与源码字符串逐字节一致
4. context 名是否正确（见[翻译上下文](#1-翻译上下文context)）
5. 是否重启了应用（语言切换需要重启生效）
6. 启动时的控制台日志是否有 `i18n: failed to load app translation` 警告

### Q: `lupdate` 报错 "lrelease not found"？

**A:** 这是 `.pro` 的优雅降级提示。安装 Qt Linguist 工具（通常随 Qt 安装），或单独安装 `qttools` 模块。conda-forge 的 `qt6-main` 包不含 Linguist 工具，需要从 Qt 官方安装器获取。

### Q: 部分字符串没有被提取到 `.ts`？

**A:** 该字符串可能没有用 `tr()` 包裹。在源码中搜索该字符串，确认它被 `tr("...")` 或 `QCoreApplication::translate("Context", "...")` 包裹。如果是 `.ui` 文件中的字符串，`lupdate` 会自动提取。

### Q: 如何更新翻译（源码变更后）？

**A:** 重新运行 `lupdate src/XTPlayer.pro`。它会：
- 保留已翻译的条目
- 新增的字符串标记为 `<translation type="unfinished">`
- 已删除的字符串标记为 `<message type="obsolete">`（可手动清理）

### Q: 可以同时翻译多个语言吗？

**A:** 可以。在 `.pro` 的 `TRANSLATIONS` 中声明多个 `.ts` 文件，`lupdate` 会同时更新所有语言。用 Qt Linguist 可以同时打开多个 `.ts` 文件。

### Q: 翻译进度怎么看？

**A:** Qt Linguist 状态栏显示进度百分比。或在命令行：
```bash
# 统计未翻译条目数
grep 'type="unfinished"' src/translations/xtplayer_<lang>.ts | wc -l
```

---

## 参考

- [Qt 国际化文档](https://doc.qt.io/qt-6/i18n-source-translation.html)
- [Qt Linguist 手册](https://doc.qt.io/qt-6/linguist-manual.html)
- [Qt 语言代码列表](https://doc.qt.io/qt-6/qlocale.html#Language-enum)
- 现有中文翻译参考：[`src/translations/xtplayer_zh_CN.ts`](src/translations/xtplayer_zh_CN.ts)
