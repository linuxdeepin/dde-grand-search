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
#ifndef UTILTOOLS_H
#define UTILTOOLS_H

#include <QtCore>

namespace GrandSearch {

class UtilTools
{
public:
    static bool splitCommand(const QString &cmd, QString &program, QStringList &args);
    static QString getJsonString(QJsonObject *json, const QString &key);
    static QJsonArray getJsonArray(QJsonObject *json, const QString &key);
};

}
#endif // UTILTOOLS_H
