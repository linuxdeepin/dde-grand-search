// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GENERALTOOLBAR_H
#define GENERALTOOLBAR_H

#include <DWidget>
#include <QToolButton>

DWIDGET_BEGIN_NAMESPACE
class DVerticalLine;
DWIDGET_END_NAMESPACE
class QHBoxLayout;

namespace GrandSearch {

class ToolButton: public QToolButton
{
    Q_OBJECT
public:
    explicit ToolButton(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void updatePalette();
};

class GeneralToolBar : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit GeneralToolBar(QWidget *parent = nullptr);

signals:
    void sigOpenClicked();
    void sigOpenPathClicked();
    void sigCopyPathClicked();

private:
    void initUi();
    void initConnect();

private:
    QHBoxLayout* m_hMainLayout = nullptr;

    ToolButton *m_openBtn = nullptr;
    ToolButton *m_openPathBtn = nullptr;
    ToolButton *m_copyPathBtn = nullptr;

    Dtk::Widget::DVerticalLine* m_vLine1 = nullptr;
    Dtk::Widget::DVerticalLine* m_vLine2 = nullptr;
};

}

#endif // GENERALTOOLBAR_H
