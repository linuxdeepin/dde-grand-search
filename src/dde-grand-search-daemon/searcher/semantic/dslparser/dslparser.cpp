// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dslparser.h"
#include <tree/ParseTreeWalker.h>
#include <global/searchhelper.h>

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
    //qDebug() << QString("BaseCond type(%1) add AND(%2)").arg(m_condType).arg(cond->getCondType());
    m_andCondList.append(cond);
    cond->setParent(this);
}

void BaseCond::addOrCond(BaseCond *cond) {
    //qDebug() << QString("BaseCond type(%1) add OR(%2)").arg(m_condType).arg(cond->getCondType());
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

void BaseCond::adjust() {
    if (m_condType == querylangParser::RulePrimary || m_condType == querylangParser::RuleBinaryExpression) {
        if (!m_andCondList.isEmpty()) {
            for (int i = m_andCondList.size() - 1; i >= 0; i--) {
                m_andCondList[i]->adjust();
            }
        } else {
            for (int i = m_orCondList.size() - 1; i >= 0; i--) {
                m_orCondList[i]->adjust();
            }
        }
    }

    // 调整的策略是：如果parent只有一个child，就把child给parent的parent，减少一级冗余
    if (m_condType != querylangParser::RuleBinaryExpression && getParent() && getParent()->getCondType() == querylangParser::RuleBinaryExpression) {
        BaseCond *parent       = getParent();
        BaseCond *adviseParent = parent->getParent();
        // parent下只有一个child，才可以调整
        if (parent->m_andCondList.size() != 1 && parent->m_orCondList.size() != 1) {
            return;
        }

        int idx = adviseParent->m_andCondList.indexOf(parent);
        if (idx != -1) {
            adviseParent->m_andCondList[idx] = this;
        } else {
            idx = adviseParent->m_orCondList.indexOf(parent);
            if (idx == -1) {
                return;
            }

            adviseParent->m_orCondList[idx] = this;
        }
        setParent(adviseParent);
        parent->m_andCondList.clear();
        delete parent;
    }
}

bool BaseCond::addMatchedItems(const MatchedItems &items) {
    m_matchedItems.append(items);
}

const MatchedItems &BaseCond::getMatchedItems() {
    if (m_condType != querylangParser::RulePrimary && m_condType != querylangParser::RuleBinaryExpression) {
        return m_matchedItems;
    }

    if (!m_andCondList.isEmpty()) {
        int quantity = -1;
        bool isGetItems = false;
        for (int i = 0; i < m_andCondList.size(); i++) {
            if (m_andCondList[i]->getCondType() == querylangParser::RuleQuantityCondition) {
                quantity = dynamic_cast<QuantityCond *>(m_andCondList[i])->getQuantity();
                continue;
            }

            if (!isGetItems) {
                m_matchedItems = m_andCondList[i]->getMatchedItems();
                isGetItems = true;
                continue;
            }

            // 存在删除操作，所以采用倒序
            for (int j = m_matchedItems.size() - 1; j >= 0; j--) {
                if (m_andCondList[i]->getMatchedItems().contains(m_matchedItems[j])) {
                    continue;
                }

                m_matchedItems.removeAt(j);
            }
        }
        if (quantity > 0 && m_matchedItems.size() > quantity) {
            while (m_matchedItems.size() > quantity) {
                m_matchedItems.removeLast();
            }
        }
        return m_matchedItems;
    }

    for (int i = 0; i < m_orCondList.size(); i++) {
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
    const qint64 _NOW = QDateTime::currentSecsSinceEpoch();

    int pos = m_cond.indexOf("\"") + 1;
    if (pos == 0) {
        m_entity.times.append(QPair<qint64, qint64>(0, _NOW));
        m_anything.setEntity(m_entity);
        SemanticWorkerPrivate::QueryFunction func = {&m_anything, &AnythingQuery::run, this};
        m_querys->append(func);
        return;
    }

    QString temp = m_cond.mid(pos, m_cond.length() - pos - 1);
    if (m_cond.contains(">")) {
        m_compType = ">";
    } else {
        m_compType = "<";
    }
    static QRegExp reg("([\\d\\.]+) ?year", Qt::CaseInsensitive);
    static QRegExp reg1("([\\d\\.]+) ?mouth", Qt::CaseInsensitive);
    static QRegExp reg2("([\\d\\.]+) ?week", Qt::CaseInsensitive);
    static QRegExp reg3("([\\d\\.]+) ?day", Qt::CaseInsensitive);
    static QRegExp reg4("([\\d\\.]+) ?hour", Qt::CaseInsensitive);
    static QRegExp reg5("([\\d\\.]+) ?(min|minute)", Qt::CaseInsensitive);
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
    qDebug() << QString("DateInfoCond(%1): temp(%2) m_compType(%3) m_timestamp(%4)").arg(m_cond).arg(temp).arg(m_compType).arg(m_timestamp);

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
    if (m_pathName == "Download") {
        m_pathName = "Downloads";
    } else if (m_pathName == "Document") {
        m_pathName = "Documents";
    } else if (m_pathName == "Picture") {
        m_pathName = "Pictures";
    } else if (m_pathName == "Video") {
        m_pathName = "Videos";
    }
    qDebug() << QString("PathCond(%1): m_isTrue(%2) m_pathName(%3)").arg(m_cond).arg(m_isTrue).arg(m_pathName);

    m_entity.partPath = m_pathName;
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
    static QRegExp reg("([\\d\\.]+) ?gb", Qt::CaseInsensitive);
    static QRegExp reg1("([\\d\\.]+) ?mb", Qt::CaseInsensitive);
    static QRegExp reg2("([\\d\\.]+) ?kb", Qt::CaseInsensitive);
    static QRegExp reg3("([\\d\\.]+) ?(b|byte|bytes)", Qt::CaseInsensitive);
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

    /*
     * #define DOCUMENT_GROUP      "text"
     * #define PICTURE_GROUP       "img"
     * #define AUDIO_GROUP         "msc"
     * #define VIDEO_GROUP         "vdo"
     * #define FILE_GROUP          "file"
     * #define FOLDER_GROUP        "fld"
     * #define APPLICATION_GROUP   "app"
     */
    // 优先匹配为类别，其实是扩展名
    if (m_typeName == "song" || m_typeName == "music") {
        m_entity.types.append(AUDIO_GROUP);
    } else if (m_typeName == "paper" || m_typeName == "document") {
        m_entity.types.append(DOCUMENT_GROUP);
    } else if (m_typeName == "picture") {
        m_entity.types.append(PICTURE_GROUP);
    } else if (m_typeName == "video") {
        m_entity.types.append(VIDEO_GROUP);
    } else if (m_typeName == "file") {
        m_entity.types.clear();
    } else {
        m_entity.suffix = m_typeName;
    }
    m_entity.isTrue = m_isTrue;
    m_anything.setEntity(m_entity);
    SemanticWorkerPrivate::QueryFunction func = {&m_anything, &AnythingQuery::run, this};
    m_querys->append(func);
}

DurationCond::DurationCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                          SemanticWorkerPrivate *worker, QObject *parent) : BaseCond(text, querys, worker, parent) {
    m_condType = querylangParser::RuleDurationSearch;
    // DURATION>"3 minute"
    if (m_cond.contains(">")) {
        m_compType = ">";
    } else if (m_cond.contains("<")) {
        m_compType = "<";
    } else if (m_cond.contains("!=")) {
        m_compType = "!=";
    } else {
        m_compType = "=";
    }
    static QRegExp reg("([\\d\\.]+) ?year", Qt::CaseInsensitive);
    static QRegExp reg1("([\\d\\.]+) ?mouth", Qt::CaseInsensitive);
    static QRegExp reg2("([\\d\\.]+) ?week", Qt::CaseInsensitive);
    static QRegExp reg3("([\\d\\.]+) ?day", Qt::CaseInsensitive);
    static QRegExp reg4("([\\d\\.]+) ?hour", Qt::CaseInsensitive);
    static QRegExp reg5("([\\d\\.]+) ?(min|minute)", Qt::CaseInsensitive);
    static QRegExp reg6("([\\d\\.]+) ?(sec|second)", Qt::CaseInsensitive);
    int pos = m_cond.indexOf("\"") + 1;
    QString temp = m_cond.mid(pos, m_cond.length() - pos - 1);
    if (reg.indexIn(temp) != -1) {
        m_duration = formatTime(static_cast<qint64>(reg.cap(1).toFloat() * 60 * 60 * 24 * 365) * 1000);
    } else if (reg1.indexIn(temp) != -1) {
        m_duration = formatTime(static_cast<qint64>(reg1.cap(1).toFloat() * 60 * 60 * 24 * 30) * 1000);
    } else if (reg2.indexIn(temp) != -1) {
        m_duration = formatTime(static_cast<qint64>(reg2.cap(1).toFloat() * 60 * 60 * 24 * 7) * 1000);
    } else if (reg3.indexIn(temp) != -1) {
        m_duration = formatTime(static_cast<qint64>(reg3.cap(1).toFloat() * 60 * 60 * 24) * 1000);
    } else if (reg4.indexIn(temp) != -1) {
        m_duration = formatTime(static_cast<qint64>(reg4.cap(1).toFloat() * 60 * 60) * 1000);
    } else if (reg5.indexIn(temp) != -1) {
        m_duration = formatTime(static_cast<qint64>(reg5.cap(1).toFloat() * 60) * 1000);
    } else {
        m_duration = formatTime(static_cast<qint64>(reg5.cap(1).toFloat()) * 1000);
    }
    qDebug() << QString("DurationCond(%1): temp(%2) compType(%3) duration(%4)").arg(m_cond).arg(temp).arg(m_compType).arg(m_duration);

    m_entity.types.append(AUDIO_GROUP);
    m_entity.types.append(VIDEO_GROUP);
    m_entity.compType = m_compType;
    m_entity.duration = m_duration;
    m_feature.setEntity(m_entity);
    SemanticWorkerPrivate::QueryFunction func = {&m_feature, &FeatureQuery::run, this};
    m_querys->append(func);
}

QString DurationCond::formatTime(qint64 msec) {
    auto hours = msec / (1000 * 60 * 60);
    int minutes = (msec % (1000 * 60 * 60)) / (1000 * 60);
    int seconds = ((msec % (1000 * 60 * 60)) % (1000 * 60)) / 1000;
    QString formattedDuration = QString("%1%2%3")
                                        .arg(hours, 2, 10, QLatin1Char('0'))
                                        .arg(minutes, 2, 10, QLatin1Char('0'))
                                        .arg(seconds, 2, 10, QLatin1Char('0'));
    return formattedDuration;
}

MetaCond::MetaCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                          SemanticWorkerPrivate *worker, QObject *parent) : BaseCond(text, querys, worker, parent) {
    m_condType = querylangParser::RuleMetaSearch;
    // META_TYPEIS"ARTIST"ANDMETA_VALUEIS"xxx"
    // META_TYPEIS"ALBUM"ANDMETA_VALUEIS"xxx"
    // META_TYPEIS"RESOLUTION"ANDMETA_VALUEIS"4K"
    if (m_cond.contains("META_VALUEIS NOT")) {
        m_isTrue = false;
    }

    static QRegExp reg("META_TYPEIS\"(.+)\"ANDMETA_VALUEIS\"(.+)\"", Qt::CaseInsensitive);
    static QRegExp reg1("META_TYPEIS\"(.+)\"ANDMETA_VALUEIS NOT\"(.+)\"", Qt::CaseInsensitive);
    if (m_isTrue) {
        if (reg.indexIn(m_cond) != -1) {
            m_metaType  = reg.cap(1);
            m_metaValue = reg.cap(2);
        }
    } else {
        if (reg1.indexIn(m_cond) != -1) {
            m_metaType  = reg1.cap(1);
            m_metaValue = reg1.cap(2);
        }
    }
    if (m_metaType == "RESOLUTION") {
        if (m_metaValue == "720P" || m_metaValue == "720p") {
            m_metaValue = "1280*720";
        } else if (m_metaValue == "1080P" || m_metaValue == "1080p") {
            m_metaValue = "1920*1080";
        } else if (m_metaValue == "4K" || m_metaValue == "4k") {
            m_metaValue = "3840*2160";
        } else if (m_metaValue == "8K" || m_metaValue == "8k") {
            m_metaValue = "7680*4320";
        }
    }
    qDebug() << QString("MetaCond(%1): isTrue(%2) metaType(%3) metaValue(%4)").arg(m_cond).arg(m_isTrue).arg(m_metaType).arg(m_metaValue);
    if (m_metaType.isEmpty() || m_metaValue.isEmpty()) {
        return;
    }

    if (m_metaType == "ARTIST") {
        m_entity.types.append(AUDIO_GROUP);
        m_entity.author = m_metaValue;
    } else if (m_metaType == "ALBUM") {
        m_entity.types.append(AUDIO_GROUP);
        m_entity.album = m_metaValue;
    } else if (m_metaType == "RESOLUTION") {
        m_entity.types.append(VIDEO_GROUP);
        m_entity.resolution = m_metaValue;
    }
    m_feature.setEntity(m_entity);
    SemanticWorkerPrivate::QueryFunction func = {&m_feature, &FeatureQuery::run, this};
    m_querys->append(func);
}

QuantityCond::QuantityCond(const QString &text, QList<SemanticWorkerPrivate::QueryFunction> *querys,
                          SemanticWorkerPrivate *worker, QObject *parent) : BaseCond(text, querys, worker, parent) {
    m_condType = querylangParser::RuleQuantityCondition;
    int pos = m_cond.indexOf("=") + 1;
    QString temp = m_cond.mid(pos);
    m_quantity = temp.toInt();
    qDebug() << QString("QuantityCond(%1): temp(%2) m_quantity(%3)").arg(m_cond).arg(temp).arg(m_quantity);
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
        //qDebug() << QString("%1=> enter(%2): %3").arg(m_prefix).arg(ruleContextId2String((int)ctx->getRuleIndex())).arg(text);
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
        //qDebug() << QString("%1<= exit(%2): %3").arg(m_prefix).arg(ruleContextId2String((int)ctx->getRuleIndex())).arg(text);
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
    //qInfo() << QString("BaseCond:\n%1").arg(m_cond.toString()).toUtf8().toStdString().c_str();
    m_cond.adjust();
    qInfo() << QString("BaseCond:\n%1").arg(m_cond.toString()).toUtf8().toStdString().c_str();
}

bool DslParser::isMatch(const QString &text) {
    qInfo() << QString("DslParser isMatch(%1)").arg(text);
    bool ret = m_cond.isMatch(text);
    qInfo() << QString("DslParser isMatch(%1) ret(%2)").arg(text).arg(ret);
    return ret;
}
