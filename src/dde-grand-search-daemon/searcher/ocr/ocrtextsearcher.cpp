// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrtextsearcher.h"
#include "ocrtextworker.h"
#include "global/builtinsearch.h"

#include <dfm-search/dsearch_global.h>

#include <DConfig>

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

DCORE_USE_NAMESPACE

using namespace GrandSearch;

OcrTextSearcher::OcrTextSearcher(QObject *parent)
    : Searcher(parent)
{
    qCDebug(logDaemon) << "OcrTextSearcher constructor";
}

QString OcrTextSearcher::name() const
{
    return GRANDSEARCH_CLASS_OCR_TEXT;
}

bool OcrTextSearcher::isActive() const
{
    // Check if OCR search is enabled via DConfig
    DConfig *dcfg = DConfig::create("org.deepin.dde.file-manager", "org.deepin.dde.file-manager.search");
    bool enabled = dcfg->value("enableOcrTextSearch").toBool() && DFMSEARCH::Global::isOcrTextIndexAvailable();
    delete dcfg;

    qCDebug(logDaemon) << "OcrTextSearcher activity check - OCR enabled:" << enabled;
    return enabled;
}

bool OcrTextSearcher::activate()
{
    qCDebug(logDaemon) << "OcrTextSearcher activate called";
    // OCR searcher doesn't need explicit activation
    return isActive();
}

ProxyWorker *OcrTextSearcher::createWorker() const
{
    qCDebug(logDaemon) << "Creating OcrTextWorker";
    auto worker = new OcrTextWorker(name());
    return worker;
}

bool OcrTextSearcher::action(const QString &action, const QString &item)
{
    Q_UNUSED(item)
    qCWarning(logDaemon) << "Unsupported action requested - Action:" << action;
    return false;
}
