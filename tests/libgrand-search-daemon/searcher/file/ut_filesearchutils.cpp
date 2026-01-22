// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "searcher/file/filesearchutils.h"
#include "utils/specialtools.h"
#include "global/builtinsearch.h"
#include "global/searchhelper.h"
#include "configuration/configer.h"
#include "configuration/configer_p.h"
#include "stubext.h"
#include "global/searchconfigdefine.h"

#include <gtest/gtest.h>

GRANDSEARCH_USE_NAMESPACE

TEST(FileSearchUtilsTest, ut_packItem)
{
    stub_ext::StubExt st;
    st.set_lamda(&QFileInfo::fileName, []() { return "test"; });
    st.set_lamda(GrandSearch::SpecialTools::getMimeType, [](const QFileInfo &) {
        QMimeType mimeType;
        return mimeType;
    });
    st.set_lamda(&QMimeType::name, []() { return "test"; });
    st.set_lamda(&QMimeType::iconName, []() { return "test.icon"; });
    st.set_lamda(&Configer::group, []() { return ConfigerPrivate::tailerData(); });

    EXPECT_NO_FATAL_FAILURE(FileSearchUtils::packItem("test", "searcher name"));
}

TEST(FileSearchUtilsTest, ut_groupKey)
{
    EXPECT_EQ(GRANDSEARCH_GROUP_FILE, FileSearchUtils::groupKey(FileSearchUtils::File));
    EXPECT_EQ(GRANDSEARCH_GROUP_FOLDER, FileSearchUtils::groupKey(FileSearchUtils::Folder));
    EXPECT_EQ(GRANDSEARCH_GROUP_FILE_PICTURE, FileSearchUtils::groupKey(FileSearchUtils::Picture));
    EXPECT_EQ(GRANDSEARCH_GROUP_FILE_AUDIO, FileSearchUtils::groupKey(FileSearchUtils::Audio));
    EXPECT_EQ(GRANDSEARCH_GROUP_FILE_VIDEO, FileSearchUtils::groupKey(FileSearchUtils::Video));
    EXPECT_EQ(GRANDSEARCH_GROUP_FILE_DOCUMNET, FileSearchUtils::groupKey(FileSearchUtils::Document));
}

TEST(FileSearchUtilsTest, ut_getGroupByName)
{
    {
        stub_ext::StubExt st;
        st.set_lamda(&QFileInfo::isDir, []() { return true; });
        EXPECT_EQ(FileSearchUtils::Folder, FileSearchUtils::getGroupByName("test"));
    }

    {
        stub_ext::StubExt st;
        st.set_lamda(&QFileInfo::isDir, []() { return false; });
        st.set_lamda(&QFileInfo::suffix, []() { return "png"; });
        EXPECT_EQ(FileSearchUtils::Picture, FileSearchUtils::getGroupByName("test.png"));
    }
}

TEST(FileSearchUtilsTest, ut_getGroupBySuffix)
{
    EXPECT_EQ(FileSearchUtils::Unknown, FileSearchUtils::getGroupBySuffix(""));
    EXPECT_EQ(FileSearchUtils::Document, FileSearchUtils::getGroupBySuffix("txt"));
    EXPECT_EQ(FileSearchUtils::Picture, FileSearchUtils::getGroupBySuffix("png"));
    EXPECT_EQ(FileSearchUtils::Video, FileSearchUtils::getGroupBySuffix("mp4"));
    EXPECT_EQ(FileSearchUtils::Audio, FileSearchUtils::getGroupBySuffix("mp3"));
    EXPECT_EQ(FileSearchUtils::File, FileSearchUtils::getGroupBySuffix("zip"));
}

TEST(FileSearchUtilsTest, ut_getGroupByGroupName)
{
    EXPECT_EQ(FileSearchUtils::Unknown, FileSearchUtils::getGroupByGroupName("test"));
    EXPECT_EQ(FileSearchUtils::Folder, FileSearchUtils::getGroupByGroupName(FOLDER_GROUP));
    EXPECT_EQ(FileSearchUtils::Document, FileSearchUtils::getGroupByGroupName(DOCUMENT_GROUP));
    EXPECT_EQ(FileSearchUtils::Picture, FileSearchUtils::getGroupByGroupName(PICTURE_GROUP));
    EXPECT_EQ(FileSearchUtils::Video, FileSearchUtils::getGroupByGroupName(VIDEO_GROUP));
    EXPECT_EQ(FileSearchUtils::Audio, FileSearchUtils::getGroupByGroupName(AUDIO_GROUP));
    EXPECT_EQ(FileSearchUtils::File, FileSearchUtils::getGroupByGroupName(FILE_GROUP));
}

TEST(FileSearchUtilsTest, ut_parseContent1)
{
    auto info = FileSearchUtils::parseContent("test");
    EXPECT_EQ("test", info.keyword);
}

TEST(FileSearchUtilsTest, ut_parseContent2)
{
    QString content = R"({
                      "Group":[
                          "text",
                          "app",
                          ""
                      ],
                      "Keyword":[
                          "abc",
                          "xyz",
                          ""
                      ],
                      "Suffix":[
                          "jpg",
                          ""
                      ]
                  })";
    auto info = FileSearchUtils::parseContent(content);
    EXPECT_TRUE(info.isCombinationSearch);
}

TEST(FileSearchUtilsTest, ut_fileShouldVisible)
{
    {
        FileSearchUtils::Group group = FileSearchUtils::File;
        FileSearchUtils::SearchInfo info;
        EXPECT_TRUE(FileSearchUtils::fileShouldVisible("test", group, info));
    }

    {
        stub_ext::StubExt st;
        st.set_lamda(&QFileInfo::isDir, []() { return true; });

        FileSearchUtils::Group group = FileSearchUtils::File;
        FileSearchUtils::SearchInfo info;
        info.isCombinationSearch = true;
        EXPECT_FALSE(FileSearchUtils::fileShouldVisible("test", group, info));
    }

    {
        stub_ext::StubExt st;
        st.set_lamda(&QFileInfo::isDir, []() { return false; });
        st.set_lamda(&QFileInfo::suffix, []() { return "png"; });

        FileSearchUtils::Group group = FileSearchUtils::Picture;
        FileSearchUtils::SearchInfo info;
        info.isCombinationSearch = true;
        EXPECT_FALSE(FileSearchUtils::fileShouldVisible("test.png", group, info));
    }

    {
        stub_ext::StubExt st;
        st.set_lamda(&QFileInfo::isDir, []() { return false; });
        st.set_lamda(&QFileInfo::suffix, []() { return "png"; });

        FileSearchUtils::Group group = FileSearchUtils::Picture;
        FileSearchUtils::SearchInfo info;
        info.isCombinationSearch = true;
        info.groupList << FileSearchUtils::File;
        EXPECT_TRUE(FileSearchUtils::fileShouldVisible("test.png", group, info));
        EXPECT_EQ(group, FileSearchUtils::File);
    }
}

TEST(FileSearchUtilsTest, ut_tailerData)
{
    stub_ext::StubExt st;

    QVariantHash tailConfig({ { GRANDSEARCH_TAILER_PARENTDIR, true },
                              { GRANDSEARCH_TAILER_TIMEMODEFIED, true } });
    UserPreferencePointer upp(new UserPreference(tailConfig));
    st.set_lamda(&Configer::group, [&upp](Configer *, const QString &name) {
        if (name == GRANDSEARCH_TAILER_GROUP)
            return upp;

        return UserPreferencePointer();
    });

    QFileInfo info(QDir::homePath());
    auto res = FileSearchUtils::tailerData(info);

    EXPECT_TRUE(res.contains(GRANDSEARCH_PROPERTY_ITEM_TAILER));
    auto datas = res[GRANDSEARCH_PROPERTY_ITEM_TAILER].toStringList();
    EXPECT_FALSE(datas.isEmpty());
}

TEST(FileSearchUtilsTest, ut_filterByBlacklist)
{
    stub_ext::StubExt st;

    QVariantHash tailConfig({ { GRANDSEARCH_BLACKLIST_PATH, { "/test" } } });
    UserPreferencePointer upp(new UserPreference(tailConfig));
    st.set_lamda(&Configer::group, [&upp](Configer *, const QString &name) {
        if (name == GRANDSEARCH_BLACKLIST_GROUP)
            return upp;

        return UserPreferencePointer();
    });

    EXPECT_TRUE(FileSearchUtils::filterByBlacklist("/test/xxx"));
}

TEST(FileSearchUtilsTest, ut_isPinyin)
{
    stub_ext::StubExt st;
    
    // Mock isPinyinAcronymSequence to return true for pure letter sequences
    st.set_lamda(DFMSEARCH::Global::isPinyinAcronymSequence, [](const QString &str) {
        // Simple mock: return true if string contains only lowercase letters
        for (const QChar &ch : str) {
            if (!ch.isLower() || !ch.isLetter()) {
                return false;
            }
        }
        return !str.isEmpty();
    });
    
    // Test pure pinyin
    EXPECT_TRUE(FileSearchUtils::isPinyin("nh"));
    EXPECT_TRUE(FileSearchUtils::isPinyin("nihao"));
    
    // Test pinyin with exclamation marks - should still detect pinyin
    EXPECT_TRUE(FileSearchUtils::isPinyin("nh!"));
    EXPECT_TRUE(FileSearchUtils::isPinyin("nh！"));
    EXPECT_TRUE(FileSearchUtils::isPinyin("!nh"));
    EXPECT_TRUE(FileSearchUtils::isPinyin("n!h"));
    
    // Test empty string
    EXPECT_FALSE(FileSearchUtils::isPinyin(""));
    
    // Test only special characters
    EXPECT_FALSE(FileSearchUtils::isPinyin("!"));
    EXPECT_FALSE(FileSearchUtils::isPinyin("！"));
    EXPECT_FALSE(FileSearchUtils::isPinyin("!@#"));
}

TEST(FileSearchUtilsTest, normalizePinyinKeyword)
{
    // Test pure pinyin - should remain unchanged
    EXPECT_EQ(FileSearchUtils::normalizePinyinKeyword("nh"), "nh");
    EXPECT_EQ(FileSearchUtils::normalizePinyinKeyword("nihao"), "nihao");
    
    // Test pinyin with special characters - should extract only letters
    EXPECT_EQ(FileSearchUtils::normalizePinyinKeyword("nh!"), "nh");
    EXPECT_EQ(FileSearchUtils::normalizePinyinKeyword("nh！"), "nh");
    EXPECT_EQ(FileSearchUtils::normalizePinyinKeyword("!nh"), "nh");
    EXPECT_EQ(FileSearchUtils::normalizePinyinKeyword("n!h"), "nh");
    EXPECT_EQ(FileSearchUtils::normalizePinyinKeyword("n!h@o"), "nho");
    
    // Test empty string
    EXPECT_EQ(FileSearchUtils::normalizePinyinKeyword(""), "");
    
    // Test only special characters - should return empty
    EXPECT_EQ(FileSearchUtils::normalizePinyinKeyword("!"), "");
    EXPECT_EQ(FileSearchUtils::normalizePinyinKeyword("！"), "");
    EXPECT_EQ(FileSearchUtils::normalizePinyinKeyword("!@#"), "");
}

TEST(FileSearchUtilsTest, charsEquivalent)
{
    // Test: 直接相等
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('a', 'a'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('!', '!'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('！', '！'));
    
    // Test: Unicode 全角半角转换 - 标点符号
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('!', '！'));  // 半角 -> 全角
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('！', '!'));  // 全角 -> 半角
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('?', '？'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('？', '?'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('(', '（'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent(')', '）'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('[', '［'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent(']', '］'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent(',', '，'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('.', '．'));  // 注意：不是句号 '。'
    EXPECT_TRUE(FileSearchUtils::charsEquivalent(';', '；'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent(':', '：'));
    
    // Test: Unicode 全角半角转换 - 数学符号
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('+', '＋'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('-', '－'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('*', '＊'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('/', '／'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('=', '＝'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('<', '＜'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('>', '＞'));
    
    // Test: Unicode 全角半角转换 - 其他符号
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('@', '＠'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('#', '＃'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('$', '＄'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('%', '％'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('&', '＆'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('_', '＿'));
    EXPECT_TRUE(FileSearchUtils::charsEquivalent('~', '～'));
        
    // Test: 不等价的字符
    EXPECT_FALSE(FileSearchUtils::charsEquivalent('!', '?'));
    EXPECT_FALSE(FileSearchUtils::charsEquivalent('！', '？'));
    EXPECT_FALSE(FileSearchUtils::charsEquivalent('(', ')'));
    EXPECT_FALSE(FileSearchUtils::charsEquivalent('a', 'b'));
    
    // Test: 中文标点符号（不在 Unicode 全角半角范围内）
    // 注意：'。' (U+3002) 不等价于 '.' (U+002E) 或 '．' (U+FF0E)
    EXPECT_FALSE(FileSearchUtils::charsEquivalent('.', '。'));
    EXPECT_FALSE(FileSearchUtils::charsEquivalent('。', '.'));
}

TEST(FileSearchUtilsTest, matchSpecialChars)
{
    // Test: 搜索关键字没有特殊字符，应该匹配任何文件
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/你好.txt", "nh"));
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/你好！.txt", "nh"));
    
    // Test: 搜索关键字有特殊字符，文件名也有相同特殊字符，应该匹配
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/你好！.txt", "nh!"));
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/你好！.txt", "nh！"));
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/test-file.txt", "tf-"));
    
    // Test: Unicode 全角半角等价匹配
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/你好！.txt", "nh!"));  // 英文!匹配中文！
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/你好!.txt", "nh！"));  // 中文！匹配英文!
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/你好？.txt", "nh?"));  // 英文?匹配中文？
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/你好?.txt", "nh？"));  // 中文？匹配英文?
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/测试（1）.txt", "cs(1)"));  // 英文括号匹配中文括号
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/测试(1).txt", "cs（1）"));  // 中文括号匹配英文括号
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/文件＠test.txt", "wj@test"));  // 英文@匹配全角＠
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/文件@test.txt", "wj＠test"));  // 全角＠匹配英文@
    
    // Test: 搜索关键字有特殊字符，但文件名没有，不应该匹配
    EXPECT_FALSE(FileSearchUtils::matchSpecialChars("/home/user/你好.txt", "nh!"));
    EXPECT_FALSE(FileSearchUtils::matchSpecialChars("/home/user/你好.txt", "nh！"));
    EXPECT_FALSE(FileSearchUtils::matchSpecialChars("/home/user/testfile.txt", "tf-"));
    
    // Test: 搜索关键字有特殊字符，文件名有不同的特殊字符，不应该匹配
    EXPECT_FALSE(FileSearchUtils::matchSpecialChars("/home/user/你好？.txt", "nh!"));
    EXPECT_FALSE(FileSearchUtils::matchSpecialChars("/home/user/test_file.txt", "tf-"));
    
    // Test: 搜索关键字有多个特殊字符
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/你好！世界？.txt", "nhsj!?"));
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/你好！世界？.txt", "nhsj！？"));  // 全角匹配
    EXPECT_FALSE(FileSearchUtils::matchSpecialChars("/home/user/你好！.txt", "nh!?"));
    
    // Test: 混合全角半角符号
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/测试！test?.txt", "cs!t?"));  // 混合匹配
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/测试!test？.txt", "cs！t?"));  // 交叉匹配
    
    // Test: 数字不应被视为特殊字符（重要！修复审查建议）
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/测试(1).txt", "cs(1)"));  // 数字1不是特殊字符
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/测试（1）.txt", "cs(1)"));  // 全角括号+数字
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/file123.txt", "f123"));  // 纯数字
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/版本2.0.txt", "bb20"));  // 数字和点号
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/test-123.txt", "t-123"));  // 减号+数字
    EXPECT_TRUE(FileSearchUtils::matchSpecialChars("/home/user/v1.2.3.txt", "v123"));  // 版本号
    EXPECT_FALSE(FileSearchUtils::matchSpecialChars("/home/user/test123.txt", "t-123"));  // 文件名没有减号
}
