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
#ifndef ACCESSRECORD_H
#define ACCESSRECORD_H

#include "global/matcheditem.h"

namespace GrandSearch {

class AccessRecord : public QObject
{
    Q_OBJECT
public:
    static AccessRecord *instance();

    void startParseRecord();
    QHash<QString, QHash<QString, int>> getRecord();  //主线程获取数据  item:次数
    void updateRecord(const MatchedItem &matchedItem, qint64 time); // 主线程更新用户点击后的数据
public slots:
    void sync();   //主线程回写数据

signals:
    void sigParseFinished(const QJsonObject &recordObj, const QHash<QString, QHash<QString, int>> &recordHash, QPrivateSignal);
protected:
    explicit AccessRecord();
    ~AccessRecord();

private:
    static void parseRecord(QString recordPath);  // 子线程创建/解析文件
    static void parseJson(const QString &recordPath); //子线程解析文件数据并过滤数据

private:
    QHash<QString, QHash<QString, int>> m_recordHash; // 存放计算次数后的数据
    QJsonObject m_recordObj; // 存放文件中的数据
    QString m_recordPath; // 记录文件的路径
    bool m_finished = false;
    std::once_flag m_initFlag;
};

}

#endif // ACCESSRECORD_H
