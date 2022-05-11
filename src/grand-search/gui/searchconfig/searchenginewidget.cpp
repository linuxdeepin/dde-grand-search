/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhaohanxi<zhaohanxi@uniontech.com>
 *
 * Maintainer: zhaohanxi<zhaohanxi@uniontech.com>
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

#include "searchenginewidget.h"
#include "business/config/searchconfig.h"
#include "global/builtinsearch.h"
#include "global/searchconfigdefine.h"
#include "global/searchhelper.h"
#include "comboboxwidget/comboboxwidget.h"

#include <DWidget>
#include <DComboBox>
#include <DStyleOption>

#define Margin  10

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
using namespace GrandSearch;

static const QHash<int, QString> searchEngineEnglish{{0, GRANDSEARCH_WEB_SEARCHENGINE_GOOGLE}, {1, GRANDSEARCH_WEB_SEARCHENGINE_YAHOO},
                          {2, GRANDSEARCH_WEB_SEARCHENGINE_BING}, {3, GRANDSEARCH_WEB_SEARCHENGINE_BAIDU}};
static const QHash<int, QString> searchEngineChinese{{0, GRANDSEARCH_WEB_SEARCHENGINE_BAIDU}, {1, GRANDSEARCH_WEB_SEARCHENGINE_SOGOU},
                                 {2, GRANDSEARCH_WEB_SEARCHENGINE_SO}, {3, GRANDSEARCH_WEB_SEARCHENGINE_GOOGLE},
                                 {4, GRANDSEARCH_WEB_SEARCHENGINE_YAHOO}, {5, GRANDSEARCH_WEB_SEARCHENGINE_BING}};

SearchEngineWidget::SearchEngineWidget(QWidget *parent)
    :DWidget(parent)
{
    m_groupLabel = new QLabel(tr("Default search engine"), this);
    m_groupLabel->adjustSize();

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(4);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addWidget(m_groupLabel);

    QString content = tr("Search for keywords by the default search engine.");

    m_contentLabel = new QLabel(content, this);
    m_contentLabel->setWordWrap(true);
    DFontSizeManager::instance()->bind(m_contentLabel, DFontSizeManager::T8);
    QPalette p(m_contentLabel->palette());
    p.setColor(QPalette::Active, QPalette::WindowText, QColor("#526A7F"));
    m_contentLabel->setPalette(p);
    m_mainLayout->addWidget(m_contentLabel);

    m_comboboxWidget = new ComboboxWidget(this);
    m_comboboxWidget->setFixedSize(COMBOBOXWIDGETWIDTH, COMBOBOXWIDGETHEIGHT);
    m_comboboxWidget->setEnableBackground(true);
    QString userChoice = SearchConfig::instance()->getConfig(GRANDSEARCH_WEB_GROUP, GRANDSEARCH_WEB_SEARCHENGINE, "").toString();
    auto comboBox = m_comboboxWidget->getComboBox();

    QString display = tr("Default search engine is");
    int maxLabelWidth = m_comboboxWidget->width() - Margin - Margin - comboBox->width();
    QFont nameFont = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    QFontMetrics contentFontMetrics(nameFont);
    QString elidedContent = contentFontMetrics.elidedText(display, Qt::ElideRight, maxLabelWidth);
    m_comboboxWidget->setTitle(elidedContent);

    QStringList list;
    {
        const QString searchEngineGoogle = tr("Google");
        const QString searchEngineBaidu = tr("Baidu");
        const QString searchEngineBing = tr("Bing");
        const QString searchEngineYahoo = tr("Yahoo");
        const QString searchEngineSo = tr("So");
        const QString searchEngineSogou = tr("Sogou");
        if (searchHelper->isSimplifiedChinese()) {
            list << searchEngineBaidu << searchEngineSogou << searchEngineSo << searchEngineGoogle << searchEngineYahoo << searchEngineBing;
        } else {
            list << searchEngineGoogle<< searchEngineYahoo << searchEngineBing << searchEngineBaidu;
        }
    }

    comboBox->addItems(list);
    if (userChoice.isEmpty()) {
        comboBox->setCurrentIndex(0);
    } else {
        int index = getIndex(userChoice);
        comboBox->setCurrentIndex(index);
    }
    m_mainLayout->addSpacerItem(new QSpacerItem(10, 10));
    m_mainLayout->addWidget(m_comboboxWidget);
    m_comboboxWidget->setProperty(GRANDSEARCH_SEARCH_GROUP, GRANDSEARCH_WEB_SEARCHENGINE);

    connect(m_comboboxWidget, &ComboboxWidget::checkedChanged, this, &SearchEngineWidget::checkedChangedIndex);
}

SearchEngineWidget::~SearchEngineWidget()
{

}

void SearchEngineWidget::checkedChangedIndex(int index)
{
    GrandSearch::ComboboxWidget *ComboboxWidget = static_cast<GrandSearch::ComboboxWidget *>(sender());

    if (ComboboxWidget) {
        QString text;
        bool language = SearchHelper::instance()->isSimplifiedChinese();
        if (language) {
            text = searchEngineChinese.value(index);
        } else {
            text = searchEngineEnglish.value(index);
        }
        SearchConfig::instance()->setConfig(GRANDSEARCH_WEB_GROUP, GRANDSEARCH_WEB_SEARCHENGINE, text);
    }
}

int SearchEngineWidget::getIndex(const QString text) const
{
    int index = 0;
    bool language = SearchHelper::instance()->isSimplifiedChinese();
    if (language) {
        index = searchEngineChinese.key(text);
    } else {
        index = searchEngineEnglish.key(text);
    }
    return index;
}
