/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtCanvas3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free 
** Software Foundation and appearing in the file LICENSE.GPL included in 
** the packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "shaderprecisionformat_p.h"

QT_CANVAS3D_BEGIN_NAMESPACE

/*!
 * \qmltype ShaderPrecisionFormat
 * \since QtCanvas3D 1.0
 * \ingroup qtcanvas3d-qml-types
 * \brief Contains the shader precision format attributes.
 *
 * An uncreatable QML type that contains the information returned from the
 * \l{Context3D::getShaderPrecisionFormat()} call.
 *
 * \sa Context3D, {QML Canvas 3D QML Types}
 */

/*!
 * \internal
 */
CanvasShaderPrecisionFormat::CanvasShaderPrecisionFormat(QObject *parent) :
    CanvasAbstractObject(parent)
{
}

/*!
 * \qmlproperty int ShaderPrecisionFormat::rangeMin
 * The base 2 log of the absolute value of the minimum value that can be represented.
 */

/*!
 * \qmlproperty int ShaderPrecisionFormat::rangeMax
 * The base 2 log of the absolute value of the maximum value that can be represented.
 */

/*!
 * \qmlproperty int ShaderPrecisionFormat::precision
 * The number of bits of precision that can be represented. 0 for integer formats.
 */

QT_CANVAS3D_END_NAMESPACE
