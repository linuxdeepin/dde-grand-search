// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dslparser.h"
#include <tree/ParseTreeWalker.h>

#include <QDebug>

using namespace GrandSearch;
using namespace antlr4;

BaseCond::BaseCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                   SemanticWorkerPrivate *worker, QObject *parent) : QObject(parent) {
    setCond(text);
    m_querys = querys;
    m_worker = worker;
}

BaseCond::~BaseCond() {
    for (BaseCond *cond : m_andCondList) {
        delete cond;
    }

    for (BaseCond *cond : m_orCondList) {
        delete cond;
    }
}

void BaseCond::addAndCond(BaseCond *cond) {
    qDebug() << QString("BaseCond type(%1) add AND(%2)").arg(m_condType).arg(cond->getCondType());
    m_andCondList.append(cond);
    cond->setParent(this);
}

void BaseCond::addOrCond(BaseCond *cond) {
    qDebug() << QString("BaseCond type(%1) add OR(%2)").arg(m_condType).arg(cond->getCondType());
    m_orCondList.append(cond);
    cond->setParent(this);
}

bool BaseCond::isMatch(const QString &text) {
    bool ret = true;
    qDebug() << QString("BaseCond type(%1) isMatch(%2) m_cond(%3)").arg(m_condType).arg(text).arg(m_cond);
    if (m_andCondList.empty() && m_orCondList.empty()) {
        ret = false;
        goto _EXIT;
    }

    if (!m_andCondList.empty()) {
        for (BaseCond *cond : m_andCondList) {
            if (!cond->isMatch(text)) {
                ret = false;
                goto _EXIT;
            }
        }

        ret = true;
        goto _EXIT;
    }

    for (BaseCond *cond : m_orCondList) {
        if (cond->isMatch(text)) {
            ret = true;
            goto _EXIT;
        }
    }

    ret = false;

_EXIT:
    qInfo() << QString("BaseCond type(%1) isMatch(%2) ret(%3) m_cond(%4)").arg(m_condType).arg(text).arg(ret).arg(m_cond);
    return ret;
}

bool BaseCond::addMatchedItems(const MatchedItems &items) {
    m_matchedItems.append(items);
}

const MatchedItems &BaseCond::getMatchedItems() {
    if (m_condType != querylangParser::RulePrimary && m_condType != querylangParser::RuleBinaryExpression) {
        return m_matchedItems;
    }

    if (!m_andCondList.isEmpty()) {
        m_matchedItems = m_andCondList[0]->getMatchedItems();
        for (int i = 1; i < m_andCondList.size(); i++) {
            for (int j = m_matchedItems.size() - 1; j >= 0; j--) {
                if (m_andCondList[i]->getMatchedItems().contains(m_matchedItems[j])) {
                    continue;
                }

                m_matchedItems.removeAt(j);
            }
        }
        return m_matchedItems;
    }

    m_matchedItems = m_orCondList[0]->getMatchedItems();
    for (int i = 1; i < m_orCondList.size(); i++) {
        const MatchedItems &items = m_orCondList[i]->getMatchedItems();
        for (int j = 0; j < items.size(); j++) {
            if (m_matchedItems.contains(items[j])) {
                continue;
            }

            m_matchedItems.append(items[j]);
        }
    }
    return m_matchedItems;
}

QString BaseCond::toString(int spaceCounts) {
    // 控制缩进
    QString space;
    for (int i = 0; i < spaceCounts; i++) {
        space.append(" ");
    }
    spaceCounts += 2;

    QString str;
    for (BaseCond *cond : m_andCondList) {
        str.append(QString("%1AND: %2(%3)\n").arg(space).arg(cond->getCondType()).arg(cond->getCond()));
        str.append(cond->toString(spaceCounts));
    }
    for (BaseCond *cond : m_orCondList) {
        str.append(QString("%1OR: %2(%3)\n").arg(space).arg(cond->getCondType()).arg(cond->getCond()));
        str.append(cond->toString(spaceCounts));
    }
    return str;
}

DateInfoCond::DateInfoCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                   SemanticWorkerPrivate *worker, QObject *parent) : BaseCond(text, querys, worker, parent) {
    m_condType = querylangParser::RuleDateSearchinfo;
    int pos = m_cond.indexOf("\"") + 1;
    QString temp = m_cond.mid(pos, m_cond.length() - pos - 1);
    if (m_cond.contains(">")) {
        m_compType = ">";
    } else {
        m_compType = "<";
    }
    const qint64 _NOW = QDateTime::currentSecsSinceEpoch();
    QRegExp reg("([\\d\\.]+) ?year", Qt::CaseInsensitive);
    QRegExp reg1("([\\d\\.]+) ?mouth", Qt::CaseInsensitive);
    QRegExp reg2("([\\d\\.]+) ?week", Qt::CaseInsensitive);
    QRegExp reg3("([\\d\\.]+) ?day", Qt::CaseInsensitive);
    QRegExp reg4("([\\d\\.]+) ?hour", Qt::CaseInsensitive);
    QRegExp reg5("([\\d\\.]+) ?(min|minute)", Qt::CaseInsensitive);
    if (reg.indexIn(temp) != -1) {
        m_timestamp = _NOW - static_cast<qint64>(reg.cap(1).toFloat() * 60 * 60 * 24 * 365);
    } else if (reg1.indexIn(temp) != -1) {
        m_timestamp = _NOW - static_cast<qint64>(reg1.cap(1).toFloat() * 60 * 60 * 24 * 30);
    } else if (reg2.indexIn(temp) != -1) {
        m_timestamp = _NOW - static_cast<qint64>(reg2.cap(1).toFloat() * 60 * 60 * 24 * 7);
    } else if (reg3.indexIn(temp) != -1) {
        m_timestamp = _NOW - static_cast<qint64>(reg3.cap(1).toFloat() * 60 * 60 * 24);
    } else if (reg4.indexIn(temp) != -1) {
        m_timestamp = _NOW - static_cast<qint64>(reg4.cap(1).toFloat() * 60 * 60);
    } else if (reg5.indexIn(temp) != -1) {
        m_timestamp = _NOW - static_cast<qint64>(reg5.cap(1).toFloat() * 60);
    } else {
        m_timestamp = _NOW;
    }
    qDebug() << QString("DateInfoCond(%1): temp(%2) m_compType(%3) m_timestamp(%4)").arg(m_cond).arg(temp).arg(m_timestamp);

    if (m_compType == ">") {
        m_entity.times.append(QPair<qint64, qint64>(m_timestamp, _NOW));
    } else {
        m_entity.times.append(QPair<qint64, qint64>(0, m_timestamp));
    }
    m_anything.setEntity(m_entity);
    SemanticWorkerPrivate::QueryFunction func = {&m_anything, &AnythingQuery::run, this};
    m_querys->append(func);
}

PathCond::PathCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                   SemanticWorkerPrivate *worker, QObject *parent) : BaseCond(text, querys, worker, parent) {
    m_condType = querylangParser::RulePathSearch;
    if (m_cond.contains("IS NOT")) {
        m_isTrue = false;
    }

    int pos = m_cond.indexOf("\"") + 1;
    m_pathName = m_cond.mid(pos, m_cond.length() - pos - 1);
    qDebug() << QString("PathCond(%1): m_isTrue(%2) m_pathName(%3)").arg(m_cond).arg(m_isTrue).arg(m_pathName);

    m_entity.absolutePath = m_pathName;
    m_entity.isTrue = m_isTrue;
    m_anything.setEntity(m_entity);
    SemanticWorkerPrivate::QueryFunction func = {&m_anything, &AnythingQuery::run, this};
    m_querys->append(func);
}

NameCond::NameCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                   SemanticWorkerPrivate *worker, QObject *parent) : BaseCond(text, querys, worker, parent) {
    m_condType = querylangParser::RuleNameSearch;
    int pos = m_cond.indexOf("\"") + 1;
    m_name = m_cond.mid(pos, m_cond.length() - pos - 1);
    qDebug() << QString("NameCond(%1): m_name(%2)").arg(m_cond).arg(m_name);

    m_entity.keys.append(m_name);
    m_anything.setEntity(m_entity);
    SemanticWorkerPrivate::QueryFunction func = {&m_anything, &AnythingQuery::run, this};
    m_querys->append(func);
}

SizeCond::SizeCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                   SemanticWorkerPrivate *worker, QObject *parent) : BaseCond(text, querys, worker, parent) {
    m_condType = querylangParser::RuleSizeSearch;
    int pos = m_cond.indexOf("\"") + 1;
    QString temp = m_cond.mid(pos, m_cond.length() - pos - 1);
    if (m_cond.contains(">")) {
        m_compType = ">";
    } else if (m_cond.contains("<")) {
        m_compType = "<";
    } else if (m_cond.contains("!=")) {
        m_compType = "!=";
    } else {
        m_compType = "=";
    }
    QRegExp reg("([\\d\\.]+) ?gb", Qt::CaseInsensitive);
    QRegExp reg1("([\\d\\.]+) ?mb", Qt::CaseInsensitive);
    QRegExp reg2("([\\d\\.]+) ?kb", Qt::CaseInsensitive);
    QRegExp reg3("([\\d\\.]+) ?(b|byte|bytes)", Qt::CaseInsensitive);
    if (reg.indexIn(temp) != -1) {
        m_fileSize = static_cast<qint64>(reg.cap(1).toFloat() * 1024 * 1024 * 1024);
    } else if (reg1.indexIn(temp) != -1) {
        m_fileSize = static_cast<qint64>(reg1.cap(1).toFloat() * 1024 * 1024);
    } else if (reg2.indexIn(temp) != -1) {
        m_fileSize = static_cast<qint64>(reg2.cap(1).toFloat() * 1024);
    } else if (reg3.indexIn(temp) != -1) {
        m_fileSize = static_cast<qint64>(reg3.cap(1).toFloat());
    } else {
        m_fileSize = 0;
    }
    qDebug() << QString("SizeCond(%1): temp(%2) m_compType(%3) m_fileSize(%4)").arg(m_cond).arg(temp).arg(m_compType).arg(m_fileSize);

    m_entity.compType = m_compType;
    m_entity.fileSize = m_fileSize;
    m_anything.setEntity(m_entity);
    SemanticWorkerPrivate::QueryFunction func = {&m_anything, &AnythingQuery::run, this};
    m_querys->append(func);
}

TypeCond::TypeCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                   SemanticWorkerPrivate *worker, QObject *parent) : BaseCond(text, querys, worker, parent) {
    m_condType = querylangParser::RuleTypeSearch;
    if (m_cond.contains("IS NOT")) {
        m_isTrue = false;
    }

    int pos = m_cond.indexOf("\"") + 1;
    m_typeName = m_cond.mid(pos, m_cond.length() - pos - 1);
    qDebug() << QString("TypeCond(%1): m_isTrue(%2) m_typeName(%3)").arg(m_cond).arg(m_isTrue).arg(m_typeName);

    m_entity.suffix = m_typeName;
    m_entity.isTrue = m_isTrue;
    m_anything.setEntity(m_entity);
    SemanticWorkerPrivate::QueryFunction func = {&m_anything, &AnythingQuery::run, this};
    m_querys->append(func);
}

ContentCond::ContentCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                         SemanticWorkerPrivate *worker, QObject *parent) : BaseCond(text, querys, worker, parent) {
    m_condType = querylangParser::RuleContentSearch;
    int pos = m_cond.indexOf("\"") + 1;
    m_content = m_cond.mid(pos, m_cond.length() - pos - 1);
    qDebug() << QString("ContentCond(%1): m_content(%2)").arg(m_cond).arg(m_content);

    m_entity.keys.append(m_content);
    m_fulltext.setEntity(m_entity);
    SemanticWorkerPrivate::QueryFunction func = {&m_fulltext, &FullTextQuery::run, this};
    m_querys->append(func);
}

void DslParserListener::enterEveryRule(antlr4::ParserRuleContext *ctx) {
    QString text = QString().fromStdString(ctx->getText());
    switch (ctx->getRuleIndex()) {
    case querylangParser::RulePrimary:
    case querylangParser::RuleBinaryExpression:
    case querylangParser::RuleDateSearchinfo:
    case querylangParser::RulePathSearch:
    case querylangParser::RuleNameSearch:
    case querylangParser::RuleSizeSearch:
    case querylangParser::RuleTypeSearch:
    case querylangParser::RuleDurationSearch:
    case querylangParser::RuleMetaSearch:
    case querylangParser::RuleQuantityCondition:
    case querylangParser::RuleContentSearch:
    case querylangParser::RuleFilename:
        m_prefix += "  ";
        qDebug() << QString("%1=> enter(%2): %3").arg(m_prefix).arg(ruleContextId2String((int)ctx->getRuleIndex())).arg(text);
        break;
    }

    BaseCond *ptr = nullptr;
    if (ctx->getRuleIndex() == querylangParser::RulePrimary) {
        if (m_prefix.length() == 2) {
            m_cond->setCond(text);
            return;
        }
    }

    bool isAnd = true, isOr = true;
    switch (ctx->getRuleIndex()) {
    case querylangParser::RulePrimary:
        ptr = new BaseCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleBinaryExpression:
        ptr = new BinaryCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        isAnd = (m_cond->getCond().contains(QString("AND") + text));
        isOr  = (m_cond->getCond().contains(QString("OR") + text));
        if (isAnd) {
            m_tempPtr->addAndCond(ptr);
        } else if (isOr) {
            m_tempPtr->addOrCond(ptr);
        } else if (m_cond->getCond().contains(text + QString("OR"))) {
            m_tempPtr->addOrCond(ptr);
        } else {
            m_tempPtr->addAndCond(ptr);
        }
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleDateSearchinfo:
        ptr = new DateInfoCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RulePathSearch:
        ptr = new PathCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleNameSearch:
        ptr = new NameCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleSizeSearch:
        ptr = new SizeCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleTypeSearch:
        ptr = new TypeCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleDurationSearch:
        ptr = new DurationCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleMetaSearch:
        ptr = new MetaCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleQuantityCondition:
        ptr = new QuantityCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleContentSearch:
        ptr = new ContentCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleFilename:
        ptr = new FileNameCond(text, m_tempPtr->m_querys, m_tempPtr->m_worker);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    }
}

void DslParserListener::exitEveryRule(antlr4::ParserRuleContext *ctx) {
    QString text = QString().fromStdString(ctx->getText());
    switch (ctx->getRuleIndex()) {
    case querylangParser::RulePrimary:
    case querylangParser::RuleBinaryExpression:
    case querylangParser::RuleDateSearchinfo:
    case querylangParser::RulePathSearch:
    case querylangParser::RuleNameSearch:
    case querylangParser::RuleSizeSearch:
    case querylangParser::RuleTypeSearch:
    case querylangParser::RuleDurationSearch:
    case querylangParser::RuleMetaSearch:
    case querylangParser::RuleQuantityCondition:
    case querylangParser::RuleContentSearch:
    case querylangParser::RuleFilename:
        qDebug() << QString("%1<= exit(%2): %3").arg(m_prefix).arg(ruleContextId2String((int)ctx->getRuleIndex())).arg(text);
        if (m_prefix.length() >= 2) {
            m_prefix = m_prefix.mid(0, m_prefix.length() - 2);
        }

        if (m_tempPtr) {
            m_tempPtr = m_tempPtr->getParent();
        }
        break;
    }
}

DslParser::DslParser(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys, FileResultsHandler *fileHandler,
                     SemanticWorkerPrivate *worker, QObject *parent) : QObject(parent), m_cond("", querys, worker) {
    qDebug() << QString("DslParser DslParser(%1)").arg(text);
    ANTLRInputStream input(text.toUtf8().toStdString());
    querylangLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    tokens.fill();
    querylangParser parser(&tokens);
    DslParserListener listener(&m_cond);
    tree::ParseTreeWalker walker;
    walker.walk(&listener, parser.query());
    qInfo() << QString("BaseCond:\n%1").arg(m_cond.toString()).toUtf8().toStdString().c_str();
}

bool DslParser::isMatch(const QString &text) {
    qInfo() << QString("DslParser isMatch(%1)").arg(text);
    bool ret = m_cond.isMatch(text);
    qInfo() << QString("DslParser isMatch(%1) ret(%2)").arg(text).arg(ret);
    return ret;
}
