// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deletedialog.h"

#include <QAbstractButton>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

using namespace GrandSearch;

DeleteDialog::DeleteDialog(QWidget *parent)
    : DDialog(parent)
{
    qCDebug(logGrandSearch) << "DeleteDialog constructor called - Parent:" << (parent ? parent->objectName() : "nullptr");

    const QString content(tr("Do you want to remove the path from the exclusion list?"));
    setMessage(content);
    qCDebug(logGrandSearch) << "Dialog message set:" << content;

    QString iconPath = QString(":/icons/%1.svg").arg("dde-grand-search-setting");
    setIcon(QIcon(iconPath));
    qCDebug(logGrandSearch) << "Dialog icon set - Path:" << iconPath;

    // the cancel button
    {
        int idx = addButton(tr("Cancel", "button"));
        auto btn = getButton(idx);
        if (btn) {
        connect(btn, &QAbstractButton::clicked, this, &QDialog::reject);
            qCDebug(logGrandSearch) << "Cancel button created and connected - Index:" << idx;
        } else {
            qCWarning(logGrandSearch) << "Failed to get cancel button - Index:" << idx;
    }
    }

    // the confirm button
    {
        auto idx = addButton(tr("Confirm", "button"), false, ButtonWarning);
        auto btn = getButton(idx);
        if (btn) {
        connect(btn, &QAbstractButton::clicked, this, &QDialog::accept);
            qCDebug(logGrandSearch) << "Confirm button created and connected - Index:" << idx;
        } else {
            qCWarning(logGrandSearch) << "Failed to get confirm button - Index:" << idx;
    }
}

    qCInfo(logGrandSearch) << "DeleteDialog initialized successfully";
}

DeleteDialog::~DeleteDialog()
{
    qCDebug(logGrandSearch) << "DeleteDialog destructor called";
}
