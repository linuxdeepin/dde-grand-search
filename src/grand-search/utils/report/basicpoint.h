#ifndef BASICPOINT_H
#define BASICPOINT_H

#include <QPair>
#include <QVariantMap>

namespace GrandSearch {
namespace report {

enum EventID {
    // Each event has its unique id defined by data platform except Default id.
    Default              = 0,  // default event id, which is only used to identify base data
    AISEARCH             = 1001700001,
};

class BasicPoint
{
public:
    explicit BasicPoint();
    virtual ~BasicPoint();

    virtual QVariantMap assemblingData() const;
    virtual void setAdditionalData(const QVariantMap &data);

protected:
    QPair<QString, EventID> m_eventId;
    QVariantMap m_additionalData;
    QString m_event;
};

}
}

#endif // BASICPOINT_H
