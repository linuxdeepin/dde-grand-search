// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUTHPROMPTWIDGET_H
#define AUTHPROMPTWIDGET_H

#include <DWidget>
#include <DTipLabel>

#include <QHBoxLayout>

namespace Dtk { namespace Core { class DConfig; } }

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
    void setSearchindexesEnable();

    // 检查文件管理器搜索 DConfig 中各功能项的开启状态
    bool isFileIndexSearchEnabled() const;
    bool isFullTextSearchEnabled() const;
    bool isOcrTextSearchEnabled() const;
    bool isSemanticSearchEnabled() const;

    // 返回当前未开启的搜索模式名称列表（为空表示全部已开启）
    QStringList disabledSearchModes();

    // 文本省略计算
    QString buildElidedText(int availableWidth) const;

private:
    QHBoxLayout *m_hLayout = nullptr;
    Dtk::Widget::DTipLabel *m_contentLabel = nullptr;
    IconButton *m_closeButton = nullptr;
    QString m_featuresText;   // 不可用功能列表文本
    Dtk::Core::DConfig *m_searchDConfig = nullptr;
};

}

#endif   // AUTHPROMPTWIDGET_H
