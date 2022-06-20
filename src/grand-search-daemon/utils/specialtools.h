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
#ifndef SPECIALTOOLS_H
#define SPECIALTOOLS_H

#include <QtCore>

namespace GrandSearch {

class SpecialTools
{
public:
    static bool splitCommand(const QString &cmd, QString &program, QStringList &args);
    static QMimeType getMimeType(const QFileInfo &file);
    static QString getJsonString(QJsonObject *json, const QString &key);
    static QJsonArray getJsonArray(QJsonObject *json, const QString &key);
    static bool isHiddenFile(const QString &fileName, QHash<QString, QSet<QString> > &filters, const QString &pathPrefix = "/");
};

}
#endif // SPECIALTOOLS_H
