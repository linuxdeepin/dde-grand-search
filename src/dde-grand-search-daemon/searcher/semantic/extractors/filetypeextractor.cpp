// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filetypeextractor.h"
#include "searcher/semantic/intentparser.h"
#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

void FileTypeExtractor::extract(const QString &input, ParsedIntent &intent)
{
    // 使用 matchAll 支持多文件类型组合（如"照片和视频"）
    QList<RegexRule> matchedRules = m_engine->matchAll("filetype", input);

    for (const RegexRule &rule : matchedRules) {
        if (rule.metadata.contains("extensions")) {
            QStringList extensions = rule.metadata["extensions"].toStringList();
            for (const QString &ext : extensions) {
                if (!intent.fileTypes.contains(ext)) {
                    intent.fileTypes.append(ext);
                }
            }

            // 记录已消费区间
            QRegularExpressionMatch match = rule.pattern.match(input);
            if (match.hasMatch()) {
                intent.consumedSpans.append(MatchSpan(match.capturedStart(), match.capturedEnd(), rule.id));
            }
            qCDebug(logDaemon) << "Detected file type:" << rule.id
                     << "extensions:" << extensions;
        }
    }
}
