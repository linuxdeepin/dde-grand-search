// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FEATURELIBENGINE_H
#define FEATURELIBENGINE_H

#include <QObject>
#include <QVariant>

namespace GrandSearch {

typedef bool (*CheckAndPushItem)(const QString &file, void *pdata);
class FeatureLibEnginePrivate;
class FeatureLibEngine : public QObject
{
    Q_OBJECT
    friend class FeatureLibEnginePrivate;
public:
    enum Property {
        And,
        Or,
        Composite,
        Path,
        FileType,
        CreatedTime,
        ModifiedTime,
        ReadTime,
        Album,
        Author,
        Duration,
        Resolution,
        Text
    };
    typedef QPair<Property, QVariant> QueryProperty;
    typedef QList<QueryProperty> QueryConditons;
    static inline QueryProperty makeProperty(Property prop, const QVariant &var = QVariant()) {
        return qMakePair(prop, var);
    }
public:
    explicit FeatureLibEngine(QObject *parent = nullptr);
    ~FeatureLibEngine();
    bool init(const QString &cache);
    void query(const QString &searchPath, const QueryConditons &cond, CheckAndPushItem func, void *pdata);
private:
    FeatureLibEnginePrivate *d;
};

}
#endif // FEATURELIBENGINE_H
