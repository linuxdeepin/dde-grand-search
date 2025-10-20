// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILENAMEQUERY_H
#define FILENAMEQUERY_H

#include "searcher/semantic/semantichelper.h"

#include <QObject>

namespace GrandSearch {

class FileResultsHandler;
class FileNameQueryPrivate;
class FileNameQuery : public QObject
{
    Q_OBJECT
    friend class FileNameQueryPrivate;
public:
    explicit FileNameQuery(QObject *parent = nullptr);
    ~FileNameQuery();
    static void run(void *ptr, PushItemCallBack callBack, void *pdata);
    void setEntity(const QList<SemanticEntity> &entity);
    void setFileHandler(FileResultsHandler *handler);
private:
    FileNameQueryPrivate *d;
};

}

#endif // FILENAMEQUERY_H
