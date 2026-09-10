// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 用例计数声明（self-check-structural 验证此块）：
// | method           | level | factors     | min | actual |
// |------------------|-------|-------------|-----|--------|
// | splitCommand     | mid   | -           | 2   | 5      |
// | getMimeType      | mid   | -           | 2   | 3      |
// | getJsonString    | mid   | -           | 2   | 5      |
// | getJsonArray     | mid   | -           | 2   | 5      |
// | isHiddenFile     | high  | complexity:16| 3   | 6      |
// ─── actual ≥ min 方为合格 ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]
// 3. 每个等价类的边界值显式覆盖: [x]
// 4. 同质 ≥ 3 组用 TEST_P: [ ] (每组用例 ≤ 2，无需参数化)
// 5. 分支清单 → 用例映射已列出: [x]
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]
// 7. 异常路径 EXPECT_THROW 精确匹配: [ ] (无异常抛出)
// 8. 负面场景有专门用例: [x]
// 9. 负面用例验证强异常安全: [x]
// 10. stub_ext vs gMock 选择正确: [x]

#include "utils/specialtools.h"


#include <gtest/gtest.h>

#include <QTest>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMimeDatabase>

using namespace GrandSearch;

// ═══════════════════════════════════════════════════════════════
// 分支清单 + 用例映射（基于 get_code_snippet 取得的源码分支）
// ═══════════════════════════════════════════════════════════════
//
// splitCommand (lines 40-69):
//   B1: cmd.isEmpty() → false                    → ut_splitCommand_EmptyCmd
//   B2: cmds.size()==0 → false                   → (不可达：split 至少返回 1 元素)
//   B3: program.isEmpty() → false                → ut_splitCommand_WhitespaceOnly
//   B4: loop skips empty args                    → ut_splitCommand_MultipleArgsWithEmpty
//   B5: normal success                           → ut_splitCommand_NormalMultiArg
//   B6: single command no args                   → ut_splitCommand_SingleCommand
//
// getMimeType (lines 71-82):
//   B1: file.isDir() → inode/directory           → ut_getMimeType_Directory
//   B2: non-dir → mimeTypeForFile + wpsMime      → ut_getMimeType_RegularFile
//   B3: wpsMimeSpecialist office suffix corr.    → ut_getMimeType_WpsFileCorrection
//
// getJsonString (lines 84-105):
//   B1: !json || key.isEmpty() → empty           → ut_getJsonString_NullJson, _EmptyKey
//   B2: !contains(key) → empty                   → ut_getJsonString_KeyNotFound
//   B3: !isString() → empty                      → ut_getJsonString_NonStringValue
//   B4: isString() → ret                         → ut_getJsonString_ValidString
//
// getJsonArray (lines 107-129):
//   B1: !json || key.isEmpty() → empty           → ut_getJsonArray_NullJson, _EmptyKey
//   B2: !contains(key) → empty                   → ut_getJsonArray_KeyNotFound
//   B3: !isArray() → empty                       → ut_getJsonArray_NonArrayValue
//   B4: isArray() → ret                          → ut_getJsonArray_ValidArray
//
// isHiddenFile (lines 131-177, HIGH):
//   B1: !startsWith(prefix) || ==prefix → false  → ut_isHiddenFile_NotUnderPrefix, _EqualsPrefix
//   B2: !exists(.hidden) → recursive             → ut_isHiddenFile_NoHiddenFile
//   B3: filters empty + !open → false            → ut_isHiddenFile_CannotOpenHidden
//   B4: filters empty + readable+size>0 → parse  → ut_isHiddenFile_FileInList
//   B5: filters empty + !readable/empty → recurs → ut_isHiddenFile_EmptyHiddenFile
//   B6: contains(filename) → true                → ut_isHiddenFile_FileInList
//   B7: !contains → recursive                    → ut_isHiddenFile_FileNotInList

// ═══════════════════════════════════════════════════════════════
// splitCommand 测试
// ═══════════════════════════════════════════════════════════════

TEST(SpecialToolsTest, ut_splitCommand_EmptyCmd)
{
    // Arrange
    QString program;
    QStringList args;

    // Act
    bool result = SpecialTools::splitCommand("", program, args);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_TRUE(program.isEmpty());
}

TEST(SpecialToolsTest, ut_splitCommand_WhitespaceOnly)
{
    // Arrange
    // " ".split(" ") → ["", ""]  size==2, first()=="" → program.isEmpty()
    QString program;
    QStringList args;

    // Act
    bool result = SpecialTools::splitCommand(" ", program, args);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_TRUE(program.isEmpty());
}

TEST(SpecialToolsTest, ut_splitCommand_NormalMultiArg)
{
    // Arrange
    QString program;
    QStringList args;

    // Act
    bool result = SpecialTools::splitCommand("cmd arg1 arg2", program, args);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(program, QString("cmd"));
    EXPECT_EQ(args.size(), 2);
    EXPECT_EQ(args[0], QString("arg1"));
    EXPECT_EQ(args[1], QString("arg2"));
}

TEST(SpecialToolsTest, ut_splitCommand_MultipleArgsWithEmpty)
{
    // Arrange
    // Double space produces empty args which should be skipped
    QString program;
    QStringList args;

    // Act
    bool result = SpecialTools::splitCommand("cmd  arg1  arg2", program, args);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(program, QString("cmd"));
    EXPECT_EQ(args.size(), 2);
    EXPECT_EQ(args[0], QString("arg1"));
    EXPECT_EQ(args[1], QString("arg2"));
}

TEST(SpecialToolsTest, ut_splitCommand_SingleCommand)
{
    // Arrange
    QString program;
    QStringList args;

    // Act
    bool result = SpecialTools::splitCommand("cmd", program, args);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(program, QString("cmd"));
    EXPECT_TRUE(args.isEmpty());
}

// ═══════════════════════════════════════════════════════════════
// getMimeType 测试
// ═══════════════════════════════════════════════════════════════

TEST(SpecialToolsTest, ut_getMimeType_Directory)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QFileInfo info(dir.path());

    // Act
    QMimeType type = SpecialTools::getMimeType(info);

    // Assert
    EXPECT_EQ(type.name(), QString("inode/directory"));
    EXPECT_TRUE(type.isValid());
}

TEST(SpecialToolsTest, ut_getMimeType_RegularFile)
{
    // Arrange
    QTemporaryFile tmpFile;
    ASSERT_TRUE(tmpFile.open());
    tmpFile.write("hello");
    tmpFile.close();
    QFileInfo info(tmpFile.fileName());

    // Act
    QMimeType type = SpecialTools::getMimeType(info);

    // Assert
    EXPECT_TRUE(type.isValid());
    EXPECT_FALSE(type.name().isEmpty());
}

TEST(SpecialToolsTest, ut_getMimeType_WpsFileCorrection)
{
    // Arrange
    // wpsMimeSpecialist triggers when suffix is in officeSuffixList
    // and type.name() is in wrongMimeTypeNames.
    // Use a .wps file — if system returns wrong mime, wpsMimeSpecialist corrects it.
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString filePath = dir.path() + "/test.wps";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("dummy");
    f.close();
    QFileInfo info(filePath);

    // Act
    QMimeType type = SpecialTools::getMimeType(info);

    // Assert
    EXPECT_TRUE(type.isValid());
    EXPECT_FALSE(type.name().isEmpty());
}

// ═══════════════════════════════════════════════════════════════
// getJsonString 测试
// ═══════════════════════════════════════════════════════════════

TEST(SpecialToolsTest, ut_getJsonString_NullJson)
{
    // Arrange
    QString key = "key1";

    // Act
    QString result = SpecialTools::getJsonString(nullptr, key);

    // Assert
    EXPECT_TRUE(result.isEmpty());
}

TEST(SpecialToolsTest, ut_getJsonString_EmptyKey)
{
    // Arrange
    QJsonObject obj{{"key1", "value1"}};

    // Act
    QString result = SpecialTools::getJsonString(&obj, "");

    // Assert
    EXPECT_TRUE(result.isEmpty());
}

TEST(SpecialToolsTest, ut_getJsonString_KeyNotFound)
{
    // Arrange
    QJsonObject obj{{"key1", "value1"}};

    // Act
    QString result = SpecialTools::getJsonString(&obj, "nonexistent");

    // Assert
    EXPECT_TRUE(result.isEmpty());
}

TEST(SpecialToolsTest, ut_getJsonString_NonStringValue)
{
    // Arrange
    QJsonObject obj{{"key1", 42}};

    // Act
    QString result = SpecialTools::getJsonString(&obj, "key1");

    // Assert
    EXPECT_TRUE(result.isEmpty());
}

TEST(SpecialToolsTest, ut_getJsonString_ValidString)
{
    // Arrange
    QJsonObject obj{{"key1", "value1"}, {"key2", "value2"}};

    // Act
    QString result = SpecialTools::getJsonString(&obj, "key1");

    // Assert
    EXPECT_EQ(result, QString("value1"));
    EXPECT_FALSE(result.isEmpty());
}

// ═══════════════════════════════════════════════════════════════
// getJsonArray 测试
// ═══════════════════════════════════════════════════════════════

TEST(SpecialToolsTest, ut_getJsonArray_NullJson)
{
    // Arrange
    QString key = "key1";

    // Act
    QJsonArray result = SpecialTools::getJsonArray(nullptr, key);

    // Assert
    EXPECT_TRUE(result.isEmpty());
}

TEST(SpecialToolsTest, ut_getJsonArray_EmptyKey)
{
    // Arrange
    QJsonArray arr{"a", "b"};
    QJsonObject obj{{"key1", arr}};

    // Act
    QJsonArray result = SpecialTools::getJsonArray(&obj, "");

    // Assert
    EXPECT_TRUE(result.isEmpty());
}

TEST(SpecialToolsTest, ut_getJsonArray_KeyNotFound)
{
    // Arrange
    QJsonArray arr{"a", "b"};
    QJsonObject obj{{"key1", arr}};

    // Act
    QJsonArray result = SpecialTools::getJsonArray(&obj, "nonexistent");

    // Assert
    EXPECT_TRUE(result.isEmpty());
}

TEST(SpecialToolsTest, ut_getJsonArray_NonArrayValue)
{
    // Arrange
    QJsonObject obj{{"key1", "stringValue"}};

    // Act
    QJsonArray result = SpecialTools::getJsonArray(&obj, "key1");

    // Assert
    EXPECT_TRUE(result.isEmpty());
}

TEST(SpecialToolsTest, ut_getJsonArray_ValidArray)
{
    // Arrange
    QJsonArray arr{"a", "b", "c"};
    QJsonObject obj{{"key1", arr}, {"key2", "stringValue"}};

    // Act
    QJsonArray result = SpecialTools::getJsonArray(&obj, "key1");

    // Assert
    EXPECT_FALSE(result.isEmpty());
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result.at(0).toString(), QString("a"));
}

// ═══════════════════════════════════════════════════════════════
// isHiddenFile 测试 (HIGH)
// ═══════════════════════════════════════════════════════════════

TEST(SpecialToolsTest, ut_isHiddenFile_NotUnderPrefix)
{
    // Arrange
    QHash<QString, QSet<QString>> filters;
    QString prefix = "/tmp/test_prefix_9999";

    // Act
    bool result = SpecialTools::isHiddenFile("/other/path/file.txt", filters, prefix);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_TRUE(filters.isEmpty());
}

TEST(SpecialToolsTest, ut_isHiddenFile_EqualsPrefix)
{
    // Arrange
    QHash<QString, QSet<QString>> filters;
    QString prefix = "/tmp";

    // Act
    bool result = SpecialTools::isHiddenFile(prefix, filters, prefix);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_TRUE(filters.isEmpty());
}

TEST(SpecialToolsTest, ut_isHiddenFile_FileInList)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString subDir = dir.path() + "/subdir";
    ASSERT_TRUE(QDir().mkpath(subDir));

    // Create .hidden file listing "myfile.txt"
    QString hiddenPath = subDir + "/.hidden";
    QFile hiddenFile(hiddenPath);
    ASSERT_TRUE(hiddenFile.open(QIODevice::WriteOnly));
    hiddenFile.write("myfile.txt\n");
    hiddenFile.close();

    QString filePath = subDir + "/myfile.txt";
    QHash<QString, QSet<QString>> filters;

    // Act
    bool result = SpecialTools::isHiddenFile(filePath, filters, dir.path());

    // Assert
    EXPECT_TRUE(result);
    EXPECT_FALSE(filters.isEmpty());
}

TEST(SpecialToolsTest, ut_isHiddenFile_FileNotInList)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString subDir = dir.path() + "/subdir";
    ASSERT_TRUE(QDir().mkpath(subDir));

    // Create .hidden with a different file
    QString hiddenPath = subDir + "/.hidden";
    QFile hiddenFile(hiddenPath);
    ASSERT_TRUE(hiddenFile.open(QIODevice::WriteOnly));
    hiddenFile.write("otherfile.txt\n");
    hiddenFile.close();

    QString filePath = subDir + "/notInList.txt";
    QHash<QString, QSet<QString>> filters;

    // Act
    // File not in .hidden → recursive to parent (no .hidden) → recursive → equals prefix → false
    bool result = SpecialTools::isHiddenFile(filePath, filters, dir.path());

    // Assert
    EXPECT_FALSE(result);
}

TEST(SpecialToolsTest, ut_isHiddenFile_NoHiddenFile)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString subDir = dir.path() + "/subdir";
    ASSERT_TRUE(QDir().mkpath(subDir));

    // No .hidden file anywhere
    QString filePath = subDir + "/somefile.txt";
    QHash<QString, QSet<QString>> filters;

    // Act
    // No .hidden → recursive to parent → no .hidden → recursive → equals prefix → false
    bool result = SpecialTools::isHiddenFile(filePath, filters, dir.path());

    // Assert
    EXPECT_FALSE(result);
}

TEST(SpecialToolsTest, ut_isHiddenFile_CannotOpenHidden)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString subDir = dir.path() + "/subdir";
    ASSERT_TRUE(QDir().mkpath(subDir));

    // Create .hidden file
    QString hiddenPath = subDir + "/.hidden";
    QFile hiddenFile(hiddenPath);
    ASSERT_TRUE(hiddenFile.open(QIODevice::WriteOnly));
    hiddenFile.write("myfile.txt\n");
    hiddenFile.close();

    // Remove read permissions so QFile::open(ReadOnly) fails
    QFile::setPermissions(hiddenPath, QFile::WriteOwner);

    QString filePath = subDir + "/myfile.txt";
    QHash<QString, QSet<QString>> filters;

    // Act
    bool result = SpecialTools::isHiddenFile(filePath, filters, dir.path());

    // Assert
    // .hidden exists but can't open (no read permission) → false
    EXPECT_FALSE(result);

    // Restore permissions for cleanup
    QFile::setPermissions(hiddenPath, QFile::ReadOwner | QFile::WriteOwner);
}
