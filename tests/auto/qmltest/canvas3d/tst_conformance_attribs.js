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

Qt.include("../../../../3rdparty/js-test-pre.js")
Qt.include("../../../../3rdparty/webgl-test-utils.js")
Qt.include("../../../../3rdparty/gl-matrix.js")

"use strict";

var gl, gl2;
var width, height;

var testfunction;

var glVertexShader, glFragmentShader;
var vertexshader, fragmentshader;

var maxVertexAttributes;

function initGL(canvas, antialias, depth, vertex, fragment, w, h, test) {
    gl = canvas.getContext("attributeTests", {depth:depth, antialias:antialias});
    gl.viewport(0, 0, w, h);
    gl.clearColor(0, 0, 0, 1);
    gl.clear(gl.COLOR_BUFFER_BIT);

    initShaders(vertex, fragment);

    width = w;
    height = h;

    maxVertexAttributes = gl.getParameter(gl.MAX_VERTEX_ATTRIBS);

    testfunction = test;
}

function renderGL(canvas) {
    return testfunction();
}

function initGL2(canvas, w, h) {
    gl2 = canvas.getContext("attributeTests");
    gl2.viewport(0, 0, w, h);
    gl2.clearColor(0, 0, 0, 1);
    gl2.clear(gl2.COLOR_BUFFER_BIT);

    width = w;
    height = h;
}

function initShaders(vertex, fragment)
{
    vertexshader = vertex;
    fragmentshader = fragment;
    if (vertex)
        glVertexShader = compileShader(vertex, gl.VERTEX_SHADER);
    if (fragment)
        glFragmentShader = compileShader(fragment, gl.FRAGMENT_SHADER);
}

function compileShader(str, type) {
    var shader = gl.createShader(type);
    gl.shaderSource(shader, str);
    gl.compileShader(shader);

    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        console.log("JS:Shader compile failed");
        console.log(gl.getShaderInfoLog(shader));
        return null;
    }

    return shader;
}

// WebGL Conformance tests

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

//
// gl-bindAttribLocation-aliasing.html
//
function testbindattriblocationaliasing() {
    var typeInfo = [
                { type: 'float', asVec4: 'vec4(0.0, $(var), 0.0, 1.0)' },
                { type: 'vec2', asVec4: 'vec4($(var), 0.0, 1.0)' },
                { type: 'vec3', asVec4: 'vec4($(var), 1.0)' },
                { type: 'vec4', asVec4: '$(var)' },
            ];
    var retval = true;
    // Test all type combinations of a_1 and a_2.
    typeInfo.forEach(function(typeInfo1) {
        typeInfo.forEach(function(typeInfo2) {
            output('attribute_1: ' + typeInfo1.type + ' attribute_2: ' + typeInfo2.type);
            var replace = {
                type_1: typeInfo1.type,
                type_2: typeInfo2.type,
                gl_Position_1: testbindattriblocationaliasing_replaceParams(typeInfo1.asVec4, {var: 'a_1'}),
                gl_Position_2: testbindattriblocationaliasing_replaceParams(typeInfo2.asVec4, {var: 'a_2'})
            };
            var strVertexShader = testbindattriblocationaliasing_replaceParams(
                        replaceAttribVertexShader, replace);
            glVertexShader = compileShader(strVertexShader, gl.VERTEX_SHADER);
            retval = assertMsg(glVertexShader !== null, "Vertex shader compiled successfully.");
            if (!retval) return false;
            // Bind both a_1 and a_2 to the same position and verify the link fails.
            // Do so for all valid positions available.
            for (var l = 0; l < maxVertexAttributes; l++) {
                var glProgram = gl.createProgram();
                gl.bindAttribLocation(glProgram, l, 'a_1');
                gl.bindAttribLocation(glProgram, l, 'a_2');
                gl.attachShader(glProgram, glVertexShader);
                gl.attachShader(glProgram, glFragmentShader);
                gl.linkProgram(glProgram);
                retval = assertMsg(!gl.getProgramParameter(glProgram, gl.LINK_STATUS),
                                   "Link should fail when both types are aliased to location " + l);
                if (!retval) return false;
            }
        });
    });
    return retval;
}

var replaceAttribVertexShader = [
            'precision mediump float;',
            'attribute $(type_1) a_1;',
            'attribute $(type_2) a_2;',
            'void main() {',
            'gl_Position = $(gl_Position_1) + $(gl_Position_2);',
            '}'].join('\n');

var testbindattriblocationaliasing_replaceParams = function(str) {
    var args = arguments;
    return str.replace(testbindattriblocationaliasing_replaceRE, function(str, p1, offset, s) {
        for (var ii = 1; ii < args.length; ++ii) {
            if (args[ii][p1] !== undefined) {
                return args[ii][p1];
            }
        }
        throw "unknown string param '" + p1 + "'";
    });
};

var testbindattriblocationaliasing_replaceRE = /\$\((\w+)\)/g;

//
// gl-bindAttribLocation-matrix.html
//
function testbindattriblocationmatrix_loadVertexShader(numMatrixDimensions) {
    var strVertexShader =
            'attribute mat' + numMatrixDimensions + ' matrix;\n' +
            'attribute vec' + numMatrixDimensions + ' vector;\n' +
            'void main(void) { gl_Position = vec4(vector*matrix';
    // Ensure the vec4 has the correct number of dimensions in order to be assignable
    // to gl_Position.
    for (var ii = numMatrixDimensions; ii < 4; ++ii) {
        strVertexShader += ",0.0";
    }
    strVertexShader += ");}\n";
    return compileShader(strVertexShader, gl.VERTEX_SHADER);
}

function testbindattriblocationmatrix_createAndLinkProgram(vertexShader, matrixLocation,
                                                           vectorLocation) {
    var glProgram = gl.createProgram();
    gl.bindAttribLocation(glProgram, matrixLocation, 'matrix');
    gl.bindAttribLocation(glProgram, vectorLocation, 'vector');
    gl.attachShader(glProgram, vertexShader);
    gl.attachShader(glProgram, glFragmentShader);
    gl.linkProgram(glProgram);
    return gl.getProgramParameter(glProgram, gl.LINK_STATUS);
}

function testbindattriblocationmatrix() {
    var retval = true;
    output('MAX_VERTEX_ATTRIBS is ' + maxVertexAttributes);
    retval = (maxVertexAttributes >= 4);
    if (!retval) return false;

    for (var mm = 2; mm <= 4; ++mm) {
        output('Testing ' + mm + ' dimensional matrices');
        glVertexShader = testbindattriblocationmatrix_loadVertexShader(mm);
        // Per the WebGL spec: "LinkProgram will fail if the attribute bindings assigned
        // by bindAttribLocation do not leave enough space to assign a location for an
        // active matrix attribute which requires multiple contiguous generic attributes."
        // We will test this by placing the vector after the matrix attribute such that there
        // is not enough room for the matrix. Vertify the link operation fails.
        // Run the test for each available attribute slot. Go to maxAttributes-mm to leave enough room
        // for the matrix itself. Leave another slot open for the vector following the matrix.
        for (var pp = 0; pp <= maxVertexAttributes - mm - 1; ++pp) {
            // For each matrix dimension, bind the vector right after the matrix such that we leave
            // insufficient room for the matrix. Verify doing this will fail the link operation.
            for (var ll = 0; ll < mm; ++ll) {
                var vectorLocation = pp + ll;
                retval = assertMsg(!testbindattriblocationmatrix_createAndLinkProgram(
                                       glVertexShader, pp, vectorLocation),
                                   "Matrix with location " + pp + " and vector with location " + vectorLocation + " should not link.");
                if (!retval) return false;
            }
            // Ensure that once we have left enough room for the matrix, the program links successfully.
            vectorLocation = pp + ll;
            retval = assertMsg(testbindattriblocationmatrix_createAndLinkProgram(
                                   glVertexShader, pp, vectorLocation),
                               "Matrix with location " + pp + " and vector with location " + vectorLocation + " should link.");
            if (!retval) return false;
        }
    }
    return retval;
}

//
// gl-disabled-vertex-attrib.html
//
function testdisabledvertexattrib() {
    var wtu = WebGLTestUtils;
    var retval = true;
    for (var ii = 0; ii < maxVertexAttributes; ++ii) {
        var colorLocation = (ii + 1) % maxVertexAttributes;
        var positionLocation = colorLocation ? 0 : 1;
        if (positionLocation !== 0) {
            // We need to use a new 3d context for testing attrib 0
            // since we've already effected attrib 0 on other tests.
            console.log(gl)
            gl = gl2;
            console.log(gl)
        }
        output("testing attrib: " + colorLocation);
        var program = wtu.setupProgram(
                    gl,
                    [disabledVertexShader, disabledFragmentShader],
                    ['a_position', 'a_color'],
                    [positionLocation, colorLocation]);
        var gridRes = 1;
        wtu.setupIndexedQuad(gl, gridRes, positionLocation);
        wtu.clearAndDrawIndexedQuad(gl, gridRes);
        retval = wtu.checkCanvas(gl, [0, 255, 0, 255], "should be green");
        if (!retval) return false;
    }
    retval = wtu.glErrorShouldBe(gl, gl.NO_ERROR, "should be no errors");
    return retval;
}

var disabledVertexShader = [
            'attribute vec4 a_position;',
            'attribute vec4 a_color;',
            'varying vec4 v_color;',
            'bool isCorrectColor(vec4 v) {',
            ' return v.x == 0.0 && v.y == 0.0 && v.z == 0.0 && v.w == 1.0;',
            '}',
            'void main() {',
            ' gl_Position = a_position;',
            ' v_color = isCorrectColor(a_color) ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);',
            '}'].join('\n');

var disabledFragmentShader = [
            'precision mediump float;',
            'varying vec4 v_color;',
            'void main() {',
            ' gl_FragColor = v_color;',
            '}'].join('\n');

//
// gl-enable-vertex-attrib.html
//
function testenablevertexattrib() {
    var wtu = WebGLTestUtils;
    var retval = true;
    var program = wtu.setupProgram(
                gl,
                [simpleColorVertexShader, enableFragmentShader],
                ["vPosition"]);
    var vertexObject = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vertexObject);
    gl.bufferData(gl.ARRAY_BUFFER,
                  new Float32Array([ 0,0.5,0, -0.5,-0.5,0, 0.5,-0.5,0 ]),
                  gl.STATIC_DRAW);
    gl.enableVertexAttribArray(0);
    gl.vertexAttribPointer(0, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(3);
    retval = wtu.glErrorShouldBe(gl, gl.NO_ERROR);
    if (!retval) return false;
    gl.drawArrays(gl.TRIANGLES, 0, 3);
    retval = wtu.glErrorShouldBe(gl, gl.INVALID_OPERATION);
    return retval;
}

var enableFragmentShader = [
            'void main() {',
            ' gl_FragColor = vec4(1.0,0.0,0.0,1.0);',
            '}'].join('\n');

//
// gl-matrix-attributes.html
//
function testmatrixattributes() {
    var wtu = WebGLTestUtils;
    var retval = true;
    glFragmentShader = compileShader(simpleColorFragmentShader, gl.FRAGMENT_SHADER);
    // Test mat2, mat3 and mat4.
    for (var mm = 2; mm <= 4; ++mm) {
        // Add maxVertexAttributes number of attributes by saving enough room in the attribute
        // list for a matrix of mm dimensions. All of the other attribute slots will be
        // filled with vectors.
        var numVectors = maxVertexAttributes - mm;
        for (var pp = 1; pp <= numVectors + 1; ++pp) {
            //output('Test ' + mm + ' dimensional matrix at position ' + pp);
            var glProgram = testmatrixattributes_prepareMatrixProgram(wtu, numVectors, mm, pp);
            retval = (glProgram !== null);
            if (!retval) return false;
            var attribMatrix = gl.getAttribLocation(glProgram, 'matrix');
            //output('Matrix is at attribute location ' + attribMatrix);
            retval = (attribMatrix > -1);
            if (!retval) return false;
            // Per the spec, when an attribute is a matrix attribute, getAttribLocation
            // returns the index of the first component of the matrix. The implementation must
            // leave sufficient room for all the components. Here we ensure none of the vectors
            // in the shader are assigned attribute locations that belong to the matrix.
            for (var vv = 1; vv <= numVectors; ++vv) {
                var strVector = 'vec_' + vv
                var attribVector = gl.getAttribLocation(glProgram, strVector);
                // Begin with the first attribute location where the matrix begins and ensure
                // the vector's attribute location is not assigned to the matrix. Loop until
                // we've checked all of the attribute locations that belong to the matrix.
                for (var ii = attribMatrix; ii < attribMatrix + mm; ++ii) {
                    var testStr = strVector + ' attribute location: ' + attribVector + '. Should not be ' + ii;
                    if (attribVector === ii) {
                        output(testStr);
                        return false;//testFailed(testStr);
                    }
                }
            }
        }
    }
    return retval;
}

// testmatrixattributes_prepareMatrixProgram creates a program with glFragmentShader as the fragment shader.
// The vertex shader has numVector number of vectors and a matrix with numMatrixDimensions
// dimensions at location numMatrixPosition in the list of attributes.
// Ensures that every vector and matrix is used by the program.
// Returns a valid program on successful link; null on link failure.
function testmatrixattributes_prepareMatrixProgram(wtu, numVectors, numMatrixDimensions,
                                                   numMatrixPosition) {
    // Add the matrix and vector attribute declarations. Declare the vectors
    // to have the same number of components as the matrix so we can perform
    // operations on them when we assign to gl_Position later on.
    var strVertexShader = "";
    for (var ii = 1; ii <= numVectors; ++ii) {
        if (numMatrixPosition === ii) {
            strVertexShader += "attribute mat" + numMatrixDimensions + " matrix;\n";
        }
        strVertexShader += "attribute vec" + numMatrixDimensions + " vec_" + ii + ";\n";
    }
    // numMatrixPosition will be one past numVectors if the caller wants it to be
    // last. Hence, we need this check outside the loop as well as inside.
    if (numMatrixPosition === ii) {
        strVertexShader += "attribute mat" + numMatrixDimensions + " matrix;\n";
    }
    // Add the body of the shader. Add up all of the vectors and multiply by the matrix.
    // The operations we perform do not matter. We just need to ensure that all the vector and
    // matrix attributes are used.
    strVertexShader += "void main(void) { \ngl_Position = vec4((";
    for (ii = 1; ii <= numVectors; ++ii) {
        if (ii > 1) {
            strVertexShader += "+"
        }
        strVertexShader += "vec_" + ii;
    }
    strVertexShader += ")*matrix";
    // Ensure the vec4 has the correct number of dimensions in order to be assignable
    // to gl_Position.
    for (ii = numMatrixDimensions; ii < 4; ++ii) {
        strVertexShader += ",0.0";
    }
    strVertexShader += ");}\n";
    // Load the shader, attach it to a program, and return the link results
    glVertexShader = compileShader(strVertexShader, gl.VERTEX_SHADER);
    var strTest = 'Load shader with ' + numVectors + ' vectors and 1 matrix';
    if (glVertexShader !== null) {
        //testPassed(strTest);
        var glProgram = gl.createProgram();
        gl.attachShader(glProgram, glVertexShader);
        gl.attachShader(glProgram, glFragmentShader);
        gl.linkProgram(glProgram);
        if (gl.getProgramParameter(glProgram, gl.LINK_STATUS)) {
            var retval = wtu.glErrorShouldBe(gl, gl.NO_ERROR, 'linkProgram');
            if (!retval) return null;
            return glProgram;
        }
    } else {
        output(strTest);
        //testFailed(strTest);
    }
    return null;
}

//
// gl-vertex-attrib-render.html
//
function testvertexattribrender() {
    var wtu = WebGLTestUtils;
    var retval = true;

    var program = wtu.setupProgram(
                gl,
                [vertexattribVertexShader, vertexattribFragmentShader],
                ['p', 'a']);
    gl.enableVertexAttribArray(gl.p);
    var pos = gl.createBuffer();
    pos.type = gl.FLOAT;
    pos.size = 2;
    pos.num = 4;
    gl.bindBuffer(gl.ARRAY_BUFFER, pos);
    gl.bufferData(gl.ARRAY_BUFFER,
                  new Float32Array([-1, -1, 1, -1, -1, 1, 1, 1]),
                  gl.STATIC_DRAW);
    gl.vertexAttribPointer(0, pos.size, pos.type, false, 0, 0);
    //output('Test vertexAttrib[1..4]fv by setting different combinations that add up to 1.5 and use that when rendering.');
    var vals = [[0.5], [0.1,0.4], [0.2,-0.2,0.5], [-1.0,0.3,0.2,2.0]];
    for (var j = 0; j < 4; ++j) {
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
        gl['vertexAttrib' + (j+1) + 'fv'](1, vals[j]);
        gl.drawArrays(gl.TRIANGLE_STRIP, 0, pos.num);
        if (!testvertexattribrender_checkRedPortion(gl, width, width * 0.7, width * 0.8)) {
            output('Attribute of size ' + (j+1) + ' was not set correctly');
            return false;//testFailed('Attribute of size ' + (j+1) + ' was not set correctly');
        }
    }

    return retval;
}

function testvertexattribrender_checkRedPortion(gl, w, low, high) {
    var buf = new Uint8Array(w * w * 4);
    gl.readPixels(0, 0, w, w, gl.RGBA, gl.UNSIGNED_BYTE, buf);
    var i = 0;
    for (; i < w; ++i) {
        if (buf[i * 4 + 0] === 255 && buf[i * 4 + 1] === 0
                && buf[i * 4 + 2] === 0 && buf[i * 4 + 3] === 255) {
            break;
        }
    }
    return low <= i && i <= high;
}

var vertexattribVertexShader = [
            'attribute vec4 a;',
            'attribute vec2 p;',
            'void main() {',
            ' gl_Position = vec4(p.x + a.x + a.y + a.z + a.w, p.y, 0.0, 1.0);',
            '}'].join('\n');

var vertexattribFragmentShader = [
            'precision mediump float;',
            'void main() {',
            ' gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);',
            '}'].join('\n');

//
// gl-vertex-attrib-zero-issues.html
//
function testvertexattribzeroissues() {
    var wtu = WebGLTestUtils;
    var retval = true;

    var p0 = testvertexattribzeroissues_setup(wtu, 0);
    var p3 = testvertexattribzeroissues_setup(wtu, 3);
    if (p0 === null || p3 === null)
        return false;
    testvertexattribzeroissues_setupVerts(60000);
    for (var ii = 0; ii < 5; ++ii) {
        // test drawing with attrib 0
        gl.useProgram(p0);
        gl.enableVertexAttribArray(0);
        gl.vertexAttribPointer(0, 3, gl.FLOAT, false, 0, 0);
        gl.clear(gl.COLOR_BUFFER_BIT);
        gl.drawElements(gl.TRIANGLES, 60000, gl.UNSIGNED_SHORT, 0);
        retval = wtu.glErrorShouldBe(
                    gl, gl.NO_ERROR,
                    "drawing using attrib 0 with 60000 verts");
        if (!retval) return false;
        retval = wtu.checkCanvas(gl, [0, 255, 0, 255], "canvas should be green");
        if (!retval) return false;
        gl.disableVertexAttribArray(0);
        // test drawing without attrib 0
        gl.useProgram(p3);
        gl.enableVertexAttribArray(3);
        gl.vertexAttribPointer(3, 3, gl.FLOAT, false, 0, 0);
        gl.clear(gl.COLOR_BUFFER_BIT);
        gl.drawArrays(gl.TRIANGLES, 0, 60000);
        retval = wtu.glErrorShouldBe(
                    gl, gl.NO_ERROR,
                    "drawing using attrib 3 with 60000 verts");
        if (!retval) return false;
        retval = wtu.checkCanvas(gl, [0, 255, 0, 255], "canvas should be green");
        if (!retval)
        gl.disableVertexAttribArray(3);
        // This second test of drawing without attrib0 unconvered a bug in chrome
        // where after the draw without attrib0 the attrib 0 emulation code disabled
        // attrib 0 and it was never re-enabled so this next draw failed.
        gl.useProgram(p3);
        gl.enableVertexAttribArray(3);
        gl.clear(gl.COLOR_BUFFER_BIT);
        gl.drawElements(gl.TRIANGLES, 60000, gl.UNSIGNED_SHORT, 0);
        retval = wtu.glErrorShouldBe(
                    gl, gl.NO_ERROR,
                    "drawing using attrib 3 with 60000 verts");
        if (!retval) return false;
        retval = wtu.checkCanvas(gl, [0, 255, 0, 255], "canvas should be green");
        if (!retval) return false;
        gl.disableVertexAttribArray(3);
    }

    return retval;
}

function testvertexattribzeroissues_setup(wtu, attribIndex) {
    var program = wtu.setupProgram(
                gl,
                [simpleColorVertexShader, vertexattribzeroFragmentShader],
                ['vPosition'],
                [attribIndex]);
    if (attribIndex === gl.getAttribLocation(program, 'vPosition'))
        return program;
    else
        return null;
}

function testvertexattribzeroissues_setupVerts(numVerts) {
    var verts = [
                1.0, 1.0, 0.0,
                -1.0, 1.0, 0.0,
                -1.0, -1.0, 0.0,
                1.0, 1.0, 0.0,
                -1.0, -1.0, 0.0,
                1.0, -1.0, 0.0
            ];
    var positions = new Float32Array(numVerts * 3);
    var indices = new Uint16Array(numVerts);
    for (var ii = 0; ii < numVerts; ++ii) {
        var ndx = ii % 6;
        var dst = ii * 3;
        var src = ndx * 3;
        for (var jj = 0; jj < 3; ++jj) {
            positions[dst + jj] = verts[src + jj];
        }
        indices[ii] = ii;
    }
    var vertexObject = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vertexObject);
    gl.bufferData(gl.ARRAY_BUFFER, positions, gl.STATIC_DRAW);
    var indexBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices, gl.STATIC_DRAW);
}

var vertexattribzeroFragmentShader = [
            'void main() {',
            ' gl_FragColor = vec4(0.0,1.0,0.0,1.0);',
            '}'].join('\n');

//
// gl-vertex-attrib.html
//
function testvertexattrib() {
    var wtu = WebGLTestUtils;
    var retval = true;

    for (var ii = 0; ii < maxVertexAttributes; ++ii) {
        // These don't work (QTBUG-45043)
//        gl.vertexAttrib1fv(ii, [1]);
//        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[0]', '1');
//        if (!retval) return false;
//        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[1]', '0');
//        if (!retval) return false;
//        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[2]', '0');
//        if (!retval) return false;
//        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[3]', '1');
//        if (!retval) return false;
//        gl.vertexAttrib2fv(ii, [1, 2]);
//        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[0]', '1');
//        if (!retval) return false;
//        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[1]', '2');
//        if (!retval) return false;
//        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[2]', '0');
//        if (!retval) return false;
//        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[3]', '1');
//        if (!retval) return false;
//        gl.vertexAttrib3fv(ii, [1, 2, 3]);
//        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[0]', '1');
//        if (!retval) return false;
//        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[1]', '2');
//        if (!retval) return false;
//        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[2]', '3');
//        if (!retval) return false;
//        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[3]', '1');
//        if (!retval) return false;
//        gl.vertexAttrib4fv(ii, [1, 2, 3, 4]);
//        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[0]', '1');
//        if (!retval) return false;
//        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[1]', '2');
//        if (!retval) return false;
//        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[2]', '3');
//        if (!retval) return false;
//        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[3]', '4');
//        if (!retval) return false;
        gl.vertexAttrib1f(ii, 5);
        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[0]', '5');
        if (!retval) return false;
        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[1]', '0');
        if (!retval) return false;
        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[2]', '0');
        if (!retval) return false;
        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[3]', '1');
        if (!retval) return false;
        gl.vertexAttrib2f(ii, 6, 7);
        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[0]', '6');
        if (!retval) return false;
        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[1]', '7');
        if (!retval) return false;
        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[2]', '0');
        if (!retval) return false;
        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[3]', '1');
        if (!retval) return false;
        gl.vertexAttrib3f(ii, 7, 8, 9);
        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[0]', '7');
        if (!retval) return false;
        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[1]', '8');
        if (!retval) return false;
        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[2]', '9');
        if (!retval) return false;
        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[3]', '1');
        if (!retval) return false;
        gl.vertexAttrib4f(ii, 6, 7, 8, 9);
        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[0]', '6');
        if (!retval) return false;
        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[1]', '7');
        if (!retval) return false;
        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[2]', '8');
        if (!retval) return false;
        retval = shouldBe('gl.getVertexAttrib(' + ii + ', gl.CURRENT_VERTEX_ATTRIB)[3]', '9');
        if (!retval) return false;
    }
    retval = wtu.glErrorShouldBe(gl, gl.NO_ERROR);

    return retval;
}

//
// gl-vertexattribpointer-offsets.html
//
function testvertexattribpointeroffsets() {
    var wtu = WebGLTestUtils;
    var retval = true;

    gl.clearColor(0, 0, 0, 0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    var program = wtu.setupProgram(
                gl,
                [simpleColorVertexShader, attribvertexoffsetFragmentShader],
                ["vPosition"]);
    var tests = [
                { data: new Float32Array([ 0, 1, 0, 1, 0, 0, 0, 0, 0 ]),
                    type: gl.FLOAT,
                    componentSize: 4,
                    normalize: false,
                },
                { data: new Float32Array([ 0, 1, 0, 1, 0, 0, 0, 0, 0 ]),
                    type: gl.FLOAT,
                    componentSize: 4,
                    normalize: false,
                },
                { data: new Uint16Array([ 0, 32767, 0, 32767, 0, 0, 0, 0, 0 ]),
                    type: gl.SHORT,
                    componentSize: 2,
                    normalize: true,
                },
                { data: new Uint16Array([ 0, 65535, 0, 65535, 0, 0, 0, 0, 0 ]),
                    type: gl.UNSIGNED_SHORT,
                    componentSize: 2,
                    normalize: true,
                },
                { data: new Uint16Array([ 0, 1, 0, 1, 0, 0, 0, 0, 0 ]),
                    type: gl.UNSIGNED_SHORT,
                    componentSize: 2,
                    normalize: false,
                },
                { data: new Uint16Array([ 0, 1, 0, 1, 0, 0, 0, 0, 0 ]),
                    type: gl.SHORT,
                    componentSize: 2,
                    normalize: false,
                },
                { data: new Uint8Array([ 0, 127, 0, 127, 0, 0, 0, 0, 0 ]),
                    type: gl.BYTE,
                    componentSize: 1,
                    normalize: true,
                },
                { data: new Uint8Array([ 0, 255, 0, 255, 0, 0, 0, 0, 0 ]),
                    type: gl.UNSIGNED_BYTE,
                    componentSize: 1,
                    normalize: true,
                },
                { data: new Uint8Array([ 0, 1, 0, 1, 0, 0, 0, 0, 0 ]),
                    type: gl.BYTE,
                    componentSize: 1,
                    normalize: false,
                },
                { data: new Uint8Array([ 0, 1, 0, 1, 0, 0, 0, 0, 0 ]),
                    type: gl.UNSIGNED_BYTE,
                    componentSize: 1,
                    normalize: false,
                }
            ];
    var vertexObject = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vertexObject);
    gl.bufferData(gl.ARRAY_BUFFER, 1024, gl.STATIC_DRAW);
    gl.enableVertexAttribArray(0);
    var colorLoc = gl.getUniformLocation(program, "color");
    var kNumVerts = 3;
    var kNumComponents = 3;
    var count = 0;
    for (var tt = 0; tt < tests.length; ++tt) {
        var test = tests[tt];
        for (var oo = 0; oo < 3; ++oo) {
            for (var ss = 0; ss < 3; ++ss) {
                var offset = (oo + 1) * test.componentSize;
                var color = (count % 2) ? [1, 0, 0, 1] : [0, 1, 0, 1];
                var stride = test.componentSize * kNumComponents + test.componentSize * ss;
                output("check with " + wtu.glEnumToString(gl, test.type) + " at offset: "
                       + offset + " with stride:" + stride + " normalize: " + test.normalize);
                gl.uniform4fv(colorLoc, color);
                var data = new Uint8Array(test.componentSize * kNumVerts * kNumComponents
                                          + stride * (kNumVerts - 1));
                var view = new Uint8Array(test.data.buffer);
                var size = test.componentSize * kNumComponents;
                for (var jj = 0; jj < kNumVerts; ++jj) {
                    var off1 = jj * size;
                    var off2 = jj * stride;
                    for (var zz = 0; zz < size; ++zz) {
                        data[off2 + zz] = view[off1 + zz];
                    }
                }
                gl.bufferSubData(gl.ARRAY_BUFFER, offset, data);
                gl.vertexAttribPointer(0, 3, test.type, test.normalize, stride, offset);
                gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
                gl.drawArrays(gl.TRIANGLES, 0, 3);
                var buf = new Uint8Array(width * height * 4);
                gl.readPixels(0, 0, width, height, gl.RGBA, gl.UNSIGNED_BYTE, buf);
                var black = [0, 0, 0, 0];
                var other = [color[0] * 255, color[1] * 255, color[2] * 255, color[3] * 255];
                var otherMsg = "should be " + ((count % 2) ? "red" : "green")
                // The following tests assume width and height are both 50
                retval = wtu.checkCanvasRect(gl, 0, 0, 1, 1, black, "should be black", 0);
                if (!retval) return false;
                retval = wtu.checkCanvasRect(gl, 0, 49, 1, 1, black, "should be black", 0);
                if (!retval) return false;
                retval = wtu.checkCanvasRect(gl, 26, 40, 1, 1, other, otherMsg, 0);
                if (!retval) return false;
                retval = wtu.checkCanvasRect(gl, 26, 27, 1, 1, other, otherMsg, 0);
                if (!retval) return false;
                retval = wtu.checkCanvasRect(gl, 40, 27, 1, 1, other, otherMsg, 0);
                if (!retval) return false;
                ++count;
            }
        }
    }

    return retval;
}

var attribvertexoffsetFragmentShader = [
            'precision mediump float;',
            'uniform vec4 color;',
            'void main() {',
            ' gl_FragColor = color;',
            '}'].join('\n');

//
// gl-vertexattribpointer.html
//
function testvertexattribpointer() {
    var wtu = WebGLTestUtils;
    var retval = true;

    if (!gl.FIXED) {
        gl.FIXED = 0x140C;
    }

    gl.vertexAttribPointer(0, 3, gl.FLOAT, 0, 0, 12);
    retval = wtu.glErrorShouldBe(gl, gl.INVALID_OPERATION,
                        "vertexAttribPointer should fail if no buffer is bound");
    if (!retval) return false;
    var vertexObject = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vertexObject);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(0), gl.STATIC_DRAW);
    gl.vertexAttribPointer(0, 1, gl.INT, 0, 0, 0);
    retval = wtu.glErrorShouldBe(gl, gl.INVALID_ENUM,
                        "vertexAttribPointer should not support INT");
    if (!retval) return false;
    gl.vertexAttribPointer(0, 1, gl.UNSIGNED_INT, 0, 0, 0);
    retval = wtu.glErrorShouldBe(gl, gl.INVALID_ENUM,
                        "vertexAttribPointer should not support UNSIGNED_INT");
    if (!retval) return false;
    gl.vertexAttribPointer(0, 1, gl.FIXED, 0, 0, 0);
    retval = wtu.glErrorShouldBe(gl, gl.INVALID_ENUM,
                        "vertexAttribPointer should not support FIXED");
    if (!retval) return false;
    var types = [
                { type:gl.BYTE, bytesPerComponent: 1 },
                { type:gl.UNSIGNED_BYTE, bytesPerComponent: 1 },
                { type:gl.SHORT, bytesPerComponent: 2 },
                { type:gl.UNSIGNED_SHORT, bytesPerComponent: 2 },
                { type:gl.FLOAT, bytesPerComponent: 4 },
            ];
    for (var ii = 0; ii < types.length; ++ii) {
        var info = types[ii];
        for (var size = 1; size <= 4; ++size) {
            output("checking: " + wtu.glEnumToString(gl, info.type) + " with size " + size);
            var bytesPerElement = size * info.bytesPerComponent;
            var offsetSet = [
                        0,
                        1,
                        info.bytesPerComponent - 1,
                        info.bytesPerComponent,
                        info.bytesPerComponent + 1,
                        info.bytesPerComponent * 2];
            for (var jj = 0; jj < offsetSet.length; ++jj) {
                var offset = offsetSet[jj];
                for (var kk = 0; kk < offsetSet.length; ++kk) {
                    var stride = offsetSet[kk];
                    var err = gl.NO_ERROR;
                    var reason = ""
                    if (offset % info.bytesPerComponent != 0) {
                        reason = "because offset is bad";
                        err = gl.INVALID_OPERATION;
                    }
                    if (stride % info.bytesPerComponent != 0) {
                        reason = "because stride is bad";
                        err = gl.INVALID_OPERATION;
                    }
                    retval = testvertexattribpointer_checkVertexAttribPointer(
                                wtu, gl, err, reason, size, info.type, false, stride, offset);
                    if (!retval) return false;
                }
                stride = Math.floor(255 / info.bytesPerComponent) * info.bytesPerComponent;
                if (offset === 0) {
                    retval = testvertexattribpointer_checkVertexAttribPointer(
                                wtu, gl, gl.NO_ERROR, "at stride limit",
                                size, info.type, false, stride, offset);
                    if (!retval) return false;
                    retval = testvertexattribpointer_checkVertexAttribPointer(
                                wtu, gl, gl.INVALID_VALUE, "over stride limit",
                                size, info.type, false,
                                stride + info.bytesPerComponent, offset);
                    if (!retval) return false;
                }
            }
        }
    }

    return retval;
}

var testvertexattribpointer_checkVertexAttribPointer = function(
    wtu, gl, err, reason, size, type, normalize, stride, offset) {
    gl.vertexAttribPointer(0, size, type, normalize, stride, offset);
    return wtu.glErrorShouldBe(gl, err,
                               "gl.vertexAttribPointer(0, " + size +
                               ", gl." + wtu.glEnumToString(gl, type) +
                               ", " + normalize +
                               ", " + stride +
                               ", " + offset +
                               ") should " + (err === gl.NO_ERROR ? "succeed " : "fail ") + reason);
}

/**
* A vertex shader for a single texture.
* @type {string}
*/
var simpleTextureVertexShader = [
            'attribute vec4 vPosition;',
            'attribute vec2 texCoord0;',
            'varying vec2 texCoord;',
            'void main() {',
            ' gl_Position = vPosition;',
            ' texCoord = texCoord0;',
            '}'].join('\n');
/**
* A fragment shader for a single texture.
* @type {string}
*/
var simpleTextureFragmentShader = [
            'precision mediump float;',
            'uniform sampler2D tex;',
            'varying vec2 texCoord;',
            'void main() {',
            ' gl_FragData[0] = texture2D(tex, texCoord);',
            '}'].join('\n');
/**
* A vertex shader for a single texture.
* @type {string}
*/
var noTexCoordTextureVertexShader = [
            'attribute vec4 vPosition;',
            'varying vec2 texCoord;',
            'void main() {',
            ' gl_Position = vPosition;',
            ' texCoord = vPosition.xy * 0.5 + 0.5;',
            '}'].join('\n');
/**
* A vertex shader for a uniform color.
* @type {string}
*/
var simpleColorVertexShader = [
            'attribute vec4 vPosition;',
            'void main() {',
            ' gl_Position = vPosition;',
            '}'].join('\n');
/**
* A fragment shader for a uniform color.
* @type {string}
*/
var simpleColorFragmentShader = [
            'precision mediump float;',
            'uniform vec4 u_color;',
            'void main() {',
            ' gl_FragData[0] = u_color;',
            '}'].join('\n');
/**
* A vertex shader for vertex colors.
* @type {string}
*/
var simpleVertexColorVertexShader = [
            'attribute vec4 vPosition;',
            'attribute vec4 a_color;',
            'varying vec4 v_color;',
            'void main() {',
            ' gl_Position = vPosition;',
            ' v_color = a_color;',
            '}'].join('\n');
/**
* A fragment shader for vertex colors.
* @type {string}
*/
var simpleVertexColorFragmentShader = [
            'precision mediump float;',
            'varying vec4 v_color;',
            'void main() {',
            ' gl_FragData[0] = v_color;',
            '}'].join('\n');
