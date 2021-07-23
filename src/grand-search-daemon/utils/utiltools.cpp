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
#include "utiltools.h"

#include <QStringList>

namespace GrandSearch {

bool UtilTools::splitCommand(const QString &cmd, QString &program, QStringList &args)
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

QString UtilTools::getJsonString(QJsonObject *json, const QString &key)
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

QJsonArray UtilTools::getJsonArray(QJsonObject *json, const QString &key)
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

}
