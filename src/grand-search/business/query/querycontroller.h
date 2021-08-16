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
#ifndef QUERYCONTROLLER_H
#define QUERYCONTROLLER_H

#include <QObject>
#include <QScopedPointer>

class QueryControllerPrivate;
class QueryController : public QObject
{
    Q_OBJECT
public:
    explicit QueryController(QObject *parent = nullptr);
    ~QueryController();

    // 搜索文本发生变化，发起新的搜索请求
    void onSearchTextChanged(const QString &txt);

    // 前端主动终止搜索，调用接口通知后端
    void onTerminateSearch();

    // 返回当前任务ID，如果没有执行中的任务，则返回空
    QString getMissionID() const;
    bool isEmptySearchText() const;

signals:
    // 搜索文本改变后，发起新的搜索前，发出该任务ID改变信号
    void missionIdChanged(const QString &missionId);

    // 发出搜索文本为空信号
    void searchTextIsEmpty();

private:
    QScopedPointer<QueryControllerPrivate> d_p;
};

#endif // QUERYCONTROLLER_H
