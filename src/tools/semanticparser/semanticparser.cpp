// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "semanticparser_p.h"

#include <QDebug>

using namespace GrandSearch;

SemanticParserPrivate::SemanticParserPrivate(SemanticParser *parent) : q(parent)
{

}

QString SemanticParser::analyze(const QString &text)
{
    Q_ASSERT(d->m_server);

    QString ret;
    auto reply = d->m_server->Analyze(text);
    ret = reply.value();
    if (reply.isError()) {
        qWarning() << "the parser server return error" << reply.error().message();
        ret.clear();
    }
    return ret;
}

SemanticParser::SemanticParser(QObject *parent)
    : QObject(parent)
    , d(new SemanticParserPrivate(this))
{

}

SemanticParser::~SemanticParser()
{
    delete d;
    d = nullptr;
}

bool SemanticParser::connectToHost(const QString &service)
{
    if (d->m_server)
        return false;

    d->m_server = new AnalyzeServer(service, "/org/deepin/ai/daemon/AnalyzeServer",
                                    QDBusConnection::sessionBus(), this);
    d->m_server->setTimeout(60 * 1000);
    return d->m_server->isValid();
}
