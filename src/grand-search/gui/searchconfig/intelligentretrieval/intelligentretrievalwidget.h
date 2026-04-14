// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INTELLIGENTRETRIEVALWIDGET_H
#define INTELLIGENTRETRIEVALWIDGET_H

#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>

#include <DWidget>

namespace GrandSearch {
class SwitchWidget;
class LLMWidget;
class EmbeddingPluginWidget;
class DetailCheckBox;
class AutoIndexStatus;
class IntelligentRetrievalWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit IntelligentRetrievalWidget(QWidget *parent = nullptr);

protected:
    void setOcrTextQuery(bool on);
    void setFulltextQuery(bool on);

protected slots:
    void checkChanged();

private:
    QVBoxLayout *m_mainLayout = nullptr;
    SwitchWidget *m_semantic = nullptr;
    QLabel *m_detailLabel = nullptr;
    QWidget *m_indexWidget = nullptr;
    QVBoxLayout *m_indexLayout = nullptr;
    SwitchWidget *m_ocrIndex = nullptr;
    SwitchWidget *m_fullTextIndex = nullptr;
    QLabel *m_fullTextLabel = nullptr;
};

}

#endif   // INTELLIGENTRETRIEVALWIDGET_H
