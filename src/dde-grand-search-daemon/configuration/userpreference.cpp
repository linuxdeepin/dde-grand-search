// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "userpreference.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;

UserPreference::UserPreference(const QVariantHash &datas)
    : m_datas(datas)
{
    qCDebug(logDaemon) << "UserPreference constructor - Data items count:" << datas.size();
}

UserPreferencePointer UserPreference::group(const QString &group) const
{
    QReadLocker lk(&m_rwLock);
    if (!m_datas.contains(group)) {
        qCDebug(logDaemon) << "UserPreference group not found:" << group;
        return nullptr;
    }

    auto var = m_datas.value(group);
    return var.value<UserPreferencePointer>();
}

void UserPreference::setValue(const QString &name, const QVariant &var)
{
    if (name.isEmpty()) {
        qCWarning(logDaemon) << "UserPreference setValue failed - Empty name provided";
        return;
    }

    QWriteLocker lk(&m_rwLock);
    m_datas.insert(name, var);
}

bool UserPreference::innerValue(const QString &name, QVariant &var) const
{
    QReadLocker lk(&m_rwLock);
    if (m_datas.contains(name)) {
        var = m_datas.value(name);
        qCDebug(logDaemon) << "UserPreference value retrieved - Name:" << name << "Type:" << var.typeName();
        return true;
    }

    qCDebug(logDaemon) << "UserPreference value not found - Name:" << name;
    return false;
}
