/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef ACINTELFUNCTIONS_H
#define ACINTELFUNCTIONS_H

#ifdef ENABLE_ACCESSIBILITY
    #define AC_SET_ACCESSIBLE_NAME(classObj,accessiblename) classObj->setAccessibleName(accessiblename);
    #define AC_SET_ACCESSIBLE_TEXT(classObj,accessibletext) classObj->setAccessibleText(accessibletext);
    #define AC_SET_OBJECT_NAME(classObj,objectname)
#else
    #define AC_SET_ACCESSIBLE_NAME(classObj,accessiblename)
    #define AC_SET_ACCESSIBLE_TEXT(classObj,accessibletext)
    #define AC_SET_OBJECT_NAME(classObj,objectname)
#endif

#endif // ACINTELFUNCTIONS_H
