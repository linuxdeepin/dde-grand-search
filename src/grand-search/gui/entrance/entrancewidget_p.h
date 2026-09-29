// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENTRANCEWIDGET_P_H
#define ENTRANCEWIDGET_P_H

#include "entrancewidget.h"

#include <DWidget>

#include "QObject"
#include <QPainter>

DWIDGET_BEGIN_NAMESPACE
class DSearchEdit;
class DLabel;
class DIconButton;
DWIDGET_END_NAMESPACE

class QHBoxLayout;
class QTimer;
class QLineEdit;
class QAction;
class QPushButton;

namespace GrandSearch {

// 自绘光标控件：覆盖在 QLineEdit 之上，按设计颜色绘制 1px 竖线
class CursorWidget : public QWidget
{
public:
    explicit CursorWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }

    void setColor(const QColor &color)
    {
        if (m_color == color)
            return;
        m_color = color;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.fillRect(rect(), m_color);
    }

private:
    QColor m_color;
};

class EntranceWidgetPrivate : public QObject
{
    Q_OBJECT
public:
    explicit EntranceWidgetPrivate(EntranceWidget *parent = nullptr);

    void delayChangeText();
    void notifyTextChanged();

    void showMenu(const QPoint& pos);

    EntranceWidget *q_p = nullptr;
    Dtk::Widget::DSearchEdit *m_searchEdit = nullptr;   // 搜索输入框控件
    QLineEdit *m_lineEdit = nullptr;                    // 输入控件
    Dtk::Widget::DLabel *m_appIconLabel = nullptr;      // 应用图标显示label
    QAction *m_appIconAction = nullptr;                 // 应用图标显示区域占位action
    QHBoxLayout *m_mainLayout = nullptr;

    QTimer *m_delayChangeTimer = nullptr;               // 延迟发出搜索文本改变

    CursorWidget *m_cursor = nullptr;                   // 自绘文本光标（Qt5 原生光标为背景反色，无法满足设计）
    QTimer *m_cursorBlinkTimer = nullptr;               // 光标闪烁定时器
    bool m_cursorOn = true;                             // 当前光标显隐状态
    Dtk::Widget::DIconButton *m_searchIconButton = nullptr;  // 缓存搜索图标按钮，避免递归 findChildren

    QString m_appIconName;                              // 当前搜索框显示的默认打开应用图标名称
};

}

#endif // ENTRANCEWIDGET_P_H
