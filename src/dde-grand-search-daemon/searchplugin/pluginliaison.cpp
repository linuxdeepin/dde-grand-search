// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginliaison.h"
#include "pluginliaison_p.h"
#include "convertors/dataconvertor.h"

#include <QDBusPendingCallWatcher>
#include <QtConcurrent>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;

PluginLiaisonPrivate::PluginLiaisonPrivate(PluginLiaison *parent)
    : QObject(parent),
      q(parent)
{
    QDBusConnection::sessionBus().connect("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus",
                                          "NameOwnerChanged", this, SLOT(onServiceStarted(QString, QString, QString)));
}

PluginLiaisonPrivate::~PluginLiaisonPrivate()
{
    qCDebug(logDaemon) << "Waiting for parse thread to exit";
    m_parseThread.waitForFinished();
    if (m_replyWatcher) {
        delete m_replyWatcher;
        m_replyWatcher = nullptr;
    }
    qCDebug(logDaemon) << "Parse thread exited successfully";
}

bool PluginLiaisonPrivate::isVaild() const
{
    if (!m_inteface)
        return false;

    if (m_inteface->isValid()) {
        // init() 已经做过检查，这处只判空即可
        return !m_ver.isEmpty();
    }

    return false;
}

void PluginLiaisonPrivate::parseResult(const QString &json, PluginLiaisonPrivate *d)
{
    QVariantList ret;

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(logDaemon) << "Invalid JSON data in search results - Error:" << error.errorString();
        emit d->q->searchFinished({});
        return;
    }

    // 中断
    if (!d->m_searching.loadAcquire())
        return;

    // 输入
    QVariantList in;
    QJsonObject root = doc.object();
    {
        in << d->m_pluginName;
        QVariant var;
        var.setValue(static_cast<void *>(&root));
        in << var;
    }

    // 解析数据耗时
    DataConvIns->convert(d->m_ver, PLUGININTERFACE_TYPE_RESULT, &in, &ret);
    qCDebug(logDaemon) << "Data conversion completed - Input size:" << json.size()
                       << "Output size:" << ret.size();

    if (ret.size() == 2) {
        auto id = ret.first().toString();
        if (!id.isEmpty()) {
            MatchedItemMap items = ret.at(1).value<MatchedItemMap>();
            // 修改状态，发送结果
            if (d->m_searching.testAndSetRelease(true, false))
                emit d->q->searchFinished(items);

            return;
        }
    }

    qCWarning(logDaemon) << "Invalid result format from plugin:" << d->m_pluginName;
    emit d->q->searchFinished({});
}

void PluginLiaisonPrivate::onSearchReplied()
{
    //! 应该在主线程执行
    Q_ASSERT(QThread::currentThread() == qApp->thread());
    QDBusPendingCallWatcher *watcher = dynamic_cast<QDBusPendingCallWatcher *>(sender());
    if (watcher == nullptr || m_replyWatcher != watcher)
        return;

    // 中断
    if (!m_searching.loadAcquire())
        return;

    auto reply = m_replyWatcher->reply();
    if (m_replyWatcher->isError() || reply.arguments().size() < 1) {
        qCWarning(logDaemon) << "Search error from plugin:" << m_pluginName
                             << "Error:" << reply.errorMessage();

        // 发送结束
        emit q->searchFinished({});
    } else if (m_searching.loadAcquire()) {   // 需要解析
        QString ret = reply.arguments().at(0).toString();

        qCDebug(logDaemon) << "Received search reply from plugin:" << m_pluginName
                           << "Data size:" << ret.size();
        m_parseThread = QtConcurrent::run(PluginLiaisonPrivate::parseResult, ret, this);
    }
}

void PluginLiaisonPrivate::onServiceStarted(QString name, QString oldOwner, QString newOwner)
{
    Q_UNUSED(oldOwner);
    Q_UNUSED(newOwner);
    auto con = QDBusConnection::sessionBus();
    if (m_inteface) {
        if (name == m_inteface->service() && con.interface()->isServiceRegistered(name)) {
            qCDebug(logDaemon) << "Plugin service started:" << name;
            con.disconnect("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus",
                           "NameOwnerChanged", this, nullptr);
            emit q->ready();
        }
    }
}

PluginLiaison::PluginLiaison(QObject *parent)
    : QObject(parent),
      d(new PluginLiaisonPrivate(this))
{
    qCDebug(logDaemon) << "PluginLiaison constructor - Created plugin liaison";
}

bool PluginLiaison::init(const QString &service, const QString &address, const QString &interface, const QString &ver, const QString &pluginName)
{
    if (Q_UNLIKELY(address.isEmpty()
                   || service.isEmpty() || interface.isEmpty()
                   || ver.isEmpty() || pluginName.isEmpty())) {
        qCWarning(logDaemon) << "Invalid initialization parameters - Service:" << service
                             << "Address:" << address
                             << "Interface:" << interface << "Version:" << ver
                             << "Plugin:" << pluginName;
        return false;
    }

    // 检查版本
    if (Q_UNLIKELY(d->m_inteface || !DataConvIns->isSupported(ver))) {
        qCWarning(logDaemon) << "Unsupported interface version or interface already exists - Version:" << ver;
        return false;
    }

    d->m_ver = ver;
    d->m_pluginName = pluginName;

    auto stdString = interface.toStdString();
    d->m_inteface = new SearchPluginInterfaceV1(service, address, stdString.c_str(),
                                                QDBusConnection::sessionBus(), this);
    // 插件搜索超时暂定为25s
    d->m_inteface->setTimeout(25 * 1000);
    qCDebug(logDaemon) << "Initialized plugin liaison - Service:" << service
                       << "Interface:" << interface << "Version:" << ver
                       << "Plugin:" << pluginName;
    return true;
}

bool PluginLiaison::isVaild() const
{
    return d->isVaild();
}

bool PluginLiaison::search(const QString &task, const QString &context)
{
    if (Q_UNLIKELY(task.isEmpty() || context.isEmpty() || !d->m_inteface)) {
        qCWarning(logDaemon) << "Invalid search parameters - Task:" << task
                             << "Context:" << context;
        return false;
    }

    // 转换成json数据
    QStringList args { d->m_ver, task, context };
    QJsonDocument doc;
    QJsonObject root;
    if (DataConvIns->convert(d->m_ver, PLUGININTERFACE_TYPE_SEARCH, &args, &root) != 0) {
        qCWarning(logDaemon) << "Failed to convert search parameters to JSON";
        return false;
    }

    // 搜索中则直接返回
    if (!d->m_searching.testAndSetRelease(false, true)) {
        qCDebug(logDaemon) << "Search already in progress";
        return false;
    }

    doc.setObject(root);
    QString input = doc.toJson(QJsonDocument::Compact);

    if (d->m_replyWatcher) {
        delete d->m_replyWatcher;
        d->m_replyWatcher = nullptr;
    }

    // 异步调用，监听返回
    qCDebug(logDaemon) << "Initiating search on service:" << d->m_inteface->service();
    auto call = d->m_inteface->Search(input);
    d->m_replyWatcher = new QDBusPendingCallWatcher(call);

    // 不在主线程，需将watcher移到主线程
    if (QThread::currentThread() != qApp->thread()) {
        d->m_replyWatcher->moveToThread(qApp->thread());
    }

    d->m_replyWatcher->setParent(this);

    // 回到主线程响应信号
    connect(d->m_replyWatcher, &QDBusPendingCallWatcher::finished, d, &PluginLiaisonPrivate::onSearchReplied, Qt::QueuedConnection);
    return true;
}

bool PluginLiaison::stop(const QString &task)
{
    if (d->m_searching.testAndSetRelease(true, false)) {
        if (!task.isEmpty() && d->m_inteface) {

            // 转换成json数据
            QStringList args { d->m_ver, task };
            QJsonObject root;
            if (DataConvIns->convert(d->m_ver, PLUGININTERFACE_TYPE_STOP, &args, &root) != 0) {
                qCWarning(logDaemon) << "Failed to convert stop parameters to JSON";
                return false;
            }

            QJsonDocument doc;
            doc.setObject(root);
            QString input = doc.toJson(QJsonDocument::Compact);
            qCDebug(logDaemon) << "Stopping search task:" << task;
            d->m_inteface->Stop(input);
        }
    }
    return true;
}

bool PluginLiaison::action(const QString &type, const QString &item)
{
    if (Q_UNLIKELY(type.isEmpty() || item.isEmpty() || !d->m_inteface)) {
        qCWarning(logDaemon) << "Invalid action parameters - Type:" << type
                             << "Item:" << item;
        return false;
    }

    // 转换成json数据
    QStringList args { d->m_ver, type, item };
    QJsonObject root;
    if (DataConvIns->convert(d->m_ver, PLUGININTERFACE_TYPE_ACTION, &args, &root) != 0) {
        qCWarning(logDaemon) << "Failed to convert action parameters to JSON";
        return false;
    }

    QJsonDocument doc;
    doc.setObject(root);
    QString input = doc.toJson(QJsonDocument::Compact);
    qCDebug(logDaemon) << "Executing action - Type:" << type << "Item:" << item;
    d->m_inteface->Action(input);

    return true;
}
