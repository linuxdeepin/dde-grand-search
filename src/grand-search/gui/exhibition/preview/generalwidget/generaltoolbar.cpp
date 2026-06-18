// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "generaltoolbar.h"
#include "utils/utils.h"

#include <DPushButton>
#include <DVerticalLine>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>

#include <QHBoxLayout>
#include <QGuiApplication>
#include <QStylePainter>
#include <QStyleOptionToolButton>

#define TOOLBTN_WIDTH_NARROW 80
#define TOOLBTN_WIDTH_WIDE 108
#define TOOLBTN_MAX_PIXELSIZE 18
#define TOOLBAR_HEIGHT 36

using namespace GrandSearch;
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

ToolButton::ToolButton(QWidget *parent)
    : QToolButton(parent)
{
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    updatePalette();
    DFontSizeManager::instance()->bind(this, DFontSizeManager::T6);

    if (this->font().pixelSize() < TOOLBTN_MAX_PIXELSIZE) {
        setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        setIconSize(QSize(24, 24));
    } else {
        setToolButtonStyle(Qt::ToolButtonIconOnly);
        setIconSize(size());
    }
}

void ToolButton::updatePalette()
{
    bool isDark = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
    QColor textColor = isDark
            ? QColor(255, 255, 255, int(255 * 0.9))
            : QColor(0, 0, 0, int(255 * 0.9));
    QColor bakgrdColor = DGuiApplicationHelper::instance()->applicationPalette().color(DPalette::ObviousBackground);

    QPalette pa = palette();
    pa.setColor(QPalette::ButtonText, textColor);
    pa.setColor(QPalette::WindowText, textColor);
    pa.setColor(QPalette::Button, bakgrdColor);
    setPalette(pa);
}

void ToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStylePainter painter(this);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    bool active = opt.state & QStyle::State_Sunken;
    QPixmap pix = icon().pixmap(opt.iconSize, QIcon::Normal, QIcon::Off);
    if (!pix.isNull() && active) {
        QColor tintColor = palette().color(QPalette::Highlight);
        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(pix.rect(), tintColor);
        p.end();
        opt.icon = QIcon(pix);
    }

    painter.drawControl(QStyle::CE_ToolButtonLabel, opt);
}

GeneralToolBar::GeneralToolBar(QWidget *parent)
    : DWidget(parent)
{
    initUi();
    initConnect();
}

void GeneralToolBar::initUi()
{
    setFixedHeight(TOOLBAR_HEIGHT);

    m_hMainLayout = new QHBoxLayout(this);
    // 下边距10
    m_hMainLayout->setContentsMargins(0, 0, 0, 0);
    m_hMainLayout->setSpacing(0);

    QString suffix = Utils::iconThemeSuffix();

    m_openBtn = new ToolButton(this);
    m_openBtn->setText(tr("Open"));
    m_openBtn->setIcon(QIcon(QString(":/icons/open%1.svg").arg(suffix)));
    m_openBtn->setFixedWidth(TOOLBTN_WIDTH_NARROW);

    m_openPathBtn = new ToolButton(this);
    m_openPathBtn->setText(tr("Open Path"));
    m_openPathBtn->setIcon(QIcon(QString(":/icons/openpath%1.svg").arg(suffix)));
    m_openPathBtn->setFixedWidth(TOOLBTN_WIDTH_WIDE);

    m_copyPathBtn = new ToolButton(this);
    m_copyPathBtn->setText(tr("Copy Path"));
    m_copyPathBtn->setIcon(QIcon(QString(":/icons/copypath%1.svg").arg(suffix)));
    m_copyPathBtn->setFixedWidth(TOOLBTN_WIDTH_WIDE);

    m_vLine1 = new DVerticalLine(this);
    m_vLine1->setFixedHeight(30);
    m_vLine2 = new DVerticalLine(this);
    m_vLine2->setFixedHeight(30);

    m_hMainLayout->addStretch(1);
    m_hMainLayout->addWidget(m_openBtn);
    m_hMainLayout->addWidget(m_vLine1);
    m_hMainLayout->addWidget(m_openPathBtn);
    m_hMainLayout->addWidget(m_vLine2);
    m_hMainLayout->addWidget(m_copyPathBtn);
    m_hMainLayout->addStretch(1);

    this->setLayout(m_hMainLayout);
}

void GeneralToolBar::initConnect()
{
    Q_ASSERT(m_openBtn);
    Q_ASSERT(m_openPathBtn);
    Q_ASSERT(m_copyPathBtn);

    connect(m_openBtn, &DPushButton::clicked, this, &GeneralToolBar::sigOpenClicked);
    connect(m_openPathBtn, &DPushButton::clicked, this, &GeneralToolBar::sigOpenPathClicked);
    connect(m_copyPathBtn, &DPushButton::clicked, this, &GeneralToolBar::sigCopyPathClicked);
}
