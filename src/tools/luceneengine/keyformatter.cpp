// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "keyformatter.h"

#include <LuceneHeaders.h>
#include <TokenGroup.h>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logToolFullText)

using namespace GrandSearch;
using namespace Lucene;

KeyFormatter::KeyFormatter() : Formatter(), LuceneObject() {
    qCDebug(logToolFullText) << "KeyFormatter constructed";
}

KeyFormatter::~KeyFormatter()
{
    qCDebug(logToolFullText) << "KeyFormatter destructor called - Total tokens collected:" << tokens.size();
}

void KeyFormatter::clear()
{
    qCDebug(logToolFullText) << "Clearing KeyFormatter - Previous tokens:" << tokens.size() << "Current token length:" << token.length();

    token.clear();
    tokens.clear();

    qCDebug(logToolFullText) << "KeyFormatter cleared";
}

String KeyFormatter::highlightTerm(const String &originalText, const TokenGroupPtr &tokenGroup)
{
    if (tokenGroup->getTotalScore() == 0) {
        if (!token.isEmpty()) {
            tokens.insert(token);
            token.clear();
        }
    } else {
        token.append(StringUtils::toUTF8(originalText).c_str());
    }

    return originalText;
}
