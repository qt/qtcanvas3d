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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtCanvas3D API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef TYPEDARRAYFACTORY_P_H
#define TYPEDARRAYFACTORY_P_H

#include "../canvas3dcommon_p.h"
#include <QObject>
#include <QVariantList>
#include <QQmlEngine>
#include <QJSEngine>

QT_CANVAS3D_BEGIN_NAMESPACE

class CanvasInt8Array;
class CanvasUint8Array;
class CanvasInt16Array;
class CanvasUint16Array;
class CanvasInt32Array;
class CanvasUint32Array;
class CanvasFloat32Array;
class CanvasFloat64Array;
class CanvasUint8ClampedArray;
class CanvasArrayBuffer;
class CanvasTypedArray;
class CanvasTextureImage;

class CanvasTypedArrayFactory : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CanvasTypedArrayFactory)

public:
    static QObject *type_array_factory_provider(QQmlEngine *engine, QJSEngine *scriptEngine);

    explicit CanvasTypedArrayFactory(QObject *parent = 0);
    ~CanvasTypedArrayFactory();

    /* Typed array creation functions. Typed arrays should be in the V4VM implementation.*/
    Q_INVOKABLE CanvasInt8Array* newInt8Array(unsigned long length);
    Q_INVOKABLE CanvasInt8Array* newInt8Array(CanvasTypedArray *array);
    Q_INVOKABLE CanvasInt8Array* newInt8Array(QVariantList array);
    Q_INVOKABLE CanvasInt8Array* newInt8Array(CanvasArrayBuffer *buffer,
                                              unsigned long byteOffset);

    Q_INVOKABLE CanvasUint8Array* newUint8Array(unsigned long length);
    Q_INVOKABLE CanvasUint8Array* newUint8Array(CanvasTypedArray *array);
    Q_INVOKABLE CanvasUint8Array* newUint8Array(QVariantList array);
    Q_INVOKABLE CanvasUint8Array* newUint8Array(CanvasArrayBuffer *buffer,
                                                unsigned long byteOffset);

    Q_INVOKABLE CanvasInt16Array* newInt16Array(unsigned long length);
    Q_INVOKABLE CanvasInt16Array* newInt16Array(CanvasTypedArray *array);
    Q_INVOKABLE CanvasInt16Array* newInt16Array(QVariantList array);
    Q_INVOKABLE CanvasInt16Array* newInt16Array(CanvasArrayBuffer *buffer,
                                                unsigned long byteOffset);

    Q_INVOKABLE CanvasUint16Array* newUint16Array(unsigned long length);
    Q_INVOKABLE CanvasUint16Array* newUint16Array(CanvasTypedArray *array);
    Q_INVOKABLE CanvasUint16Array* newUint16Array(QVariantList array);
    Q_INVOKABLE CanvasUint16Array* newUint16Array(CanvasArrayBuffer *buffer,
                                                  unsigned long byteOffset);

    Q_INVOKABLE CanvasInt32Array* newInt32Array(unsigned long length);
    Q_INVOKABLE CanvasInt32Array* newInt32Array(CanvasTypedArray *array);
    Q_INVOKABLE CanvasInt32Array* newInt32Array(QVariantList array);
    Q_INVOKABLE CanvasInt32Array* newInt32Array(CanvasArrayBuffer *buffer,
                                                unsigned long byteOffset);

    Q_INVOKABLE CanvasUint32Array* newUint32Array(unsigned long length);
    Q_INVOKABLE CanvasUint32Array* newUint32Array(CanvasTypedArray *array);
    Q_INVOKABLE CanvasUint32Array* newUint32Array(QVariantList array);
    Q_INVOKABLE CanvasUint32Array* newUint32Array(CanvasArrayBuffer *buffer,
                                                  unsigned long byteOffset);

    Q_INVOKABLE CanvasFloat32Array* newFloat32Array(unsigned long length);
    Q_INVOKABLE CanvasFloat32Array* newFloat32Array(CanvasTypedArray *array);
    Q_INVOKABLE CanvasFloat32Array* newFloat32Array(QVariantList array);
    Q_INVOKABLE CanvasFloat32Array* newFloat32Array(CanvasArrayBuffer *buffer,
                                                    unsigned long byteOffset);

    Q_INVOKABLE CanvasFloat64Array* newFloat64Array(unsigned long length);
    Q_INVOKABLE CanvasFloat64Array* newFloat64Array(CanvasTypedArray *array);
    Q_INVOKABLE CanvasFloat64Array* newFloat64Array(QVariantList array);
    Q_INVOKABLE CanvasFloat64Array* newFloat64Array(CanvasArrayBuffer *buffer,
                                                    unsigned long byteOffset);

    Q_INVOKABLE CanvasUint8ClampedArray* newUint8ClampedArray(unsigned long length);
    Q_INVOKABLE CanvasUint8ClampedArray* newUint8ClampedArray(CanvasTypedArray *array);
    Q_INVOKABLE CanvasUint8ClampedArray* newUint8ClampedArray(QVariantList array);
    Q_INVOKABLE CanvasUint8ClampedArray* newUint8ClampedArray(CanvasArrayBuffer *buffer,
                                                              unsigned long byteOffset);
};

QT_CANVAS3D_END_NAMESPACE

#endif // TYPEDARRAYFACTORY_P_H
