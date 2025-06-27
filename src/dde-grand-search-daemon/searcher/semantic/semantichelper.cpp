// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "semantichelper.h"
#include "global/searchhelper.h"

#include <QMap>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDateTime>
#include <QLoggingCategory>

#include <unistd.h>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)
using namespace GrandSearch;

SemanticHelper::SemanticHelper()
{
    qCDebug(logDaemon) << "SemanticHelper constructor - Created semantic helper";
}

bool SemanticHelper::entityFromJson(const QString &json, SemanticEntity &out)
{
    qCDebug(logDaemon) << "Parsing entity from JSON";
    QJsonParseError er;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &er);
    if (er.error != QJsonParseError::NoError) {
        qCWarning(logDaemon) << "Failed to parse JSON - Error:" << er.errorString();
        return false;
    }

    auto root = doc.array();
    if (root.isEmpty()) {
        qCWarning(logDaemon) << "Empty JSON array";
        return false;
    }

    static QMap<QString, QString> typeTable = {
        { "text", DOCUMENT_GROUP },
        { "picture", PICTURE_GROUP },
        { "music", AUDIO_GROUP },
        { "movie", VIDEO_GROUP },
        { "file", FILE_GROUP },
        { "folder", FOLDER_GROUP },
        { "appliaction", APPLICATION_GROUP },
    };

    out = {};

    // 只取第一个
    const QJsonObject &obj = root.first().toObject();
    QSet<QString> typeOfKey;
    auto typeObj = obj.value("type").toArray();
    for (auto ro = typeObj.begin(); ro != typeObj.end(); ++ro) {
        if (ro->isObject()) {
            auto type = ro->toObject().value("type");
            auto word = ro->toObject().value("word");
            if (type.isString()) {
                QString text = typeTable.value(type.toString().trimmed());
                if (!text.isEmpty() && !out.types.contains(text)) {
                    out.types.append(text);
                    if (word.isString()) {
                        text = word.toString().trimmed();
                        if (!text.isEmpty())
                            typeOfKey.insert(text);
                    }
                }
            }
        }
    }

    auto entityObj = obj.value("entity").toArray();
    for (auto ro = entityObj.begin(); ro != entityObj.end(); ++ro) {
        if (ro->isString()) {
            QString key = ro->toString().trimmed();
            if (!key.isEmpty()) {
                // 去除模型概率性带出的[SEP]...后缀
                int pos = key.indexOf("[");
                if (pos != -1) {
                    key = key.mid(0, pos);
                }
                out.keys.append(key);
            }
        }
    }

    QSet<QString> timeOfKey;
    auto timeObj = obj.value("time").toArray();
    for (auto ro = timeObj.begin(); ro != timeObj.end(); ++ro) {
        if (ro->isObject()) {
            auto detail = ro->toObject().value("detail");
            if (detail.isObject()) {
                auto time = detail.toObject().value("time");
                if (time.isArray()) {
                    auto timeArray = time.toArray();
                    if (timeArray.size() == 2) {
                        QString t1 = timeArray.first().toString().trimmed();
                        QString t2 = timeArray.last().toString().trimmed();
                        QPair<qint64, qint64> value;
                        value.first = QDateTime::fromString(t1, "yyyy-MM-dd hh:mm:ss").toSecsSinceEpoch();
                        value.second = QDateTime::fromString(t2, "yyyy-MM-dd hh:mm:ss").toSecsSinceEpoch();
                        if (value.first > 0 || value.second > 0)
                            out.times.append(value);
                    }
                }
                auto text = ro->toObject().value("text");
                if (text.isString()) {
                    QString txt = text.toString().trimmed();
                    if (!txt.isEmpty())
                        timeOfKey.insert(txt);
                }
            }
        }
    }

    // remove text of type
    for (const QString &txt : typeOfKey)
        out.keys.removeOne(txt);

    // remove text of time;
    for (const QString &txt : timeOfKey)
        out.keys.removeOne(txt);

    qCInfo(logDaemon) << "Entity parsed successfully - Types:" << out.types.join(", ")
                      << "Keys:" << out.keys.join(", ");

    return true;
}

QString SemanticHelper::vectorServiceName()
{
    return "org.deepin.ai.daemon.VectorIndex";
}

QString SemanticHelper::querylangServiceName()
{
    return "org.deepin.ai.daemon.QueryLang";
}

bool SemanticHelper::isMatchTime(qint64 time, const QList<QPair<qint64, qint64>> &intervals)
{
    bool match = intervals.isEmpty();
    for (const QPair<qint64, qint64> &interval : intervals) {
        if (interval.first > 0 && time < interval.first)
            continue;
        if (interval.second > 0 && time > interval.second)
            continue;

        match = true;
        break;
    }

    qCDebug(logDaemon) << "Time match check - Time:" << time
                       << "Match:" << match
                       << "Intervals:" << intervals.size();
    return match;
}

QStringList SemanticHelper::typeTosuffix(const QStringList &type)
{
    QStringList ret;
    for (const QString &t : type) {
        auto s = SearchHelper::instance()->getSuffixByGroupName(t);
        if (!s.isEmpty())
            ret.append(s);
    }

    qCDebug(logDaemon) << "Converting types to suffixes - Types:" << type.join(", ")
                       << "Suffixes:" << ret.join(", ");
    return ret;
}

QString SemanticEntity::toString() const
{
    QString content;
    if (!types.isEmpty())
        content.append(" Types:").append(types.join(","));

    if (!keys.isEmpty())
        content.append(" Keys:").append(keys.join(","));

    if (!author.isEmpty())
        content.append(" Author:").append(author);

    if (!album.isEmpty())
        content.append(" Album:").append(album);

    if (!durationCompType.isEmpty())
        content.append("; DurationCompType:").append(durationCompType);

    if (!duration.isEmpty())
        content.append(" Duration:").append(duration);

    if (!resolution.isEmpty())
        content.append(" Resolution:").append(resolution);

    if (!partPath.isEmpty())
        content.append(" partPath:").append(partPath);

    if (!suffix.isEmpty())
        content.append(" suffix:").append(suffix);

    content.append(" IsContainPath:").append(QString::number(isContainPath ? 1 : 0));
    content.append(" IsContainType:").append(QString::number(isContainType ? 1 : 0));

    if (!fileCompType.isEmpty())
        content.append(" FileCompType:").append(fileCompType);

    if (fileSize != 0)
        content.append(" FileSize:").append(QString::number(fileSize));

    if (!times.isEmpty()) {
        content.append(" Times:");
        for (const QPair<qint64, qint64> &t : times)
            content.append(QString("(%0,%1)").arg(t.first).arg(t.second));
    }

    return content;
}
