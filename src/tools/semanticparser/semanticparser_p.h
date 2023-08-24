// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEMANTICPARSER_P_H
#define SEMANTICPARSER_P_H

#include "semanticparser.h"
#include "analyzeserver.h"

using namespace org::deepin::ai::daemon;

namespace GrandSearch {
class SemanticParserPrivate
{
public:
    explicit SemanticParserPrivate(SemanticParser *parent);
    //~SemanticParserPrivate();
public:
    AnalyzeServer *m_server = nullptr;
private:
    SemanticParser *q;
};
}
#endif // SEMANTICPARSER_P_H
