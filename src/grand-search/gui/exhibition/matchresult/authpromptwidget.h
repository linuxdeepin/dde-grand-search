// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUTHPROMPTWIDGET_H
#define AUTHPROMPTWIDGET_H

#include <DWidget>
#include <DLabel>
#include <DHorizontalLine>

#include <QHBoxLayout>
#include <QVBoxLayout>

namespace GrandSearch {

class IconButton;

/**
 * @brief 授权提示控件
 *
 * 当全文搜索、图片文本搜索或智能搜索不可用时显示提示信息，
 * 引导用户进行一键授权。支持文本省略（保留链接完整）、
 * hover 显示关闭按钮、关闭后记录状态不再显示。
 */
class AuthPromptWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT

public:
    explicit AuthPromptWidget(QWidget *parent = nullptr);
    ~AuthPromptWidget() override;

protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void initUi();
    void initConnect();
    void adjustElidedText();
    void updatePromptContent();
    void enableSearchindexes();
    bool contentSearchAvailable();
    bool ocrTextSearchAvailable();
    bool semanticSearchAvailable();

    // 索引可用性检查
    QStringList checkUnavailableFeatures();

    // 文本省略计算
    QString buildElidedText(int availableWidth) const;

private:
    // 布局
    QVBoxLayout *m_vLayout = nullptr;
    QHBoxLayout *m_hLayout = nullptr;
    Dtk::Widget::DLabel *m_contentLabel = nullptr;
    Dtk::Widget::DHorizontalLine *m_separator = nullptr;
    IconButton *m_closeButton = nullptr;

    // 内容缓存
    QString m_featuresText;  // 不可用功能列表文本
};

}

#endif // AUTHPROMPTWIDGET_H
