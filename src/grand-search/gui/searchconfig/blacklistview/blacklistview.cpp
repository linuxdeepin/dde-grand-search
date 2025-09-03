// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "blacklistview.h"
#include "blacklistmodel.h"
#include "blacklistdelegate.h"
#include "business/config/searchconfig.h"
#include "global/searchconfigdefine.h"

#include <DGuiApplicationHelper>

#include <QString>
#include <QMimeData>
#include <QDropEvent>
#include <QFileInfo>
#include <QPainter>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

DCORE_USE_NAMESPACE

#define MaxWidth 476
#define MaxHeight 272

#define IconSize   24
#define ListItemMaxWidth    362
#define InitCount       7   // 初始显示数量

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace GrandSearch;

BlackListView::BlackListView(QWidget *parent)
    : DListView(parent)
{
    qCDebug(logGrandSearch) << "BlackListView constructor called";

    m_model = new BlackListModel(0, 0, this);
    setModel(m_model);
    m_delegate = new BlackListDelegate(this);
    setItemDelegate(m_delegate);

    setViewportMargins(0, 0, 0, 0);
    setUniformItemSizes(true);
    setViewMode(QListView::ListMode);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Static);
    setLayoutMode(QListView::Batched);
    setBatchSize(2000);
    setEditTriggers(DListView::NoEditTriggers);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAcceptDrops(true);

    init();
}

BlackListView::~BlackListView()
{
    qCDebug(logGrandSearch) << "BlackListView destructor called";
}

bool BlackListView::removeRows(const int &row, const int &count)
{
    qCDebug(logGrandSearch) << "Removing rows from blacklist - Row:" << row << "Count:" << count;
    if (m_model->removeRows(row, count, QModelIndex())) {
        m_validRows -= count;
        updateEmptyRows();
        blackModelChanged();
        qCDebug(logGrandSearch) << "Blacklist rows removal completed - Row:" << row << "Count:" << count;
        return true;
    } else {
        qCWarning(logGrandSearch) << "Failed to remove rows from model - Row:" << row << "Count:" << count;
        return false;
    }
}

void BlackListView::insertRow(const QString &path)
{
    qCDebug(logGrandSearch) << "Inserting row into blacklist - Path:" << path;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        auto item = m_model->item(i);
        if (item->data(DATAROLE).toString().isEmpty()){
            QModelIndex index = m_model->index(i, 0, QModelIndex());
            setData(index, path);
            qCDebug(logGrandSearch) << "Path inserted at existing empty row:" << i;
            break;
        }
    }
}

void BlackListView::appendRow(const QString &path)
{
    qCDebug(logGrandSearch) << "Appending row to blacklist - Path:" << path;

    QStandardItem *newItem = new QStandardItem;
    m_model->appendRow(newItem);
    auto row = m_model->rowCount() - 1;
    QModelIndex index = m_model->index(row, 0, QModelIndex());
    setData(index, path);
}

void BlackListView::updateEmptyRows()
{
    for (int i = m_model->rowCount(); i < InitCount; ++i)
        appendRow(QString(""));
}

void BlackListView::addModel(const QString &path)
{
    qCDebug(logGrandSearch) << "Adding path to model - Path:" << path;
    if (m_validRows < m_model->rowCount())
        insertRow(path);
    else
        appendRow(path);
}

void BlackListView::addRow(const QString &path)
{
    qCInfo(logGrandSearch) << "Adding row to blacklist - Path:" << path;
    if (!match(path).isValid()) {
        addModel(path);
        blackModelChanged();
        qCInfo(logGrandSearch) << "New path added to blacklist:" << path;
    } else {
        qCDebug(logGrandSearch) << "Path already exists in blacklist, moving to last position:" << path;
        moveRowToLast(match(path));
    }
}

void BlackListView::moveRowToLast(const QModelIndex &index)
{
    if (!index.isValid()) {
        qCWarning(logGrandSearch) << "Invalid index provided for row movement";
        return;
    }

    addModel(index.data(DATAROLE).toString());

    if (!removeRows(index.row(), 1))
        qCWarning(logGrandSearch) << "Failed to move item - Row:" << index.row();
}

void BlackListView::dropEvent(QDropEvent *e)
{
    qCDebug(logGrandSearch) << "Drop event received";
    this->clearSelection();
    auto urls = e->mimeData()->urls();
    for (auto &url : urls) {
        QFileInfo info(url.toLocalFile());
        if (info.isDir() && info.exists()) {
            addRow(info.absoluteFilePath());
            e->accept();
        } else {
            return;
        }
    }
}

void BlackListView::dragEnterEvent(QDragEnterEvent *e)
{
    auto urls = e->mimeData()->urls();
    for (auto &url : urls) {
        QFileInfo info(url.toLocalFile());
        if (info.isSymLink() || !info.isDir() || !info.exists()) {
            e->setDropAction(Qt::IgnoreAction);
            return;
        }
    }
    e->setDropAction(Qt::CopyAction);
    e->accept();
}

void BlackListView::blackModelChanged() const
{
    qCDebug(logGrandSearch) << "Blacklist model changed - Updating configuration";

    QStringList blackList;
    for (int row = 0; row < m_model->rowCount(); ++row) {
        QString data = m_model->data(m_model->index(row, 0, QModelIndex()), DATAROLE).toString();
        if (!data.isEmpty())
            blackList << data.toLocal8Bit().toBase64();
    }
    SearchConfig::instance()->setConfig(GRANDSEARCH_BLACKLIST_GROUP, GRANDSEARCH_BLACKLIST_PATH, QVariant::fromValue(blackList));
}

QModelIndex BlackListView::match(const QString &path) const
{
    qCDebug(logGrandSearch) << "Searching for path in blacklist - Path:" << path;
    for (int index = 0; index < m_model->rowCount(); ++index) {
        if (path == m_model->data(m_model->index(index, 0, QModelIndex()), DATAROLE)) {
            qCDebug(logGrandSearch) << "Path found in blacklist at row:" << index;
            return m_model->index(index, 0, QModelIndex());
    }
    }

    qCDebug(logGrandSearch) << "Path not found in blacklist:" << path;
    return QModelIndex();
}

void BlackListView::setData(const QModelIndex &index, const QString &path)
{
    if (!index.isValid()) {
        qCWarning(logGrandSearch) << "Invalid index provided for data setting";
        return;
    }

    qCDebug(logGrandSearch) << "Setting data for index - Row:" << index.row() << "Path:" << path;
    QVariant pathMeta;
    pathMeta.setValue(path);
    m_model->setData(index, pathMeta, DATAROLE);

    if (!path.isEmpty()) {
        QVariant iconMeta;
        static const QSize iconSize(IconSize, IconSize);
        static QIcon icon = QIcon::fromTheme("inode-directory");
        iconMeta.setValue(icon.pixmap(iconSize));
        m_model->setData(index, iconMeta, Qt::DecorationRole);

        m_validRows++;
        qCDebug(logGrandSearch) << "Data set successfully - Row:" << index.row() << "Valid rows:" << m_validRows;
    } else {
        qCDebug(logGrandSearch) << "Empty path set for row:" << index.row();
    }
}

void BlackListView::init()
{
    qCDebug(logGrandSearch) << "Initializing BlackListView";
    m_paths = SearchConfig::instance()->getConfig(GRANDSEARCH_BLACKLIST_GROUP, GRANDSEARCH_BLACKLIST_PATH, QStringList()).toStringList();
    for (QString path : m_paths) {
        path = QByteArray::fromBase64(path.toLocal8Bit());
        addRow(path);
    }

    m_validRows = m_paths.size();
    updateEmptyRows();
}

BlackListWrapper::BlackListWrapper(QWidget *parent)
    : DWidget(parent)
{
    qCDebug(logGrandSearch) << "BlackListWrapper constructor called";

    m_listView = new BlackListView(this);
    setFixedSize(MaxWidth, MaxHeight);
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->addWidget(m_listView);

    connect(m_listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &BlackListWrapper::selectedChanged);
}

BlackListWrapper::~BlackListWrapper()
{
    qCDebug(logGrandSearch) << "BlackListWrapper destructor called";
}

QItemSelectionModel *BlackListWrapper::selectionModel() const
{
    return m_listView->selectionModel();
}

void BlackListWrapper::removeRows(int row, int count)
{
    m_listView->removeRows(row, count);
}

void BlackListWrapper::clearSelection()
{
    qCDebug(logGrandSearch) << "BlackListWrapper clearing selection";
    m_listView->clearSelection();
}

void BlackListWrapper::addRow(const QString &path) const
{
    qCDebug(logGrandSearch) << "BlackListWrapper adding row - Path:" << path;
    m_listView->addRow(path);
}

void BlackListWrapper::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QColor color;
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
        color.setRgb(0, 0, 0, static_cast<int>(255 * 0.1));
    else
        color.setRgb(255, 255, 255, static_cast<int>(255 * 0.1));
    p.setPen(color);
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);
}
