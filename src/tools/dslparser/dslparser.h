// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DSLPARSER_H
#define DSLPARSER_H

#include <QObject>
#include <QVariant>

#include <vector>
#include <set>

#include <antlr4-runtime.h>
#include <querylangParser.h>
#include <querylangLexer.h>

namespace GrandSearch {
// 对应RulePrimary
class BaseCond : public QObject {
    Q_OBJECT
public:
    explicit BaseCond(QObject *parent = nullptr) : QObject(parent) {}
    explicit BaseCond(const QString &text, QObject *parent = nullptr) : QObject(parent) { setCond(text); }
    virtual ~BaseCond();
    void setCond(const QString &text) { m_cond = text; }
    QString &getCond() { return m_cond; }
    int getCondType() { return m_condType; }
    void setParent(BaseCond *cond) { m_parent = cond; }
    BaseCond *getParent() { return m_parent; }
    void addAndCond(BaseCond *cond);
    void addOrCond(BaseCond *cond);
    virtual bool isMatch(const QString &text);
    virtual QString toString(int spaceCounts = 0);

public:
    QString m_cond;
    int m_condType = querylangParser::RulePrimary;
    BaseCond *m_parent = nullptr;
    std::vector<BaseCond *> m_andCondVec;
    std::vector<BaseCond *> m_orCondVec;
};

// 对应RuleBinaryExpression
class BinaryCond : public BaseCond {
public:
    explicit BinaryCond(const QString &text, QObject *parent = nullptr) : BaseCond(text, parent) {
        m_condType = querylangParser::RuleBinaryExpression;
    }
};

// 对应RuleDateSearchinfo
class DateInfoCond : public BaseCond {
public:
    explicit DateInfoCond(const QString &text, QObject *parent = nullptr) : BaseCond(text, parent) {}
};

// 对应RulePathSearch
class PathCond : public BaseCond {
public:
    explicit PathCond(const QString &text, QObject *parent = nullptr);
    virtual bool isMatch(const QString &text) override;

private:
    bool m_isTrue = true;
    QString m_pathName;
};

// 对应RuleNameSearch
class NameCond : public BaseCond {
public:
    explicit NameCond(const QString &text, QObject *parent = nullptr) : BaseCond(text, parent) {}
};

// 对应RuleSizeSearch
class SizeCond : public BaseCond {
public:
    explicit SizeCond(const QString &text, QObject *parent = nullptr) : BaseCond(text, parent) {}
};

// 对应RuleTypeSearch
class TypeCond : public BaseCond {
public:
    explicit TypeCond(const QString &text, QObject *parent = nullptr);
    virtual bool isMatch(const QString &text) override;

private:
    const std::set<QString> SONG_SET = {
        "mp3",
        "au3",
        "wav",
        "ogg",
        "flac",
        "aac",
        "wma",
        "m4a",
    };
    const std::set<QString> IMG_SET = {
        "jpg",
        "jpeg",
        "png",
        "gif",
        "tiff",
        "bmp",
    };
    // TODO: 需增加文档、视频等更多格式
    const std::set<QString> *m_curSet = nullptr;
    bool m_isTrue = true;
    QString m_typeName;
};

// 对应RuleDurationSearch
class DurationCond : public BaseCond {
public:
    explicit DurationCond(const QString &text, QObject *parent = nullptr) : BaseCond(text, parent) {}
};

// 对应RuleMetaSearch
class MetaCond : public BaseCond {
public:
    explicit MetaCond(const QString &text, QObject *parent = nullptr) : BaseCond(text, parent) {}
};

// 对应RuleQuantityCondition
class QuantityCond : public BaseCond {
public:
    explicit QuantityCond(const QString &text, QObject *parent = nullptr) : BaseCond(text, parent) {}
};

// 对应RuleContentSearch
class ContentCond : public BaseCond {
public:
    explicit ContentCond(const QString &text, QObject *parent = nullptr) : BaseCond(text, parent) {}
};

// 对应RuleFilename
class FileNameCond : public BaseCond {
public:
    explicit FileNameCond(const QString &text, QObject *parent = nullptr) : BaseCond(text, parent) {}
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
    bool isMatch(const QString &text);

private:
    BaseCond m_cond;
};
}

#endif // DSLPARSER_H
