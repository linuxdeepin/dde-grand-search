// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANYTHINGQUERY_P_H
#define ANYTHINGQUERY_P_H

#include "anythingquery.h"
#include "searcher/semantic/fileresultshandler.h"
#include "interfaces/anything_interface.h"

namespace GrandSearch {
class AnythingQueryPrivate
{
public:
    explicit AnythingQueryPrivate(AnythingQuery *qq);
    ~AnythingQueryPrivate();
    void initAnything();
    bool searchUserPath(PushItemCallBack callBack, void *pdata);
    bool searchByAnything(PushItemCallBack callBack, void *pdata);
    QFileInfoList traverseDirAndFile(const QString &path);
    QString getRegExp() const;
    bool timeToPush() const;
public:
    ComDeepinAnythingInterface *m_anythingInterface = nullptr;
    SemanticEntity m_entity;
    QString m_searchPath;
    QStringList m_searchDirList;
    bool m_hasAddDataPrefix = false;
    FileResultsHandler *m_handler = nullptr;
    QTime m_time;
    int m_lastPush = 0;
private:
    AnythingQuery *q;
};
}
#endif // ANYTHINGQUERY_P_H
