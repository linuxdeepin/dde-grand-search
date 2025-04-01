#ifndef EVENTLOGUTIL_H
#define EVENTLOGUTIL_H

#include <QVariantMap>

namespace GrandSearch {
namespace report {

class CommitLog;
class EventLogUtil : public QObject
{
    Q_OBJECT
public:
    ~EventLogUtil();

    static EventLogUtil *instance();
    void writeEvent(const QVariantMap &data);

Q_SIGNALS:
    void appendData(const QString &data);

protected:
    explicit EventLogUtil(QObject *parent = nullptr);

private:
    QJsonObject castToJson(const QVariantMap &data) const;
    void init();

private:
    bool m_isInit = false;
    CommitLog *m_commitLog = nullptr;
    QThread *m_commitThread = nullptr;
};

}
}

#define ReportIns GrandSearch::report::EventLogUtil::instance

#endif   // EVENTLOGUTIL_H
