// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTPREVIEWPLUGIN_H
#define TEXTPREVIEWPLUGIN_H

#include <previewplugin.h>

class PropertyLabel;
class TextView;
class TextPreviewPlugin : public QObject, public GrandSearch::PreviewPlugin
{
    Q_OBJECT
public:
    explicit TextPreviewPlugin(QObject *parent = 0);
    ~TextPreviewPlugin() Q_DECL_OVERRIDE;
    void init(QObject *proxyInter) Q_DECL_OVERRIDE;
    bool previewItem(const GrandSearch::ItemInfo &item) Q_DECL_OVERRIDE;
    GrandSearch::ItemInfo item() const Q_DECL_OVERRIDE;
    QWidget *contentWidget() const Q_DECL_OVERRIDE;
    bool stopPreview() Q_DECL_OVERRIDE;
    QWidget *toolBarWidget() const Q_DECL_OVERRIDE;
    bool showToolBar() const Q_DECL_OVERRIDE;
    GrandSearch::DetailInfoList getAttributeDetailInfo() const Q_DECL_OVERRIDE;
protected:
    GrandSearch::ItemInfo m_item;
    TextView *m_view = nullptr;
    GrandSearch::DetailInfoList m_detailInfo;
};

#endif // TEXTPREVIEWPLUGIN_H
