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
#ifndef SWITCHWIDGET_H
#define SWITCHWIDGET_H

#include <DSwitchButton>

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

namespace GrandSearch {

class SwitchWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SwitchWidget(const QString &title, QWidget *parent = nullptr);
    explicit SwitchWidget(QWidget *parent = nullptr, QWidget *leftWidget = nullptr);

    void setChecked(const bool checked = true);
    bool checked() const;

    void setTitle(const QString &title);
    QString title() const;

    void setIcon(const QIcon &icon, const QSize &size);

    void setEnableBackground(const bool enable = true);
    bool enableBackground() const;

protected:
    void paintEvent(QPaintEvent *event) override;


signals:
    void checkedChanged(const bool checked) const;

private:
    bool m_hasBack = false;

    QHBoxLayout *m_mainLayout = nullptr;
    QLabel *m_iconLabel = nullptr;
    QWidget *m_leftWidget = nullptr;
    Dtk::Widget::DSwitchButton *m_switchBtn = nullptr;
};

}

#endif // SWITCHWIDGET_H
