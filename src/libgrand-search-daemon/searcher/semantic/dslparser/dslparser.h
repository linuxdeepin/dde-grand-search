// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DSLPARSER_H
#define DSLPARSER_H

#include <QObject>
#include <QVariant>

#include <antlr4-runtime.h>
#include <querylangParser.h>
#include <querylangLexer.h>

#include "../fileresultshandler.h"
#include "../semanticworker_p.h"

#include "../database/anythingquery.h"
#include "../database/fulltextquery.h"
#include "../database/featurequery.h"
#include "../database/vectorquery.h"

namespace GrandSearch {
class SemanticWorkerPrivate;
// 对应RulePrimary
class BaseCond : public QObject {
    Q_OBJECT
public:
    explicit BaseCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                      SemanticWorkerPrivate *worker, QObject *parent = nullptr);
    virtual ~BaseCond();
    void setCond(const QString &text) { m_cond = text; }
    QString getCond() { return m_cond; }
    int getCondType() { return m_condType; }
    void setParent(BaseCond *cond) { m_parent = cond; }
    BaseCond *getParent() { return m_parent; }
    void addAndCond(BaseCond *cond);
    void addOrCond(BaseCond *cond);
    // 调试使用
    virtual bool isMatch(const QString &text);
    // 某些条件没有存在的必要
    virtual bool isValid() { return m_isValid; }
    // 结构优化调整
    virtual void adjust();
    virtual bool addMatchedItems(const MatchedItems &items);
    virtual const MatchedItems &getMatchedItems();
    virtual QString toString(int spaceCounts = 0);

public:
    QString m_cond;
    int m_condType = querylangParser::RulePrimary;
    QList<SemanticWorkerPrivate::QueryFunction> *m_querys = nullptr;
    SemanticWorkerPrivate *m_worker = nullptr;
    BaseCond *m_parent = nullptr;
    MatchedItems m_matchedItems;
    QList<BaseCond *> m_andCondList;
    QList<BaseCond *> m_orCondList;
    bool m_isValid = true;
};

// 对应RuleBinaryExpression
class BinaryCond : public BaseCond {
public:
    explicit BinaryCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                        SemanticWorkerPrivate *worker, QObject *parent = nullptr) : BaseCond(text, querys, worker, parent) {
        m_condType = querylangParser::RuleBinaryExpression;
    }
};

// 对应RuleDateSearchinfo  DATE <= CURRENT - "16 hour"
class DateInfoCond : public BaseCond {
public:
    explicit DateInfoCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                          SemanticWorkerPrivate *worker, QObject *parent = nullptr);

private:
    QString m_compType;
    qint64 m_timestamp;
    AnythingQuery m_anything;
    SemanticEntity m_entity;
};

// 对应RulePathSearch  PATH IS "/home/david/readme.txt"
class PathCond : public BaseCond {
public:
    explicit PathCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                      SemanticWorkerPrivate *worker, QObject *parent = nullptr);

private:
    bool m_isTrue = true;
    QString m_pathName;
    AnythingQuery m_anything;
    SemanticEntity m_entity;
};

// 对应RuleNameSearch  NAME CONTAINS "周杰伦"
class NameCond : public BaseCond {
public:
    explicit NameCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                      SemanticWorkerPrivate *worker, QObject *parent = nullptr);

private:
    QString m_name;
    AnythingQuery m_anything;
    SemanticEntity m_entity;
};

// 对应RuleSizeSearch  SIZE <= "4GB"
class SizeCond : public BaseCond {
public:
    explicit SizeCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                      SemanticWorkerPrivate *worker, QObject *parent = nullptr);

private:
    QString m_compType;
    qint64 m_fileSize;
    AnythingQuery m_anything;
    SemanticEntity m_entity;
};

// 对应RuleTypeSearch  TYPE IS "txt"
class TypeCond : public BaseCond {
public:
    explicit TypeCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                      SemanticWorkerPrivate *worker, QObject *parent = nullptr);

private:
    bool m_isTrue = true;
    QString m_typeName;
    AnythingQuery m_anything;
    SemanticEntity m_entity;
};

// 对应RuleDurationSearch
class DurationCond : public BaseCond {
public:
    explicit DurationCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                          SemanticWorkerPrivate *worker, QObject *parent = nullptr) : BaseCond(text, querys, worker, parent) {}
};

// 对应RuleMetaSearch
class MetaCond : public BaseCond {
public:
    explicit MetaCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                      SemanticWorkerPrivate *worker, QObject *parent = nullptr) : BaseCond(text, querys, worker, parent) {}
};

// 对应RuleQuantityCondition  QUANTITY = 10
class QuantityCond : public BaseCond {
public:
    explicit QuantityCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                          SemanticWorkerPrivate *worker, QObject *parent = nullptr);
    int getQuantity() { return m_quantity; }

private:
    int m_quantity = 0;
};

// 对应RuleContentSearch  CONTENT CONTAINS "read"
class ContentCond : public BaseCond {
public:
    explicit ContentCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                         SemanticWorkerPrivate *worker, QObject *parent = nullptr);

private:
    QString m_content;
    FullTextQuery m_fulltext;
    SemanticEntity m_entity;
};

// 对应RuleFilename
class FileNameCond : public BaseCond {
public:
    explicit FileNameCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                          SemanticWorkerPrivate *worker, QObject *parent = nullptr) : BaseCond(text, querys, worker, parent) {}
};

class DslParserListener : public antlr4::tree::ParseTreeListener {
public:
    static QString ruleContextId2String(int id) {
        switch (id) {
        case querylangParser::RuleQuery:
            return "RuleQuery";
        case querylangParser::RuleInvalideQuery:
            return "RuleInvalideQuery";
        case querylangParser::RulePrimary:
            return "RulePrimary";
        case querylangParser::RuleBinaryExpression:
            return "RuleBinaryExpression";
        case querylangParser::RuleSearchCondition:
            return "RuleSearchCondition";
        case querylangParser::RuleDateSearch:
            return "RuleDateSearch";
        case querylangParser::RuleBinaryDateSearch:
            return "RuleBinaryDateSearch";
        case querylangParser::RuleDateSearchinfo:
            return "RuleDateSearchinfo";
        case querylangParser::RuleAbsolutedate:
            return "RuleAbsolutedate";
        case querylangParser::RuleRelativelydate:
            return "RuleRelativelydate";
        case querylangParser::RulePathSearch:
            return "RulePathSearch";
        case querylangParser::RuleNameSearch:
            return "RuleNameSearch";
        case querylangParser::RuleSizeSearch:
            return "RuleSizeSearch";
        case querylangParser::RuleTypeSearch:
            return "RuleTypeSearch";
        case querylangParser::RuleDurationSearch:
            return "RuleDurationSearch";
        case querylangParser::RuleMetaSearch:
            return "RuleMetaSearch";
        case querylangParser::RuleQuantityCondition:
            return "RuleQuantityCondition";
        case querylangParser::RuleContentSearch:
            return "RuleContentSearch";
        case querylangParser::RuleComparison_type:
            return "RuleComparison_type";
        case querylangParser::RuleFilename:
            return "RuleFilename";
        case querylangParser::RuleString:
            return "RuleString";
        case querylangParser::RuleIs_or_not:
            return "RuleIs_or_not";
        default:
            return "";
        }
    }

    explicit DslParserListener(BaseCond *cond) {
        m_cond    = cond;
        m_tempPtr = cond;
    }
    void visitTerminal(antlr4::tree::TerminalNode *node) override {}
    void visitErrorNode(antlr4::tree::ErrorNode *node) override {}
    void enterEveryRule(antlr4::ParserRuleContext *ctx) override;
    void exitEveryRule(antlr4::ParserRuleContext *ctx) override;

private:
    QString m_prefix;
    BaseCond *m_cond    = nullptr;
    BaseCond *m_tempPtr = nullptr;
};

class DslParser : public QObject {
    Q_OBJECT
public:
    explicit DslParser(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys, FileResultsHandler *fileHandler,
                       SemanticWorkerPrivate *worker, QObject *parent = nullptr);
    ~DslParser() {}
    bool isMatch(const QString &text);
    const MatchedItems &getMatchedItems() { return m_cond.getMatchedItems(); }

private:
    BaseCond m_cond;
};
}

#endif // DSLPARSER_H
