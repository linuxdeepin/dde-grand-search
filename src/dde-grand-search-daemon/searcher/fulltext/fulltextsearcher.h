// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FULLTEXTSEARCHER_H
#define FULLTEXTSEARCHER_H

#include "searcher/searcher.h"

namespace GrandSearch {

class FullTextSearcher : public Searcher
{
    Q_OBJECT
public:
    explicit FullTextSearcher(QObject *parent = nullptr);
    QString name() const Q_DECL_OVERRIDE;
    bool isActive() const Q_DECL_OVERRIDE;
    bool activate() Q_DECL_OVERRIDE;
    ProxyWorker *createWorker() const Q_DECL_OVERRIDE;
    bool action(const QString &action, const QString &item) Q_DECL_OVERRIDE;
};

}

#endif   // FULLTEXTSEARCHER_H
