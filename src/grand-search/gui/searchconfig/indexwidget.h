// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXWIDGET_H
#define INDEXWIDGET_H

#include <DWidget>

#include <QVBoxLayout>
#include <QLabel>

namespace GrandSearch {

class BlackListWidget;
class SmartSearchWidget;

class IndexWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit IndexWidget(QWidget *parent = nullptr);
    ~IndexWidget();

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QLabel *m_groupLabel = nullptr;
    SmartSearchWidget *m_smartSearchWidget = nullptr;
    BlackListWidget *m_blackListWidget = nullptr;
};

}
#endif // INDEXWIDGET_H
