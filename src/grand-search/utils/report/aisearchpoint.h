#ifndef AISEARCH_POINT_H
#define AISEARCH_POINT_H

#include "basicpoint.h"

namespace GrandSearch {
namespace report {

class AiSearchPoint : public BasicPoint
{
public:
    explicit AiSearchPoint(bool isOn) : BasicPoint()
    {
        this->m_eventId.second = EventID::AISEARCH;
        this->m_event = "aisearch";

        QVariantMap map;
        map.insert("is_open", isOn);
        this->setAdditionalData(map);
    }
    ~AiSearchPoint() {}
};

}
}

#endif // AISEARCH_POINT_H
