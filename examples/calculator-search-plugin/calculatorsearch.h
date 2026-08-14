// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CALCULATORSEARCH_H
#define CALCULATORSEARCH_H

#include <QObject>
#include <QHash>
#include <QMutex>

class CalculatorSearch : public QObject
{
    Q_OBJECT
public:
    explicit CalculatorSearch(QObject *parent = nullptr);
    ~CalculatorSearch() override;

    QString search(const QString &json);
    bool stop(const QString &json);
    bool action(const QString &json);

private:
    struct SearchInput {
        QString ver;
        QString mID;
        QString cont;
    };

    struct ActionResult {
        QString expression;
        double result;
    };

    SearchInput parseSearchInput(const QString &json) const;
    QString buildSearchResult(const QString &mID, const QString &expression, double result) const;
    QString buildEmptyResult(const QString &mID) const;

    bool isMathExpression(const QString &text) const;
    bool tryEvaluate(const QString &expression, double &result) const;
    QString formatResult(double value) const;

    QHash<QString, ActionResult> m_lastResults;
    QMutex m_mutex;
};

#endif // CALCULATORSEARCH_H
