// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dslparser.h"
#include <tree/ParseTreeWalker.h>
#include <global/searchhelper.h>

#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;
using namespace antlr4;

bool BaseCond::kIsNeedAdjustBase = false;

BaseCond::BaseCond(const QString &text, QObject *parent)
    : QObject(parent)
{
    setCond(text);
}

BaseCond::~BaseCond()
{
    for (BaseCond *cond : m_andCondList) {
        delete cond;
    }

    for (BaseCond *cond : m_orCondList) {
        delete cond;
    }
}

void BaseCond::addAndCond(BaseCond *cond)
{
    // qDebug() << QString("BaseCond type(%1) add AND(%2)").arg(m_condType).arg(cond->getCondType());
    m_andCondList.append(cond);
    cond->setParent(this);
}

void BaseCond::addOrCond(BaseCond *cond)
{
    // qDebug() << QString("BaseCond type(%1) add OR(%2)").arg(m_condType).arg(cond->getCondType());
    m_orCondList.append(cond);
    cond->setParent(this);
}

void BaseCond::adjust()
{
    // 消除冗余的Binary层级
    this->mergeBinary();
    // 消除冗余的Base层级，因为Base层级可以嵌套，所以可能要多次调用（如果存在层级变化，使用kIsNeedAdjustBase进行标记，直到不需变化）
    do {
        BaseCond::kIsNeedAdjustBase = false;
        this->mergeBase();
    } while (BaseCond::kIsNeedAdjustBase);
    qCInfo(logDaemon) << "Condition tree after merge:\n"
                      << this->toString();

    this->merge4Engine();
    this->adjust4OrCond();
}

void BaseCond::mergeBinary()
{
    // 合并DateInfo类型，原理：一般都是成对出现的
    if (m_condType == querylangParser::RuleBinaryExpression && m_andCondList.size() >= 2
        && m_andCondList[0]->getCondType() == querylangParser::RuleDateSearchinfo) {
        bool isAllDate = true;
        for (int i = 0; i < m_andCondList.size(); i++) {
            if (m_andCondList[i]->getCondType() != querylangParser::RuleDateSearchinfo) {
                isAllDate = false;
                break;
            }
        }

        if (isAllDate) {
            DateInfoCond *allCond = new DateInfoCond("");
            allCond->m_compType = "==";
            for (int i = m_andCondList.size() - 1; i >= 0; i--) {
                DateInfoCond *cond = dynamic_cast<DateInfoCond *>(m_andCondList[i]);
                if (cond->m_compType.contains(">")) {
                    allCond->m_timestamp = cond->m_timestamp;
                    allCond->m_cond += cond->m_cond + "  ";
                } else {
                    allCond->m_timestamp2 = cond->m_timestamp;
                    allCond->m_cond += cond->m_cond + "  ";
                }
                delete m_andCondList[i];
                m_andCondList.removeAt(i);
            }
            m_andCondList.append(allCond);
            allCond->setParent(this);
        }
    }

    if (m_condType == querylangParser::RulePrimary || m_condType == querylangParser::RuleBinaryExpression) {
        if (!m_andCondList.isEmpty()) {
            for (int i = m_andCondList.size() - 1; i >= 0; i--) {
                m_andCondList[i]->mergeBinary();
            }
        } else {
            for (int i = m_orCondList.size() - 1; i >= 0; i--) {
                m_orCondList[i]->mergeBinary();
            }
        }
    }

    // 调整的策略是：如果parent只有一个child，就把child给parent的parent，减少一级冗余
    if (m_condType != querylangParser::RuleBinaryExpression && getParent() && getParent()->getCondType() == querylangParser::RuleBinaryExpression) {
        BaseCond *parent = getParent();
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

void BaseCond::mergeBase()
{
    // 合并DateInfo类型，原理：一般都是成对出现的
    if (m_condType == querylangParser::RulePrimary && m_andCondList.size() >= 2
        && m_andCondList[0]->getCondType() == querylangParser::RuleDateSearchinfo) {
        bool isAllDate = true;
        for (int i = 0; i < m_andCondList.size(); i++) {
            if (m_andCondList[i]->getCondType() != querylangParser::RuleDateSearchinfo) {
                isAllDate = false;
                break;
            }
        }

        if (isAllDate) {
            DateInfoCond *allCond = new DateInfoCond("");
            allCond->m_compType = "==";
            for (int i = m_andCondList.size() - 1; i >= 0; i--) {
                DateInfoCond *cond = dynamic_cast<DateInfoCond *>(m_andCondList[i]);
                if (cond->m_compType.contains(">")) {
                    allCond->m_timestamp = cond->m_timestamp;
                    allCond->m_cond += cond->m_cond + "  ";
                } else {
                    allCond->m_timestamp2 = cond->m_timestamp;
                    allCond->m_cond += cond->m_cond + "  ";
                }
                delete m_andCondList[i];
                m_andCondList.removeAt(i);
            }
            m_andCondList.append(allCond);
            allCond->setParent(this);
        }
    }

    if (m_condType == querylangParser::RulePrimary) {
        if (!m_andCondList.isEmpty()) {
            for (int i = m_andCondList.size() - 1; i >= 0; i--) {
                m_andCondList[i]->mergeBase();
            }
        } else {
            for (int i = m_orCondList.size() - 1; i >= 0; i--) {
                m_orCondList[i]->mergeBase();
            }
        }
    }

    // 调整的策略是：如果parent只有一个child，就把child给parent的parent，减少一级冗余
    if (m_condType != querylangParser::RulePrimary && getParent() && getParent()->getCondType() == querylangParser::RulePrimary
        && getParent()->getParent()) {
        BaseCond *parent = getParent();
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
        BaseCond::kIsNeedAdjustBase = true;
    }
}

void BaseCond::merge4Engine()
{
    if (m_condType == querylangParser::RulePrimary || m_condType == querylangParser::RuleBinaryExpression) {
        if (m_andCondList.isEmpty()) {
            for (int i = 0; i < m_orCondList.size(); i++) {
                m_orCondList[i]->merge4Engine();
            }
            return;
        }

        EngineCond *cond = new EngineCond(&m_andCondList);
        if (cond->isValid()) {
            m_andCondList.append(cond);
            cond->setParent(this);
        } else {
            delete cond;
        }
    }
}

void BaseCond::adjust4OrCond()
{
    if (m_andCondList.size() == 2 && m_andCondList[0]->getCondType() == querylangParser::RulePrimary
        && !m_andCondList[0]->m_orCondList.isEmpty()
        && m_andCondList[1]->getCondType() == BinaryCond::RuleEngine) {
        bool isAllSimpleCond = true;
        for (int i = 0; i < m_andCondList[0]->m_orCondList.size(); i++) {
            BaseCond *cond = m_andCondList[0]->m_orCondList[i];
            if (cond->getCondType() != querylangParser::RuleTypeSearch) {
                isAllSimpleCond = false;
                break;
            }
        }

        if (isAllSimpleCond) {
            // 吸收 Type 类型条件
            for (int i = 0; i < m_andCondList[0]->m_orCondList.size(); i++) {
                BaseCond *cond = m_andCondList[0]->m_orCondList[i];
                EngineCond *engineCond = dynamic_cast<EngineCond *>(m_andCondList[1]);
                if (!engineCond->absorbCond(cond)) {
                    return;
                }
            }

            delete m_andCondList[1];
            m_andCondList.removeAt(1);
            return;
        }
    }

    if (m_andCondList.size() == 1 && m_andCondList[0]->getCondType() == querylangParser::RulePrimary) {
        m_andCondList[0]->adjust4OrCond();
    }
}

QList<SemanticEntity> BaseCond::entityList()
{
    QList<SemanticEntity> list;
    if (m_condType == querylangParser::RulePrimary || m_condType == querylangParser::RuleBinaryExpression) {
        if (!m_andCondList.isEmpty()) {
            list.append(m_andCondList[0]->entityList());
            return list;
        } else {
            for (int i = 0; i < m_orCondList.size(); i++) {
                list.append(m_orCondList[i]->entityList());
            }
            return list;
        }
    }

    list.append(m_entity);
    return list;
}

QString BaseCond::toString(int spaceCounts)
{
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

DateInfoCond::DateInfoCond(const QString &text, QObject *parent)
    : BaseCond(text, parent)
{
    m_condType = querylangParser::RuleDateSearchinfo;

    const QDateTime curDT = QDateTime::currentDateTime();
    const QDate curDate = curDT.date();

    m_timestamp = 0;
    int pos = m_cond.indexOf("\"") + 1;
    if (pos == 0) {
        m_compType = ">";
        qCDebug(logDaemon) << "No date range specified, using default comparison type:" << m_compType;
        return;
    }

    QString temp = m_cond.mid(pos, m_cond.length() - pos - 1);
    if (m_cond.contains(">")) {
        m_compType = ">";
    } else {
        m_compType = "<";
    }
    static QRegularExpression reg("([\\d\\.]+) ?year", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression reg1("([\\d\\.]+) ?month", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression reg2("([\\d\\.]+) ?week", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression reg3("([\\d\\.]+) ?day", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression reg4("([\\d\\.]+) ?hour", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression reg5("([\\d\\.]+) ?(min|minute)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match;
    if ((match = reg.match(temp)).hasMatch()) {   // 年
        int offset = match.captured(1).toInt() - 1;
        auto anchor = QDateTime(QDate(curDate.year(), 1, 1), QTime(0, 0, 0));
        m_timestamp2 = anchor.addSecs(-1).toSecsSinceEpoch();
        m_timestamp = anchor.addYears(-1 * offset).toSecsSinceEpoch();
    } else if ((match = reg1.match(temp)).hasMatch()) {   // 月
        int offset = match.captured(1).toInt() - 1;
        auto anchor = QDateTime(QDate(curDate.year(), curDate.month(), 1), QTime(0, 0, 0));
        m_timestamp2 = anchor.addSecs(-1).toSecsSinceEpoch();
        m_timestamp = anchor.addMonths(-1 * offset).toSecsSinceEpoch();
    } else if ((match = reg2.match(temp)).hasMatch()) {   // 周
        int offset = match.captured(1).toInt() - 1;
        auto anchor = QDateTime(curDate.addDays(1 - curDate.dayOfWeek()), QTime(0, 0, 0));
        m_timestamp2 = anchor.addSecs(-1).toSecsSinceEpoch();
        m_timestamp = anchor.addDays(-1 * offset * 7).toSecsSinceEpoch();
    } else if ((match = reg3.match(temp)).hasMatch()) {   // 天
        int offset = match.captured(1).toInt() - 1;
        auto anchor = QDateTime(curDate, QTime(0, 0, 0));
        m_timestamp2 = anchor.addSecs(-1).toSecsSinceEpoch();
        m_timestamp = anchor.addDays(-1 * offset).toSecsSinceEpoch();
    } else if ((match = reg4.match(temp)).hasMatch()) {   // 时
        int offset = match.captured(1).toInt();
        m_timestamp2 = curDT.toSecsSinceEpoch();
        m_timestamp = curDT.addSecs(-1 * offset * 60 * 60).toSecsSinceEpoch();
    } else if ((match = reg5.match(temp)).hasMatch()) {   // 分
        int offset = match.captured(1).toInt();
        m_timestamp2 = curDT.toSecsSinceEpoch();
        m_timestamp = curDT.addSecs(-1 * offset * 60).toSecsSinceEpoch();
    } else {
        m_timestamp = curDT.toSecsSinceEpoch();
    }
    qCDebug(logDaemon) << "DateInfoCond parsed - Text:" << m_cond
                       << "Temp:" << temp
                       << "Comparison type:" << m_compType
                       << "Timestamp:" << m_timestamp;

    if (m_compType == ">") {
        m_entity.times.append(QPair<qint64, qint64>(m_timestamp, m_timestamp2));
    } else {
        m_entity.times.append(QPair<qint64, qint64>(0, m_timestamp));
    }
}

PathCond::PathCond(const QString &text, QObject *parent)
    : BaseCond(text, parent)
{
    m_condType = querylangParser::RulePathSearch;
    if (m_cond.contains("IS NOT")) {
        m_isTruePath = false;
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
    qCDebug(logDaemon) << "PathCond parsed - Text:" << m_cond
                       << "Is true path:" << m_isTruePath
                       << "Path name:" << m_pathName;

    m_entity.partPath = m_pathName;
    m_entity.isContainPath = m_isTruePath;
}

NameCond::NameCond(const QString &text, QObject *parent)
    : BaseCond(text, parent)
{
    m_condType = querylangParser::RuleNameSearch;
    int pos = m_cond.indexOf("\"") + 1;
    m_name = m_cond.mid(pos, m_cond.length() - pos - 1);
    qDebug() << QString("NameCond(%1): m_name(%2)").arg(m_cond).arg(m_name);

    m_entity.keys.append(m_name);
}

SizeCond::SizeCond(const QString &text, QObject *parent)
    : BaseCond(text, parent)
{
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
    static QRegularExpression reg("([\\d\\.]+) ?gb", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression reg1("([\\d\\.]+) ?mb", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression reg2("([\\d\\.]+) ?kb", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression reg3("([\\d\\.]+) ?(b|byte|bytes)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match;
    if ((match = reg.match(temp)).hasMatch()) {
        m_fileSize = static_cast<qint64>(match.captured(1).toFloat() * 1024 * 1024 * 1024);
    } else if ((match = reg1.match(temp)).hasMatch()) {
        m_fileSize = static_cast<qint64>(match.captured(1).toFloat() * 1024 * 1024);
    } else if ((match = reg2.match(temp)).hasMatch()) {
        m_fileSize = static_cast<qint64>(match.captured(1).toFloat() * 1024);
    } else if ((match = reg3.match(temp)).hasMatch()) {
        m_fileSize = static_cast<qint64>(match.captured(1).toFloat());
    } else {
        m_fileSize = 0;
    }
    qDebug() << QString("SizeCond(%1): temp(%2) m_compType(%3) m_fileSize(%4)").arg(m_cond).arg(temp).arg(m_compType).arg(m_fileSize);

    m_entity.fileCompType = m_compType;
    m_entity.fileSize = m_fileSize;
}

TypeCond::TypeCond(const QString &text, QObject *parent)
    : BaseCond(text, parent)
{
    m_condType = querylangParser::RuleTypeSearch;
    if (m_cond.contains("IS NOT")) {
        m_isTrueType = false;
    }

    int pos = m_cond.indexOf("\"") + 1;
    m_typeName = m_cond.mid(pos, m_cond.length() - pos - 1);
    qDebug() << QString("TypeCond(%1): m_isTrue(%2) m_typeName(%3)").arg(m_cond).arg(m_isTrueType).arg(m_typeName);

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
    } else if (m_typeName == "folder") {
        m_entity.types.append(FOLDER_GROUP);
    } else if (m_typeName == "file") {
        m_entity.types.clear();
    } else {
        m_entity.suffix = m_typeName;
    }
    m_entity.isContainType = m_isTrueType;
}

DurationCond::DurationCond(const QString &text, QObject *parent)
    : BaseCond(text, parent)
{
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
    static QRegularExpression reg("([\\d\\.]+) ?year", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression reg1("([\\d\\.]+) ?mouth", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression reg2("([\\d\\.]+) ?week", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression reg3("([\\d\\.]+) ?day", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression reg4("([\\d\\.]+) ?hour", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression reg5("([\\d\\.]+) ?(min|minute)", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression reg6("([\\d\\.]+) ?(sec|second)", QRegularExpression::CaseInsensitiveOption);
    int pos = m_cond.indexOf("\"") + 1;
    QString temp = m_cond.mid(pos, m_cond.length() - pos - 1);
    QRegularExpressionMatch match;
    if ((match = reg.match(temp)).hasMatch()) {
        m_duration = formatTime(static_cast<qint64>(match.captured(1).toFloat() * 60 * 60 * 24 * 365) * 1000);
    } else if ((match = reg1.match(temp)).hasMatch()) {
        m_duration = formatTime(static_cast<qint64>(match.captured(1).toFloat() * 60 * 60 * 24 * 30) * 1000);
    } else if ((match = reg2.match(temp)).hasMatch()) {
        m_duration = formatTime(static_cast<qint64>(match.captured(1).toFloat() * 60 * 60 * 24 * 7) * 1000);
    } else if ((match = reg3.match(temp)).hasMatch()) {
        m_duration = formatTime(static_cast<qint64>(match.captured(1).toFloat() * 60 * 60 * 24) * 1000);
    } else if ((match = reg4.match(temp)).hasMatch()) {
        m_duration = formatTime(static_cast<qint64>(match.captured(1).toFloat() * 60 * 60) * 1000);
    } else if ((match = reg5.match(temp)).hasMatch()) {
        m_duration = formatTime(static_cast<qint64>(match.captured(1).toFloat() * 60) * 1000);
    } else {
        m_duration = formatTime(static_cast<qint64>(match.captured(1).toFloat()) * 1000);
    }
    qDebug() << QString("DurationCond(%1): temp(%2) compType(%3) duration(%4)").arg(m_cond).arg(temp).arg(m_compType).arg(m_duration);

    m_entity.types.append(AUDIO_GROUP);
    m_entity.types.append(VIDEO_GROUP);
    m_entity.durationCompType = m_compType;
    m_entity.duration = m_duration;
}

QString DurationCond::formatTime(qint64 msec)
{
    auto hours = msec / (1000 * 60 * 60);
    int minutes = (msec % (1000 * 60 * 60)) / (1000 * 60);
    int seconds = ((msec % (1000 * 60 * 60)) % (1000 * 60)) / 1000;
    QString formattedDuration = QString("%1%2%3")
                                        .arg(hours, 2, 10, QLatin1Char('0'))
                                        .arg(minutes, 2, 10, QLatin1Char('0'))
                                        .arg(seconds, 2, 10, QLatin1Char('0'));
    return formattedDuration;
}

MetaCond::MetaCond(const QString &text, QObject *parent)
    : BaseCond(text, parent)
{
    m_condType = querylangParser::RuleMetaSearch;
    // META_TYPEIS"ARTIST"ANDMETA_VALUEIS"xxx"
    // META_TYPEIS"ALBUM"ANDMETA_VALUEIS"xxx"
    // META_TYPEIS"RESOLUTION"ANDMETA_VALUEIS"4K"
    if (m_cond.contains("META_VALUEIS NOT")) {
        m_isTrue = false;
    }

    static QRegularExpression reg("META_TYPEIS\"(.+)\"ANDMETA_VALUEIS\"(.+)\"", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression reg1("META_TYPEIS\"(.+)\"ANDMETA_VALUEIS NOT\"(.+)\"", QRegularExpression::CaseInsensitiveOption);
    if (m_isTrue) {
        QRegularExpressionMatch match = reg.match(m_cond);
        if (match.hasMatch()) {
            m_metaType = match.captured(1);
            m_metaValue = match.captured(2);
        }
    } else {
        QRegularExpressionMatch match = reg1.match(m_cond);
        if (match.hasMatch()) {
            m_metaType = match.captured(1);
            m_metaValue = match.captured(2);
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
}

QuantityCond::QuantityCond(const QString &text, QObject *parent)
    : BaseCond(text, parent)
{
    m_condType = querylangParser::RuleQuantityCondition;
    int pos = m_cond.indexOf("=") + 1;
    QString temp = m_cond.mid(pos);
    m_quantity = temp.toInt();
    qDebug() << QString("QuantityCond(%1): temp(%2) m_quantity(%3)").arg(m_cond).arg(temp).arg(m_quantity);
}

ContentCond::ContentCond(const QString &text, QObject *parent)
    : BaseCond(text, parent)
{
    m_condType = querylangParser::RuleContentSearch;
    int pos = m_cond.indexOf("\"") + 1;
    m_content = m_cond.mid(pos, m_cond.length() - pos - 1);
    qDebug() << QString("ContentCond(%1): m_content(%2)").arg(m_cond).arg(m_content);

    m_entity.keys.append(m_content);
}

EngineCond::EngineCond(QList<BaseCond *> *andList, QObject *parent)
    : BaseCond("", parent)
{
    const qint64 _NOW = QDateTime::currentSecsSinceEpoch();
    m_condType = BaseCond::RuleEngine;
    m_isValid = false;

    // 支持：DateInfo Path Name Size Type 特殊1（Meta Duration） 特殊2（Content）
    // Date的条件，一般都是成对出现的
    qint64 timestamp1 = 0, timestamp2 = _NOW;
    // 使用倒序的原因：含有删除操作
    for (int i = andList->size() - 1; i >= 0; i--) {
        // Meta
        if ((*andList)[i]->getCondType() == querylangParser::RuleMetaSearch) {
            m_isValid = true;
            MetaCond *cond = dynamic_cast<MetaCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            if (cond->m_metaType == "ARTIST") {
                if (!m_entity.types.contains(AUDIO_GROUP)) {
                    m_entity.types.append(AUDIO_GROUP);
                }
                m_entity.author = cond->m_metaValue;
            } else if (cond->m_metaType == "ALBUM") {
                if (!m_entity.types.contains(AUDIO_GROUP)) {
                    m_entity.types.append(AUDIO_GROUP);
                }
                m_entity.album = cond->m_metaValue;
            } else if (cond->m_metaType == "RESOLUTION") {
                if (!m_entity.types.contains(VIDEO_GROUP)) {
                    m_entity.types.append(VIDEO_GROUP);
                }
                m_entity.resolution = cond->m_metaValue;
            }
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        // Duration
        if ((*andList)[i]->getCondType() == querylangParser::RuleDurationSearch) {
            m_isValid = true;
            DurationCond *cond = dynamic_cast<DurationCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            m_entity.durationCompType = cond->m_compType;
            m_entity.duration = cond->m_duration;
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        // Content
        if ((*andList)[i]->getCondType() == querylangParser::RuleContentSearch) {
            m_isValid = true;
            ContentCond *cond = dynamic_cast<ContentCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            m_entity.keys.append(cond->m_content);
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        // DateInfo
        if ((*andList)[i]->getCondType() == querylangParser::RuleDateSearchinfo) {
            m_isValid = true;
            DateInfoCond *cond = dynamic_cast<DateInfoCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            if (cond->m_compType == "==") {
                timestamp1 = cond->m_timestamp;
                timestamp2 = cond->m_timestamp2;
            } else if (cond->m_compType.contains(">")) {
                timestamp1 = cond->m_timestamp;
            } else {
                timestamp2 = cond->m_timestamp;
            }
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        // Path
        if ((*andList)[i]->getCondType() == querylangParser::RulePathSearch) {
            m_isValid = true;
            PathCond *cond = dynamic_cast<PathCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            m_entity.isContainPath = cond->m_isTruePath;
            m_entity.partPath = cond->m_pathName;
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        // Name
        if ((*andList)[i]->getCondType() == querylangParser::RuleNameSearch) {
            m_isValid = true;
            NameCond *cond = dynamic_cast<NameCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            m_entity.keys.append(cond->m_name);
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        // Size
        if ((*andList)[i]->getCondType() == querylangParser::RuleSizeSearch) {
            m_isValid = true;
            SizeCond *cond = dynamic_cast<SizeCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
            m_entity.fileCompType = cond->m_compType;
            m_entity.fileSize = cond->m_fileSize;
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        // Type
        if ((*andList)[i]->getCondType() == querylangParser::RuleTypeSearch) {
            m_isValid = true;
            TypeCond *cond = dynamic_cast<TypeCond *>((*andList)[i]);
            m_cond += cond->getCond() + "  ";
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
            if (cond->m_typeName == "song" || cond->m_typeName == "music") {
                m_entity.types.append(AUDIO_GROUP);
            } else if (cond->m_typeName == "paper" || cond->m_typeName == "document") {
                m_entity.types.append(DOCUMENT_GROUP);
            } else if (cond->m_typeName == "picture") {
                m_entity.types.append(PICTURE_GROUP);
            } else if (cond->m_typeName == "video") {
                m_entity.types.append(VIDEO_GROUP);
            } else if (cond->m_typeName == "folder") {
                m_entity.types.append(FOLDER_GROUP);
            } else if (cond->m_typeName == "file") {
                m_entity.types.clear();
            } else {
                m_entity.suffix = cond->m_typeName;
            }
            m_entity.isContainType = cond->m_isTrueType;
            delete (*andList)[i];
            andList->removeAt(i);
            continue;
        }

        (*andList)[i]->merge4Engine();
    }
    if (timestamp1 != 0 || timestamp2 != _NOW) {
        m_entity.times.append(QPair<qint64, qint64>(timestamp1, timestamp2));
    }
}

bool EngineCond::absorbCond(BaseCond *cond)
{
    if (cond->getCondType() == querylangParser::RuleTypeSearch) {
        cond->m_condType = BaseCond::RuleEngine;
        cond->m_cond = m_cond + cond->m_cond;
        SemanticEntity entity = m_entity;
        entity.types.append(cond->m_entity.types);
        entity.suffix = cond->m_entity.suffix;
        cond->m_entity = entity;
        return true;
    }

    return false;
}

void DslParserListener::enterEveryRule(antlr4::ParserRuleContext *ctx)
{
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
        // qDebug() << QString("%1=> enter(%2): %3").arg(m_prefix).arg(ruleContextId2String((int)ctx->getRuleIndex())).arg(text);
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
        isOr = (m_cond->getCond().contains(QString("OR") + text));
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

void DslParserListener::exitEveryRule(antlr4::ParserRuleContext *ctx)
{
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
        // qDebug() << QString("%1<= exit(%2): %3").arg(m_prefix).arg(ruleContextId2String((int)ctx->getRuleIndex())).arg(text);
        if (m_prefix.length() >= 2) {
            m_prefix = m_prefix.mid(0, m_prefix.length() - 2);
        }

        if (m_tempPtr) {
            m_tempPtr = m_tempPtr->getParent();
        }
        break;
    }
}

DslParser::DslParser(const QString &text, QObject *parent)
    : QObject(parent), m_cond("")
{
    qCDebug(logDaemon) << "Creating DslParser with text:" << text;
    ANTLRInputStream input(text.toUtf8().toStdString());
    querylangLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    tokens.fill();
    querylangParser parser(&tokens);
    DslParserListener listener(&m_cond);
    tree::ParseTreeWalker walker;
    walker.walk(&listener, parser.query());
    // qInfo() << QString("before merge:\n%1").arg(m_cond.toString()).toUtf8().toStdString().c_str();
    m_cond.adjust();
    qCInfo(logDaemon) << "Final condition tree:\n"
                      << m_cond.toString();
}
