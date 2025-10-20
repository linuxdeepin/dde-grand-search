// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FULLTEXTENGINE_P_H
#define FULLTEXTENGINE_P_H

#include "fulltextengine.h"

#include <dfm-search/searchfactory.h>
#include <dfm-search/searchengine.h>
#include <dfm-search/contentsearchapi.h>

#include <QSet>
#include <QString>

namespace GrandSearch {

class FullTextEnginePrivate
{
public:
    struct ContentContext
    {
        QStringList keywords;
        QString highlightedContent;
        QSet<QString> matchedKeys;
    };

public:
    explicit FullTextEnginePrivate(FullTextEngine *qq);
    ~FullTextEnginePrivate();

    bool isContentIndexAvailable() const;
    QSet<QString> extractMatchedKeys(const QString &content, const QStringList &keywords) const;

public:
    DFMSEARCH::SearchEngine *m_engine = nullptr;
    QObject *m_engineHolder = nullptr;

private:
    FullTextEngine *q;
};
}

#endif   // FULLTEXTENGINE_P_H
