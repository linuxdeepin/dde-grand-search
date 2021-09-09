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
#include "generalpreviewplugin_p.h"
#include "generalpreviewplugin.h"
#include "utils/utils.h"

#include <DFontSizeManager>
#include <DMessageBox>
#include <DGuiApplicationHelper>

#include <QIcon>
#include <QGuiApplication>
#include <QClipboard>
#include <QToolButton>
#include <QPainter>

#define ICON_SIZE               96
#define HOR_MARGIN_SIZE         10
#define LAYOUT_SPACING          15
#define MARGIN_SIZE             15
#define NAME_HEIGHT             25
#define TOOLBTN_WIDTH           100

DWIDGET_USE_NAMESPACE

using namespace GrandSearch;

#define GET_BASICINFO_VALUE_LABEL(ROW, LABEL) SectionValueLabel* LABEL = qobject_cast<SectionValueLabel*>(m_basicInfoLayout->getRowField(ROW)); \
                                           Q_ASSERT(LABEL);

#define SET_BASICINFO_VALUE_LABEL_TEXT(ROW, TEXT) { \
                                                    GET_BASICINFO_VALUE_LABEL(ROW, LABEL) \
                                                    QString t = LABEL->fontMetrics().elidedText(TEXT, Qt::ElideRight, LABEL->width());\
                                                    LABEL->setText(t); \
                                                    if (t != TEXT) \
                                                        LABEL->setToolTip(TEXT); \
                                                  }

NameLabel::NameLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("NameLabel");
    setFixedHeight(NAME_HEIGHT);

    QColor textColor = QColor(0, 0, 0, static_cast<int>(255 * 0.9));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, static_cast<int>(255 * 0.9));
    QPalette pa = palette();
    pa.setColor(QPalette::WindowText, textColor);
    setPalette(pa);

    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    font.setWeight(QFont::Medium);

    setAlignment(Qt::AlignBottom | Qt::AlignLeft);
}

SizeLabel::SizeLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("SizeLabel");
    setFixedHeight(17);
    QColor textColor = QColor(124, 124, 124, static_cast<int>(255 * 1));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, static_cast<int>(255 * 1));
    QPalette pa = palette();
    pa.setColor(QPalette::WindowText, textColor);
    setPalette(pa);

    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T8);
    font.setWeight(QFont::Normal);

    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
}

IconButton::IconButton(QWidget *parent)
    : QToolButton(parent)
{
    setFixedWidth(TOOLBTN_WIDTH);
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    QColor textColor = QColor(82, 106, 127, static_cast<int>(255 * 1));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, static_cast<int>(255 * 1));
    QPalette pa = palette();
    pa.setColor(QPalette::ButtonText, textColor);
    setPalette(pa);


    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T8);
    font.setWeight(QFont::Normal);
    font.setFamily("SourceHanSansSC");
    setFont(font);
}


GeneralToolBar::GeneralToolBar(QWidget *parent)
    : QWidget(parent)
{
    initUi();
    initConnect();
}

GeneralToolBar::~GeneralToolBar()
{

}

void GeneralToolBar::onBtnClicked()
{
    QToolButton* pBtn = qobject_cast<QToolButton*>(sender());
    if (!pBtn)
        return;

    if (pBtn == m_openBtn)
        emit sigBtnClicked(Btn_Open);
    else if(pBtn == m_openPathBtn)
        emit sigBtnClicked(Btn_OpenPath);
    else if(pBtn == m_copyPathBtn)
        emit sigBtnClicked(Btn_CopyPath);
}

void GeneralToolBar::initUi()
{
    setFixedHeight(28);

    m_hMainLayout = new QHBoxLayout(this);
    m_hMainLayout->setContentsMargins(0, 0, 0, 0);
    m_hMainLayout->setSpacing(0);

    m_openBtn = new IconButton(this);
    m_openBtn->setText(QObject::tr("Open"));
    m_openBtn->setIcon(QIcon::fromTheme("deepin-defender"));

    m_openPathBtn = new IconButton(this);
    m_openPathBtn->setText(QObject::tr("Open path"));

    m_copyPathBtn = new IconButton(this);
    m_copyPathBtn->setText(QObject::tr("Copy path"));

    m_vLine1 = new DVerticalLine(this);
    m_vLine2 = new DVerticalLine(this);

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

    connect(m_openBtn, SIGNAL(clicked()), SLOT(onBtnClicked()));
    connect(m_openPathBtn, SIGNAL(clicked()), SLOT(onBtnClicked()));
    connect(m_copyPathBtn, SIGNAL(clicked()), SLOT(onBtnClicked()));
}

GeneralPreviewPluginPrivate::GeneralPreviewPluginPrivate(GeneralPreviewPlugin *parent)
    : q_p(parent)
    , m_contentWidget(new QWidget())
{
    // 图标和名称
    m_iconLabel = new QLabel(m_contentWidget);
    m_iconLabel->setObjectName("IconLabel");
    m_iconLabel->setFixedSize(QSize(ICON_SIZE, ICON_SIZE));
    m_nameLabel = new NameLabel("", m_contentWidget);
    m_sizeLabel = new SizeLabel("", m_contentWidget);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    vLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
    vLayout->addWidget(m_nameLabel);
    vLayout->addWidget(m_sizeLabel);
    vLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(HOR_MARGIN_SIZE - 3, MARGIN_SIZE, HOR_MARGIN_SIZE, MARGIN_SIZE);
    hLayout->setSpacing(HOR_MARGIN_SIZE);
    hLayout->addWidget(m_iconLabel);
    hLayout->addLayout(vLayout);

    m_vMainLayout = new QVBoxLayout(m_contentWidget);

    m_vMainLayout->setContentsMargins(MARGIN_SIZE, 0, MARGIN_SIZE, MARGIN_SIZE);
    m_vMainLayout->addLayout(hLayout);

    m_toolBar = new GeneralToolBar();
}

QString GeneralPreviewPluginPrivate::getBasicInfoLabelName(GeneralPreviewPluginPrivate::BasicInfoRow eRow)
{
    QString name = "";

    switch (eRow) {
    case Location_Row:
        name = QObject::tr("Location");
        break;
    case ContainSize_Row:
        name = QObject::tr("Contains:");
        break;
    case TimeModified_Row:
        name = QObject::tr("Time modified:");
        break;
    default:
        break;
    }

    return name;
}

GeneralPreviewPlugin::GeneralPreviewPlugin(QObject *parent)
    : PreviewPlugin(parent)
    , d_p(new GeneralPreviewPluginPrivate(this))
{
    d_p->m_contentWidget->installEventFilter(this);
    initConnect();
}

GeneralPreviewPlugin::~GeneralPreviewPlugin()
{
    Q_ASSERT(d_p->m_contentWidget);
    d_p->m_contentWidget->deleteLater();
}

bool GeneralPreviewPlugin::previewItem(const MatchedItem &item)
{
    if (!item.item.isEmpty()
            && !item.name.isEmpty()
            && d_p->m_item.item == item.item
            && d_p->m_item.name == item.name)
        return true;

    d_p->m_item = item;

    // 设置图标
    QPixmap pixmap;
    const QSize iconSize(ICON_SIZE, ICON_SIZE);
    const QString &strIcon = item.icon;
    if (!strIcon.isEmpty()) {
        // 判断icon是路径还是图标名
        if (strIcon.contains('/')) {
            QFileInfo file(strIcon);
            if (file.exists())
                pixmap = QIcon(strIcon).pixmap(iconSize);
            else
                pixmap = Utils::defaultIcon(item).pixmap(iconSize);
        } else {
            QIcon icon = QIcon::fromTheme(strIcon);
            if (icon.isNull())
                pixmap = Utils::defaultIcon(item).pixmap(iconSize);
            else
                pixmap = icon.pixmap(iconSize);
        }
    } else {
        pixmap = Utils::defaultIcon(item).pixmap(iconSize);
    }
    d_p->m_iconLabel->setPixmap(pixmap);

    // 设置名称，并计算换行内容
    QString elidedText = lineFeed(item.name, d_p->m_nameLabel->width(), d_p->m_nameLabel->font(), 1);
    d_p->m_nameLabel->setText(elidedText);
    if (elidedText != item.name)
        d_p->m_nameLabel->setToolTip(item.name);

    QFileInfo fi(item.item);

    // 获取文件(夹)大小
    d_p->m_sizeLabel->setText(Utils::formatFileSize(fi.size()));

    // 设置属性详情信息
    d_p->m_detailInfos.clear();
    DetailInfo info;

    info.clear();
    info[d_p->getBasicInfoLabelName(GeneralPreviewPluginPrivate::Location_Row)] = fi.absoluteFilePath();
    d_p->m_detailInfos.push_back(info);

    info.clear();
    info[d_p->getBasicInfoLabelName(GeneralPreviewPluginPrivate::TimeModified_Row)] = fi.lastModified().toString(Utils::dateTimeFormat());
    d_p->m_detailInfos.push_back(info);

    return true;
}

MatchedItem GeneralPreviewPlugin::item() const
{
    return d_p->m_item;
}

QWidget *GeneralPreviewPlugin::contentWidget() const
{
    return d_p->m_contentWidget;
}

DetailInfoList GeneralPreviewPlugin::getAttributeDetailInfo() const
{
    return d_p->m_detailInfos;
}

QWidget *GeneralPreviewPlugin::toolBarWidget() const
{
    return d_p->m_toolBar;
}

void GeneralPreviewPlugin::initConnect()
{
    connect(d_p->m_toolBar, &GeneralToolBar::sigBtnClicked, this, &GeneralPreviewPlugin::onToolBtnClicked);
}

void GeneralPreviewPlugin::onToolBtnClicked(int nBtnId)
{
    if (nBtnId == GeneralToolBar::Btn_Open)
        onOpenClicked();
    else if (nBtnId == GeneralToolBar::Btn_OpenPath)
        onOpenpathClicked();
    else if (nBtnId == GeneralToolBar::Btn_CopyPath)
        onCopypathClicked();
}

void GeneralPreviewPlugin::onOpenClicked()
{
    MatchedItem item;
    item.name = d_p->m_item.name;
    Utils::openFile(item);
}

void GeneralPreviewPlugin::onOpenpathClicked()
{
    QFileInfo fi(d_p->m_item.item);
    if (fi.exists()) {
        MatchedItem item = d_p->m_item;
        item.icon = "inode-directory";
        item.item = fi.absolutePath();
        item.name = fi.dir().dirName();
        item.type = "inode/directory";

        Utils::openFile(item);
    }
}

void GeneralPreviewPlugin::onCopypathClicked()
{
    QWidget* topWidget = contentWidget()->topLevelWidget();
    Q_ASSERT(topWidget);

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(d_p->m_item.item);

    auto showPoint = topWidget->mapToGlobal(QPoint(topWidget->width() / 2, topWidget->height() / 2 + 40));
    auto color = topWidget->palette().background().color();

    Utils::showAlertMessage(showPoint, color, QObject::tr("Path information has been copied to the clipboard."), 2000);
}

QString GeneralPreviewPlugin::lineFeed(const QString &text, int nWidth, const QFont &font, int nElidedRow)
{
    if (nElidedRow < 0)
        nElidedRow = 2;

    QString strText = text;
    int nIndex = 1;
    QFontMetrics fm(font);
    if (!strText.isEmpty()) {
        for (int i = 1; i < strText.size() + 1; i++) {
            if (fm.width(strText.left(i)) > nWidth * nIndex) {
                nIndex++;
                if (nIndex > nElidedRow) {
                    strText = fm.elidedText(strText, Qt::ElideRight, nWidth * nElidedRow);
                    break;
                }
                strText.insert(i - 1, "\n");
            }
        }
    }

    return strText;
}
