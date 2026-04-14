// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEMANTICSEARCHER_P_H
#define SEMANTICSEARCHER_P_H

#include "semanticsearcher.h"

namespace GrandSearch {

class SemanticSearcherPrivate
{
public:
    explicit SemanticSearcherPrivate(SemanticSearcher *parent);
    ~SemanticSearcherPrivate();

public:
    bool m_semanticEnabled = false;   // 语义搜索是否启用
    bool m_fulltextEnabled = false;   // 全文搜索是否启用
    bool m_ocrtextEnabled = false;   // OCR 文本搜索是否启用

private:
    SemanticSearcher *q;
};

}

#endif // SEMANTICSEARCHER_P_H
