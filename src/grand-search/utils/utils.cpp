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
#include <ctype.h>
}
#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>

#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

#include "utils.h"

#include "global/builtinsearch.h"
#include "global/searchhelper.h"
#include "contacts/interface/daemongrandsearchinterface.h"
#include "business/config/accessrecord/accessrecord.h"

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

static const int WeightDiffLimit = 21;

static const int CreateDateType = 1;
static const int ModifyDateType = 2;
static const int ReadDateType = 3;

static const double TimesWeight = 0.5;

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
    qStableSort(list.begin(), list.end(), [order](MatchedItem node1, MatchedItem node2) {
        return compareByString(node1.name, node2.name, order);
    });

    qDebug() << QString("sort matchItems done. cost [%1]ms").arg(time.elapsed());
    return true;
}

bool Utils::compareByString(QString &str1, QString &str2, Qt::SortOrder order)
{
    thread_local static DCollator sortCollator;

    // ?????????????????????????????????
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

    // ???????????????????????????
    if (startWithSymbol(str1)) {
        if (!startWithSymbol(str2)) {
            return order == Qt::DescendingOrder;
        }
    } else if (startWithSymbol(str2)) {
        return order != Qt::DescendingOrder;
    }

    // ?????????????????????
    if (startWidthNum(str1)) {
        if (!startWidthNum(str2)) {
            return order == Qt::DescendingOrder;
        }
    } else if (startWidthNum(str2)) {
        return order != Qt::DescendingOrder;
    }

    // ????????????????????????
    if (startWithOtherLang(str1)) {
        if (!startWithOtherLang(str2)) {
            return order == Qt::DescendingOrder;
        }
    } else if (startWithOtherLang(str2)) {
        return order != Qt::DescendingOrder;
    }

    // ??????????????????
    if (startWithLatin(str1)) {
        if (!startWithLatin(str2)) {
            return order == Qt::DescendingOrder;
        }
    } else if (startWithLatin(str2)) {
        return order != Qt::DescendingOrder;
    }

    // ??????????????????
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
    // ???????????????????????????
    QRegExp regExp("^[\u3002\uff1b\uff0c\uff1a\u201c\u201d\uff08\uff09\u3001\uff1f\u300a\u300b].*$");
    bRet = regExp.exactMatch(text.at(0));

    // ????????????????????????????????????????????????????????????
    if (!bRet)
        bRet = ispunct(text.at(0).toLatin1());

    return bRet;
}

bool Utils::startWithHanzi(const QString &text)
{
    if (text.isEmpty())
        return false;

    // ????????????
    QRegExp regExp("^[\u4e00-\u9fa5].*$");
    bool bRet = regExp.exactMatch(text.at(0));

    return bRet;
}

bool Utils::startWithLatin(const QString &text)
{
    if (text.isEmpty())
        return false;

    // ????????????
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

    // ????????????
    QRegExp regExp("^[0-9].*$");
    bool bRet = regExp.exactMatch(text.at(0));

    return bRet;
}

bool Utils::sortByWeight(MatchedItemMap &map, Qt::SortOrder order)
{
    QTime time;
    time.start();

    for (const QString &searchGroupName : map.keys()) {
        MatchedItems &list = map[searchGroupName];

        qStableSort(list.begin(), list.end(), [order](MatchedItem node1, MatchedItem node2){
            return compareByWeight(node1, node2, order);
        });
    }

    qDebug() << QString("sort matchItems by weight done.cost %1ms").arg(time.elapsed());
    return true;
}

/**
 * @brief Utils::compareByWeight ???????????????????????????
 * @param node1 ??????1
 * @param node2 ??????2
 * @param order ?????????????????????/??????
 * @return ??????1>??????2??????true???????????????false
 */
bool Utils::compareByWeight(MatchedItem &node1, MatchedItem &node2, Qt::SortOrder order)
{
    bool hasWeight1 = node1.extra.toHash().contains(GRANDSEARCH_PROPERTY_ITEM_WEIGHT);
    bool hasWeight2 = node2.extra.toHash().contains(GRANDSEARCH_PROPERTY_ITEM_WEIGHT);

    if (hasWeight1 && hasWeight2) {
        // ????????????????????????????????????
        double weight1 = node1.extra.toHash().value(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 0).toDouble();
        double weight2 = node2.extra.toHash().value(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 0).toDouble();

        return order == Qt::DescendingOrder ? weight1 > weight2 : weight1 < weight2;
    } else if (hasWeight1) {

        return order == Qt::DescendingOrder;
    } else if (hasWeight2) {

        return order != Qt::DescendingOrder;
    } else {
        // ???????????????????????????????????????
        return compareByString(node1.item, node2.item);
    }
}

/**
 * @brief Utils::updateItemsWeight ??????????????????
 * @param map ???????????????????????????
 * @param content ????????????????????????
 */
void Utils::updateItemsWeight(MatchedItemMap &map, const QString &content)
{
    // ??????????????????????????????????????????
    QStringList groupList, suffixList, keys;
    if (!searchHelper->parseKeyword(content, groupList, suffixList, keys))
        keys = QStringList() << content;

    for (const QString &searchGroupName : map.keys()) {
        MatchedItems &list = map[searchGroupName];
        for (MatchedItem &item : list) {
            if (!setWeightMethod(item))
                continue;

            // ????????????
            double weight = 0;
            {
                const QVariantHash &extData = item.extra.toHash();
                const QString &method = extData.value(GRANDSEARCH_PROPERTY_WEIGHT_METHOD).toString();

                if (method == GRANDSEARCH_PROPERTY_WEIGHT_METHOD_LOCALFILE) {
                    weight = calcFileWeight(item.item, item.name, keys);
                } else if (method == GRANDSEARCH_PROPERTY_WEIGHT_METHOD_APP) {
                    weight = calcAppWeight(item, keys);
                } else if (method == GRANDSEARCH_PROPERTY_WEIGHT_METHOD_SETTING) {
                    weight = calcSettingWeight(item, keys);
                } else {
                    continue;
                }
            }

            QVariant &extra = item.extra;
            QVariantHash originalValue = extra.toHash();
            originalValue.insert(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, weight);
            extra = QVariant::fromValue(originalValue);
        }
    }
}

/**
 * @brief Utils::setWeightMethod ???????????????????????????????????????
 * @param item ????????????
 * @return ???????????????????????????true
 */
bool Utils::setWeightMethod(GrandSearch::MatchedItem &item)
{
    QVariantHash ext = item.extra.toHash();

    // ????????????????????????
    if (ext.contains(GRANDSEARCH_PROPERTY_ITEM_WEIGHT))
        return false;

    // ???????????????????????????
    if (ext.contains(GRANDSEARCH_PROPERTY_WEIGHT_METHOD))
        return true;

    const QString &search = item.searcher;
    if (search == GRANDSEARCH_CLASS_FILE_DEEPIN ||
            search == GRANDSEARCH_CLASS_FILE_FSEARCH) {

        ext.insert(GRANDSEARCH_PROPERTY_WEIGHT_METHOD,
                   GRANDSEARCH_PROPERTY_WEIGHT_METHOD_LOCALFILE);
    } else if (search == GRANDSEARCH_CLASS_APP_DESKTOP) {
        ext.insert(GRANDSEARCH_PROPERTY_WEIGHT_METHOD,
                   GRANDSEARCH_PROPERTY_WEIGHT_METHOD_APP);
    } else if (search == GRANDSEARCH_CLASS_SETTING_CONTROLCENTER) {
        ext.insert(GRANDSEARCH_PROPERTY_WEIGHT_METHOD,
                   GRANDSEARCH_PROPERTY_WEIGHT_METHOD_SETTING);
    } else {
        // ?????????????????????
        return false;
    }

    item.extra = QVariant::fromValue(ext);

    return true;
}

/**
 * @brief Utils::calcFileWeight ??????????????????????????????
 * @param path ????????????
 * @param name ?????????
 * @param keys ?????????????????????
 * @return ????????????
 */
int Utils::calcFileWeight(const QString &path, const QString &name, const QStringList &keys)
{
    int weight = 0;
    for (const QString &key : keys) {
        if (name.contains(key)) {
            weight += 43;
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

    int createDayWeight = calcWeightByDateDiff(createDayDiff, CreateDateType);
    int lastModifyDayWeight = calcWeightByDateDiff(lastModifyDayDiff, ModifyDateType);
    int lastReadDayWeight = calcWeightByDateDiff(lastReadDayDiff, ReadDateType);

    weight += createDayWeight + lastModifyDayWeight + lastReadDayWeight;
    return weight;
}

/**
 * @brief Utils::calcDateDiff ????????????????????????
 * @param date1 ??????1
 * @param date2 ??????2
 * @return ??????????????????
 */
qint64 Utils::calcDateDiff(const QDateTime &date1, const QDateTime &date2)
{
    static const qint64 day = 24 * 60 * 60;
    return date1.secsTo(date2) / day;
}

/**
 * @brief Utils::calcWeightByDateDiff ????????????????????????
 * @param diff ????????????
 * @param type ?????????????????????/??????/??????
 * @return ???????????????
 */
int Utils::calcWeightByDateDiff(const qint64 &diff, const int &type)
{
    switch (type) {
    case CreateDateType :
    case ModifyDateType : {
        switch (diff) {
        case 0:
            return 24;
        case 1:
        case 2:
            return 16;
        case 3:
        case 4:
        case 5:
        case 6:
            return 8;
        default:
            return 0;
        }
    }
        break;
    case ReadDateType : {
        switch (diff) {
        case 0:
            return 9;
        case 1:
        case 2:
            return 6;
        case 3:
        case 4:
        case 5:
        case 6:
            return 3;
        default:
            return 0;
        }
    }
        break;
    default:
        return 0;
    }
}

/**
 * @brief Utils::calcAppWeight ??????????????????
 * @param item ??????
 * @param keys ?????????
 * @return ??????
 */
double Utils::calcAppWeight(const GrandSearch::MatchedItem &item, const QStringList &keys)
{
    double weight = 0;
    for (const QString &key : keys) {
        if (item.name.contains(key)) {
            weight += 20;
            break;
        }
    }
    weight += Utils::calcRecordWeight(item);

    weight += 60;
    return weight;
}

double Utils::calcSettingWeight(const GrandSearch::MatchedItem &item, const QStringList &keys)
{
    double weight = 0;
    for (const QString &key : keys) {
        if (item.name.contains(key)) {
            weight += 20;
            break;
        }
    }
    weight += Utils::calcRecordWeight(item);

    weight += 60;
    return weight;
}

/**
 * @brief Utils::packageBestMatch ?????????????????????????????????????????????????????????????????????????????????
 * @param map ???????????????????????????
 * @param maxQuantity ????????????????????????
 */
void Utils::packageBestMatch(MatchedItemMap &map, int maxQuantity)
{
    if (map.isEmpty() || maxQuantity <= 0)
        return;

    QTime time;
    time.start();

    static const QMap<QString, bool> supprotedSeracher = {
        {GRANDSEARCH_CLASS_FILE_DEEPIN, true},
        {GRANDSEARCH_CLASS_FILE_FSEARCH, true},
        {GRANDSEARCH_CLASS_APP_DESKTOP, true},
        {GRANDSEARCH_CLASS_SETTING_CONTROLCENTER, true},
    };

    // ????????????????????????????????????????????????
    static const QMap<QString, bool> blackGroup = {
        {GRANDSEARCH_GROUP_FILE_VIDEO, false},
        {GRANDSEARCH_GROUP_FILE_AUDIO, false},
        {GRANDSEARCH_GROUP_FILE_PICTURE, false},
        {GRANDSEARCH_GROUP_FILE_DOCUMNET, false},
        {GRANDSEARCH_GROUP_FOLDER, false}
    };

    MatchedItems bestList;
    QList<QPair<MatchedItem, QString>> tempBestList;

    for (auto group : map.keys()) {
        // ?????????????????????
        if (blackGroup.contains(group))
            continue;

        MatchedItems &list = map[group];

        for (auto item = list.begin(); item != list.end(); ++item) {
            // ????????????????????????????????????
            if (!supprotedSeracher.contains(item->searcher))
                continue;

            if (!tempBestList.isEmpty()) {
                double weight1 = tempBestList.first().first.extra.toHash().value(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 0).toDouble();
                double weight2 = item->extra.toHash().value(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 0).toDouble();
                // ??????????????????????????????????????????????????????21
                if (weight1 - weight2 >= WeightDiffLimit) {
                    break;
                } else if (weight2 - weight1 > WeightDiffLimit) {
                    tempBestList.clear();
                    tempBestList.append(qMakePair(*item, group));
                } else {
                    // ??????????????????????????????????????????21
                    auto it = tempBestList.begin();
                    for (; it != tempBestList.end(); ++it) {
                        // ??????????????????
                        double weightBest = it->first.extra.toHash().value(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 0).toDouble();
                        if (weight2 > weightBest) {
                            tempBestList.insert(it, qMakePair(*item, group));
                            break;
                        }
                    }

                    // ???????????????????????????????????????????????????????????????
                    if (it == tempBestList.end() && tempBestList.size() < maxQuantity)
                        tempBestList.append(qMakePair(*item, group));

                    // ?????????????????????????????????????????????
                    if (tempBestList.size() > maxQuantity)
                        tempBestList.removeLast();

                    // ?????????????????????last????????????????????????????????????
                    if (tempBestList.size() == maxQuantity && tempBestList.last().second == group)
                        break;
                }
            } else {
                tempBestList.append(qMakePair(*item, group));
            }
        }
    }

    for (auto list : tempBestList) {
        // ????????????????????????????????????
        map[list.second].removeOne(list.first);
        bestList.append(list.first);
    }

    qDebug() << "find best match count:" << bestList.count() << QString(".cost [%1]ms").arg(time.elapsed());
    if (!bestList.isEmpty()) {
        map.insert(GRANDSEARCH_GROUP_BEST, bestList);
    }
}

double Utils::calcRecordWeight(const GrandSearch::MatchedItem &item)
{
    auto recordHash = AccessRecord::instance()->getRecord();

    double weight = 0.0;
    if (recordHash.contains(item.searcher)) {
        auto itemHash = recordHash[item.searcher];
        if (itemHash.contains(item.item)) {
            weight += itemHash[item.item] * TimesWeight;
        }
    }

    return weight;
}

bool Utils::isResetSearcher(QString searcher)
{
    static const QStringList searchers = {
        GRANDSEARCH_CLASS_APP_DESKTOP,
        GRANDSEARCH_CLASS_SETTING_CONTROLCENTER
    };

    return searchers.contains(searcher);
}

QString Utils::appIconName(const MatchedItem &item)
{
    QString strAppIconName;
    if (item.item.isEmpty())
        return strAppIconName;

    // ??????????????????????????????app,?????????????????????,????????????????????????isShowAppIcon??????.
    if (!isShowAppIcon(item))
        return strAppIconName;

    // ????????????????????????????????????????????????
    if (item.searcher == GRANDSEARCH_CLASS_APP_DESKTOP) {
        return item.icon;
    } else if (item.searcher == GRANDSEARCH_CLASS_WEB_STATICTEXT) {
        //?????????????????????
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
    // ??????????????????????????????????????????????????????????????????????????????
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
    //???????????????
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

    // ??????gio??????qt??????????????????mimetype??????????????????????????????
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
    // ??????????????????????????????????????????mimetype
    if (!QFileInfo(path).suffix().isEmpty()) {
        mimetype = m_mimeDb.mimeTypeForFile(path, QMimeDatabase::MatchExtension).name();
    } else if (fi.size() != 0 && fi.exists()) {
        // ?????????????????????????????????????????????????????????????????????mimetype
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
    if (Utils::isResetSearcher(item.searcher)) {
        QDateTime time = QDateTime::currentDateTime();
        qint64 timeT = time.toSecsSinceEpoch();
        AccessRecord::instance()->updateRecord(item, timeT);
    }
    bool bFromBuiltSearch = isResultFromBuiltSearch(item);
    if (!bFromBuiltSearch) {
        return openExtendSearchMatchedItem(item);
    }

    if (item.item.isEmpty())
        return false;

    bool result = false;
    if (item.searcher == GRANDSEARCH_CLASS_APP_DESKTOP) {   // ????????????
        result = Utils::launchApp(item.item);
    } else if (item.searcher == GRANDSEARCH_CLASS_WEB_STATICTEXT) { // ???????????????
        result = Utils::openWithBrowser(item.item);
    } else {    // ????????????
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
    // ???????????????????????????????????????Dbus????????????????????????????????????????????????????????????
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

    // ??????mimetype
    QString mimetype = item.type;
    if (mimetype.isEmpty())
        mimetype = getFileMimetype(item.item);

    // fix bug98384 ???????????????????????????????????????????????????????????????????????????????????????
    QString fileUrlString = QUrl::fromLocalFile(filePath).toString();

    // ??????????????????????????????
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
    // ??????gio??????????????????
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

    // ?????????????????????
    QString defaultDesktopFile = defaultBrowser();
    if (defaultDesktopFile.isEmpty()) {
        qWarning() << "no browser to open url.";
        return false;
    }
    //???????????????
    qDebug() << "open with" << defaultDesktopFile;
    return launchApp(defaultDesktopFile, {words});
}

QIcon Utils::defaultIcon(const MatchedItem &item)
{
    if (item.searcher == GRANDSEARCH_CLASS_APP_DESKTOP)
        return QIcon::fromTheme("application-x-desktop");
    else if (item.searcher == GRANDSEARCH_CLASS_FILE_DEEPIN || item.searcher == GRANDSEARCH_CLASS_FILE_FSEARCH) {
        return QIcon::fromTheme(m_mimeDb.mimeTypeForFile(item.item).genericIconName());
    } else if (item.searcher == GRANDSEARCH_CLASS_WEB_STATICTEXT) {
        // ??????????????????????????????
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
    //! ??????????????????QApplication???????????????????????????????????????????????????platformName???????????????
    Q_ASSERT(qApp);

    static bool wayland = QApplication::platformName() == "wayland";

    return wayland;
}
