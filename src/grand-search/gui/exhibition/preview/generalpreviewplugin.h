/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     houchengqiu<houchengqiu@uniontech.com>
 *
 * Maintainer: houchengqiu<houchengqiu@uniontech.com>
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
#ifndef GENERALPREVIEWPLUGIN_H
#define GENERALPREVIEWPLUGIN_H

#include "../../datadefine.h"
#include "global/matcheditem.h"
#include "global/builtinsearch.h"
#include "previewplugin.h"
#include <QObject>
#include <QScopedPointer>

class QFrame;
class GeneralPreviewPluginPrivate;
class GeneralPreviewPlugin : public PreviewPlugin
{
    Q_OBJECT
public:
    explicit GeneralPreviewPlugin(QObject *parent = nullptr);
    ~GeneralPreviewPlugin() override;

    // 预览指定的搜索结果项
    virtual bool previewItem(const GrandSearch::MatchedItem &strJson) override;

    // 获取当前预览的搜索项
    virtual GrandSearch::MatchedItem item() const override;

    // 返回预览插件自定义预览内容部件
    virtual QWidget *contentWidget() const override;

    virtual DetailInfoList getAttributeDetailInfo() const override;

    virtual QWidget *toolBarWidget() const override;

private slots:
    void onToolBtnClicked(int nBtnId);
    void onOpenClicked();
    void onOpenpathClicked();
    void onCopypathClicked();

private:
    void initConnect();

    // 计算换行内容
    QString lineFeed(const QString &text, int nWidth, const QFont &font, int nElidedRow = 2);

private:
    QScopedPointer<GeneralPreviewPluginPrivate> d_p;
};

#endif // GENERALPREVIEWPLUGIN_H
