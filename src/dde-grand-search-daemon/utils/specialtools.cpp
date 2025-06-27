// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "specialtools.h"

#include <QStringList>
#include <QMimeDatabase>
#include <QFileInfo>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;

class MimeDatabaseGlobal : public QMimeDatabase {};
Q_GLOBAL_STATIC(MimeDatabaseGlobal, mimeDatabaseGlobal)

static void wpsMimeSpecialist(const QFileInfo &file, QMimeType &type)
{
    //wps文件的特殊处理
    static QStringList officeSuffixList {
        "docx", "xlsx", "pptx", "doc", "ppt", "xls", "wps"
    };
    static QStringList wrongMimeTypeNames {
        "application/x-ole-storage", "application/zip"
    };

    if (officeSuffixList.contains(file.suffix()) && wrongMimeTypeNames.contains(type.name())) {
        qCDebug(logDaemon) << "Applying WPS MIME type specialist for file:" << file.fileName()
                           << "Original type:" << type.name();
        QList<QMimeType> results = mimeDatabaseGlobal->mimeTypesForFileName(file.fileName());
        if (!results.isEmpty()) {
            type = results.first();
            qCDebug(logDaemon) << "WPS MIME type corrected to:" << type.name();
        }
    }
}

bool SpecialTools::splitCommand(const QString &cmd, QString &program, QStringList &args)
{
    if (cmd.isEmpty()) {
        qCDebug(logDaemon) << "Command string is empty";
        return false;
    }

    QStringList cmds = cmd.split(" ");
    if (cmds.size() == 0) {
        qCWarning(logDaemon) << "Failed to split command - No parts found:" << cmd;
        return false;
    }

    program = cmds.first();
    if (program.isEmpty()) {
        qCWarning(logDaemon) << "Program name is empty after splitting command:" << cmd;
        return false;
    }

    for (int i = 1; i < cmds.size(); ++i) {
        const QString &arg = cmds.at(i);
        if (arg.isEmpty())
            continue;
        args << arg;
    }

    qCDebug(logDaemon) << "Command split successfully - Program:" << program
                       << "Arguments count:" << args.size();
    return true;
}

QMimeType SpecialTools::getMimeType(const QFileInfo &file)
{
    if (file.isDir())
        return mimeDatabaseGlobal->mimeTypeForName(QLatin1String("inode/directory"));

    QMimeType result = mimeDatabaseGlobal->mimeTypeForFile(file, QMimeDatabase::MatchDefault);

    //wps特殊处理
    wpsMimeSpecialist(file, result);

    return result;
}

QString SpecialTools::getJsonString(QJsonObject *json, const QString &key)
{
    QString ret;
    if (!json || key.isEmpty()) {
        qCDebug(logDaemon) << "Invalid parameters for JSON string extraction - JSON:" << (json != nullptr)
                           << "Key:" << key;
        return ret;
    }

    if (json->contains(key)) {
        auto value = json->value(key);
        if (value.isString()) {
            ret = value.toString();
        } else {
            qCDebug(logDaemon) << "JSON value is not a string for key:" << key;
        }
    } else {
        qCDebug(logDaemon) << "JSON key not found:" << key;
    }

    return ret;
}

QJsonArray SpecialTools::getJsonArray(QJsonObject *json, const QString &key)
{
    QJsonArray ret;

    if (!json || key.isEmpty()) {
        qCDebug(logDaemon) << "Invalid parameters for JSON array extraction - JSON:" << (json != nullptr)
                           << "Key:" << key;
        return ret;
    }

    if (json->contains(key)) {
        auto value = json->value(key);
        if (value.isArray()) {
            ret = value.toArray();
        } else {
            qCDebug(logDaemon) << "JSON value is not an array for key:" << key;
        }
    } else {
        qCDebug(logDaemon) << "JSON key not found:" << key;
    }

    return ret;
}

bool SpecialTools::isHiddenFile(const QString &fileName, QHash<QString, QSet<QString>> &filters, const QString &pathPrefix)
{
    if (!fileName.startsWith(pathPrefix) || fileName == pathPrefix) {
        qCDebug(logDaemon) << "File not under path prefix - File:" << fileName
                           << "Prefix:" << pathPrefix;
        return false;
    }

    QFileInfo fileInfo(fileName);
    const auto &fileParentPath = fileInfo.absolutePath();
    const auto &hiddenFileConfig = fileParentPath + "/.hidden";

    // 判断.hidden文件是否存在，不存在说明该路径下没有隐藏文件
    if (!QFile::exists(hiddenFileConfig))
        return isHiddenFile(fileParentPath, filters, pathPrefix);

    if (filters[fileParentPath].isEmpty()) {
        QFile file(hiddenFileConfig);
        if (!file.open(QFile::ReadOnly)) {
            qCWarning(logDaemon) << "Failed to open .hidden file:" << hiddenFileConfig;
            return false;
        }

        // 判断.hidden文件中的内容是否为空，空则表示该路径下没有隐藏文件
        if (file.isReadable() && file.size() > 0) {
            QByteArray data = file.readAll();
            file.close();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            const auto &hiddenFiles = QSet<QString>::fromList(QString(data).split('\n', QString::SkipEmptyParts));
#else
            const QList<QString> splitResult = QString(data).split('\n', Qt::SkipEmptyParts);
            const auto &hiddenFiles = QSet<QString>(splitResult.begin(), splitResult.end());
#endif
            filters[fileParentPath] = hiddenFiles;
            qCDebug(logDaemon) << "Loaded hidden files from" << hiddenFileConfig
                               << "- Count:" << hiddenFiles.size();
        } else {
            qCDebug(logDaemon) << "Empty .hidden file:" << hiddenFileConfig;
            return isHiddenFile(fileParentPath, filters, pathPrefix);
        }
    }

    return filters[fileParentPath].contains(fileInfo.fileName())
            ? true
            : isHiddenFile(fileParentPath, filters, pathPrefix);
}
