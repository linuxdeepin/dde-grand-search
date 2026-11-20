// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "generaltoolbar.h"
#include "utils/utils.h"

#include <DPushButton>
#include <DVerticalLine>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>
#include <DStyle>
#include <DPlatformTheme>

#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOptionToolButton>
#include <QGuiApplication>

#define TOOLBTN_WIDTH_NARROW    80
#define TOOLBTN_WIDTH_WIDE      108
#define TOOLBTN_MAX_PIXELSIZE   18
#define TOOLBAR_HEIGHT          36
#define TOOLBAR_LEFT_MARGIN     30
#define TOOLBAR_RIGHT_MARGIN    30
#define TOOLBAR_BOTTOM_MARGIN   10
#define TOOLBTN_CORNER_RADIUS   6

using namespace GrandSearch;
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

IconButton::IconButton(QWidget *parent)
    : QToolButton(parent)
{
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    QColor textColor(0, 0, 0, int(255 * 0.9));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, int(255 * 0.9));
    QPalette pa = palette();
    pa.setColor(QPalette::ButtonText, textColor);
    setPalette(pa);

    DFontSizeManager::instance()->bind(this, DFontSizeManager::T6);

    if (this->font().pixelSize() < TOOLBTN_MAX_PIXELSIZE) {
        setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        setIconSize(QSize(24, 24));
    } else {
        setToolButtonStyle(Qt::ToolButtonIconOnly);
        setIconSize(QSize(24, 24));
    }

    connect(DGuiApplicationHelper::instance()->systemTheme(), &DPlatformTheme::activeColorChanged, this, [this]() { update(); });
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [this]() { update(); });
}

void IconButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::NoBrush);

    QColor backgroundColor(0, 0, 0, 0);
    QColor textColor(0, 0, 0, static_cast<int>(255 * 0.9));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, static_cast<int>(255 * 0.9));

    QColor activeColor = DGuiApplicationHelper::instance()->systemTheme()->activeColor();

    // hover 态仅背景变化，图标与文字保持默认颜色；
    // press 态背景加深，同时图标和文字跟随系统活动色变化
    bool useActiveColor = false;

    // normal状态、hover状态、press状态
    if (opt.state & (QStyle::State_Active | QStyle::State_MouseOver | QStyle::State_Sunken)) {
        auto state = DStyle::getState(&opt);
        switch (state) {
        case DStyle::SS_NormalState: {
            break;
        }
        case DStyle::SS_HoverState: {
            backgroundColor = QColor(0, 0, 0, static_cast<int>(255 * 0.05));
            if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
                backgroundColor = QColor(255, 255, 255, static_cast<int>(255 * 0.05));
            break;
        }
        case DStyle::SS_PressState: {
            backgroundColor = QColor(0, 0, 0, static_cast<int>(255 * 0.1));
            if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
                backgroundColor = QColor(255, 255, 255, static_cast<int>(255 * 0.1));
            textColor = activeColor;
            useActiveColor = true;
            break;
        }
        default:
            break;
        }
    }

    p.setBrush(backgroundColor);
    p.drawRoundedRect(opt.rect, TOOLBTN_CORNER_RADIUS, TOOLBTN_CORNER_RADIUS);

    bool hasIcon = !opt.icon.isNull() && opt.toolButtonStyle != Qt::ToolButtonTextOnly;
    bool hasText = !opt.text.isEmpty() && opt.toolButtonStyle != Qt::ToolButtonIconOnly;

    if (hasIcon) {
        QIcon::Mode mode = QIcon::Normal;
        if (!(opt.state & QStyle::State_Enabled))
            mode = QIcon::Disabled;
        else if (opt.state & QStyle::State_Sunken)
            mode = QIcon::Active;

        QPixmap pix = opt.icon.pixmap(opt.iconSize, mode);

        // press状态下将图标着色为活动色（hover 态保持默认图标颜色）
        if (useActiveColor) {
            QPainter pixPainter(&pix);
            pixPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            pixPainter.fillRect(pix.rect(), activeColor);
        }

        int iconY = opt.rect.y() + (opt.rect.height() - pix.height()) / 2;

        if (hasText) {
            int textWidth = opt.fontMetrics.horizontalAdvance(opt.text);
            int totalWidth = pix.width() + 4 + textWidth;
            int startX = opt.rect.x() + (opt.rect.width() - totalWidth) / 2;
            p.drawPixmap(startX, iconY, pix);
            QRect textRect(startX + pix.width() + 4, opt.rect.y(), textWidth, opt.rect.height());
            p.setPen(textColor);
            p.setFont(opt.font);
            p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextShowMnemonic, opt.text);
        } else {
            int iconX = opt.rect.x() + (opt.rect.width() - pix.width()) / 2;
            p.drawPixmap(iconX, iconY, pix);
        }
    } else if (hasText) {
        p.setPen(textColor);
        p.setFont(opt.font);
        p.drawText(opt.rect, Qt::AlignCenter | Qt::TextShowMnemonic, opt.text);
    }
}

GeneralToolBar::GeneralToolBar(QWidget *parent)
    : DWidget(parent)
{
    initUi();
    initConnect();
}

void GeneralToolBar::initUi()
{
    setFixedHeight(TOOLBAR_HEIGHT + TOOLBAR_BOTTOM_MARGIN); //高36 + 下边距10

    m_hMainLayout = new QHBoxLayout(this);
    //下边距10
    m_hMainLayout->setContentsMargins(TOOLBAR_LEFT_MARGIN, 0, TOOLBAR_RIGHT_MARGIN, TOOLBAR_BOTTOM_MARGIN);
    m_hMainLayout->setSpacing(0);

    QString suffix = Utils::iconThemeSuffix();

    m_openBtn = new IconButton(this);
    m_openBtn->setText(tr("Open"));
    m_openBtn->setIcon(QIcon(QString(":/icons/open%1.svg").arg(suffix)));
    m_openBtn->setFixedWidth(TOOLBTN_WIDTH_NARROW);

    m_openPathBtn = new IconButton(this);
    m_openPathBtn->setText(tr("Open Path"));
    m_openPathBtn->setIcon(QIcon(QString(":/icons/openpath%1.svg").arg(suffix)));
    m_openPathBtn->setFixedWidth(TOOLBTN_WIDTH_WIDE);

    m_copyPathBtn = new IconButton(this);
    m_copyPathBtn->setText(tr("Copy Path"));
    m_copyPathBtn->setIcon(QIcon(QString(":/icons/copypath%1.svg").arg(suffix)));
    m_copyPathBtn->setFixedWidth(TOOLBTN_WIDTH_WIDE);

    m_vLine1 = new DVerticalLine(this);
    m_vLine1->setFixedHeight(30);
    m_vLine2 = new DVerticalLine(this);
    m_vLine2->setFixedHeight(30);

    m_hMainLayout->addWidget(m_openBtn);
    m_hMainLayout->addWidget(m_vLine1);
    m_hMainLayout->addWidget(m_openPathBtn);
    m_hMainLayout->addWidget(m_vLine2);
    m_hMainLayout->addWidget(m_copyPathBtn);

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
