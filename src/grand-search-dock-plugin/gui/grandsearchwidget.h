// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRANDSEARCHWIDGET_H
#define GRANDSEARCHWIDGET_H

#include <QWidget>
#include <QIcon>
#include <QPixmap>

class GrandSearchInterface;

class GrandSearchWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GrandSearchWidget(QWidget *parent = nullptr);
    ~GrandSearchWidget() override;

    QString itemCommand(const QString &itemKey);

private slots:
    void grandSearchVisibleChanged(bool visible);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    const QPixmap loadSvg(const QString &fileName, const QSize &size) const;

private:
    bool m_hover = false;       // 鼠标是否悬浮
    bool m_pressed = false;     // 鼠标是否按下
    QIcon m_icon;
    QPixmap m_pixmap;
    bool m_grandSearchVisible = false;
    GrandSearchInterface *m_grandSearchInterface = nullptr;
};

#endif // GRANDSEARCHWIDGET_H
