// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHEDIT_P_H
#define SEARCHEDIT_P_H

#include "searchedit.h"

#include <DIconButton>

#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>
#include <QAction>

namespace GrandSearch {

class SearchEditPrivate
{
    SearchEditPrivate(SearchEdit *qq);

    void init();
    void toEditMode(bool focus);
    void showContextMenu(const QPoint &pos);
    void delayTextChanged();
    void notifyTextChanged();

    SearchEdit *q = nullptr;

    QLineEdit *m_lineEdit = nullptr;
    QWidget *m_iconWidget = nullptr;
    QLabel *m_placeholderLabel = nullptr;
    DTK_WIDGET_NAMESPACE::DIconButton *m_appIconLabel = nullptr;
    DTK_WIDGET_NAMESPACE::DIconButton *m_clearButton = nullptr;
    QAction *m_searchAction = nullptr;
    QAction *m_clearAction = nullptr;
    QAction *m_appIconAction = nullptr;

    QHBoxLayout *m_lineEditLayout = nullptr;

    QTimer *m_delayTimer = nullptr;

    QString m_placeHolder;
    QString m_placeholderText;
    QString m_appIconName;

    friend class SearchEdit;
};

}

#endif // SEARCHEDIT_P_H
