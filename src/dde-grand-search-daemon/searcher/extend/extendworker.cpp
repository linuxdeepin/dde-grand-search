// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extendworker.h"
#include "searchplugin/pluginliaison.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;

ExtendWorker::ExtendWorker(const QString &name, QObject *parent)
    : ProxyWorker(name, parent)
{
    m_timeout.setInterval(25 * 1000);
    m_timeout.setSingleShot(true);
    connect(&m_timeout, &QTimer::timeout, this, [this]() {
        qCWarning(logDaemon) << "Worker timeout - Name:" << m_name;
        onWorkFinished({});
    });
}

ExtendWorker::~ExtendWorker()
{
}

bool ExtendWorker::setService(const QString &service, const QString &address, const QString &interface, const QString &ver)
{
    if (Q_UNLIKELY(address.isEmpty()
                   || service.isEmpty() || interface.isEmpty()
                   || ver.isEmpty())) {
        qCWarning(logDaemon) << "Invalid service configuration - Service:" << service
                             << "Address:" << address
                             << "Interface:" << interface << "Version:" << ver;
        return false;
    }

    if (m_liaison) {
        qCWarning(logDaemon) << "Service already configured for worker:" << name();
        return false;
    }

    // 创建通信模块
    auto liaison = new PluginLiaison(this);
    if (!liaison->init(service, address, interface, ver, name())) {
        qCWarning(logDaemon) << "Failed to initialize PluginLiaison - Service:" << service
                             << "Address:" << address
                             << "Interface:" << interface << "Version:" << ver;
        delete liaison;
        return false;
    }

    m_liaison = liaison;
    qCDebug(logDaemon) << "Service configured successfully for worker:" << name();

    // 插件返回数据解析完毕，使用队列切入主线程，信号会来自子线程，也会来自主线程
    connect(m_liaison, &PluginLiaison::searchFinished, this, &ExtendWorker::onWorkFinished, Qt::QueuedConnection);
    // dbus服务就绪尝试调用,在working函数中,可能服务还未启动,因此需在服务启动后再次尝试调用
    connect(m_liaison, &PluginLiaison::ready, this, &ExtendWorker::tryWorking, Qt::QueuedConnection);
    return true;
}

void ExtendWorker::setContext(const QString &context)
{
    m_context = context;
}

bool ExtendWorker::isAsync() const
{
    return true;
}

bool ExtendWorker::working(void *context)
{
    // 准备状态切运行中，否则直接返回
    if (!m_status.testAndSetRelease(Ready, Runing))
        return false;

    QString id = *static_cast<QString *>(context);
    if (!m_liaison || id.isEmpty() || m_context.isEmpty()) {
        qCWarning(logDaemon) << "Invalid working state - Worker:" << name()
                             << "Has liaison:" << (m_liaison != nullptr)
                             << "Task ID:" << id
                             << "Context empty:" << m_context.isEmpty();
        m_status.storeRelease(Completed);
        return false;
    }

    m_taskID = id;

    // 服务是否可用
    QMutexLocker lk(&m_callMtx);
    if (m_liaison->isVaild()) {
        m_callSerach = Called;
        lk.unlock();

        // 直接调用
        qCDebug(logDaemon) << "Starting search - Worker:" << name() << "Task ID:" << id;
        if (m_liaison->search(m_taskID, m_context)) {
            return true;
        } else {
            qCWarning(logDaemon) << "Search request failed - Worker:" << name() << "Task ID:" << id;
            m_status.storeRelease(Completed);
            return false;
        }
    }

    // 等待服务启动后再调用
    qCDebug(logDaemon) << "Waiting for service to start - Worker:" << name() << "Task ID:" << id;
    m_callSerach = WaitCall;

    // 主线程中启动超时
    QMetaObject::invokeMethod(&m_timeout, "start", Qt::QueuedConnection);
    return true;
}

void ExtendWorker::terminate()
{
    qCDebug(logDaemon) << "Terminating worker - Name:" << name() << "Task ID:" << m_taskID;
    m_status.storeRelease(Terminated);

    if (m_liaison)
        m_liaison->stop(m_taskID);
}

ProxyWorker::Status ExtendWorker::status()
{
    return static_cast<ProxyWorker::Status>(m_status.loadAcquire());
}

bool ExtendWorker::hasItem() const
{
    QMutexLocker lk(&m_mtx);
    return !m_items.isEmpty();
}

MatchedItemMap ExtendWorker::takeAll()
{
    QMutexLocker lk(&m_mtx);
    MatchedItemMap items = std::move(m_items);

    Q_ASSERT(m_items.isEmpty());
    lk.unlock();

    return items;
}

void ExtendWorker::tryWorking()
{
    Q_ASSERT(m_liaison);

    //! 由dbus服务启动信号触发，在主线程执行
    QMutexLocker lk(&m_callMtx);
    if (m_callSerach != WaitCall) {
        qCDebug(logDaemon) << "Service started but search already called - Worker:" << name();
        return;
    }
    m_callSerach = Called;
    m_timeout.stop();
    lk.unlock();

    qCInfo(logDaemon) << "Service started, initiating search - Worker:" << name() << "Task ID:" << m_taskID;
    if (!m_liaison->search(m_taskID, m_context)) {
        // 发起搜索失败
        qCWarning(logDaemon) << "Search request failed after service start - Worker:" << name() << "Task ID:" << m_taskID;
        m_status.storeRelease(Completed);

        // 发送结束信号
        emit asyncFinished(this);
    }
}

void ExtendWorker::onWorkFinished(const MatchedItemMap &ret)
{
    m_status.storeRelease(Completed);
    QMutexLocker lk(&m_mtx);
    m_items = ret;
    lk.unlock();

    if (!ret.isEmpty())
        emit unearthed(this);

    qCDebug(logDaemon) << "Search completed - Worker:" << name() << "Task ID:" << m_taskID
                       << "Results:" << ret.size();
    // 异步搜索结束
    emit asyncFinished(this);
}
