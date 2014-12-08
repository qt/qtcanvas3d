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

#ifndef ARRAYUTILS_P_H
#define ARRAYUTILS_P_H

#include "typedarray/typedarray_p.h"
#include "typedarray/int8array_p.h"
#include "typedarray/uint8array_p.h"
#include "typedarray/int16array_p.h"
#include "typedarray/uint16array_p.h"
#include "typedarray/int32array_p.h"
#include "typedarray/uint32array_p.h"
#include "typedarray/float32array_p.h"
#include "typedarray/float64array_p.h"
#include "typedarray/uint8clampedarray_p.h"

QT_CANVAS3D_BEGIN_NAMESPACE

// Contains static template functions used by the TypedArray classes.
class ArrayUtils
{
public:
    template<typename T>
    static bool copyToTarget(T* targetArray, CanvasTypedArray *sourceArray, unsigned long length)
    {
        CanvasInt8Array *int8Source       = qobject_cast<CanvasInt8Array*>(sourceArray);
        CanvasUint8Array *uint8Source     = qobject_cast<CanvasUint8Array*>(sourceArray);
        CanvasUint8ClampedArray *uint8ClampedSource = qobject_cast<CanvasUint8ClampedArray*>(sourceArray);
        CanvasInt16Array *int16Source     = qobject_cast<CanvasInt16Array*>(sourceArray);
        CanvasUint16Array *uint16Source   = qobject_cast<CanvasUint16Array*>(sourceArray);
        CanvasInt32Array *int32Source     = qobject_cast<CanvasInt32Array*>(sourceArray);
        CanvasUint32Array *uint32Source   = qobject_cast<CanvasUint32Array*>(sourceArray);
        CanvasFloat32Array *float32Source = qobject_cast<CanvasFloat32Array*>(sourceArray);
        CanvasFloat64Array *float64Source = qobject_cast<CanvasFloat64Array*>(sourceArray);
        if (int8Source) {
            for (unsigned long idx = 0; idx < length; idx++) {
                targetArray[idx] = (T)(int8Source->get(idx));
            }
        } else if (uint8Source) {
            for (unsigned long idx = 0; idx < length; idx++) {
                targetArray[idx] = (T)(uint8Source->get(idx));
            }
        } else if (uint8ClampedSource) {
            for (unsigned long idx = 0; idx < length; idx++) {
                targetArray[idx] = (T)(uint8ClampedSource->get(idx));
            }
        } else if (int16Source) {
            for (unsigned long idx = 0; idx < length; idx++) {
                targetArray[idx] = (T)(int16Source->get(idx));
            }
        } else if (uint16Source) {
            for (unsigned long idx = 0; idx < length; idx++) {
                targetArray[idx] = (T)(uint16Source->get(idx));
            }
        } else if (int32Source) {
            for (unsigned long idx = 0; idx < length; idx++) {
                targetArray[idx] = (T)(int32Source->get(idx));
            }
        } else if (uint32Source) {
            for (unsigned long idx = 0; idx < length; idx++) {
                targetArray[idx] = (T)(uint32Source->get(idx));
            }
        } else if (float32Source) {
            for (unsigned long idx = 0; idx < length; idx++) {
                targetArray[idx] = (T)(float32Source->get(idx));
            }
        } else if (float64Source) {
            for (unsigned long idx = 0; idx < length; idx++) {
                targetArray[idx] = (T)(float64Source->get(idx));
            }
        } else {
            return false;
        }
        return true;
    }

    static bool copyToTargetClampedByte(unsigned char *targetArray, CanvasTypedArray *sourceArray,
                                        unsigned long length)
    {
        CanvasInt8Array *int8Source       = qobject_cast<CanvasInt8Array*>(sourceArray);
        CanvasUint8Array *uint8Source     = qobject_cast<CanvasUint8Array*>(sourceArray);
        CanvasUint8ClampedArray *uint8ClampedSource = qobject_cast<CanvasUint8ClampedArray*>(sourceArray);
        CanvasInt16Array *int16Source     = qobject_cast<CanvasInt16Array*>(sourceArray);
        CanvasUint16Array *uint16Source   = qobject_cast<CanvasUint16Array*>(sourceArray);
        CanvasInt32Array *int32Source     = qobject_cast<CanvasInt32Array*>(sourceArray);
        CanvasUint32Array *uint32Source   = qobject_cast<CanvasUint32Array*>(sourceArray);
        CanvasFloat32Array *float32Source = qobject_cast<CanvasFloat32Array*>(sourceArray);
        CanvasFloat64Array *float64Source = qobject_cast<CanvasFloat64Array*>(sourceArray);
        if (int8Source) {
            for (unsigned long idx = 0; idx < length; idx++) {
                targetArray[idx] = (unsigned char)(int8Source->get(idx));
            }
        } else if (uint8Source) {
            for (unsigned long idx = 0; idx < length; idx++) {
                targetArray[idx] = (unsigned char)(uint8Source->get(idx));
            }
        } else if (uint8ClampedSource) {
            for (unsigned long idx = 0; idx < length; idx++) {
                targetArray[idx] = uint8ClampedSource->get(idx);
            }
        } else if (int16Source) {
            for (unsigned long idx = 0; idx < length; idx++) {
                short value = int16Source->get(idx);
                if (value < 0)
                    value = 0;
                if (value > 255)
                    value = 255;
                targetArray[idx] = (unsigned char) value;
            }
        } else if (uint16Source) {
            for (unsigned long idx = 0; idx < length; idx++) {
                unsigned short value = uint16Source->get(idx);
                if (value > 255)
                    value = 255;
                targetArray[idx] = (unsigned char) value;
            }
        } else if (int32Source) {
            for (unsigned long idx = 0; idx < length; idx++) {
                int value = int32Source->get(idx);
                if (value < 0)
                    value = 0;
                if (value > 255)
                    value = 255;
                targetArray[idx] = (unsigned char) value;
            }
        } else if (uint32Source) {
            for (unsigned long idx = 0; idx < length; idx++) {
                unsigned int value = uint32Source->get(idx);
                if (value > 255)
                    value = 255;
                targetArray[idx] = (unsigned char) value;
            }
        } else if (float32Source) {
            for (unsigned long idx = 0; idx < length; idx++) {
                float value = float32Source->get(idx);
                if (value < 0)
                    value = 0;
                if (value > 255)
                    value = 255;
                targetArray[idx] = (unsigned char) value;
            }
        } else if (float64Source) {
            for (unsigned long idx = 0; idx < length; idx++) {
                double value = float64Source->get(idx);
                if (value < 0)
                    value = 0;
                if (value > 255)
                    value = 255;
                targetArray[idx] = (unsigned char) value;
            }
        } else {
            return false;
        }
        return true;
    }

    static void fillFloatArrayFromVariantList(const QVariantList  &list, float *outArray)
    {
        int idx = 0;
        for (QVariantList::const_iterator it = list.begin(); it != list.end(); ++it) {
            QVariant element = *it;
            if (element.canConvert<float>()) {
                outArray[idx] = element.toFloat();
            }
            else {
                // Conversion failed, use 0.0 as default value
                outArray[idx] = 0.f;
            }
            idx++;
        }
    }

    static void fillIntArrayFromVariantList(const QVariantList  &list, int *outArray)
    {
        int idx = 0;
        for (QVariantList::const_iterator it = list.begin(); it != list.end(); ++it) {
            QVariant element = *it;
            if (element.canConvert<int>()) {
                outArray[idx] = element.toInt();
            }
            else {
                // Conversion failed, use 0.0 as default value
                outArray[idx] = 0;
            }
            idx++;
        }
    }
};

QT_CANVAS3D_END_NAMESPACE

#endif // ARRAYUTILS_P_H
