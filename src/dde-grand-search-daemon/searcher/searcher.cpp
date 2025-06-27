// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searcher.h"
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;

Searcher::Searcher(QObject *parent) : QObject(parent)
{
    qCDebug(logDaemon) << "Searcher base constructor - Created searcher";
}

bool Searcher::activate()
{
    qCDebug(logDaemon) << "Searcher base activate - No activation implementation";
    return false;
}
