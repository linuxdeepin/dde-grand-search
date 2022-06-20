/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "specialtools.h"

#include <QStringList>
#include <QMimeDatabase>
#include <QFileInfo>

class MimeDatabaseGlobal : public QMimeDatabase {};
Q_GLOBAL_STATIC(MimeDatabaseGlobal, mimeDatabaseGlobal)

namespace GrandSearch {

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
        QList<QMimeType> results = mimeDatabaseGlobal->mimeTypesForFileName(file.fileName());
        if (!results.isEmpty()) {
            type = results.first();
        }
    }
}

bool SpecialTools::splitCommand(const QString &cmd, QString &program, QStringList &args)
{
    if (cmd.isEmpty())
        return false;
    QStringList cmds = cmd.split(" ");
    if (cmds.size() == 0)
        return false;

    program = cmds.first();
    if (program.isEmpty())
        return false;

    for (int i = 1; i < cmds.size(); ++i) {
        const QString &arg = cmds.at(i);
        if (arg.isEmpty())
            continue;
        args << arg;
    }

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
    if (!json || key.isEmpty())
        return ret;

    if (json->contains(key)) {
        auto value = json->value(key);
        if (value.isString()) {
            ret = value.toString();
        }
    }

    return ret;
}

QJsonArray SpecialTools::getJsonArray(QJsonObject *json, const QString &key)
{
    QJsonArray ret;

    if (!json || key.isEmpty())
        return ret;

    if (json->contains(key)) {
        auto value = json->value(key);
        if (value.isArray()) {
            ret = value.toArray();
        }
    }

    return ret;
}

bool SpecialTools::isHiddenFile(const QString &fileName, QHash<QString, QSet<QString>> &filters, const QString &pathPrefix)
{
    if (!fileName.startsWith(pathPrefix) || fileName == pathPrefix)
        return false;

    QFileInfo fileInfo(fileName);
    const auto &fileParentPath = fileInfo.absolutePath();
    const auto &hiddenFileConfig = fileParentPath + "/.hidden";

    // 判断.hidden文件是否存在，不存在说明该路径下没有隐藏文件
    if (!QFile::exists(hiddenFileConfig))
        return isHiddenFile(fileParentPath, filters, pathPrefix);

    if (filters[fileParentPath].isEmpty()) {
        QFile file(hiddenFileConfig);
        if (!file.open(QFile::ReadOnly))
            return false;

        // 判断.hidden文件中的内容是否为空，空则表示该路径下没有隐藏文件
        if (file.isReadable() && file.size() > 0) {
            QByteArray data = file.readAll();
            file.close();

            const auto &hiddenFiles = QSet<QString>::fromList(QString(data).split('\n', QString::SkipEmptyParts));
            filters[fileParentPath] = hiddenFiles;
        } else {
            return isHiddenFile(fileParentPath, filters, pathPrefix);
        }
    }

    return filters[fileParentPath].contains(fileInfo.fileName())
            ? true
            : isHiddenFile(fileParentPath, filters, pathPrefix);
}

}
