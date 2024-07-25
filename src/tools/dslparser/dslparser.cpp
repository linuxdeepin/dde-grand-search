// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dslparser.h>
#include <tree/ParseTreeWalker.h>

#include <QDebug>

using namespace GrandSearch;
using namespace antlr4;

BaseCond::~BaseCond() {
    for (BaseCond *cond : m_andCondVec) {
        delete cond;
    }

    for (BaseCond *cond : m_orCondVec) {
        delete cond;
    }
}

void BaseCond::addAndCond(BaseCond *cond) {
    qDebug() << QString("BaseCond type(%1) add AND(%2)").arg(m_condType).arg(cond->getCondType());
    m_andCondVec.emplace_back(cond);
    cond->setParent(this);
}

void BaseCond::addOrCond(BaseCond *cond) {
    qDebug() << QString("BaseCond type(%1) add OR(%2)").arg(m_condType).arg(cond->getCondType());
    m_orCondVec.emplace_back(cond);
    cond->setParent(this);
}

bool BaseCond::isMatch(const QString &text) {
    bool ret = true;
    qDebug() << QString("BaseCond type(%1) isMatch(%2) m_cond(%3)").arg(m_condType).arg(text).arg(m_cond);
    if (m_andCondVec.empty() && m_orCondVec.empty()) {
        ret = false;
        goto _EXIT;
    }

    if (!m_andCondVec.empty()) {
        for (BaseCond *cond : m_andCondVec) {
            if (!cond->isMatch(text)) {
                ret = false;
                goto _EXIT;
            }
        }

        ret = true;
        goto _EXIT;
    }

    for (BaseCond *cond : m_orCondVec) {
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

QString BaseCond::toString(int spaceCounts) {
    // 控制缩进
    QString space;
    for (int i = 0; i < spaceCounts; i++) {
        space.append(" ");
    }
    spaceCounts += 2;

    QString str;
    for (BaseCond *cond : m_andCondVec) {
        str.append(QString("%1AND: %2(%3)\n").arg(space).arg(cond->getCondType()).arg(cond->getCond()));
        str.append(cond->toString(spaceCounts));
    }
    for (BaseCond *cond : m_orCondVec) {
        str.append(QString("%1OR: %2(%3)\n").arg(space).arg(cond->getCondType()).arg(cond->getCond()));
        str.append(cond->toString(spaceCounts));
    }
    return str;
}

PathCond::PathCond(const QString &text, QObject *parent) : BaseCond(text, parent) {
    m_condType = querylangParser::RulePathSearch;
    if (m_cond.contains("IS NOT")) {
        m_isTrue = false;
    }

    int pos = m_cond.indexOf("\"") + 1;
    m_pathName = m_cond.mid(pos, m_cond.length() - pos - 1);
    qDebug() << QString("PathCond(%1): m_isTrue(%2) m_pathName(%3)").arg(m_cond).arg(m_isTrue).arg(m_pathName);
}

bool PathCond::isMatch(const QString &text) {
    bool ret = true;
    if (text.contains(m_pathName)) {
        ret = true;
    } else {
        ret = false;
    }
    if (!m_isTrue) {
        ret = !ret;
    }
    qInfo() << QString("PathCond isMatch(%1) ret(%2) m_cond(%3)").arg(text).arg(ret).arg(m_cond);
    return ret;
}

TypeCond::TypeCond(const QString &text, QObject *parent) : BaseCond(text, parent) {
    m_condType = querylangParser::RuleTypeSearch;
    if (m_cond.contains("IS NOT")) {
        m_isTrue = false;
    }

    int pos = m_cond.indexOf("\"") + 1;
    m_typeName = m_cond.mid(pos, m_cond.length() - pos - 1);
    if (m_typeName == "歌曲" || m_typeName == "戏曲" || m_typeName == "song" || m_typeName == "music") {
        m_curSet = &SONG_SET;
    } else if (m_typeName == "图片" || m_typeName == "照片" || m_typeName == "相片") {
        m_curSet = &IMG_SET;
    } else {
        qWarning() << QString("TypeCond(%1) INVALID").arg(m_cond);
    }
    qDebug() << QString("TypeCond(%1): m_isTrue(%2) m_typeName(%3)").arg(m_cond).arg(m_isTrue).arg(m_typeName);
}

bool TypeCond::isMatch(const QString &text) {
    bool ret = false;
    int pos = text.lastIndexOf(".");
    if (pos == -1) {
        ret = false;
    } else if (m_curSet) {
        QString suffix = text.mid(pos + 1);
        if (m_curSet->count(suffix) > 0) {
            ret = true;
        } else {
            ret = false;
        }
    }
    if (!m_isTrue) {
        ret = !ret;
    }
    qInfo() << QString("TypeCond isMatch(%1) ret(%2) m_cond(%3)").arg(text).arg(ret).arg(m_cond);
    return ret;
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
        ptr = new BaseCond(text);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleBinaryExpression:
        ptr = new BinaryCond(text);
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
        ptr = new DateInfoCond(text);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RulePathSearch:
        ptr = new PathCond(text);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleNameSearch:
        ptr = new NameCond(text);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleSizeSearch:
        ptr = new SizeCond(text);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleTypeSearch:
        ptr = new TypeCond(text);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleDurationSearch:
        ptr = new DurationCond(text);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleMetaSearch:
        ptr = new MetaCond(text);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleQuantityCondition:
        ptr = new QuantityCond(text);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleContentSearch:
        ptr = new ContentCond(text);
        m_tempPtr->addAndCond(ptr);
        m_tempPtr = ptr;
        break;
    case querylangParser::RuleFilename:
        ptr = new FileNameCond(text);
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

DslParser::DslParser(const QString &text, QObject *parent) : QObject(parent) {
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
