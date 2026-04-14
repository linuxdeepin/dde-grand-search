// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ruleconfigloader.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

// 本地配置子目录（相对于 ~/.config/deepin/）
static const char kLocalConfigSubDir[] = "dde-grand-search-daemon/semantic";

// qrc 内置规则目录前缀
static const char kQrcRulesPrefix[] = ":/rules";

RuleConfigLoader::RuleConfigLoader(QObject *parent)
    : QObject(parent)
{
    // 构建本地配置目录路径：~/.config/deepin/dde-grand-search-daemon/semantic/
    const QString configRoot = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    m_localConfigDir = configRoot + "/deepin/" + kLocalConfigSubDir;
    m_qrcPrefix = kQrcRulesPrefix;
}

RuleConfigLoader::~RuleConfigLoader() = default;

QString RuleConfigLoader::resolveRulePath(RuleType type) const
{
    // 优先级：本地配置 > qrc 内置配置
    const QString localPath = localRulePath(type);

    if (QFileInfo::exists(localPath) && validateRuleFile(localPath)) {
        qCDebug(logDaemon) << "Using local rule file:" << localPath;
        return localPath;
    }

    // 本地不存在或无效，回退到 qrc
    const QString qrcPath = qrcRulePath(type);
    qCDebug(logDaemon) << "Using built-in rule file:" << qrcPath;
    return qrcPath;
}

QStringList RuleConfigLoader::resolveAllRulePaths() const
{
    QStringList paths;
    const auto types = allRuleTypes();
    for (RuleType type : types) {
        paths.append(resolveRulePath(type));
    }
    return paths;
}

bool RuleConfigLoader::ensureLocalConfig()
{
    if (!ensureLocalDir()) {
        emit localConfigInitialized(false);
        return false;
    }

    bool allSuccess = exportAllToLocal(false);
    emit localConfigInitialized(allSuccess);
    return allSuccess;
}

bool RuleConfigLoader::exportToLocal(RuleType type, bool overwrite)
{
    const QString localPath = localRulePath(type);
    const QString qrcPath = qrcRulePath(type);

    // 如果不覆盖且本地已存在，跳过
    if (!overwrite && QFileInfo::exists(localPath)) {
        qCDebug(logDaemon) << "Local rule file already exists, skipping:" << localPath;
        return true;
    }

    if (!ensureLocalDir()) {
        return false;
    }

    // 从 qrc 读取内容
    QFile qrcFile(qrcPath);
    if (!qrcFile.open(QIODevice::ReadOnly)) {
        qCWarning(logDaemon) << "Cannot open built-in rule file:" << qrcPath;
        return false;
    }

    QByteArray data = qrcFile.readAll();
    qrcFile.close();

    // 写入本地文件
    QFile localFile(localPath);
    if (!localFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCWarning(logDaemon) << "Cannot write local rule file:" << localPath
                             << "-" << localFile.errorString();
        return false;
    }

    localFile.write(data);
    localFile.close();

    qCDebug(logDaemon) << "Exported rule file to local:" << localPath;
    return true;
}

bool RuleConfigLoader::exportAllToLocal(bool overwrite)
{
    bool allSuccess = true;
    const auto types = allRuleTypes();
    for (RuleType type : types) {
        if (!exportToLocal(type, overwrite)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

bool RuleConfigLoader::validateRuleFile(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    // 空文件视为无效
    if (data.isEmpty()) {
        emit const_cast<RuleConfigLoader *>(this)->ruleFileInvalid(
            filePath, tr("Rule file is empty"));
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit const_cast<RuleConfigLoader *>(this)->ruleFileInvalid(
            filePath,
            tr("JSON parse error: %1 (at position: %2)")
                .arg(parseError.errorString())
                .arg(parseError.offset));
        return false;
    }

    if (!doc.isObject()) {
        emit const_cast<RuleConfigLoader *>(this)->ruleFileInvalid(
            filePath, tr("Root element must be a JSON object"));
        return false;
    }

    QJsonObject root = doc.object();

    // 必须包含 version 和 groups 字段
    if (!root.contains("version") || !root.contains("groups")) {
        emit const_cast<RuleConfigLoader *>(this)->ruleFileInvalid(
            filePath, tr("Missing required fields: version, groups"));
        return false;
    }

    // groups 必须是数组且非空
    QJsonArray groups = root["groups"].toArray();
    if (groups.isEmpty()) {
        emit const_cast<RuleConfigLoader *>(this)->ruleFileInvalid(
            filePath, tr("Groups array is empty"));
        return false;
    }

    return true;
}

QString RuleConfigLoader::localConfigDir() const
{
    return m_localConfigDir;
}

QString RuleConfigLoader::qrcConfigPrefix() const
{
    return m_qrcPrefix;
}

QString RuleConfigLoader::ruleFileName(RuleType type)
{
    switch (type) {
    case Noise:
        return QStringLiteral("noise_rules.json");
    case Keyword:
        return QStringLiteral("keyword_rules.json");
    case Time:
        return QStringLiteral("time_rules.json");
    case FileType:
        return QStringLiteral("filetype_rules.json");
    }
    return QString();
}

QList<RuleConfigLoader::RuleType> RuleConfigLoader::allRuleTypes()
{
    return { Noise, Keyword, Time, FileType };
}

QString RuleConfigLoader::qrcRulePath(RuleType type) const
{
    return m_qrcPrefix + "/" + ruleFileName(type);
}

QString RuleConfigLoader::localRulePath(RuleType type) const
{
    return m_localConfigDir + "/" + ruleFileName(type);
}

bool RuleConfigLoader::ensureLocalDir() const
{
    QDir dir(m_localConfigDir);
    if (dir.exists()) {
        return true;
    }

    if (!dir.mkpath(".")) {
        qCWarning(logDaemon) << "Cannot create local config directory:" << m_localConfigDir;
        return false;
    }

    qCDebug(logDaemon) << "Created local config directory:" << m_localConfigDir;
    return true;
}
