/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtCanvas3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.2
import QtCanvas3D 1.0
import QtTest 1.0

import "tst_conformance_attribs.js" as Content

// Covers the following WebGL conformance attribute tests:
// gl-bindAttribLocation-aliasing.html
// gl-bindAttribLocation-matrix.html
// gl-disabled-vertex-attrib.html
// gl-enable-vertex-attrib.html
// gl-matrix-attributes.html
// gl-vertex-attrib-render.html
// gl-vertex-attrib-zero-issues.html
// gl-vertex-attrib.html
// gl-vertexattribpointer-offsets.html
// gl-vertexattribpointer.html

// Doesn't cover the following attribute tests:
// -

Item {
    id: top
    height: 200
    width: 200

    property var canvas3d: null
    property var canvas3d2: null
    property var activeContent: Content
    property bool renderOk: false
    property bool antialiasOn: false
    property bool depthOn: false
    property var vertexShader: null
    property var fragmentShader: null
    property var testfunction: null
    property bool testresult: false

    function createCanvas(antialias, depth, vertex, fragment, w, h) {
        // reset state flags
        renderOk = false
        testresult = false
        antialiasOn = antialias
        depthOn = depth
        vertexShader = vertex
        fragmentShader = fragment
        var width = w
        var height = h
        canvas3d = Qt.createQmlObject("
        import QtQuick 2.2
        import QtCanvas3D 1.0
        Canvas3D {
            onInitGL: activeContent.initGL(canvas3d, antialiasOn, depthOn, vertexShader,
                                           fragmentShader, width, height, testfunction)
            onRenderGL: {
                if (!renderOk) // Render only once
                    testresult = activeContent.renderGL(canvas3d)
                renderOk = true
            }
        }", top)
        canvas3d.width = w
        canvas3d.height = h
    }

    function createCanvas2(w, h) {
        var width = w
        var height = h
        canvas3d2 = Qt.createQmlObject("
        import QtQuick 2.2
        import QtCanvas3D 1.0
        Canvas3D {
            onInitGL: activeContent.initGL2(canvas3d2, width, height)
        }", top)
        canvas3d2.anchors.fill = top
    }

    /*
    ** Copyright (c) 2012 The Khronos Group Inc.
    **
    ** Permission is hereby granted, free of charge, to any person obtaining a
    ** copy of this software and/or associated documentation files (the
    ** "Materials"), to deal in the Materials without restriction, including
    ** without limitation the rights to use, copy, modify, merge, publish,
    ** distribute, sublicense, and/or sell copies of the Materials, and to
    ** permit persons to whom the Materials are furnished to do so, subject to
    ** the following conditions:
    **
    ** The above copyright notice and this permission notice shall be included
    ** in all copies or substantial portions of the Materials.
    **
    ** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    ** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    ** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    ** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    ** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    ** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    ** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
    */

    // gl-bindAttribLocation-aliasing.html
    // QTBUG-44955
    /*
    TestCase {
        // This test verifies combinations of valid, active attribute types cannot be bound to the same location with bindAttribLocation.
        name: "Canvas3D_conformance_bindAttribLocation_aliasing"
        when: windowShown

        function test_bindAttribLocation_aliasing() {
            testfunction = Content.testbindattriblocationaliasing
            createCanvas(false, false,
                         null,
                         activeContent.simpleColorFragmentShader,
                         8, 8)
            waitForRendering(canvas3d)
            tryCompare(top, "renderOk", true)
            verify(testresult)
            canvas3d.destroy()
        }
    }
    */

    // gl-bindAttribLocation-matrix.html
    // QTBUG-44955
    /*
    TestCase {
        // This test verifies that vectors placed via bindAttribLocation right after matricies will fail if there is insufficient room for the matrix.
        name: "Canvas3D_conformance_bindAttribLocation_matrix"
        when: windowShown

        function test_bindAttribLocation_matrix() {
            testfunction = Content.testbindattriblocationmatrix
            createCanvas(false, false,
                         activeContent.simpleColorVertexShader,
                         activeContent.simpleColorFragmentShader,
                         8, 8)
            waitForRendering(canvas3d)
            tryCompare(top, "renderOk", true)
            verify(testresult)
            canvas3d.destroy()
        }
    }
    */

    // gl-disabled-vertex-attrib.html
    // QTBUG-45075
    // Readpixels returns the correct value only for the pixels on the first row for some reason.
    // Might be an issue with two contexts that are required to run this.
    /*
    TestCase {
        // This test verifies that vectors placed via bindAttribLocation right after matricies will fail if there is insufficient room for the matrix.
        name: "Canvas3D_conformance_disabled_vertex_attrib"
        when: windowShown

        function test_disabled_vertex_attrib() {
            createCanvas2(50, 50)
            waitForRendering(canvas3d2)
            testfunction = Content.testdisabledvertexattrib
            createCanvas(false, false, null, null, 50, 50)
            waitForRendering(canvas3d)
            tryCompare(top, "renderOk", true)
            verify(testresult)
            canvas3d.destroy()
            canvas3d2.destroy()
        }
    }
    */

    // gl-enable-vertex-attrib.html
    // QTBUG-45074
    /*
    TestCase {
        // This tests that turning on attribs that have no buffer bound fails to draw.
        name: "Canvas3D_conformance_enable_vertex_attrib"
        when: windowShown

        function test_enable_vertex_attrib() {
            testfunction = Content.testenablevertexattrib
            createCanvas(false, false, null, null, 50, 50)
            waitForRendering(canvas3d)
            tryCompare(top, "renderOk", true)
            verify(testresult)
            canvas3d.destroy()
        }
    }
    */

    // gl-matrix-attributes.html
    TestCase {
        // This test ensures that matrix attribute locations do not clash with other shader attributes.
        name: "Canvas3D_conformance_matrix_attributes"
        when: windowShown

        function test_matrix_attributes() {
            testfunction = Content.testmatrixattributes
            createCanvas(false, false, null, null, 8, 8)
            waitForRendering(canvas3d)
            tryCompare(top, "renderOk", true)
            verify(testresult)
            canvas3d.destroy()
        }
    }

    // gl-vertex-attrib-render.html
    // QTBUG-45126 (Only fails on Windows Angle tests)
    /*
    TestCase {
        // This test verifies that using constant attributes works.
        name: "Canvas3D_conformance_vertex_attrib_render"
        when: windowShown

        function test_vertex_attrib_render() {
            testfunction = Content.testvertexattribrender
            createCanvas(false, true, null, null, 50, 50)
            waitForRendering(canvas3d)
            tryCompare(top, "renderOk", true)
            verify(testresult)
            canvas3d.destroy()
        }
    }
    */

    // gl-vertex-attrib-zero-issues.html
    // QTBUG-45175
    /*
    TestCase {
        //  Test some of the issues of the difference between attrib 0 on OpenGL vs WebGL.
        name: "Canvas3D_conformance_vertex_attrib_zero_issues"
        when: windowShown

        function test_vertex_attrib_zero_issues() {
            testfunction = Content.testvertexattribzeroissues
            createCanvas(false, false, null, null, 50, 50)
            waitForRendering(canvas3d)
            tryCompare(top, "renderOk", true)
            verify(testresult)
            canvas3d.destroy()
        }
    }
    */

    // gl-vertex-attrib.html
    // QTBUG-45043 (vertexAttrib...fv doesn't work). Failing parts are commented out in tst_conformance_attribs.js
    // QTBUG-45175
    /*
    TestCase {
        // This test ensures WebGL implementations vertexAttrib can be set and read.
        name: "Canvas3D_conformance_vertex_attrib"
        when: windowShown

        function test_vertex_attrib() {
            testfunction = Content.testvertexattrib
            createCanvas(false, false, null, null, 2, 2)
            waitForRendering(canvas3d)
            tryCompare(top, "renderOk", true)
            verify(testresult)
            canvas3d.destroy()
        }
    }
    */

    // gl-vertexattribpointer-offsets.html
    TestCase {
        // This test ensures vertexattribpointer offsets work.
        name: "Canvas3D_conformance_vertex_attrib_pointer_offsets"
        when: windowShown

        function test_vertex_attrib_pointer_offsets() {
            testfunction = Content.testvertexattribpointeroffsets
            createCanvas(false, false, null, null, 50, 50)
            waitForRendering(canvas3d)
            tryCompare(top, "renderOk", true)
            verify(testresult)
            canvas3d.destroy()
        }
    }

    // gl-vertexattribpointer.html
    TestCase {
        // This test checks vertexAttribPointer behaviors in WebGL.
        name: "Canvas3D_conformance_vertex_attrib_pointer"
        when: windowShown

        function test_vertex_attrib_pointer() {
            testfunction = Content.testvertexattribpointer
            createCanvas(false, false, null, null, 2, 2)
            waitForRendering(canvas3d)
            tryCompare(top, "renderOk", true)
            verify(testresult)
            canvas3d.destroy()
        }
    }
}
