// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EMBEDDINGPLUGINWIDGET_H
#define EMBEDDINGPLUGINWIDGET_H

#include "modelmanagebutton.h"

#include <DLabel>
#include <DWidget>

namespace GrandSearch {
class EmbeddingPluginWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit EmbeddingPluginWidget(QWidget *parent = nullptr);
    void setText(const QString &theme, const QString &summary);
    bool isInstalled();
    void checkInstallStatus();
public slots:
    void openAppStore();
protected:
    void paintEvent(QPaintEvent* e) Q_DECL_OVERRIDE;
private:
    void initUI();
signals:
    void pluginStateChanged(bool);

private:
    Dtk::Widget::DLabel *m_pLabelTheme = nullptr;
    Dtk::Widget::DLabel *m_pLabelSummary = nullptr;
    Dtk::Widget::DLabel *m_pLabelStatus = nullptr;
    ModelManageButton *m_pManageModel = nullptr;
};

}
#endif // EMBEDDINGPLUGINWIDGET_H
