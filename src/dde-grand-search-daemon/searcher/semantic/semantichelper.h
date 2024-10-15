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
    QString duration; // 时长
    QString resolution; // 分辨率
    QString partPath;
    QString suffix;
    bool isTruePath = true; // Path包含与否
    bool isTrueType = true; // Type包含与否
    QString compType;
    qint64 fileSize;
    QList<QPair<qint64, qint64>> times;
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
