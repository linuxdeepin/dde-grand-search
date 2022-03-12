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
extern "C" {
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <ctype.h>
}

#include "utils.h"

#include "global/builtinsearch.h"
#include "global/searchhelper.h"
#include "contacts/interface/daemongrandsearchinterface.h"

#include <DArrowRectangle>
#include <DDesktopEntry>
#include <DGuiApplicationHelper>
#include <DDesktopServices>

#include <QCollator>
#include <QFileInfo>
#include <QDesktopServices>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusReply>
#include <QDateTime>
#include <QIcon>
#include <QApplication>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace GrandSearch;

static const QString SessionManagerService = "com.deepin.SessionManager";
static const QString StartManagerPath = "/com/deepin/StartManager";
static const QString StartManagerInterface = "com.deepin.StartManager";

static const int WeightDiffLimit = 18;

class DCollator : public QCollator
{
public:
    DCollator() : QCollator()
    {
        setCaseSensitivity(Qt::CaseInsensitive);
    }
};

QMap<QString, QString> Utils::m_appIconNameMap;
QMimeDatabase Utils::m_mimeDb;

bool Utils::sort(MatchedItems &list, Qt::SortOrder order/* = Qt::AscendingOrder*/)
{
    QTime time;
    time.start();
    qSort(list.begin(), list.end(), [order](MatchedItem node1, MatchedItem node2) {
        return compareByString(node1.name, node2.name, order);
    });

    qDebug() << QString("sort matchItems done. cost [%1]ms").arg(time.elapsed());
    return true;
}

bool Utils::compareByString(QString &str1, QString &str2, Qt::SortOrder order)
{
    thread_local static DCollator sortCollator;

    // 先去掉前边重复的字符串
    int nMinLength = qMin(str1.size(), str2.size());
    int nMidIndex = -1;
    for (int i = 0; i < nMinLength; i++) {
        if (str1[i] != str2[i]) {
            nMidIndex = i;
            break;
        }
    }
    if (nMidIndex > 0) {
        str1=str1.mid(nMidIndex);
        str2=str2.mid(nMidIndex);
    }

    // 其他符号要排在最后
    if (startWithSymbol(str1)) {
        if (!startWithSymbol(str2)) {
            return order == Qt::DescendingOrder;
        }
    } else if (startWithSymbol(str2)) {
        return order != Qt::DescendingOrder;
    }

    // 数字排在次次后
    if (startWidthNum(str1)) {
        if (!startWidthNum(str2)) {
            return order == Qt::DescendingOrder;
        }
    } else if (startWidthNum(str2)) {
        return order != Qt::DescendingOrder;
    }

    // 其他语言排在次后
    if (startWithOtherLang(str1)) {
        if (!startWithOtherLang(str2)) {
            return order == Qt::DescendingOrder;
        }
    } else if (startWithOtherLang(str2)) {
        return order != Qt::DescendingOrder;
    }

    // 英文排在第二
    if (startWithLatin(str1)) {
        if (!startWithLatin(str2)) {
            return order == Qt::DescendingOrder;
        }
    } else if (startWithLatin(str2)) {
        return order != Qt::DescendingOrder;
    }

    // 中文排在最前
    if (startWithHanzi(str1)) {
        if (!startWithHanzi(str2)) {
            return order == Qt::DescendingOrder;
        }
    } else if (startWithHanzi(str2)) {
        return order != Qt::DescendingOrder;
    }

    return ((order == Qt::DescendingOrder) ^ (sortCollator.compare(str1, str2) < 0)) == 0x01;
}

bool Utils::startWithSymbol(const QString &text)
{
    if (text.isEmpty())
        return false;

    bool bRet = false;
    // 先匹配中文标点符号
    QRegExp regExp("^[\u3002\uff1b\uff0c\uff1a\u201c\u201d\uff08\uff09\u3001\uff1f\u300a\u300b].*$");
    bRet = regExp.exactMatch(text.at(0));

    // 若不是中文标点符号，再判断是否为其他符号
    if (!bRet)
        bRet = ispunct(text.at(0).toLatin1());

    return bRet;
}

bool Utils::startWithHanzi(const QString &text)
{
    if (text.isEmpty())
        return false;

    // 匹配中文
    QRegExp regExp("^[\u4e00-\u9fa5].*$");
    bool bRet = regExp.exactMatch(text.at(0));

    return bRet;
}

bool Utils::startWithLatin(const QString &text)
{
    if (text.isEmpty())
        return false;

    // 匹配英文
    QRegExp regExp("^[a-zA-Z].*$");
    bool bRet = regExp.exactMatch(text.at(0));

    return bRet;
}

bool Utils::startWithOtherLang(const QString &text)
{
    if (text.isEmpty())
        return false;

    QChar::Script textScript = text.at(0).script();

    bool bRet = textScript >= QChar::Script_Inherited
            && textScript <= QChar::Script_ZanabazarSquare
            && textScript != QChar::Script_Latin
            && textScript != QChar::Script_Han;

    //qDebug() <<QString("%1 is %2 otherLang").arg(text).arg(bRet ? "" : "not");
    return bRet;
}

bool Utils::startWidthNum(const QString &text)
{
    if (text.isEmpty())
        return false;

    // 匹配数字
    QRegExp regExp("^[0-9].*$");
    bool bRet = regExp.exactMatch(text.at(0));

    return bRet;
}

bool Utils::sortByWeight(MatchedItemMap &map, Qt::SortOrder order)
{
    QTime time;
    time.start();

    for (const QString &searchGroupName : map.keys()) {
        if (!isSupportWeight(searchGroupName))
            continue;
        MatchedItems &list = map[searchGroupName];

        qSort(list.begin(), list.end(), [order](MatchedItem node1, MatchedItem node2){
            return compareByWeight(node1, node2, order);
        });
    }

    qDebug() << QString("sort matchItems by weight done.cost %1ms").arg(time.elapsed());
    return true;
}

bool Utils::compareByWeight(MatchedItem &node1, MatchedItem &node2, Qt::SortOrder order)
{
    bool hasWeight1 = node1.extra.toHash().contains(GRANDSEARCH_PROPERTY_ITEM_WEIGHT);
    bool hasWeight2 = node2.extra.toHash().contains(GRANDSEARCH_PROPERTY_ITEM_WEIGHT);

    if (hasWeight1 && hasWeight2) {
        // 两项均有权重，则对比权重
        int weight1 = node1.extra.toHash().value(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 0).toInt();
        int weight2 = node2.extra.toHash().value(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 0).toInt();

        return order == Qt::DescendingOrder ? weight1 > weight2 : weight1 < weight2;
    } else if (hasWeight1) {

        return order == Qt::DescendingOrder;
    } else if (hasWeight2) {

        return order != Qt::DescendingOrder;
    } else {
        // 两项均无权重，采用名称排序
        return compareByString(node1.item, node2.item);
    }
}

void Utils::updateItemsWeight(MatchedItemMap &map, const QString &content)
{
    // 类目、后缀搜索时的关键字解析
    QStringList groupList, suffixList, keys;
    if (!searchHelper->parseKeyword(content, groupList, suffixList, keys))
        keys = QStringList() << content;

    for (const QString &searchGroupName : map.keys()) {
        if (!isSupportWeight(searchGroupName))
            continue;
        MatchedItems &list = map[searchGroupName];
        for (MatchedItem &item : list) {
            int weight = calcFileWeight(item.item, item.name, keys);

            QVariant &extra = item.extra;
            if (extra.isNull()) {
                QVariantHash itemWeight({{GRANDSEARCH_PROPERTY_ITEM_WEIGHT, weight}});
                extra = QVariant::fromValue(itemWeight);
            } else if (extra.type() == QVariant::Hash) {
                QVariantHash originalValue = extra.toHash();
                originalValue.insert(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, weight);
                extra = QVariant::fromValue(originalValue);
            } else {
                qWarning() << "item extra error:" << item.name << item.item << item.extra;
            }
        }
    }
}

bool Utils::isSupportWeight(const QString &searchGroupName)
{
    static const QStringList supportWeightGroup{
        GRANDSEARCH_GROUP_FILE_VIDEO
      , GRANDSEARCH_GROUP_FILE_AUDIO
      , GRANDSEARCH_GROUP_FILE_PICTURE
      , GRANDSEARCH_GROUP_FILE_DOCUMNET
      , GRANDSEARCH_GROUP_FILE
      , GRANDSEARCH_GROUP_FOLDER
    };

    return supportWeightGroup.contains(searchGroupName);
}

int Utils::calcFileWeight(const QString &path, const QString &name, const QStringList &keys)
{
    int weight = 0;
    for (const QString &key : keys) {
        if (name.contains(key)) {
            weight += 46;
            break;
        }
    }

    QFileInfo fileInfo(path);

    const QDateTime &currentDateTime = QDateTime::currentDateTime();
    const QDateTime &createDateTime = fileInfo.created();
    const QDateTime &lastModifyTime = fileInfo.lastModified();
    const QDateTime &lastReadTime = fileInfo.lastRead();

    qint64 createDayDiff = calcDateDiff(createDateTime, currentDateTime);
    qint64 lastModifyDayDiff = calcDateDiff(lastModifyTime, currentDateTime);
    qint64 lastReadDayDiff = calcDateDiff(lastReadTime, currentDateTime);

    int createDayWeight = calcWeightByDateDiff(createDayDiff);
    int lastModifyDayWeight = calcWeightByDateDiff(lastModifyDayDiff);
    int lastReadDayWeight = calcWeightByDateDiff(lastReadDayDiff);

    weight += createDayWeight + lastModifyDayWeight + lastReadDayWeight;
    return weight;
}

qint64 Utils::calcDateDiff(const QDateTime &date1, const QDateTime &date2)
{
    static const qint64 day = 24*60*60;
    return date1.secsTo(date2) / day;
}

int Utils::calcWeightByDateDiff(const qint64 &diff)
{
    switch (diff) {
    case 0:
        return 18;
    case 1:
    case 2:
        return 12;
    case 3:
    case 4:
    case 5:
    case 6:
        return 6;
    default:
        return 0;
    }
}

void Utils::packageBestMatch(MatchedItemMap &map, int maxQuantity)
{
    // 从文件类目提取权重最高项，打包为最佳匹配项
    if (map.isEmpty() || !map.keys().contains(GRANDSEARCH_GROUP_FILE) || maxQuantity <= 0)
        return;

    QTime time;
    time.start();

    MatchedItems &list = map[GRANDSEARCH_GROUP_FILE];
    MatchedItems bestlist;
    while (maxQuantity-- && !list.isEmpty()) {
        MatchedItem item = list.takeFirst();
        if (!bestlist.isEmpty()) {
            // 计算当前项与权重最高项的偏差
            int weight1 = bestlist.first().extra.toHash().value(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 0).toInt();
            int weight2 = item.extra.toHash().value(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 0).toInt();
            if (weight1 - weight2 >= WeightDiffLimit) {
                qDebug() << QString("diff more than %1,give up.").arg(WeightDiffLimit) << weight1 << weight2 << bestlist.first().item << item.item;
                list.prepend(item);
                break;
            }
        }
        bestlist << item;
    }

    qDebug() << "find best match count:" << bestlist.count() << QString(".cost [%1]ms").arg(time.elapsed());
    if (!bestlist.isEmpty())
        map.insert(GRANDSEARCH_GROUP_BEST, bestlist);
}

QString Utils::appIconName(const MatchedItem &item)
{
    QString strAppIconName;
    if (item.item.isEmpty())
        return strAppIconName;

    // 搜索结果是否支持显示app,扩展插件不支持,如需特殊处理的放isShowAppIcon函数.
    if (!isShowAppIcon(item))
        return strAppIconName;

    // 搜索结果为应用，直接返回应用图标
    if (item.searcher == GRANDSEARCH_CLASS_APP_DESKTOP) {
        return item.icon;
    } else if (item.searcher == GRANDSEARCH_CLASS_WEB_STATICTEXT) {
        //默认浏览器图标
        auto defaultDesktopFile = defaultBrowser();
        if (!defaultDesktopFile.isEmpty()) {
            DDesktopEntry entry(defaultDesktopFile);
            strAppIconName = entry.stringValue("Icon");
        }
    } else if (item.searcher == GRANDSEARCH_CLASS_SETTING_CONTROLCENTER) {
        auto defaultDesktopFile = "/usr/share/applications/dde-control-center.desktop";
        if (QFile::exists(defaultDesktopFile)) {
            DDesktopEntry entry(defaultDesktopFile);
            strAppIconName = entry.stringValue("Icon");
        } else {
            qWarning() << defaultDesktopFile << "lost.";
        }

        if (strAppIconName.isEmpty())
            strAppIconName = "preferences-system";
    } else {
    // 搜索结果为文件，查询该文件对应默认打开应用图标的名称
        QString mimetype = item.type;
        if (mimetype.isEmpty())
            mimetype = getFileMimetype(item.item);

        QString defaultDesktopFile = getDefaultAppDesktopFileByMimeType(mimetype);
        if (m_appIconNameMap.find(defaultDesktopFile) == m_appIconNameMap.end()) {
            DDesktopEntry entry(defaultDesktopFile);
            strAppIconName = entry.stringValue("Icon");
            m_appIconNameMap[defaultDesktopFile] = strAppIconName;
        } else {
            strAppIconName = m_appIconNameMap[defaultDesktopFile];
        }
    }

    return strAppIconName;
}

bool Utils::isShowAppIcon(const MatchedItem &item)
{
    //特殊搜索项
    static QHash<QString, bool> extendSearcher = {
        {GRANDSEARCH_CLASS_SETTING_CONTROLCENTER, true}
    };
    return isResultFromBuiltSearch(item) || extendSearcher.value(item.searcher, false);
}

QString Utils::getFileMimetype(const QString &filePath)
{
    QString mimetype = getFileMimetypeByQt(filePath);
    if (mimetype.isEmpty())
        mimetype = getFileMimetypeByGio(filePath);

    // 通过gio库和qt库都获取不到mimetype，则默认返回文本类型
    if (mimetype.isEmpty())
        mimetype = "text/plain";

    return mimetype;
}

QString Utils::getFileMimetypeByGio(const QString &path)
{
    GFile *file;
    GFileInfo *info;
    QString result;

    file = g_file_new_for_path(path.toUtf8());
    info = g_file_query_info(file, "standard::content-type", G_FILE_QUERY_INFO_NONE, nullptr, nullptr);
    result = g_file_info_get_content_type(info);

    g_object_unref(file);

    return result;
}

QString Utils::getFileMimetypeByQt(const QString &path)
{
    QString mimetype;

    QFile fi(path);
    // 包含文件名后缀，通过后缀获取mimetype
    if (!QFileInfo(path).suffix().isEmpty()) {
        mimetype = m_mimeDb.mimeTypeForFile(path, QMimeDatabase::MatchExtension).name();
    } else if (fi.size() != 0 && fi.exists()) {
        // 文件名后缀为空，且内容不为空，根据文件内容获取mimetype
        mimetype = m_mimeDb.mimeTypeForFile(path, QMimeDatabase::MatchContent).name();
    } else {
        mimetype = "";
    }

    return mimetype;
}

QString Utils::getDefaultAppDesktopFileByMimeType(const QString &mimeType)
{
    GAppInfo* defaultApp = g_app_info_get_default_for_type(mimeType.toLocal8Bit().constData(), FALSE);
    if(!defaultApp)
        return "";

    const char* desktop_id = g_app_info_get_id(defaultApp);
    GDesktopAppInfo* desktopAppInfo = g_desktop_app_info_new(desktop_id);
    if(!desktopAppInfo) {
        g_object_unref(defaultApp);
        return "";
    }
    QString desktopFile = g_desktop_app_info_get_filename(desktopAppInfo);

    g_object_unref(defaultApp);
    g_object_unref(desktopAppInfo);

    return desktopFile;
}

bool Utils::isResultFromBuiltSearch(const MatchedItem &item)
{
    DEF_BUILTISEARCH_NAMES;
    if (predefBuiltinSearches.contains(item.searcher)) {
        return true;
    }

    return false;
}

bool Utils::openMatchedItem(const MatchedItem &item)
{
    bool bFromBuiltSearch = isResultFromBuiltSearch(item);
    if (!bFromBuiltSearch) {
        return openExtendSearchMatchedItem(item);
    }

    if (item.item.isEmpty())
        return false;

    bool result = false;
    if (item.searcher == GRANDSEARCH_CLASS_APP_DESKTOP) {   // 启动应用
        result = Utils::launchApp(item.item);
    } else if (item.searcher == GRANDSEARCH_CLASS_WEB_STATICTEXT) { // 跳转浏览器
        result = Utils::openWithBrowser(item.item);
    } else {    // 打开文件
        result = openFile(item);
    }

    return result;
}

bool Utils::openMatchedItemWithCtrl(const MatchedItem &item)
{
    if (GRANDSEARCH_CLASS_FILE_DEEPIN == item.searcher || GRANDSEARCH_CLASS_FILE_FSEARCH == item.searcher) {
        QFileInfo fileInfo(item.item);
        if (!fileInfo.isDir())
            return openInFileManager(item);
    }

    return openMatchedItem(item);
}

bool Utils::openInFileManager(const MatchedItem &item)
{
    return DDesktopServices::showFileItem(item.item);
}

bool Utils::openExtendSearchMatchedItem(const MatchedItem &item)
{
    // 搜索结果来自扩展插件，使用Dbus通知主控调用扩展插件打开接口打开搜索结果
    DaemonGrandSearchInterface daemonDbus;
    daemonDbus.OpenWithPlugin(item.searcher, item.item);

    return true;
}

bool Utils::openFile(const MatchedItem &item)
{
    QString filePath = item.item;
    if (filePath.isEmpty())
        return false;

    bool result = false;

    // 获取mimetype
    QString mimetype = item.type;
    if (mimetype.isEmpty())
        mimetype = getFileMimetype(item.item);

    // fix bug98384 社区版在打开特殊字符名称的文件夹时，丢失路径导致打开主目录
    QString fileUrlString = QUrl::fromLocalFile(filePath).toString();

    // 获取对应默认打开应用
    QString defaultDesktopFile = getDefaultAppDesktopFileByMimeType(mimetype);
    if (defaultDesktopFile.isEmpty()) {
        result = QProcess::startDetached(QString("dde-file-manager"), {QString("-o"), fileUrlString});
        qDebug() << "open file dialog" << result;
    } else {
        QStringList filePaths(fileUrlString);
        result = launchApp(defaultDesktopFile, filePaths);
    }

    return result;
}

bool Utils::launchApp(const QString& desktopFile, const QStringList &filePaths)
{
    bool ok = launchAppByDBus(desktopFile, filePaths);
    if (!ok) {
        ok = launchAppByGio(desktopFile, filePaths);
    }

    return ok;
}

bool Utils::launchAppByDBus(const QString &desktopFile, const QStringList &filePaths)
{
    QDBusInterface interface(SessionManagerService,
                             StartManagerPath,
                             StartManagerInterface,
                             QDBusConnection::sessionBus(),
                             qApp);

    QList<QVariant> args;
    args << desktopFile
         << QDateTime::currentDateTime().toTime_t()
         << filePaths;

    QDBusPendingCall call = interface.asyncCallWithArgumentList("LaunchApp", args);

    QDBusReply<QString> reply = call.reply();

    return true;
}

bool Utils::launchAppByGio(const QString &desktopFile, const QStringList &filePaths)
{
    // 使用gio接口启动应用
    std::string stdDesktopFilePath = desktopFile.toStdString();
    const char *cDesktopPath = stdDesktopFilePath.data();

    GDesktopAppInfo *appInfo = g_desktop_app_info_new_from_filename(cDesktopPath);
    if (!appInfo) {
        //qDebug() << "Failed to open desktop file with gio: g_desktop_app_info_new_from_filename returns NULL. Check PATH maybe?";
        return false;
    }

    GList *g_files = nullptr;
    foreach (const QString &filePath, filePaths) {
        std::string stdFilePath = filePath.toStdString();
        const char *cFilePath = stdFilePath.data();
        GFile *f = g_file_new_for_uri(cFilePath);
        g_files = g_list_append(g_files, f);
    }

    GError *gError = nullptr;
    gboolean ok = g_app_info_launch(reinterpret_cast<GAppInfo *>(appInfo), g_files, nullptr, &gError);

    if (gError) {
        qWarning() << "Error when trying to open desktop file with gio:" << gError->message;
        g_error_free(gError);
    }

    if (!ok) {
        qWarning() << "Failed to open desktop file with gio: g_app_info_launch returns false";
    }
    g_object_unref(appInfo);
    g_list_free(g_files);

    for (auto filePath : filePaths) {
        if (!filePath.isEmpty())
            QProcess::startDetached("gio", QStringList() << "open" << filePath);
    }

    return ok;
}

bool Utils::openWithBrowser(const QString &words)
{
    if (words.isEmpty())
        return false;

    static QString defaultUrl = "https://www.baidu.com/s?wd=%0&ie=UTF-8";

    QString encodeString(QUrl::toPercentEncoding(words));
    QString url = defaultUrl.arg(QString(encodeString));

    // 获取默认浏览器
    QString defaultDesktopFile = defaultBrowser();
    if (defaultDesktopFile.isEmpty()) {
        qWarning() << "no browser to open url.";
        return false;
    }
    //默认浏览器
    qDebug() << "open with" << defaultDesktopFile;
    return launchApp(defaultDesktopFile, {url});
}

QIcon Utils::defaultIcon(const MatchedItem &item)
{
    if (item.searcher == GRANDSEARCH_CLASS_APP_DESKTOP)
        return QIcon::fromTheme("application-x-desktop");
    else if (item.searcher == GRANDSEARCH_CLASS_FILE_DEEPIN || item.searcher == GRANDSEARCH_CLASS_FILE_FSEARCH) {
        return QIcon::fromTheme(m_mimeDb.mimeTypeForFile(item.item).genericIconName());
    } else if (item.searcher == GRANDSEARCH_CLASS_WEB_STATICTEXT) {
        // 使用默认浏览器的图标
        QString iconName = appIconName(item);
        if (QFile::exists(iconName))
            return QIcon(iconName);
        else
            return QIcon::fromTheme(iconName, QIcon::fromTheme("application-x-desktop"));
    }

    return QIcon();
}

QString Utils::defaultBrowser()
{
    static QString mimetype = "x-scheme-handler/http";
    QString defaultDesktopFile = getDefaultAppDesktopFileByMimeType(mimetype);
    return defaultDesktopFile;
}

bool Utils::isLevelItem(const MatchedItem &item, int &level)
{
    if (QVariant::Hash == item.extra.type() && item.extra.toHash().keys().contains(GRANDSEARCH_PROPERTY_ITEM_LEVEL)) {
        bool ok = false;
        level = item.extra.toHash().value(GRANDSEARCH_PROPERTY_ITEM_LEVEL).toInt(&ok);
        return ok;
    }
    return false;
}

bool Utils::isLevelGroup(const QString &searchGroupName)
{
    static const QStringList containLevelGroup{
        GRANDSEARCH_GROUP_FILE_VIDEO
      , GRANDSEARCH_GROUP_FILE_AUDIO
      , GRANDSEARCH_GROUP_FILE_PICTURE
      , GRANDSEARCH_GROUP_FILE_DOCUMNET
      , GRANDSEARCH_GROUP_FILE
    };

    return containLevelGroup.contains(searchGroupName);
}

bool Utils::canPreview(const QString &searchGroupName)
{
    static const QStringList containPreviewGroup{
        GRANDSEARCH_GROUP_FOLDER
      , GRANDSEARCH_GROUP_FILE
      , GRANDSEARCH_GROUP_FILE_VIDEO
      , GRANDSEARCH_GROUP_FILE_AUDIO
      , GRANDSEARCH_GROUP_FILE_PICTURE
      , GRANDSEARCH_GROUP_FILE_DOCUMNET
      , GRANDSEARCH_GROUP_BEST
    };

    return containPreviewGroup.contains(searchGroupName);
}

QString Utils::iconThemeSuffix()
{
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        return QString("-light");
    return QString("-dark");
}

bool Utils::isWayland()
{
    //! 该函数只能在QApplication之后调用才能返回有效的值，在此之前platformName会返回空值
    Q_ASSERT(qApp);

    static bool wayland = QApplication::platformName() == "wayland";

    return wayland;
}
