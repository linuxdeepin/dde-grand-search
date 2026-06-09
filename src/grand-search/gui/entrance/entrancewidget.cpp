// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "entrancewidget_p.h"
#include "entrancewidget.h"
#include "searchedit.h"
#include "utils/utils.h"

#include <DGuiApplicationHelper>

#include <QLineEdit>
#include <QHBoxLayout>
#include <QEvent>

DGUI_USE_NAMESPACE
using namespace GrandSearch;

static const uint EntraceWidgetWidth = 740;
static const uint EntraceWidgetHeight = 48;
static const uint WidgetMargins = 10;

EntranceWidgetPrivate::EntranceWidgetPrivate(EntranceWidget *parent)
    : q_p(parent)
{
}

EntranceWidget::EntranceWidget(QWidget *parent)
    : QFrame(parent), d_p(new EntranceWidgetPrivate(this))
{
    initUI();
    initConnections();
}

EntranceWidget::~EntranceWidget() = default;

void EntranceWidget::showLabelAppIcon(bool visible)
{
    Q_ASSERT(d_p->m_searchEdit);
    d_p->m_searchEdit->setAppIconVisible(visible);
}

bool EntranceWidget::event(QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        d_p->m_searchEdit->setFocus();
        return true;
    }
    return QFrame::event(event);
}

void EntranceWidget::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
}

void EntranceWidget::initUI()
{
    d_p->m_searchEdit = new SearchEdit(this);
    d_p->m_searchEdit->lineEdit()->setFixedSize(EntraceWidgetWidth, EntraceWidgetHeight);
    d_p->m_searchEdit->setPlaceHolder(tr("Search"));
    d_p->m_searchEdit->setPlaceholderText(tr("What would you like to search for?"));

    d_p->m_mainLayout = new QHBoxLayout(this);
    d_p->m_mainLayout->addWidget(d_p->m_searchEdit);
    d_p->m_mainLayout->setSpacing(0);
    d_p->m_mainLayout->setContentsMargins(WidgetMargins, WidgetMargins, WidgetMargins, WidgetMargins);

    this->setLayout(d_p->m_mainLayout);
}

void EntranceWidget::initConnections()
{
    Q_ASSERT(d_p->m_searchEdit);

    connect(d_p->m_searchEdit, &SearchEdit::debouncedTextChanged, this, &EntranceWidget::searchTextChanged);
    connect(d_p->m_searchEdit, &SearchEdit::debouncedTextChanged, this, [this](const QString &text) {
        if (text == d_p->m_currentSearchText)
            return;

        d_p->m_currentSearchText = text;
        MatchedItem item;
        onAppIconChanged(QString(), item);
    });

    connect(d_p->m_searchEdit, &SearchEdit::selectNextItem, this, &EntranceWidget::sigSelectNextItem);
    connect(d_p->m_searchEdit, &SearchEdit::selectPreviousItem, this, &EntranceWidget::sigSelectPreviousItem);
    connect(d_p->m_searchEdit, &SearchEdit::handleItem, this, &EntranceWidget::sigHandleItem);
    connect(d_p->m_searchEdit, &SearchEdit::closeWindow, this, &EntranceWidget::sigCloseWindow);

    connect(d_p->m_searchEdit, &SearchEdit::searchAborted, d_p->m_searchEdit->lineEdit(), qOverload<>(&QLineEdit::setFocus));
}

void EntranceWidget::onAppIconChanged(const QString &searchGroupName, const MatchedItem &item)
{
    Q_UNUSED(searchGroupName)

    const QString appIconName = Utils::appIconName(item);
    if (appIconName.isEmpty()) {
        d_p->m_searchEdit->setAppIconVisible(false);
        d_p->m_appIconName.clear();
        return;
    }

    if (appIconName == d_p->m_appIconName)
        return;

    d_p->m_appIconName = appIconName;
    d_p->m_searchEdit->setAppIconVisible(true);
    d_p->m_searchEdit->setAppIcon(appIconName);
}
