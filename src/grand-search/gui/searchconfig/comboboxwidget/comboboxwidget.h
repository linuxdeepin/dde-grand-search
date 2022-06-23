/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhaohanxi<zhaohanxi@uniontech.com>
 *
 * Maintainer: zhaohanxi<zhaohanxi@uniontech.com>
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

#ifndef COMBOBOXWIDGET_H
#define COMBOBOXWIDGET_H

#include <QWidget>
#include <DComboBox>
#include <QLabel>
#include <QHBoxLayout>

namespace GrandSearch {

class ComboboxWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ComboboxWidget(const QString &title, QWidget *parent = nullptr);
    explicit ComboboxWidget(QWidget *parent = nullptr, QWidget *leftWidget = nullptr);

    void setChecked(const QString &text);
    QString checked() const;

    void setTitle(const QString &title);
    QString title() const;

    void setEnableBackground(const bool enable = true);
    bool enableBackground() const;
    Dtk::Widget::DComboBox *getComboBox();

protected:
    void paintEvent(QPaintEvent *event) override;

signals:
    void checkedChanged(int index) const;

private:
    bool m_hasBack = false;
    QHBoxLayout *m_mainLayout = nullptr;
    QWidget *m_leftWidget = nullptr;
    Dtk::Widget::DComboBox *m_comboBox = nullptr;
};

}

#endif // COMBOBOXWIDGET_H
