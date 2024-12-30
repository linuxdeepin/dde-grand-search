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
#include "../semanticworker_p.h"

namespace GrandSearch {
// 对应RulePrimary
class BaseCond : public QObject {
    Q_OBJECT
public:
    explicit BaseCond(const QString &text, QObject *parent = nullptr);
    virtual ~BaseCond();
    void setCond(const QString &text) { m_cond = text; }
    QString getCond() { return m_cond; }
    int getCondType() { return m_condType; }
    void setParent(BaseCond *cond) { m_parent = cond; }
    BaseCond *getParent() { return m_parent; }
    void addAndCond(BaseCond *cond);
    void addOrCond(BaseCond *cond);
    // 某些条件没有存在的必要
    virtual bool isValid() { return m_isValid; }
    // 结构优化调整
    void adjust();
    // 合并Binary层级的条件
    virtual void mergeBinary();
    // 合并Base层级的条件（Binary层级之上）
    virtual void mergeBase();
    // 为搜索引擎，合并各条件
    virtual void merge4Engine();
    // 为OR条件调整条件组合
    void adjust4OrCond();

    virtual QList<SemanticEntity> entityList();
    virtual QString toString(int spaceCounts = 0);

public:
    // mergeBase环节，用于记录收敛情况
    static bool kIsNeedAdjustBase;
    QString m_cond;
    int m_condType = querylangParser::RulePrimary;
    BaseCond *m_parent = nullptr;
    QList<BaseCond *> m_andCondList;
    QList<BaseCond *> m_orCondList;
    bool m_isValid = true;
    SemanticEntity m_entity;

public:
    enum {
        RuleEngine = 100,
        RuleAnything,
        RuleFeature,
        RuleFulltext,
    };
};

// 对应RuleBinaryExpression
class BinaryCond : public BaseCond {
public:
    explicit BinaryCond(const QString &text, QObject *parent = nullptr) : BaseCond(text, parent) {
        m_condType = querylangParser::RuleBinaryExpression;
    }
};

// 对应RuleDateSearchinfo  DATE <= CURRENT - "16 hour"
class DateInfoCond : public BaseCond {
public:
    explicit DateInfoCond(const QString &text, QObject *parent = nullptr);

public:
    QString m_compType;
    qint64 m_timestamp;
    qint64 m_timestamp2;
};

// 对应RulePathSearch  PATH IS "Downloads"
class PathCond : public BaseCond {
public:
    explicit PathCond(const QString &text, QObject *parent = nullptr);

public:
    bool m_isTruePath = true;
    QString m_pathName;
};

// 对应RuleNameSearch  NAME CONTAINS "名字"
class NameCond : public BaseCond {
public:
    explicit NameCond(const QString &text, QObject *parent = nullptr);

public:
    QString m_name;
};

// 对应RuleSizeSearch  SIZE <= "4GB"
class SizeCond : public BaseCond {
public:
    explicit SizeCond(const QString &text, QObject *parent = nullptr);

public:
    QString m_compType;
    qint64 m_fileSize;
};

// 对应RuleTypeSearch  TYPE IS "txt"
class TypeCond : public BaseCond {
public:
    explicit TypeCond(const QString &text, QObject *parent = nullptr);

public:
    bool m_isTrueType = true;
    QString m_typeName;
};

// 对应RuleDurationSearch  DURATION > \"3 minute\"
class DurationCond : public BaseCond {
public:
    explicit DurationCond(const QString &text, QObject *parent = nullptr);
    static QString formatTime(qint64 msec);

public:
    QString m_compType;
    QString m_duration;
};

// 对应RuleMetaSearch  META_TYPE IS \"ARTIST\" AND META_VALUE IS \"xxx\"
class MetaCond : public BaseCond {
public:
    explicit MetaCond(const QString &text, QObject *parent = nullptr);

public:
    bool m_isTrue = true;
    QString m_metaType;
    QString m_metaValue;
};

// 对应RuleQuantityCondition  QUANTITY = 10
class QuantityCond : public BaseCond {
public:
    explicit QuantityCond(const QString &text, QObject *parent = nullptr);
    int getQuantity() { return m_quantity; }

private:
    int m_quantity = 0;
};

// 对应RuleContentSearch  CONTENT CONTAINS "read"
class ContentCond : public BaseCond {
public:
    explicit ContentCond(const QString &text, QObject *parent = nullptr);

public:
    QString m_content;
};

// 对应RuleFilename
class FileNameCond : public BaseCond {
public:
    explicit FileNameCond(const QString &text, QObject *parent = nullptr) : BaseCond(text, parent) {}
};

// 对应合并后的Engine
class EngineCond : public BaseCond {
public:
    explicit EngineCond(QList<BaseCond *> *andList, QObject *parent = nullptr);
    // 吸收目标条件至自身
    bool absorbCond(BaseCond *cond);
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
    explicit DslParser(const QString &text, QObject *parent = nullptr);
    ~DslParser() {}
    QList<SemanticEntity> entityList() { return m_cond.entityList(); }

private:
    BaseCond m_cond;
};
}

#endif // DSLPARSER_H
