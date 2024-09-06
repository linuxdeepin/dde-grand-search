// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEMANTICSEARCHER_P_H
#define SEMANTICSEARCHER_P_H

#include "semanticsearcher.h"

#include <QUrl>

namespace GrandSearch {
class SemanticSearcherPrivate
{
public:
    explicit SemanticSearcherPrivate(SemanticSearcher *parent);
    ~SemanticSearcherPrivate();
private:
    SemanticSearcher *q;
};

}
#endif // SEMANTICSEARCHER_P_H
