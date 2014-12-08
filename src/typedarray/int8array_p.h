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

#ifndef INT8ARRAY_P_H
#define INT8ARRAY_P_H

#include "typedarray_p.h"

QT_CANVAS3D_BEGIN_NAMESPACE

class CanvasInt8Array : public CanvasTypedArray
{
    Q_OBJECT
public:
    explicit CanvasInt8Array(QObject *parent = 0);
    explicit CanvasInt8Array(unsigned long length, QObject *parent = 0);
    explicit CanvasInt8Array(CanvasTypedArray *array, QObject *parent = 0);
    explicit CanvasInt8Array(QVariantList *array, QObject *parent = 0);
    explicit CanvasInt8Array(CanvasArrayBuffer *buffer, unsigned long byteOffset,
                             QObject *parent = 0);

    Q_INVOKABLE signed char operator [] (unsigned long index);
    Q_INVOKABLE signed char get(unsigned long index);
    Q_INVOKABLE void set(unsigned long index, signed char value);
    virtual inline int bytesPerElement() { return sizeof(signed char); }
    virtual inline CanvasContext::glEnums dataType() { return CanvasContext::BYTE; }
    virtual inline canvasArrayBufferTypes bufferType() { return CanvasArrayBufferView::ARRAY_BUFFER_INT_8; }

    //Q_INVOKABLE void set(TypedArray *array, unsigned long offset);
    //Q_INVOKABLE void set(signed char *array, unsigned long offset);
    //Q_INVOKABLE TypedArray *subarray(long begin);
    //Q_INVOKABLE TypedArray *subarray(long begin, long end);

private:
    signed char *m_typedBuffer;
};

QT_CANVAS3D_END_NAMESPACE

#endif // INT8ARRAY_P_H
