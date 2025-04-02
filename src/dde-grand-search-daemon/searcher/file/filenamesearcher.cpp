// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filenamesearcher.h"
#include "global/builtinsearch.h"
#include "filenameworker.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnectionInterface>

using namespace GrandSearch;

FileNameSearcher::FileNameSearcher(QObject *parent) : Searcher(parent)
{
}

QString FileNameSearcher::name() const
{
    return GRANDSEARCH_CLASS_FILE_DEEPIN;
}

bool FileNameSearcher::isActive() const
{
    QDBusConnectionInterface *cif = QDBusConnection::systemBus().interface();
    Q_ASSERT(cif);

    return cif->isServiceRegistered("com.deepin.anything");
}

bool FileNameSearcher::activate()
{
    return false;
}

ProxyWorker *FileNameSearcher::createWorker() const
{
    auto worker = new FileNameWorker(name(), supportParallelSearch());
    return worker;
}

bool FileNameSearcher::action(const QString &action, const QString &item)
{
    Q_UNUSED(item)
    qWarning() << "no such action:" << action << ".";
    return false;
}

bool FileNameSearcher::supportParallelSearch() const
{
    static QStringList methodNameList;
    if (!methodNameList.isEmpty())
        return methodNameList.contains("parallelsearch");

    QDBusInterface introspectable("com.deepin.anything",
                                  "/com/deepin/anything",
                                  "org.freedesktop.DBus.Introspectable",
                                  QDBusConnection::systemBus());
    QDBusReply<QString> reply = introspectable.call("Introspect");
    if (!reply.isValid())
        return false;

    QXmlStreamReader xmlReader;
    xmlReader.addData(reply.value());
    xmlReader.readNext();
    while (!xmlReader.atEnd()) {
        if (xmlReader.isStartElement() && xmlReader.name() == "method") {
            methodNameList << xmlReader.attributes().value("name").toString();
            xmlReader.skipCurrentElement();
        } else {
            xmlReader.readNext();
        }
    }

    return methodNameList.contains("parallelsearch");
}
