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

#include "blacklistwidget.h"
#include "blacklistview/blacklistview.h"
#include "blacklistview/deletedialog.h"
#include "blacklistview/blacklistmodel.h"

#include <DWidget>
#include <DFontSizeManager>
#include <DPushButton>

#include <QVBoxLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QItemSelection>

DWIDGET_USE_NAMESPACE
BlackListWidget::BlackListWidget(QWidget *parent)
    :DWidget(parent)
{
    m_groupLabel = new QLabel(tr("Excluded path"), this);
    m_groupLabel->adjustSize();

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    m_childHLayout = new QHBoxLayout(this);
    m_childHLayout->addWidget(m_groupLabel);

    m_contentLabel = new QLabel(tr("Add paths to the exclusion list to prevent searching in them"), this);

    m_contentLabel->setWordWrap(true);
    DFontSizeManager::instance()->bind(m_contentLabel, DFontSizeManager::T8);
    QPalette p(m_contentLabel->palette());
    p.setColor(QPalette::Active, QPalette::WindowText, QColor("#526A7F"));
    m_contentLabel->setPalette(p);

    m_childHLayout->addStretch();

    m_deleteButton = new DIconButton(this);
    m_deleteButton->setFixedSize(36, 36);
    DStyle style;
    m_deleteButton->setIcon(style.standardIcon(DStyle::SP_DecreaseElement));
    m_deleteButton->setEnabled(false);
    m_addButton = new DIconButton(this);
    m_addButton->setFixedSize(36, 36);
    m_addButton->setIcon(style.standardIcon(DStyle::SP_IncreaseElement));
    m_childHLayout->addWidget(m_deleteButton);
    m_childHLayout->addSpacerItem(new QSpacerItem(10, 10));
    m_childHLayout->addWidget(m_addButton);
    m_mainLayout->addLayout(m_childHLayout);

    m_mainLayout->addWidget(m_contentLabel);

    m_listWrapper = new BlackListWrapper(this);
    m_mainLayout->addSpacerItem(new QSpacerItem(10, 10));
    m_mainLayout->addWidget(m_listWrapper);

    m_deletedialog = new DeleteDialog(this);

    connect(m_addButton, &DIconButton::clicked, this, &BlackListWidget::addButtonClicked);
    connect(m_deleteButton, &DIconButton::clicked, m_deletedialog, &DeleteDialog::exec);
    connect(m_deletedialog->getButton(m_deletedialog->getButtonIndexByText(ConfirmButton))
            , &QAbstractButton::clicked
            , this, &BlackListWidget::confirmButtonClicked);
    connect(m_listWrapper, &BlackListWrapper::selectedChanged, this, &BlackListWidget::selectedChanged);
}

BlackListWidget::~BlackListWidget()
{

}

void BlackListWidget::addButtonClicked()
{
    QFileDialog fileDialog;
    auto url = fileDialog.getExistingDirectoryUrl(this, QString("")
                                                  , QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    QFileInfo info(url.toLocalFile());
    if (!url.isEmpty() && !info.isSymLink() && info.exists()) {
        m_listWrapper->addRow(info.absoluteFilePath());
    } else {
        qInfo() << "add path failed";
    }
}

void BlackListWidget::confirmButtonClicked()
{
    auto selectedRows = m_listWrapper->selectionModel()->selectedRows();
    QList<int> rowList;
    for (auto index : selectedRows) {
        rowList << index.row();
    }
    qStableSort(rowList.begin(), rowList.end(), qGreater<int>());
    for (auto index : rowList) {
        m_listWrapper->removeRows(index, 1);
    }
}

void BlackListWidget::selectedChanged(const QItemSelection &selected, const QItemSelection &deselected) const
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)
    m_deleteButton->setEnabled(!m_listWrapper->selectionModel()->selectedRows().isEmpty());
}
