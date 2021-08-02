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
}

#include "utils.h"

#include "global/builtinsearch.h"
#include "contacts/interface/daemongrandsearchinterface.h"

#include <QCollator>
#include <QFileInfo>
#include <QDesktopServices>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusReply>
#include <QDateTime>

using namespace GrandSearch;

static const QString SessionManagerService = "com.deepin.SessionManager";
static const QString StartManagerPath = "/com/deepin/StartManager";
static const QString StartManagerInterface = "com.deepin.StartManager";

class DCollator : public QCollator
{
public:
    DCollator() : QCollator()
    {
        setNumericMode(true);
        setCaseSensitivity(Qt::CaseInsensitive);
    }
};

QMap<QString, QString> Utils::m_appIconNameMap;
QMimeDatabase Utils::m_mimeDb;
bool Utils::sort(MatchedItems &list, Qt::SortOrder order/* = Qt::AscendingOrder*/)
{
    qSort(list.begin(), list.end(), [order](const MatchedItem node1, const MatchedItem node2) {
        return compareByString(node1.name, node2.name, order);
    });

    qDebug() << QString("sort matchItems done.");
    return true;
}

bool Utils::compareByString(const QString &str1, const QString &str2, Qt::SortOrder order)
{
    thread_local static DCollator sortCollator;

    // 其他符号要排在最后
    if (startWithSymbol(str1)) {
        if (!startWithSymbol(str2))
            return order == Qt::DescendingOrder;
    } else if (startWithSymbol(str2))
        return order != Qt::DescendingOrder;

    // 其他语言显示排在次后
    if (startWithOtherLang(str1)) {
        if (!startWithOtherLang(str2)) {
            return order == Qt::DescendingOrder;
        }
    } else if (startWithOtherLang(str2)) {
        return order != Qt::DescendingOrder;
    }

    // 英文显示排在第二
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

    //匹配字母、数字和中文开头的字符串
    QRegExp regExp("^[a-zA-Z0-9\u4e00-\u9fa5].*$");
    return !regExp.exactMatch(text);
}

bool Utils::startWithHanzi(const QString &text)
{
    if (text.isEmpty())
        return false;

    return text.at(0).script() == QChar::Script_Han;
}

bool Utils::startWithLatin(const QString &text)
{
    if (text.isEmpty())
        return false;

    return text.at(0).script() == QChar::Script_Latin;
}

bool Utils::startWithOtherLang(const QString &text)
{
    if (text.isEmpty())
        return false;

    return text.at(0).script() != QChar::Script_Latin
            && text.at(0).script() != QChar::Script_Han;
}

QString Utils::appIconName(const MatchedItem &item)
{
    // 获取搜索结果对应的默认打开应用图标
    QString strAppIconName;
    if (item.item.isEmpty())
        return strAppIconName;

    QString defaultDesktopFile;
    if (QFileInfo(item.item).suffix() == "desktop") {
        defaultDesktopFile = item.item;
    } else {
        QString mimetype = getFileMimetype(item.item);
        defaultDesktopFile = getDefaultAppDesktopFileByMimeType(mimetype);
    }

    if (m_appIconNameMap.find(defaultDesktopFile) == m_appIconNameMap.end()) {
        QSettings settings(defaultDesktopFile, QSettings::IniFormat);
        settings.beginGroup("Desktop Entry");
        strAppIconName = settings.value("Icon").toString();
        m_appIconNameMap[defaultDesktopFile] = strAppIconName;
    } else {
        strAppIconName = m_appIconNameMap[defaultDesktopFile];
    }

    return strAppIconName;
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
    if(!desktopAppInfo)
        return "";
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
        // 搜索结果来自扩展插件，使用Dbus通知主控调用扩展插件打开接口打开搜索结果
        DaemonGrandSearchInterface daemonDbus;
        daemonDbus.OpenWithPlugin(item.searcher, item.item);

        return true;
    }

    return openFile(item.item);
}

bool Utils::openFile(const QString &filePath)
{
    if (filePath.isEmpty())
        return false;

    // 传入的文件为应用，直接启动应用
    bool result = false;
    if (QFileInfo(filePath).suffix() == "desktop") {
        result = Utils::launchApp(filePath);
        return result;
    }

    // 获取mimetype
    QString mimetype = getFileMimetype(filePath);

    // 获取对应默认打开应用
    QString defaultDesktopFile = getDefaultAppDesktopFileByMimeType(mimetype);
    if (defaultDesktopFile.isEmpty()) {
        qDebug() << "no default application for" << filePath;
    }

    qDebug() << QString("defaultDesktopFile:%1").arg(defaultDesktopFile);

    QStringList filePaths(filePath);
    result =launchApp(defaultDesktopFile, filePaths);
    if (result) {
        // 加入到最近打开  因缺少DESKTOP类 用于获取appName和appExc
        // 后续优化  TODO
        ;
    }

    if (!result) {
        QUrl url(filePath);
        if (url.isValid()) {
            qDebug() << "openUrl" << filePath;
            return QDesktopServices::openUrl(url);
        }
    }

    return result;
}

bool Utils::launchApp(const QString& desktopFile, const QStringList &filePaths)
{
    qDebug() << "launchApp " << desktopFile << filePaths;

    bool ok = launchAppByDBus(desktopFile, filePaths);
    if (!ok) {
        ok = launchAppByGio(desktopFile, filePaths);
    }

    return ok;
}

bool Utils::launchAppByDBus(const QString &desktopFile, const QStringList &filePaths)
{
    qDebug() << "launchApp by Dbus:" << desktopFile << filePaths;

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
    qDebug() << "launchApp by gio:" << desktopFile << filePaths;

    std::string stdDesktopFilePath = desktopFile.toStdString();
    const char *cDesktopPath = stdDesktopFilePath.data();

    GDesktopAppInfo *appInfo = g_desktop_app_info_new_from_filename(cDesktopPath);
    if (!appInfo) {
        qDebug() << "Failed to open desktop file with gio: g_desktop_app_info_new_from_filename returns NULL. Check PATH maybe?";
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
