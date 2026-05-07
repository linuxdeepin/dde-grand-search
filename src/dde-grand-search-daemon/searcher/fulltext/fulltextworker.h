// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FULLTEXTWORKER_H
#define FULLTEXTWORKER_H

#include "searcher/proxyworker.h"

namespace GrandSearch {

class FullTextWorkerPrivate;
class FullTextWorker : public ProxyWorker
{
    Q_OBJECT
public:
    explicit FullTextWorker(const QString &name, QObject *parent = nullptr);
    ~FullTextWorker();
    void setContext(const QString &context) Q_DECL_OVERRIDE;
    bool isAsync() const Q_DECL_OVERRIDE;
    bool working(void *context) Q_DECL_OVERRIDE;
    void terminate() Q_DECL_OVERRIDE;
    Status status() Q_DECL_OVERRIDE;
    bool hasItem() const Q_DECL_OVERRIDE;
    MatchedItemMap takeAll() Q_DECL_OVERRIDE;

private:
    FullTextWorkerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(FullTextWorker)
};

}

#endif   // FULLTEXTWORKER_H
