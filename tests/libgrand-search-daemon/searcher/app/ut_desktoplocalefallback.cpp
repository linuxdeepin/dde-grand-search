// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searcher/app/desktopappsearcher_p.h"
#include <gtest/gtest.h>
#include <QTemporaryFile>

using namespace GrandSearch;
DCORE_USE_NAMESPACE

TEST(DesktopLocaleFallback, SearchFieldsSelectOneLocale)
{
    QTemporaryFile file;
    ASSERT_TRUE(file.open());
    file.write("[Desktop Entry]\nType=Application\nName=Fixture\nName[zh_CN]=LegacyChineseName\nExec=true\n"
               "GenericName=BaseGeneric\nGenericName[en_US]=ExactGeneric\nGenericName[en]=LanguageGeneric\n"
               "GenericName[zh_CN]=ChineseGeneric\nGenericName[de]=GermanGeneric\n"
               "Keywords=BaseKey;SecondBaseKey;\nKeywords[en_US]=ExactKey;SecondExactKey;\n"
               "Keywords[en]=LanguageKey;\nKeywords[zh_CN]=ChineseKey;\nKeywords[de]=GermanKey;\n");
    file.flush();
    DesktopEntryPointer entry(new DDesktopEntry(file.fileName()));
    const QSet<QString> allSearchFields = {"BaseGeneric", "BaseKey", "SecondBaseKey", "ExactGeneric", "LanguageGeneric", "ChineseGeneric", "GermanGeneric",
                                        "ExactKey", "SecondExactKey", "LanguageKey", "ChineseKey", "GermanKey"};
    const QList<QPair<QString, QSet<QString>>> cases = {
        {"en_US", {"ExactGeneric", "ExactKey", "SecondExactKey"}},
        {"en_GB", {"LanguageGeneric", "LanguageKey"}},
        {"en", {"LanguageGeneric", "LanguageKey"}},
        {"zh_CN", {"ChineseGeneric", "ChineseKey"}},
        {"de_DE", {"GermanGeneric", "GermanKey"}},
        {"fr_FR", {"BaseGeneric", "BaseKey", "SecondBaseKey"}},
        {"", {"BaseGeneric", "BaseKey", "SecondBaseKey"}},
        {"C", {"BaseGeneric", "BaseKey", "SecondBaseKey"}}
    };
    for (const auto &test : cases) {
        SCOPED_TRACE(test.first.toStdString());
        const auto index = DesktopAppSearcherPrivate::desktopIndex(entry, test.first);
        for (const auto &word : {"Fixture", "LegacyChineseName"})
            EXPECT_TRUE(index.contains(word)) << word;
        EXPECT_EQ(index & allSearchFields, test.second);
    }
}

TEST(DesktopLocaleFallback, PreserveNameFallbackAndListDecoding)
{
    QTemporaryFile file;
    ASSERT_TRUE(file.open());
    file.write("[Desktop Entry]\nName=DefaultName\nName[en]=LanguageName\n"
               "GenericName=DefaultGeneric\nGenericName[en_US]=ExactGeneric\n"
               "GenericName[en]=LanguageGeneric\nKeywords=one;escaped\\;semicolon;space\\sword;\n"
               "Keywords[en_US]=\nKeywords[en]=FallbackKey;\n");
    file.flush();
    DesktopEntryPointer entry(new DDesktopEntry(file.fileName()));
    EXPECT_EQ(DesktopAppSearcherPrivate::desktopName(entry, "", true), "DefaultGeneric");
    EXPECT_EQ(DesktopAppSearcherPrivate::desktopName(entry, "en_US", true), "ExactGeneric");
    EXPECT_EQ(DesktopAppSearcherPrivate::desktopName(entry, "en_US", false), "LanguageName");
    EXPECT_TRUE(DesktopAppSearcherPrivate::desktopName(entry, "fr_FR", false).isEmpty());
    EXPECT_TRUE(DesktopAppSearcherPrivate::desktopName(nullptr, "en_US", true).isEmpty());
    const auto index = DesktopAppSearcherPrivate::desktopIndex(entry, "en_US");
    EXPECT_TRUE(index.contains("FallbackKey"));
    for (const auto &word : {"one", "escaped;semicolon", "space word"})
        EXPECT_FALSE(index.contains(word)) << word;
    const auto defaultIndex = DesktopAppSearcherPrivate::desktopIndex(entry, "fr_FR");
    for (const auto &word : {"one", "escaped;semicolon", "space word"})
        EXPECT_TRUE(defaultIndex.contains(word)) << word;
    EXPECT_FALSE(index.contains("LanguageGeneric"));
}
