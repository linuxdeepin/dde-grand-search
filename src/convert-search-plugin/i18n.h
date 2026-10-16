// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef I18N_H
#define I18N_H

#include <QString>
#include <QLocale>

// 集中式国际化辅助：所有转换结果的「说明文字」走这里，
// 跟随系统语言返回本地化文本，避免各 provider 各自维护 isChineseLocale。
// 当前支持：中文(zh)、英文(en)；其余语言回退英文。
namespace I18n {

// 判定是否为中文环境
inline bool isChinese()
{
    return QLocale().language() == QLocale::Chinese;
}

// 本地化单位/代码说明：张「原值（本地化名）」格式，非中文显示英文全称。
// 例：zh -> "km（千米/公里）"  en -> "km (kilometer)"
QString unitName(const QString &key);

// 本地化货币名：zh -> "CNY（人民币）"  en -> "CNY (Chinese Yuan)"
QString currencyName(const QString &code);

// 本地化杂项标签（进制/颜色等）：key 如 HEX/BIN/OCT/DEC/RGB/HSL
QString miscName(const QString &key);

// 本地化分组名：key 如 currency/unit/time/calculator/developer/date/color
QString groupName(const QString &key);

} // namespace I18n

#endif // I18N_H
