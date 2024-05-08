// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FULLTEXTENGINE_P_H
#define FULLTEXTENGINE_P_H

#include "fulltextengine.h"

#include <lucene++/LuceneHeaders.h>

namespace GrandSearch {
class FullTextEnginePrivate
{
public:
    explicit FullTextEnginePrivate(FullTextEngine *qq);
    Lucene::IndexReaderPtr createReader(const QString &cache);
public:
    Lucene::IndexReaderPtr m_reader;
private:
   FullTextEngine *q;
};
}

#endif // FULLTEXTENGINE_P_H
