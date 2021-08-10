/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "pluginliaison.h"
#include "pluginliaisonprivate.h"
#include "dataconvertor.h"

#include <QDBusPendingCallWatcher>
#include <QtConcurrent>

PluginLiaisonPrivate::PluginLiaisonPrivate(PluginLiaison *parent)
    : QObject(parent)
    , q(parent)
{

}

PluginLiaisonPrivate::~PluginLiaisonPrivate()
{
    qDebug() << "parse thread: waiting exit";
    m_parseThread.waitForFinished();
    if (m_replyWatcher) {
        delete m_replyWatcher;
        m_replyWatcher = nullptr;
    }
    qDebug() << "parse thread: exited.";
}

bool PluginLiaisonPrivate::isVaild() const
{
    if (!m_inteface)
        return false;

    if (m_inteface->isValid()) {
        //init() 已经做过检查，这处只判空即可
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
        qWarning() << "search results: is not a json data.";
        emit d->q->searchFinished({});
        return;
    }

    //中断
    if (!d->m_searching.loadAcquire())
        return;

    //输入
    QVariantList in;
    QJsonObject root = doc.object();
    {
        in << d->m_pluginName;
        QVariant var;
        var.setValue(static_cast<void *>(&root));
        in << var;
    }

    //解析数据耗时
    DataConvIns->convert(d->m_ver, PLUGININTERFACE_TYPE_RESULT, &in, &ret);
    qDebug() << "convert size" << json.size() << ret.size();

    if (ret.size() == 2) {
        auto id = ret.first().toString();
        if (!id.isEmpty()) {
            GrandSearch::MatchedItemMap items = ret.at(1).value<GrandSearch::MatchedItemMap>();
            //修改状态，发送结果
            if (d->m_searching.testAndSetRelease(true, false))
                emit d->q->searchFinished(items);

            return;
        }
    }

    qWarning() << "error result from" << d->m_pluginName;
    emit d->q->searchFinished({});
}

void PluginLiaisonPrivate::onSearchReplied()
{
    //! 应该在主线程执行
    Q_ASSERT(QThread::currentThread() == qApp->thread());
    QDBusPendingCallWatcher *watcher = dynamic_cast<QDBusPendingCallWatcher *>(sender());
    if (watcher == nullptr || m_replyWatcher != watcher)
        return;

    //中断
    if (!m_searching.loadAcquire())
        return;

    auto reply = m_replyWatcher->reply();
    if (m_replyWatcher->isError() || reply.arguments().size() < 1) {
        qWarning() << m_pluginName << reply.errorMessage();

        //发送结束
        emit q->searchFinished({});
    } else if (m_searching.loadAcquire()) { //需要解析
        QString ret = reply.arguments().at(0).toString();

        qDebug() << "get reply" << m_pluginName << ret.size();
        m_parseThread = QtConcurrent::run(PluginLiaisonPrivate::parseResult, ret, this);
    }
}

PluginLiaison::PluginLiaison(QObject *parent)
    : QObject(parent)
    , d(new PluginLiaisonPrivate(this))
{

}

bool PluginLiaison::init(const QString &service, const QString &address, const QString &interface, const QString &ver, const QString &pluginName)
{
    if (Q_UNLIKELY(address.isEmpty()
            || service.isEmpty() || interface.isEmpty()
            || ver.isEmpty() || pluginName.isEmpty()))
        return false;

    //检查版本
    if (Q_UNLIKELY(d->m_inteface || !DataConvIns->isSupported(ver)))
        return false;

    d->m_ver = ver;
    d->m_pluginName = pluginName;

    auto stdStrng = interface.toStdString();
    d->m_inteface = new SearchPluginInterfaceV1(service, address, stdStrng.c_str(),
                                                QDBusConnection::sessionBus(), this);
    //插件搜索超时暂定为30s，todo后续根据实际调整
    d->m_inteface->setTimeout(30 * 1000);
    return true;
}

bool PluginLiaison::isVaild() const
{
    return d->isVaild();
}

bool PluginLiaison::search(const QString &task, const QString &context)
{
    if (Q_UNLIKELY(task.isEmpty() || context.isEmpty() || !d->m_inteface))
        return false;

    //转换成json数据
    QStringList args{d->m_ver, task, context};
    QJsonDocument doc;
    QJsonObject root;
    if (DataConvIns->convert(d->m_ver, PLUGININTERFACE_TYPE_SEARCH, &args, &root) != 0)
        return false;

    //搜索中则直接返回
    if (!d->m_searching.testAndSetRelease(false, true))
        return false;

    doc.setObject(root);
    QString input = doc.toJson(QJsonDocument::Compact);

    if (d->m_replyWatcher) {
        delete d->m_replyWatcher;
        d->m_replyWatcher = nullptr;
    }

    //异步调用，监听返回
    qDebug() << "call search" << d->m_inteface->service();
    auto call = d->m_inteface->Search(input);
    d->m_replyWatcher = new QDBusPendingCallWatcher(call);

    //不在主线程，需将watcher移到主线程
    if (QThread::currentThread() != qApp->thread()) {
        d->m_replyWatcher->moveToThread(qApp->thread());
    }

    d->m_replyWatcher->setParent(this);

    //回到主线程响应信号
    connect(d->m_replyWatcher, &QDBusPendingCallWatcher::finished, d, &PluginLiaisonPrivate::onSearchReplied, Qt::QueuedConnection);
    return true;
}

bool PluginLiaison::stop(const QString &task)
{
    if (d->m_searching.testAndSetRelease(true, false)) {
        if (!task.isEmpty() && d->m_inteface) {

            //转换成json数据
            QStringList args{d->m_ver, task};
            QJsonObject root;
            if (DataConvIns->convert(d->m_ver, PLUGININTERFACE_TYPE_STOP, &args, &root) != 0)
                return false;

            QJsonDocument doc;
            doc.setObject(root);
            QString input = doc.toJson(QJsonDocument::Compact);
            d->m_inteface->Stop(input);
        }
    }
    return true;
}

bool PluginLiaison::action(const QString &type, const QString &item)
{
    if (Q_UNLIKELY(type.isEmpty() || item.isEmpty() || !d->m_inteface))
        return false;

    //转换成json数据
    QStringList args{d->m_ver, type, item};
    QJsonObject root;
    if (DataConvIns->convert(d->m_ver, PLUGININTERFACE_TYPE_ACTION, &args, &root) != 0)
        return false;

    QJsonDocument doc;
    doc.setObject(root);
    QString input = doc.toJson(QJsonDocument::Compact);
    d->m_inteface->Action(input);

    return true;
}

