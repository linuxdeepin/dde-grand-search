/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *             houchengqiu<houchengqiu@uniontech.com>
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
#ifndef EXHIBITIONWIDGET_H
#define EXHIBITIONWIDGET_H

#include <DWidget>

#include <QScopedPointer>

class ExhibitionWidgetPrivate;
class MatchWidget;
class QHBoxLayout;
class QVBoxLayout;
class QPushButton;
class ExhibitionWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit ExhibitionWidget(QWidget *parent = nullptr);
    ~ExhibitionWidget();
    void connectToController();

public slots:
    void onSelectNextItem();
    void onSelectPreviousItem();
    void onHandleItem();

protected:
    void initUi();
    void initConnect();

    void paintEvent(QPaintEvent *event) override;
private:
    QScopedPointer<ExhibitionWidgetPrivate> d_p;

    QHBoxLayout *m_hLayout = nullptr;

    QVBoxLayout *m_vLayout = nullptr;
    QPushButton *m_btnSearch = nullptr;
    QPushButton *m_btnClear = nullptr;
    MatchWidget *m_matchWidget = nullptr;
};

#endif // EXHIBITIONWIDGET_H
