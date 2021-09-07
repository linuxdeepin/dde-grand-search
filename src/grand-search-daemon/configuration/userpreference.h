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
#ifndef USERPREFERENCE_H
#define USERPREFERENCE_H

#include "preferenceitem.h"

#include <QVariantHash>
#include <QSharedPointer>
#include <QReadWriteLock>

typedef QSharedPointer<class UserPreference> UserPreferencePointer;

class UserPreference
{
public:
    explicit UserPreference(const QVariantHash &datas);
    virtual UserPreferencePointer group(const QString &group) const;

    template<typename T>
    T value(const QString &name, const T &def = T()) const
    {
        QVariant var;
        if (innerValue(name, var)) {
            return var.value<T>();
        }

        return def;
    }
    void setValue(const QString &name, const QVariant &var);
protected:
    virtual bool innerValue(const QString &name, QVariant &var) const;
private:
    mutable QReadWriteLock m_rwLock;
    QVariantHash m_datas;
};

Q_DECLARE_METATYPE(UserPreferencePointer)

#endif // USERPREFERENCE_H
