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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DBlurEffectWidget>

#include <QScopedPointer>

class MainWindowPrivate;
class MainWindow : public Dtk::Widget::DBlurEffectWidget
{
    Q_OBJECT
public:
    static MainWindow *instance();
    ~MainWindow() Q_DECL_OVERRIDE;
    void connectToController();

private:
    void initUI();
    void initConnect();

protected:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    virtual void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    virtual void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;
    virtual void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

signals:
    void visibleChanged(const bool visible);
    void searchTextChanged(const QString &txt);

private:
    QScopedPointer<MainWindowPrivate> d_p;
};

#endif // MAINWINDOW_H
