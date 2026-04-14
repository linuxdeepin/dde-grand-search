// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILETYPEEXTRACTOR_H
#define FILETYPEEXTRACTOR_H

#include "dimensionextractor.h"
#include "searcher/semantic/semanticruleengine.h"

/**
 * @brief 文件类型维度提取器
 *
 * 从输入文本中提取文件类型约束，支持多类型组合匹配。
 */
class FileTypeExtractor : public DimensionExtractor
{
public:
    explicit FileTypeExtractor(SemanticRuleEngine *engine)
        : DimensionExtractor(engine) { }

    void extract(const QString &input, ParsedIntent &intent) override;
    QString name() const override { return "FileTypeExtractor"; }
};

#endif   // FILETYPEEXTRACTOR_H
