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
#include "userpreference.h"

using namespace GrandSearch;

UserPreference::UserPreference(const QVariantHash &datas)
    : m_datas(datas)
{

}

UserPreferencePointer UserPreference::group(const QString &group) const
{
    QReadLocker lk(&m_rwLock);
    if (!m_datas.contains(group))
        return nullptr;

    auto var = m_datas.value(group);
    return var.value<UserPreferencePointer>();
}

void UserPreference::setValue(const QString &name, const QVariant &var)
{
    if (name.isEmpty())
        return;
    QWriteLocker lk(&m_rwLock);
    m_datas.insert(name, var);
}

bool UserPreference::innerValue(const QString &name, QVariant &var) const
{
    QReadLocker lk(&m_rwLock);
    if (m_datas.contains(name)) {
        var = m_datas.value(name);
        return true;
    }

    return false;
}
