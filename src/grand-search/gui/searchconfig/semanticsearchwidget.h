// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEMANTICSEARCHWIDGET_H
#define SEMANTICSEARCHWIDGET_H

#include <DWidget>
#include <DSwitchButton>

namespace Dtk { namespace Core { class DConfig; } }

namespace GrandSearch {

class TipsLabel;

class SemanticSearchWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit SemanticSearchWidget(QWidget *parent = nullptr);
    ~SemanticSearchWidget() override;

private slots:
    void onSwitchToggled(bool checked);
    void onFileIndexChanged(const QString &key);

private:
    void initUi();
    void initConnect();
    bool isFileIndexSearchEnabled() const;
    void setFileIndexSearchEnabled(bool enabled);

private:
    DTK_WIDGET_NAMESPACE::DSwitchButton *m_switchBtn = nullptr;
    Dtk::Core::DConfig *m_searchDConfig = nullptr;
};

}

#endif // SEMANTICSEARCHWIDGET_H
