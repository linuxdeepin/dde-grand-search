// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEMANTICSEARCHER_H
#define SEMANTICSEARCHER_H

#include "searcher/searcher.h"

namespace GrandSearch {

class SemanticSearcher : public Searcher
{
    Q_OBJECT
public:
    explicit SemanticSearcher(QObject *parent = nullptr);
    QString name() const Q_DECL_OVERRIDE;
    bool isActive() const Q_DECL_OVERRIDE;
    ProxyWorker *createWorker() const Q_DECL_OVERRIDE;
    bool action(const QString &action, const QString &item) Q_DECL_OVERRIDE;
};

}

#endif // SEMANTICSEARCHER_H
