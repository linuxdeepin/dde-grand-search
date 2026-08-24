// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENTRANCEWIDGET_P_H
#define ENTRANCEWIDGET_P_H

#include "entrancewidget.h"

#include "QObject"

class QHBoxLayout;

namespace GrandSearch {

class SearchEdit;

class EntranceWidgetPrivate : public QObject
{
public:
    explicit EntranceWidgetPrivate(EntranceWidget *parent = nullptr);

    EntranceWidget *q_p = nullptr;
    SearchEdit *m_searchEdit = nullptr;
    QHBoxLayout *m_mainLayout = nullptr;

    QString m_appIconName;
    QString m_currentSearchText;
};

}

#endif // ENTRANCEWIDGET_P_H
