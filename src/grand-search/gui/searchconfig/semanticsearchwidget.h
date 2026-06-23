// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEMANTICSEARCHWIDGET_H
#define SEMANTICSEARCHWIDGET_H

#include <DWidget>
#include <DSwitchButton>

namespace GrandSearch {

class TipsLabel;

class SemanticSearchWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit SemanticSearchWidget(QWidget *parent = nullptr);
    ~SemanticSearchWidget() override;

private slots:
    void onSwitchToggled(bool checked);

private:
    void initUi();
    void initConnect();

private:
    DTK_WIDGET_NAMESPACE::DSwitchButton *m_switchBtn = nullptr;
};

}

#endif // SEMANTICSEARCHWIDGET_H
