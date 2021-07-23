/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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

#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>

class MainControllerPrivate;
class MainController : public QObject
{
    Q_OBJECT
    friend class MainControllerPrivate;
public:
    explicit MainController(QObject *parent = nullptr);
    bool init();

    bool newSearch(const QString &key);
    void terminate();
    QByteArray getResults() const;
    QByteArray readBuffer() const;
    bool isEmptyBuffer() const;
    bool searcherAction(const QString &name, const QString &action, const QString &item);
signals:
    void matched();
    void searchCompleted();
public slots:
private:
    MainControllerPrivate *d = nullptr;
};

#endif // MAINCONTROLLER_H
