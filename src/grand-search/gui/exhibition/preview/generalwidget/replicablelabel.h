/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef REPLICABLELABEL_H
#define REPLICABLELABEL_H

#include <QLineEdit>

class ReplicableLabel : public QLineEdit
{
    Q_OBJECT
public:
    explicit ReplicableLabel(QWidget *parent = nullptr);

    void setText(const QString &text);

    void setElideMode(Qt::TextElideMode elideMode);
    Qt::TextElideMode elideMode() const;

protected:
    bool event(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;


private:
    Qt::TextElideMode m_elideMode;      // 截断显示模式
    QString m_fullText;                 // 文本-用于复制
    QString m_elideText;                // 截断文本-用于显示
    bool m_pressed = false;             // 记录是否按照鼠标左键
};

#endif // REPLICABLELABEL_H
