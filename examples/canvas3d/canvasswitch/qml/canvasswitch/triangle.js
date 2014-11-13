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

Qt.include("glMatrix-0.9.5.min.js")

//
// Draws a single blue triangle on black background in clipspace.
//

var gl;
var positionLocation;
var shaderProgram;
var buffer;
var vertexShader;
var fragmentShader;

var canvas3d;

function log(message) {
    if (canvas3d.logAllCalls)
        console.log(message)
}

function initGL(canvas) {
    canvas3d = canvas
    log("initGL("+canvas+") BEGIN");
    try {
        gl = canvas.getContext("3d");

        // Create a buffer and put a single clipspace rectangle in
        // it (2 triangles)
        buffer = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
        gl.bufferData(
                    gl.ARRAY_BUFFER, Arrays.newFloat32Array(
                        [-1.0, -1.0,
                         1.0, -1.0,
                         -1.0,  1.0
                        ]),
                    gl.STATIC_DRAW);


        if (!initShaders()) {
            console.log("initGL(): FAILURE!");
            return;
        }

        gl.useProgram(shaderProgram);

        // look up where the vertex data needs to go.
        positionLocation = gl.getAttribLocation(shaderProgram, "a_position");
        gl.enableVertexAttribArray(positionLocation);
        gl.vertexAttribPointer(positionLocation, 2, gl.FLOAT, false, 0, 0);

        gl.clearColor(0.0, 0.0, 0.0, 1.0);

        gl.viewport(0, 0, canvas.width * canvas.devicePixelRatio, canvas.height * canvas.devicePixelRatio);
    } catch(e) {
        console.log("initGL(): FAILURE!");
        console.log(""+e);
        console.log(""+e.message);
    }
    log("initGL("+canvas+") END");
}

function renderGL(canvas) {
    log("renderGL("+canvas+") BEGIN");
    // Clear background
    gl.clear(gl.COLOR_BUFFER_BIT);

    // Draw single green triangle
    gl.drawArrays(gl.TRIANGLES, 0, 3);
    log("renderGL("+canvas+") END");
}

function initShaders()
{
    // setup a GLSL program
    vertexShader = compileShader("attribute vec2 a_position;                        \
                                  void main() {                                     \
                                      gl_Position = vec4(a_position, 0.0, 1.0);     \
                                 }", gl.VERTEX_SHADER);
    fragmentShader = compileShader("void main() {                                   \
                                        gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);    \
                                   }", gl.FRAGMENT_SHADER);

    shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, vertexShader);
    gl.attachShader(shaderProgram, fragmentShader);
    gl.linkProgram(shaderProgram);

    // Check linking status
    if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
        console.log("Could not initialise shaders");
        console.log(gl.getProgramInfoLog(shaderProgram));
        return false;
    }

    return true;
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
