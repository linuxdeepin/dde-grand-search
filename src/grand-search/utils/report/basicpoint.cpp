#include "basicpoint.h"

#include <QDateTime>
#include <QJsonObject>

using namespace GrandSearch;
using namespace report;

BasicPoint::BasicPoint()
    : m_eventId(QPair<QString, EventID>("tid", EventID::Default))
{

}

BasicPoint::~BasicPoint()
{

}

QVariantMap BasicPoint::assemblingData() const
{
    QVariantMap map;
    map.insert(m_eventId.first, m_eventId.second);
    map.insert("event", m_event);
    map.insert("app", "dde-grand-search");
    map.insert("app_version", APP_VERSION);
    QDateTime curTime = QDateTime::currentDateTime();
    map.insert("time", curTime.toString("yyyy-MM-dd HH:mm:ss.zzz"));
    map.insert("timestamp", QString::number(curTime.toMSecsSinceEpoch()));

    if (!m_additionalData.isEmpty()) {
        QJsonObject msgJson;
        for (auto it = m_additionalData.begin(); it != m_additionalData.end(); ++it) {
            msgJson.insert(it.key(), it.value().toJsonValue());
        }
        map.insert("message", msgJson);
    }

    return map;
}

void BasicPoint::setAdditionalData(const QVariantMap &data)
{
    m_additionalData = data;
}
