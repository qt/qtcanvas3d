/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the test suite of the Qt Toolkit.
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

import QtQuick 2.2
import QtCanvas3D 1.0
import QtTest 1.0

Item {
    id: top
    height: 150
    width: 150

    Canvas3D {
        id: empty
    }

    Canvas3D {
        id: log_only_calls
        logAllCalls: true
        logAllErrors: false
    }

    Canvas3D {
        id: log_only_errors
        logAllErrors: true
    }

    Canvas3D {
        id: basic_context
        anchors.fill: parent
        function initGL() {}
        function renderGL() {}
    }

    Canvas3D {
        id: depth_context
        anchors.fill: parent
        function initGL() {}
        function renderGL() {}
    }

    Canvas3D {
        id: aa_context
        anchors.fill: parent
        function initGL() {}
        function renderGL() {}
    }

    TestCase {
        name: "Canvas3D"
        when: windowShown

        function test_empty() {
            compare(empty.width, 0)
            compare(empty.height, 0)
            compare(empty.logAllCalls, false)
            compare(empty.logAllErrors, false)
            compare(empty.context, null)
        }

        function test_log_only_calls() {
            compare(log_only_calls.logAllCalls, true)
            compare(log_only_calls.logAllErrors, false)
            log_only_calls.logAllCalls = false
        }

        function log_only_errors() {
            compare(no_logs.logAllCalls, false)
            compare(no_logs.logAllErrors, true)
        }

        function test_basic_context() {
            waitForRendering(basic_context)
            basic_context.getContext("basic")
            compare(basic_context.context.canvas, basic_context)
        }

        function test_depth_context() {
            waitForRendering(depth_context)
            depth_context.getContext("depth", {depth:true})
            compare(depth_context.context.canvas, depth_context)
        }

        function test_aa_context() {
            waitForRendering(aa_context)
            aa_context.getContext("antialias", {antialias:true})
            compare(aa_context.context.canvas, aa_context)
        }
    }
}
