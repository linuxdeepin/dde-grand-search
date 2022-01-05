/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

extern "C" {
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <ctype.h>
}

#include "utils/utils.h"
#include "global/matcheditem.h"
#include "global/builtinsearch.h"
#include "contacts/interface/daemongrandsearchinterface.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <DArrowRectangle>
#include <DGuiApplicationHelper>
#include <DDesktopServices>

#include <QtTest>
#include <QColor>
#include <QProcess>
#include <QIcon>

using namespace testing;
using namespace GrandSearch;
DWIDGET_USE_NAMESPACE;
DGUI_USE_NAMESPACE;

TEST(UtilsTest, constructor)
{
    Utils *utils = new Utils;

    EXPECT_TRUE(utils);

    delete utils;
}

TEST(UtilsTest, sort)
{
    stub_ext::StubExt stu;

    bool ut_call_compareByString = false;

    stu.set_lamda(&Utils::compareByString, [&](){
        ut_call_compareByString = true;
        return ut_call_compareByString;
    });

    MatchedItem item;
    MatchedItems items{item, item};

    bool result = Utils::sort(items);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_compareByString);
}

TEST(UtilsTest, compareByString)
{
    QString str1("abcdABCD");
    QString str2("abcdefg");

    // 覆盖重复字符逻辑
    bool result = Utils::compareByString(str1, str2, Qt::AscendingOrder);
    EXPECT_TRUE(result);

    str1 = "abcd";
    str2 = "efgh";

    // 其他符号逻辑
    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&](const QString &text1) {

            if (text1 == str1)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_FALSE(result);
    }

    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&](const QString &text1) {

            if (text1 == str2)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_TRUE(result);
    }

    // 数字逻辑
    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWidthNum, [&](const QString &text1) {

            if (text1 == str1)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_FALSE(result);
    }

    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWidthNum, [&](const QString &text1) {

            if (text1 == str2)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_TRUE(result);
    }

    // 其他语言逻辑
    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWidthNum, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithOtherLang, [&](const QString &text1) {

            if (text1 == str1)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_FALSE(result);
    }

    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWidthNum, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithOtherLang, [&](const QString &text1) {

            if (text1 == str2)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_TRUE(result);
    }

    // 英文逻辑
    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWidthNum, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithOtherLang, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithLatin, [&](const QString &text1) {

            if (text1 == str1)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_FALSE(result);
    }

    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWidthNum, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithOtherLang, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithLatin, [&](const QString &text1) {

            if (text1 == str2)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_TRUE(result);
    }

    // 中文逻辑
    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWidthNum, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithOtherLang, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithLatin, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithHanzi, [&](const QString &text1) {

            if (text1 == str1)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_FALSE(result);
    }

    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWidthNum, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithOtherLang, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithLatin, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithHanzi, [&](const QString &text1) {

            if (text1 == str2)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_TRUE(result);
    }
}

TEST(UtilsTest, startWithSymbol)
{
    QString text;

    bool result = Utils::startWithSymbol(text);
    EXPECT_FALSE(result);

    text = "abcd";
    result = Utils::startWithSymbol(text);
    EXPECT_FALSE(result);

    text = "+-abcd";
    result = Utils::startWithSymbol(text);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, startWithHanzi)
{
    QString text;

    bool result = Utils::startWithHanzi(text);
    EXPECT_FALSE(result);

    text = "abcd";
    result = Utils::startWithHanzi(text);
    EXPECT_FALSE(result);

    text = "测试中文";
    result = Utils::startWithHanzi(text);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, startWithLatin)
{
    QString text;

    bool result = Utils::startWithLatin(text);
    EXPECT_FALSE(result);

    text = "测试";
    result = Utils::startWithLatin(text);
    EXPECT_FALSE(result);

    text = "abcd";
    result = Utils::startWithLatin(text);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, startWithOtherLang)
{
    QString text;

    bool result = Utils::startWithOtherLang(text);
    EXPECT_FALSE(result);

    text = "abcd";
    result = Utils::startWithOtherLang(text);
    EXPECT_FALSE(result);

    // test 白俄罗斯语
    text = "Праверка";
    result = Utils::startWithOtherLang(text);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, startWidthNum)
{
    QString text;

    bool result = Utils::startWidthNum(text);
    EXPECT_FALSE(result);

    text = "abcd";
    result = Utils::startWidthNum(text);
    EXPECT_FALSE(result);

    text = "1234";
    result = Utils::startWidthNum(text);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, appIconName)
{
    MatchedItem item;

    // 名称为空的逻辑
    QString name = Utils::appIconName(item);
    EXPECT_TRUE(name.isEmpty());

    // 设置item
    item.item = "testItem";

    // 不支持显示图标的逻辑
    bool ut_isShowAppIcon = false;

    stub_ext::StubExt stu;
    stu.set_lamda(&Utils::isShowAppIcon, [&]() {
        return ut_isShowAppIcon;
    });
    name = Utils::appIconName(item);
    EXPECT_TRUE(name.isEmpty());

    // 设置支持
    ut_isShowAppIcon = true;

    // 结果为应用
    item.searcher = GRANDSEARCH_CLASS_APP_DESKTOP;
    item.icon = "test.png";
    name = Utils::appIconName(item);
    EXPECT_EQ(name, item.icon);

    // 结果为WEB
    item.searcher = GRANDSEARCH_CLASS_WEB_STATICTEXT;
    QString defaultDesktopFile("/usr/share/applications/dde-control-center.desktop");
    stu.set_lamda(&Utils::defaultBrowser, [&](){
        return defaultDesktopFile;
    });
    name = Utils::appIconName(item);
    if (QFile::exists(defaultDesktopFile))
        EXPECT_FALSE(name.isEmpty());
    else
        EXPECT_TRUE(name.isEmpty());

    // 结果为控制中心
    item.searcher = GRANDSEARCH_CLASS_SETTING_CONTROLCENTER;

    name = Utils::appIconName(item);
    EXPECT_FALSE(name.isEmpty());

    // 结果为文件
    item.searcher = GRANDSEARCH_CLASS_FILE_DEEPIN;
    stu.set_lamda(&Utils::getFileMimetype, [&]() {
        return "inode/directory";
    });

    name = Utils::appIconName(item);
    EXPECT_FALSE(name.isEmpty());
}

TEST(UtilsTest, isShowAppIcon)
{
    MatchedItem item;
    item.searcher = GRANDSEARCH_CLASS_SETTING_CONTROLCENTER;

    bool result = Utils::isShowAppIcon(item);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, getFileMimetype)
{
    QString filePath("testFilePath");

    QString mimetype = Utils::getFileMimetype(filePath);

    EXPECT_EQ(mimetype, QString("text/plain"));
}

TEST(UtilsTest, getFileMimetypeByGio)
{
    QString path("testPath");

    QString mimetype = Utils::getFileMimetypeByGio(path);
    EXPECT_TRUE(mimetype.isEmpty());

    path = "/usr/bin/";
    mimetype = Utils::getFileMimetypeByGio(path);
    EXPECT_EQ(mimetype, QString("inode/directory"));
}

TEST(UtilsTest, getFileMimetypeByQt)
{
    // 后缀判断
    QString path("/usr/share/applications/dde-control-center.desktop");

    QString mimetype = Utils::getFileMimetypeByQt(path);
    EXPECT_FALSE(mimetype.isEmpty());

    // 后缀为空
    path = "/usr/bin/ls";
    mimetype = Utils::getFileMimetypeByQt(path);
    EXPECT_FALSE(mimetype.isEmpty());

    // 文件不存在
    path.clear();
    mimetype = Utils::getFileMimetypeByQt(path);
    EXPECT_TRUE(mimetype.isEmpty());
}

TEST(UtilsTest, getDefaultAppDesktopFileByMimeType)
{
    QString mimetype("test");

    QString defaultApp = Utils::getDefaultAppDesktopFileByMimeType(mimetype);
    EXPECT_TRUE(defaultApp.isEmpty());

    mimetype = "inode/directory";
    defaultApp = Utils::getDefaultAppDesktopFileByMimeType(mimetype);
    EXPECT_FALSE(defaultApp.isEmpty());
}

TEST(UtilsTest, isResultFromBuiltSearch)
{
    MatchedItem item;

    item.searcher = GRANDSEARCH_CLASS_FILE_DEEPIN;

    bool result = Utils::isResultFromBuiltSearch(item);
    EXPECT_TRUE(result);

    item.searcher = "test";
    result = Utils::isResultFromBuiltSearch(item);
    EXPECT_FALSE(result);
}

TEST(UtilsTest, openMatchedItem)
{
    stub_ext::StubExt stu;

    // 非内置搜索项
    bool ut_isResultFromBuiltSearch = false;
    stu.set_lamda(&Utils::isResultFromBuiltSearch, [&]() {
        return ut_isResultFromBuiltSearch;
    });

    bool ut_call_openExtendSearchMatchedItem = false;
    stu.set_lamda(&Utils::openExtendSearchMatchedItem, [&]() {
        ut_call_openExtendSearchMatchedItem = true;
        return ut_call_openExtendSearchMatchedItem;
    });

    MatchedItem item;
    bool result = Utils::openMatchedItem(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_openExtendSearchMatchedItem);

    // 内置搜索项
    ut_isResultFromBuiltSearch = true;
    ut_call_openExtendSearchMatchedItem = false;

    // item为空
    result = Utils::openMatchedItem(item);
    EXPECT_FALSE(result);
    EXPECT_FALSE(ut_call_openExtendSearchMatchedItem);

    // 设置item
    item.item = "testItem";

    // 应用
    item.searcher = GRANDSEARCH_CLASS_APP_DESKTOP;

    bool ut_call_launchApp = false;
    stu.set_lamda(&Utils::launchApp, [&](){
       ut_call_launchApp = true;
       return ut_call_launchApp;
    });

    result = Utils::openMatchedItem(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_launchApp);

    // 浏览器
    item.searcher = GRANDSEARCH_CLASS_WEB_STATICTEXT;

    bool ut_call_openWithBrowser = false;
    stu.set_lamda(&Utils::openWithBrowser, [&](){
       ut_call_openWithBrowser = true;
       return ut_call_openWithBrowser;
    });

    result = Utils::openMatchedItem(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_openWithBrowser);

    // 文件
    item.searcher = GRANDSEARCH_CLASS_FILE_DEEPIN;

    bool ut_call_openFile = false;
    stu.set_lamda(&Utils::openFile, [&](){
        ut_call_openFile = true;
        return ut_call_openFile;
    });

    result = Utils::openMatchedItem(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_openFile);
}

TEST(UtilsTest, openMatchedItemWithCtrl)
{
    stub_ext::StubExt stu;
    bool ut_call_openMatchedItem = false;
    stu.set_lamda(&Utils::openMatchedItem, [&](){
        ut_call_openMatchedItem = true;
        return ut_call_openMatchedItem;
    });

    MatchedItem item;
    bool result = Utils::openMatchedItemWithCtrl(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_openMatchedItem);


    bool ut_call_openInFileManager = false;
    stu.set_lamda(&Utils::openInFileManager, [&](){
        ut_call_openInFileManager = true;
        return ut_call_openInFileManager;
    });
    stu.set_lamda(&QFileInfo::isDir, [](){
        return false;
    });

    item.searcher = GRANDSEARCH_CLASS_FILE_DEEPIN;
    result = Utils::openMatchedItemWithCtrl(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_openInFileManager);
}

TEST(UtilsTest, openInFileManager)
{
    stub_ext::StubExt stu;
    bool ut_call_showFileItem = false;
    bool (*showFileItem_addr)(QString, const QString &) = &DDesktopServices::showFileItem;
    stu.set_lamda(showFileItem_addr, [&](){
        ut_call_showFileItem = true;
        return ut_call_showFileItem;
    });

    MatchedItem item;
    bool result = Utils::openInFileManager(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_showFileItem);
}

TEST(UtilsTest, openExtendSearchMatchedItem)
{
    stub_ext::StubExt stu;

    bool ut_OpenWithPlugin = false;
    stu.set_lamda(&DaemonGrandSearchInterface::OpenWithPlugin, [&]() {
        ut_OpenWithPlugin = true;
        return QDBusPendingReply<bool>();
    });

    MatchedItem item;

    bool result = Utils::openExtendSearchMatchedItem(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_OpenWithPlugin);
}

TEST(UtilsTest, openFile)
{
    MatchedItem item;

    // item为空
    bool result =Utils::openFile(item);
    EXPECT_FALSE(result);

    // 设置item
    item.item = "/home/";

    stub_ext::StubExt stu;

    QString ut_getDefaultAppDesktopFileByMimeType;
    stu.set_lamda(&Utils::getDefaultAppDesktopFileByMimeType, [&]() {
        return ut_getDefaultAppDesktopFileByMimeType;
    });

    bool ut_startDetached = false;

    stu.set_lamda((bool(*)(const QString &, const QStringList &))ADDR(QProcess, startDetached), [&](){
       ut_startDetached = true;
       return ut_startDetached;
    });

    bool ut_launchApp = false;
    stu.set_lamda(&Utils::launchApp, [&](){
        ut_launchApp = true;
        return ut_launchApp;
    });

    result = Utils::openFile(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_startDetached);
    EXPECT_FALSE(ut_launchApp);

    ut_getDefaultAppDesktopFileByMimeType = "inode/directory";
    ut_startDetached = false;
    ut_launchApp = false;

    result = Utils::openFile(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_launchApp);
    EXPECT_FALSE(ut_startDetached);
}

TEST(UtilsTest, launchApp)
{
    stub_ext::StubExt stu;

    bool ut_call_launchAppByDBus = false;
    stu.set_lamda(&Utils::launchAppByDBus, [&]() {
        return ut_call_launchAppByDBus;
    });

    bool ut_call_launchAppByGio = false;
    stu.set_lamda(&Utils::launchAppByGio, [&]() {
        return ut_call_launchAppByGio;
    });

    QString desktopFile;
    QStringList filePaths;

    bool result = Utils::launchApp(desktopFile, filePaths);
    EXPECT_FALSE(result);

    ut_call_launchAppByGio = true;
    result = Utils::launchApp(desktopFile, filePaths);
    EXPECT_TRUE(result);

    ut_call_launchAppByDBus = true;
    result = Utils::launchApp(desktopFile, filePaths);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, launchAppByDBus)
{
    stub_ext::StubExt stu;

    bool ut_call_asyncCallWithArgumentList = false;
    stu.set_lamda(&QDBusAbstractInterface::asyncCallWithArgumentList, [&]() {
        ut_call_asyncCallWithArgumentList = true;
        return QDBusPendingCall::fromError(QDBusError());
    });

    bool ut_call_reply = false;
    stu.set_lamda(&QDBusPendingCall::reply, [&]() {
       ut_call_reply = true;
       return QDBusMessage();
    });

    QString desktopFile;
    QStringList filePaths;

    bool result = Utils::launchAppByDBus(desktopFile, filePaths);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_asyncCallWithArgumentList);
    EXPECT_TRUE(ut_call_reply);
}

TEST(UtilsTest, launchAppByGio)
{
    QString desktopFile;
    QStringList filePaths;

    bool result = Utils::launchAppByGio(desktopFile, filePaths);
    EXPECT_FALSE(result);

    desktopFile = "/usr/share/applications/dde-file-manager.desktop";
    filePaths << "/home/" << "/usr/bin/";

    bool ut_gio_launch = false;

    stub_ext::StubExt stu;

    stu.set_lamda(g_app_info_launch, [&]() {
        ut_gio_launch = true;
        return 0;
    });

    bool ut_startDetached = false;

    stu.set_lamda((bool(*)(const QString &, const QStringList &))ADDR(QProcess, startDetached), [&](){
       ut_startDetached = true;
       return ut_startDetached;
    });

    result = Utils::launchAppByGio(desktopFile, filePaths);
    EXPECT_FALSE(result);
    EXPECT_TRUE(ut_gio_launch);
    EXPECT_TRUE(ut_startDetached);
}

TEST(UtilsTest, openWithBrowser)
{
    QString words;

    bool result = Utils::openWithBrowser(words);
    EXPECT_FALSE(result);

    words = "testWord";
    stub_ext::StubExt stu;

    QString defaultBorwser;
    stu.set_lamda(&Utils::defaultBrowser, [&]() {
        return defaultBorwser;
    });

    result = Utils::openWithBrowser(words);
    EXPECT_FALSE(result);

    defaultBorwser = "testDefaultBorwser";

    bool ut_call_launchApp = false;
    stu.set_lamda(&Utils::launchApp, [&](){
       ut_call_launchApp = true;
       return ut_call_launchApp;
    });

    result = Utils::openWithBrowser(words);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, defaultIcon)
{
    MatchedItem item;
    item.searcher = GRANDSEARCH_CLASS_APP_DESKTOP;

    QIcon result = Utils::defaultIcon(item);
    EXPECT_EQ(result.name(), QIcon::fromTheme("application-x-desktop").name());

    item.searcher = GRANDSEARCH_CLASS_FILE_DEEPIN;
    result = Utils::defaultIcon(item);
    EXPECT_TRUE(result.isNull());

    item.searcher = GRANDSEARCH_CLASS_WEB_STATICTEXT;

    stub_ext::StubExt stu;

    QString ut_appIconName;
    stu.set_lamda(&Utils::appIconName, [&]() {
        return ut_appIconName;
    });

    result = Utils::defaultIcon(item);
    EXPECT_EQ(result.name(), QIcon::fromTheme("application-x-desktop").name());

    item.searcher = "testSearcher";
    result = Utils::defaultIcon(item);
    EXPECT_TRUE(result.isNull());
}

TEST(UtilsTest, defaultBrowser)
{
    stub_ext::StubExt stu;

    QString defaultBorwser("tsetBorwser");
    stu.set_lamda(&Utils::getDefaultAppDesktopFileByMimeType, [&]() {
        return defaultBorwser;
    });

    QString result = Utils::defaultBrowser();
    EXPECT_EQ(result, defaultBorwser);
}

TEST(UtilsTest, isLevelItem)
{
    MatchedItem item;
    int level = -1;

    bool result = Utils::isLevelItem(item, level);
    EXPECT_FALSE(result);

    QVariantHash showLevelHash({{GRANDSEARCH_PROPERTY_ITEM_LEVEL, 1}});
    item.extra = QVariant::fromValue(showLevelHash);

    result = Utils::isLevelItem(item, level);
    EXPECT_TRUE(result);
    EXPECT_EQ(level, 1);
}

TEST(UtilsTest, isLevelGroup)
{
    QString searchGroupName(GRANDSEARCH_GROUP_FILE);

    bool result = Utils::isLevelGroup(searchGroupName);
    EXPECT_TRUE(result);

    searchGroupName = "testOther";
    result = Utils::isLevelGroup(searchGroupName);
    EXPECT_FALSE(result);
}

TEST(UtilsTest, canPreview)
{
    QString searchGroupName(GRANDSEARCH_GROUP_FILE);

    bool result = Utils::canPreview(searchGroupName);
    EXPECT_TRUE(result);

    searchGroupName = "testOther";
    result = Utils::canPreview(searchGroupName);
    EXPECT_FALSE(result);
}

TEST(UtilsTest, isOnlyDisplayAll)
{
    QString searchGroupName(GRANDSEARCH_GROUP_SETTING);

    bool result = Utils::isOnlyDisplayAll(searchGroupName);
    EXPECT_TRUE(result);

    searchGroupName = "testOther";
    result = Utils::isOnlyDisplayAll(searchGroupName);
    EXPECT_FALSE(result);
}

TEST(UtilsTest, iconThemeSuffix)
{
    stub_ext::StubExt stu;

    DGuiApplicationHelper::ColorType ut_themeType = DGuiApplicationHelper::DarkType;
    stu.set_lamda(ADDR(DGuiApplicationHelper, themeType), [&](){
        return ut_themeType;
    });

    QString result = Utils::iconThemeSuffix();
    EXPECT_EQ(result, QString("-light"));

    ut_themeType = DGuiApplicationHelper::LightType;
    result = Utils::iconThemeSuffix();
    EXPECT_EQ(result, QString("-dark"));
}
