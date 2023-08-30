// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEMANTICPARSER_H
#define SEMANTICPARSER_H

#include <QObject>
#include <QVariant>

namespace GrandSearch {
class SemanticParserPrivate;
class SemanticParser : public QObject
{
    Q_OBJECT
public:
    explicit SemanticParser(QObject *parent = nullptr);
    ~SemanticParser();
    bool connectToHost(const QString &service);
    QString analyze(const QString &text);
public slots:
private:
    SemanticParserPrivate *d;
};
}


#endif // SEMANTICPARSER_H