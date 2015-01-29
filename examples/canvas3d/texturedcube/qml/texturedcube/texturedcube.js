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
// Draws a cube that has the Qt logo texture on each face.
// Each face also has a unique color that modulates the grayscale Qt logo.
//

var gl;
var width = 0;
var height = 0;
var cubeTexture = 0;
var mvMatrix = mat4.create();
var pMatrix  = mat4.create();

var pMatrixUniform;
var mvMatrixUniform;

var vertexPositionAttribute;
var textureCoordAttribute;
var vertexColorAttribute;

var canvas3d;

function log(message) {
    if (canvas3d.logAllCalls)
        console.log(message)
}

//! [0]
function initGL(canvas) {
    canvas3d = canvas
    log("*******************************************************************************************")
    log("initGL ENTER...")

    // Get the OpenGL context object that represents the API we call
    gl = canvas.getContext("canvas3d", {depth:true, antialias:true});

    // Setup the OpenGL state
    gl.enable(gl.DEPTH_TEST);
    gl.enable(gl.CULL_FACE);
    gl.cullFace(gl.BACK);
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.clearDepth(1.0);
    gl.depthFunc(gl.LESS);
    gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);

    // Set the viewport
    var pixelRatio = canvas.devicePixelRatio;
    gl.viewport(0, 0, pixelRatio * canvas.width, pixelRatio * canvas.height);

    // Initialize the shader program
    initShaders();

    // Initialize vertex and color buffers
    initBuffers();

    // Load the texture
    var qtLogoImage = TextureImageFactory.newTexImage();
    qtLogoImage.imageLoaded.connect(function() {
        cubeTexture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, cubeTexture);
        gl.texImage2D(gl.TEXTURE_2D,    // target
                      0,                // level
                      gl.RGBA,          // internalformat
                      gl.RGBA,          // format
                      gl.UNSIGNED_BYTE, // type
                      qtLogoImage);    // pixels

        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST);
        gl.generateMipmap(gl.TEXTURE_2D);
    });
    qtLogoImage.imageLoadingFailed.connect(function() {
        console.log("Texture load FAILED, "+qtLogoImage.errorString);
    });
    qtLogoImage.src = "qrc:/qml/texturedcube/qtlogo_gray.png";

    log("...initGL EXIT");
    log("*******************************************************************************************");
}
//! [0]

function degToRad(degrees) {
    return degrees * Math.PI / 180;
}

//! [1]
function renderGL(canvas) {
    log("*******************************************************************************************");
    // Check if dimensions or device pixel ratio has changed
    var pixelRatio = canvas.devicePixelRatio;
    var currentWidth = canvas.width * pixelRatio;
    var currentHeight = canvas.height * pixelRatio;
    if (currentWidth !== width || currentHeight !== height ) {
        width = currentWidth;
        height = currentHeight;
        mat4.perspective(pMatrix, degToRad(45), width / height, 0.1, 500.0);
        gl.uniformMatrix4fv(pMatrixUniform, false, pMatrix);
    }

    // Clear the screen
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    // Set and update the model matrix
    mat4.identity(mvMatrix);
    // Apply Canvas3D rotation properties to cube translation to make it move about a bit
    mat4.translate(mvMatrix, mvMatrix, [(canvas.yRotAnim - 120.0) / 120.0,
                                        (canvas.xRotAnim -  60.0) / 50.0,
                                        -10.0]);
    // Apply rotations gotten from Canvas3D properties
    mat4.rotate(mvMatrix, mvMatrix, degToRad(canvas.xRotAnim), [1, 0, 0]);
    mat4.rotate(mvMatrix, mvMatrix, degToRad(canvas.yRotAnim), [0, 1, 0]);
    mat4.rotate(mvMatrix, mvMatrix, degToRad(canvas.zRotAnim), [0, 0, 1]);

    // Set the matrix to shader
    gl.uniformMatrix4fv(mvMatrixUniform, false, mvMatrix);

    // Draw the cube
    gl.drawElements(gl.TRIANGLES, 36, gl.UNSIGNED_SHORT, 0);
    log("*******************************************************************************************");
}
//! [1]

function initBuffers()
{
    var cubeVertexPositionBuffer = gl.createBuffer();
    cubeVertexPositionBuffer.name = "cubeVertexPositionBuffer";
    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVertexPositionBuffer);
    gl.bufferData(
                gl.ARRAY_BUFFER,
                Arrays.newFloat32Array([// Front face
                                        -1.0, -1.0,  1.0,
                                        1.0, -1.0,  1.0,
                                        1.0,  1.0,  1.0,
                                        -1.0,  1.0,  1.0,

                                        // Back face
                                        -1.0, -1.0, -1.0,
                                        -1.0,  1.0, -1.0,
                                        1.0,  1.0, -1.0,
                                        1.0, -1.0, -1.0,

                                        // Top face
                                        -1.0,  1.0, -1.0,
                                        -1.0,  1.0,  1.0,
                                        1.0,  1.0,  1.0,
                                        1.0,  1.0, -1.0,

                                        // Bottom face
                                        -1.0, -1.0, -1.0,
                                        1.0, -1.0, -1.0,
                                        1.0, -1.0,  1.0,
                                        -1.0, -1.0,  1.0,

                                        // Right face
                                        1.0, -1.0, -1.0,
                                        1.0,  1.0, -1.0,
                                        1.0,  1.0,  1.0,
                                        1.0, -1.0,  1.0,

                                        // Left face
                                        -1.0, -1.0, -1.0,
                                        -1.0, -1.0,  1.0,
                                        -1.0,  1.0,  1.0,
                                        -1.0,  1.0, -1.0
                                       ]),
                gl.STATIC_DRAW);
    gl.enableVertexAttribArray(vertexPositionAttribute);
    gl.vertexAttribPointer(vertexPositionAttribute, 3, gl.FLOAT, false, 0, 0);

    var cubeVertexColorBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVertexColorBuffer);
    var colors = [
                [0.0,  1.0,  1.0,  1.0],    // Front face: white
                [1.0,  0.0,  0.0,  1.0],    // Back face: red
                [0.0,  1.0,  0.0,  1.0],    // Top face: green
                [0.0,  0.0,  1.0,  1.0],    // Bottom face: blue
                [1.0,  1.0,  0.0,  1.0],    // Right face: yellow
                [1.0,  0.0,  1.0,  1.0]     // Left face: purple
            ];
    var generatedColors = [];
    for (var j=0; j<6; j++) {
        var c = colors[j];

        for (var i=0; i<4; i++) {
            generatedColors = generatedColors.concat(c);
        }
    }
    gl.bufferData(gl.ARRAY_BUFFER, Arrays.newFloat32Array(generatedColors), gl.STATIC_DRAW);
    gl.enableVertexAttribArray(vertexColorAttribute);
    gl.vertexAttribPointer(vertexColorAttribute, 4, gl.FLOAT, false, 0, 0);

    var cubeVerticesTextureCoordBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVerticesTextureCoordBuffer);
    var textureCoordinates = [
                // Front
                1.0,  0.0,
                0.0,  0.0,
                0.0,  1.0,
                1.0,  1.0,
                // Back
                1.0,  0.0,
                0.0,  0.0,
                0.0,  1.0,
                1.0,  1.0,
                // Top
                1.0,  0.0,
                0.0,  0.0,
                0.0,  1.0,
                1.0,  1.0,
                // Bottom
                1.0,  0.0,
                0.0,  0.0,
                0.0,  1.0,
                1.0,  1.0,
                // Right
                1.0,  0.0,
                0.0,  0.0,
                0.0,  1.0,
                1.0,  1.0,
                // Left
                1.0,  0.0,
                0.0,  0.0,
                0.0,  1.0,
                1.0,  1.0
            ];
    gl.bufferData(gl.ARRAY_BUFFER, Arrays.newFloat32Array(textureCoordinates),
                  gl.STATIC_DRAW);
    gl.enableVertexAttribArray(textureCoordAttribute);
    gl.vertexAttribPointer(textureCoordAttribute, 2, gl.FLOAT, false, 0, 0);

    var cubeVertexIndexBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, cubeVertexIndexBuffer);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,
                  Arrays.newUint16Array([
                                            0,  1,  2,      0,  2,  3,    // front
                                            4,  5,  6,      4,  6,  7,    // back
                                            8,  9,  10,     8,  10, 11,   // top
                                            12, 13, 14,     12, 14, 15,   // bottom
                                            16, 17, 18,     16, 18, 19,   // right
                                            20, 21, 22,     20, 22, 23    // left
                                        ]),
                  gl.STATIC_DRAW);

    log("    } initBuffers EXIT")
}

function initShaders()
{
    log("    initShaders ENTER {")
    var vertexShader = getShader(gl,
                                 "attribute highp vec3 aVertexPosition;                 \
                              attribute mediump vec4 aVertexColor;                   \
                              attribute highp vec2 aTextureCoord;                    \
                                                                                     \
                              uniform mat4 uMVMatrix;                                \
                              uniform mat4 uPMatrix;                                 \
                                                                                     \
                              varying mediump vec4 vColor;                           \
                              varying highp vec2 vTextureCoord;                      \
                              varying highp vec4 vPos;                               \
                                                                                     \
                              void main(void) {                                      \
                                  vPos = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);                 \
                                  gl_Position = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);          \
                                  vColor = aVertexColor;                             \
                                  vTextureCoord = aTextureCoord;                     \
                              }", gl.VERTEX_SHADER);
    var fragmentShader = getShader(gl,
                                   "varying mediump vec4 vColor;                   \
                               varying highp vec2 vTextureCoord;               \
                               varying highp vec4 vPos;                        \
                                                                               \
                               uniform sampler2D uSampler;                     \
                                                                               \
                               void main(void) {                               \
                                   gl_FragColor = vColor * texture2D(uSampler, vec2(vTextureCoord.s, vTextureCoord.t));                   \
                               }", gl.FRAGMENT_SHADER);

    var shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, vertexShader);
    gl.attachShader(shaderProgram, fragmentShader);
    gl.linkProgram(shaderProgram);
    gl.deleteShader(vertexShader);
    gl.deleteShader(fragmentShader);

    if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
        console.log("Could not initialise shaders");
        console.log(gl.getProgramInfoLog(shaderProgram));
    }

    gl.useProgram(shaderProgram);

    // look up where the vertex data needs to go.
    vertexPositionAttribute = gl.getAttribLocation(shaderProgram, "aVertexPosition");
    gl.enableVertexAttribArray(vertexPositionAttribute);
    vertexColorAttribute = gl.getAttribLocation(shaderProgram, "aVertexColor");
    gl.enableVertexAttribArray(vertexColorAttribute);
    textureCoordAttribute = gl.getAttribLocation(shaderProgram, "aTextureCoord");
    gl.enableVertexAttribArray(textureCoordAttribute);

    pMatrixUniform  = gl.getUniformLocation(shaderProgram, "uPMatrix");
    mvMatrixUniform = gl.getUniformLocation(shaderProgram, "uMVMatrix");

    var textureSamplerUniform = gl.getUniformLocation(shaderProgram, "uSampler")
    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, cubeTexture);
    gl.uniform1i(textureSamplerUniform, 0);

    log("    } initShaders EXIT");
}

function getShader(gl, str, type) {
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
