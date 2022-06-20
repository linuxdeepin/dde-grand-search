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
#include "statictextworker.h"
#include "global/builtinsearch.h"
#include "global/searchconfigdefine.h"
#include "configuration/configer.h"
#include "global/searchhelper.h"

StaticTextWorker::StaticTextWorker(const QString &name, QObject *parent) : ProxyWorker(name, parent)
{

}

void StaticTextWorker::setContext(const QString &context)
{
    if (context.isEmpty())
        qWarning() << "search key is empty.";
    m_context = context;
}

bool StaticTextWorker::isAsync() const
{
    //同步搜索，需分配线程资源
    return false;
}

bool StaticTextWorker::working(void *context)
{
    //准备状态切运行中，否则直接返回
    if (!m_status.testAndSetRelease(Ready, Runing))
        return false;

    Q_UNUSED(context);
    if (m_context.isEmpty()) {
        m_status.storeRelease(Completed);
        return true;
    }

    auto config = Configer::instance()->group(GRANDSEARCH_WEB_GROUP);
    auto searchEngine = config->value(GRANDSEARCH_WEB_SEARCHENGINE, QString(""));
    QString defaultUrl = createUrl(searchEngine);

    GrandSearch::MatchedItem ret;

    QString encodeString(QUrl::toPercentEncoding(m_context));
    QString url = defaultUrl.arg(QString(encodeString));

    ret.item = url;
    ret.name = tr("Search for \"%1\"").arg(m_context);
    ret.type = "x-scheme-handler/http";
    ret.icon = ret.type; //给一个无效的图标名，由前端寻找默认浏览器图标
    ret.searcher = name();

    {
        QMutexLocker lk(&m_mtx);
        m_items.append(ret);
    }

    m_status.storeRelease(Completed);

    qDebug() << "echo text...";
    //不能使用QTimer延迟发送，会出现任务结束后再发信号，此时已经无效。
    QThread::msleep(100);
    emit unearthed(this);

    return true;
}

void StaticTextWorker::terminate()
{
    m_status.storeRelease(Terminated);
}

ProxyWorker::Status StaticTextWorker::status()
{
    return static_cast<ProxyWorker::Status>(m_status.loadAcquire());
}

bool StaticTextWorker::hasItem() const
{
    QMutexLocker lk(&m_mtx);
    return !m_items.isEmpty();
}

GrandSearch::MatchedItemMap StaticTextWorker::takeAll()
{
    QMutexLocker lk(&m_mtx);
    GrandSearch::MatchedItems items = std::move(m_items);

    Q_ASSERT(m_items.isEmpty());
    lk.unlock();

    //添加分组
    GrandSearch::MatchedItemMap ret;
    ret.insert(group(), items);

    return ret;
}

QString StaticTextWorker::group() const
{
    return GRANDSEARCH_GROUP_WEB;
}

QString StaticTextWorker::createUrl(const QString &searchEngine) const
{
    static const QString baidu("https://www.baidu.com/s?&wd=%0");
    static const QString google("https://www.google.com/search?q=%0");

    static const QHash<QString, QString> searchEngineChinese{{GRANDSEARCH_WEB_SEARCHENGINE_GOOGLE, "https://www.google.com/search?q=%0"},
                                                {GRANDSEARCH_WEB_SEARCHENGINE_BING, "https://bing.com/search?q=%0"},
                                                {GRANDSEARCH_WEB_SEARCHENGINE_YAHOO, "https://search.yahoo.com/search?p=%0"},
                                                {GRANDSEARCH_WEB_SEARCHENGINE_360, "https://www.so.com/s?&q=%0"},
                                                {GRANDSEARCH_WEB_SEARCHENGINE_SOGOU, "https://www.sogou.com/web?query=%0"}};
    static const QHash<QString, QString> searchEngineEnglish{{GRANDSEARCH_WEB_SEARCHENGINE_BING, "https://bing.com/search?q=%0"},
                                                {GRANDSEARCH_WEB_SEARCHENGINE_YAHOO, "https://search.yahoo.com/search?p=%0"},
                                                {GRANDSEARCH_WEB_SEARCHENGINE_BAIDU, "https://www.baidu.com/s?&wd=%0"}};
    if (SearchHelper::instance()->isSimplifiedChinese()) {
        return searchEngineChinese.value(searchEngine, baidu);
    } else {
        return searchEngineEnglish.value(searchEngine, google);
    }
}
