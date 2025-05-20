// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "semanticparser_p.h"

#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logToolSemantic, "org.deepin.dde.grandsearch.tool.semantic")

using namespace GrandSearch;

SemanticParserPrivate::SemanticParserPrivate(SemanticParser *parent)
    : q(parent)
{
}

QString SemanticParser::vectorSearch(const QString &context)
{
    QString ret;
    if (!d->m_vector) {
        qCWarning(logToolSemantic) << "Vector search failed - Vector service not initialized";
        return ret;
    }
    qCDebug(logToolSemantic) << "Performing vector search - Context:" << context;
    ret = d->m_vector->Search("dde-grand-search", context, 50);
    return ret;
}

QString SemanticParser::query(const QString &text)
{
    Q_ASSERT(d->m_querylang);

    QString ret;
    qCDebug(logToolSemantic) << "Performing semantic query - Text:" << text;
    auto reply = d->m_querylang->Query(text);
    ret = reply.value();
    if (reply.isError()) {
        qCWarning(logToolSemantic) << "Semantic query failed - Error:" << reply.error().message();
        ret.clear();
    }
    return ret;
}

bool SemanticParser::isVectorSupported()
{
    if (!d->m_vector) {
        qCDebug(logToolSemantic) << "Vector support check failed - Service not initialized";
        return false;
    }

    bool supported = d->m_vector->Enable();
    qCDebug(logToolSemantic) << "Vector support status - Enabled:" << supported;
    return supported;
}

bool SemanticParser::isQueryLangSupported()
{
    if (!d->m_querylang) {
        qCDebug(logToolSemantic) << "Query language support check failed - Service not initialized";
        return false;
    }

    bool supported = d->m_querylang->Enable();
    qCDebug(logToolSemantic) << "Query language support status - Enabled:" << supported;
    return supported;
}

SemanticParser::SemanticParser(QObject *parent)
    : QObject(parent), d(new SemanticParserPrivate(this))
{
}

SemanticParser::~SemanticParser()
{
    delete d;
    d = nullptr;
}

bool SemanticParser::connectToVector(const QString &service)
{
    if (d->m_vector) {
        qCWarning(logToolSemantic) << "Vector service already connected";
        return false;
    }

    qCDebug(logToolSemantic) << "Connecting to vector service - Service:" << service;
    d->m_vector = new VectorIndex(service, "/org/deepin/ai/daemon/VectorIndex",
                                  QDBusConnection::sessionBus(), this);
    d->m_vector->setTimeout(60 * 1000);
    bool valid = d->m_vector->isValid();
    qCDebug(logToolSemantic) << "Vector service connection status - Valid:" << valid;
    return valid;
}

bool SemanticParser::connectToQueryLang(const QString &service)
{
    if (d->m_querylang) {
        qCWarning(logToolSemantic) << "Query language service already connected";
        return false;
    }

    qCDebug(logToolSemantic) << "Connecting to query language service - Service:" << service;
    d->m_querylang = new QueryLang(service, "/org/deepin/ai/daemon/QueryLang",
                                   QDBusConnection::sessionBus(), this);
    d->m_querylang->setTimeout(60 * 1000);
    bool valid = d->m_querylang->isValid();
    qCDebug(logToolSemantic) << "Query language service connection status - Valid:" << valid;
    return valid;
}
