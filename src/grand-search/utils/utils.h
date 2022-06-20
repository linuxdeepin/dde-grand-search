/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     houchengqiu<houchengqiu@uniontech.com>
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

#ifndef SORTMATCHITEM_H
#define SORTMATCHITEM_H

#include "global/matcheditem.h"
#include "global/builtinsearch.h"

#include <QStringList>

class Utils
{
public:
    // 排序算法 用于搜索结果排序 规则为：中文 > 英文 > 其他语言 > 标点符号
    static bool sort(GrandSearch::MatchedItems &list, Qt::SortOrder order = Qt::AscendingOrder);

    static bool compareByString(QString &str1, QString &str2, Qt::SortOrder order = Qt::AscendingOrder);
    static bool startWithSymbol(const QString &text);
    static bool startWithHanzi(const QString &text);
    static bool startWithLatin(const QString &text);
    static bool startWithOtherLang(const QString &text);
    static bool startWidthNum(const QString &text);

    // 通过权重进行排序
    static bool sortByWeight(GrandSearch::MatchedItemMap &map, Qt::SortOrder order = Qt::DescendingOrder);
    static bool compareByWeight(GrandSearch::MatchedItem &node1, GrandSearch::MatchedItem &node2, Qt::SortOrder order = Qt::DescendingOrder);

    // 更新结果项的权重
    static void updateItemsWeight(GrandSearch::MatchedItemMap &map, const QString &content);

    // 设置权重函数
    static bool setWeightMethod(GrandSearch::MatchedItem &item);

    // 计算文件类的权重
    static int calcFileWeight(const QString &path, const QString &name, const QStringList &keys);
    static qint64 calcDateDiff(const QDateTime &date1, const QDateTime &date2);
    static int calcWeightByDateDiff(const qint64 &diff, const int &type);

    // 计算应用和设置的权重
    static double calcAppWeight(const GrandSearch::MatchedItem &item, const QStringList &keys);
    static double calcSettingWeight(const GrandSearch::MatchedItem &item, const QStringList &keys);

    // 组装最佳匹配类目
    static void packageBestMatch(GrandSearch::MatchedItemMap &map, int maxQuantity);

    // 计算点选事件的权重
    static double calcRecordWeight(const GrandSearch::MatchedItem &item);

    // 判断类目所属搜索项是否支持重排
    static bool isResetSearcher(QString searcher);

    // 获取搜索结果默认打开应用图标
    static QString appIconName(const GrandSearch::MatchedItem &item);
    static bool isShowAppIcon(const GrandSearch::MatchedItem &item);

    // 根据文件路径获取mimetype
    static QString getFileMimetype(const QString &filePath);
    // 通过Gio接口获取mimeType
    static QString getFileMimetypeByGio(const QString &path);
    // 通过Qt接口获取mimeType
    static QString getFileMimetypeByQt(const QString &path);
    // 根据mimeType获取默认打开应用路径
    static QString getDefaultAppDesktopFileByMimeType(const QString &mimeType);
    // 搜索结果是否来自内置搜索项
    static bool isResultFromBuiltSearch(const GrandSearch::MatchedItem &item);
    // 打开搜索结果项，如扩展搜索项/应用/文件/浏览器等
    static bool openMatchedItem(const GrandSearch::MatchedItem &item);
    // 按住Ctrl键时打开搜索项
    static bool openMatchedItemWithCtrl(const GrandSearch::MatchedItem &item);
    // 在文件管理器中显示
    static bool openInFileManager(const GrandSearch::MatchedItem &item);
    // 打开扩展搜索项
    static bool openExtendSearchMatchedItem(const GrandSearch::MatchedItem &item);
    // 打开文件
    static bool openFile(const GrandSearch::MatchedItem &item);
    // 启动应用，若filePaths不为空，则用该应用打开传入的文件, 否则，仅启动应用
    static bool launchApp(const QString& desktopFile, const QStringList &filePaths = {});
    // 使用Dbus启动应用
    static bool launchAppByDBus(const QString &desktopFile, const QStringList &filePaths = {});
    // 使用gio启动应用
    static bool launchAppByGio(const QString &desktopFile, const QStringList &filePaths = {});
    // 跳转到浏览器
    static bool openWithBrowser(const QString &words);
    // 列表左边图标
    static QIcon defaultIcon(const GrandSearch::MatchedItem &item);

    // 默认浏览器的desktop文件
    static QString defaultBrowser();
    /*!
     * \brief isLevelItem 判断item是否属于分层项，并输出具体的层级
     * \param item 待判断的搜索结果项
     * \param level 输出参数，item所属的层级
     * \return 项item是否属于分层项
     */
    static bool isLevelItem(const GrandSearch::MatchedItem &item, int &level);

    // 返回该组是否包分层显示项
    static bool isLevelGroup(const QString &searchGroupName);

    // 返回该组是否支持预览
    static bool canPreview(const QString &searchGroupName);

    // 根据当前主题返回合适的图标后缀（-light or -dark）
    static QString iconThemeSuffix();

    static bool isWayland();

private:
    static QMap<QString, QString> m_appIconNameMap;// 存放应用desktop文件对应的图标名称，用于搜索框应用图标刷新
    static QMimeDatabase m_mimeDb;
};

#endif // SORTMATCHITEM_H
