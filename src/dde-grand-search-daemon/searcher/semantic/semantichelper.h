// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEMANTICHELPER_H
#define SEMANTICHELPER_H

#include "global/matcheditem.h"

#include <QStringList>

namespace GrandSearch {

struct SemanticEntity
{
    QStringList types;
    QStringList keys;
    QString author; // 歌手
    QString album; // 专辑名
    QString durationCompType;
    QString duration; // 时长
    QString resolution; // 分辨率
    QString partPath;
    QString suffix;
    bool isContainPath = true; // Path包含与否
    bool isContainType = true; // Type包含与否
    QString fileCompType;
    qint64 fileSize = 0;
    QList<QPair<qint64, qint64>> times;

    QString toString() {
        return QString("%1 | %2 | %3 | %4 | %5 | %6 | %7 | %8 | %9").arg(types.size()).arg(keys.size())
                .arg(author).arg(album).arg(duration).arg(resolution).arg(partPath).arg(suffix).arg(fileSize);
    }
};

typedef bool (*PushItemCallBack)(const MatchedItemMap &items, void *pdata);

class SemanticHelper
{
public:
    static bool entityFromJson(const QString &json, SemanticEntity &out);
    static QString vectorServiceName();
    static QString querylangServiceName();
    static bool isMatchTime(qint64 time, const QList<QPair<qint64, qint64>> &intervals);
    static QStringList typeTosuffix(const QStringList &type);
private:
    SemanticHelper();
};

}

#endif // SEMANTICHELPER_H
