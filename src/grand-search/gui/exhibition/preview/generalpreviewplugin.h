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
class GeneralPreviewPlugin : public QObject, public GrandSearch::PreviewPlugin
{
    Q_OBJECT
public:
    explicit GeneralPreviewPlugin(QObject *parent = nullptr);
    ~GeneralPreviewPlugin() override;
    virtual bool previewItem(const GrandSearch::ItemInfo &info) override;
    virtual GrandSearch::ItemInfo item() const override;
    bool stopPreview() override;
    virtual QWidget *contentWidget() const override;
    virtual GrandSearch::DetailInfoList getAttributeDetailInfo() const override;
    virtual QWidget *toolBarWidget() const override;
    bool showToolBar() const override;

private:
    // 计算换行内容
    QString lineFeed(const QString &text, int nWidth, const QFont &font, int nElidedRow = 2);

private:
    QScopedPointer<GeneralPreviewPluginPrivate> d_p;
};

#endif // GENERALPREVIEWPLUGIN_H
