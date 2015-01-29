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

var gl;
var cubeTexture = 0;

var cubeShaderProgram;
var cubeAttribute_vertexPosition;
var cubeAttribute_textureCoord;
var cubeAttribute_vertexColor;
var cubeUniform_time;
var cubeUniform_pMatrix;
var cubeUniform_mvMatrix;
var cubeUniform_textureSampler;

var cubeVertexPositionBuffer;
var cubeVertexIndexBuffer;
var cubeVertexColorBuffer;
var cubeVerticesTextureCoordBuffer;

var bkgShaderProgram;
var bkgAttribute_vertexPosition;
var bkgAttribute_textureCoord;
var bkgUniform_time;

var bkgVertexPositionBuffer;
var bkgVerticesTextureCoordBuffer;

var mvMatrix = mat4.create();
var pMatrix  = mat4.create();

var startTime;
var elapsedTime;
var skipTime = 0;
var pausedTime;
var wasRunning = true;

var canvas3d;

function log(message) {
    if (canvas3d.logAllCalls)
        console.log(message)
}

function initGL(canvas) {
    canvas3d = canvas;
    log("*******************************************************************************************");
    log("initGL ENTER...");

    try {
        startTime = Date.now();

        // Get the OpenGL context object that represents the API we call
        gl = canvas.getContext("canvas3d", {antialias:true, depth:true});

        // Setup the OpenGL state
        gl.enable(gl.DEPTH_TEST);
        gl.enable(gl.DEPTH_WRITE);
        gl.depthMask(true);
        gl.depthFunc(gl.LESS);

        gl.enable(gl.CULL_FACE);
        gl.cullFace(gl.BACK);
        gl.disable(gl.BLEND);
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);

        // Set viewport
        gl.viewport(0, 0, canvas.width * canvas.devicePixelRatio,
                    canvas.height * canvas.devicePixelRatio);

        // Initialize vertex and color buffers
        initBuffers();

        // Initialize the shader program
        initShaders();

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
                          qtLogoImage);     // pixels

            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST);
            gl.generateMipmap(gl.TEXTURE_2D);
        });
        qtLogoImage.imageLoadingFailed.connect(function() {
            console.log("Texture load FAILED, "+qtLogoImage.errorString);
        });
        qtLogoImage.src = "qrc:/qml/plasmaeffects/qtlogo_gray.png";

        log("...initGL EXIT");
    } catch(e) {
        console.log("...initGL FAILURE!");
        console.log(""+e);
        console.log(""+e.message);
    }
    log("*******************************************************************************************");
}

function degToRad(degrees) {
    return degrees * Math.PI / 180;
}

function renderGL(canvas) {
    if (canvas.isRunning) {
        if (!wasRunning) {
            wasRunning = true;
            startTime += skipTime;
        }
        elapsedTime = Date.now() - startTime;
    } else {
        if (wasRunning) {
            // First time renderGL hit when paused
            wasRunning = false;
            pausedTime = Date.now();
        }
        skipTime = Date.now() - pausedTime;
    }

    // Clear screen
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.clearDepth(1.0);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    // Set states for background drawing
    gl.disable(gl.DEPTH_TEST);
    gl.depthMask(false);
    gl.disable(gl.DEPTH_WRITE);

    //! [4]
    // Draw background
    gl.useProgram(bkgShaderProgram);

    //! [4]

    // Update time
    gl.uniform1f(bkgUniform_time, elapsedTime / 1000.0);

    // Draw the screen space rect
    gl.bindBuffer(gl.ARRAY_BUFFER, bkgVertexPositionBuffer);
    gl.enableVertexAttribArray(bkgAttribute_vertexPosition);
    gl.vertexAttribPointer(bkgAttribute_vertexPosition, 2, gl.FLOAT, false, 0, 0);

    gl.bindBuffer(gl.ARRAY_BUFFER, bkgVerticesTextureCoordBuffer);
    gl.enableVertexAttribArray(bkgAttribute_textureCoord);
    gl.vertexAttribPointer(bkgAttribute_textureCoord, 2, gl.FLOAT, false, 0, 0);

    gl.drawArrays(gl.TRIANGLES, 0, 6);

    gl.disableVertexAttribArray(bkgAttribute_vertexPosition);
    gl.disableVertexAttribArray(bkgAttribute_textureCoord);

    gl.enable(gl.DEPTH_TEST);
    gl.depthMask(true);
    gl.enable(gl.DEPTH_WRITE);

    //! [5]
    // Draw cubes
    gl.useProgram(cubeShaderProgram);
    //! [5]

    // Calculate the perspective projection
    mat4.perspective(pMatrix, degToRad(45), canvas.width / canvas.height, 0.1, 100.0);
    gl.uniformMatrix4fv(cubeUniform_pMatrix, false, pMatrix);

    // Bind the correct buffers
    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVertexPositionBuffer);
    gl.enableVertexAttribArray(cubeAttribute_vertexPosition);
    gl.vertexAttribPointer(cubeAttribute_vertexPosition, 3, gl.FLOAT, false, 0, 0);

    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVertexColorBuffer);
    gl.enableVertexAttribArray(cubeAttribute_vertexColor);
    gl.vertexAttribPointer(cubeAttribute_vertexColor, 4, gl.FLOAT, false, 0, 0);

    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVerticesTextureCoordBuffer);
    gl.enableVertexAttribArray(cubeAttribute_textureCoord);
    gl.vertexAttribPointer(cubeAttribute_textureCoord, 2, gl.FLOAT, false, 0, 0);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, cubeTexture);
    gl.uniform1i(cubeUniform_textureSampler, 0);

    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, cubeVertexIndexBuffer);

    // Update time
    gl.uniform1f(cubeUniform_time, elapsedTime / 1000.0);

    // Draw first cube
    mat4.identity(mvMatrix);
    mat4.translate(mvMatrix, mvMatrix, [-(canvas.yRotAnim - 120.0) / 90.0,
                                        -(canvas.xRotAnim -  60.0) / 40.0,
                                        -25.0]);
    mat4.rotate(mvMatrix, mvMatrix, degToRad(canvas.yRotAnim) * degToRad(canvas.xRotAnim),
                [1, 0, 0]);
    mat4.rotate(mvMatrix, mvMatrix, degToRad(canvas.zRotAnim), [0, 0, 1]);

    gl.uniformMatrix4fv(cubeUniform_mvMatrix, false, mvMatrix);
    gl.drawElements(gl.TRIANGLES, 36, gl.UNSIGNED_SHORT, 0);

    // Draw first cube
    mat4.identity(mvMatrix);
    mat4.translate(mvMatrix, mvMatrix, [-(canvas.xRotAnim - 60.0) / 30.0,
                                        -(canvas.zRotAnim -  100.0) / 60.0,
                                        -20.0]);
    mat4.rotate(mvMatrix, mvMatrix, degToRad(10), [0, 1, 0]);
    mat4.rotate(mvMatrix, mvMatrix, degToRad(30), [1, 0, 0]);
    mat4.rotate(mvMatrix, mvMatrix, degToRad(canvas.xRotAnim), [0, 0, 1]);

    gl.uniformMatrix4fv(cubeUniform_mvMatrix, false, mvMatrix);
    gl.drawElements(gl.TRIANGLES, 36, gl.UNSIGNED_SHORT, 0);

    // Draw second cube
    mat4.identity(mvMatrix);
    mat4.translate(mvMatrix, mvMatrix, [(canvas.xRotAnim - 60.0) / 30.0,
                                        (canvas.zRotAnim -  100.0) / 60.0,
                                        -15.0]);
    mat4.rotate(mvMatrix, mvMatrix, degToRad(canvas.zRotAnim), [0, 1, 0]);
    mat4.rotate(mvMatrix, mvMatrix, degToRad(canvas.xRotAnim), [1, 0, 0]);
    mat4.rotate(mvMatrix, mvMatrix, degToRad(canvas.yRotAnim), [0, 0, 1]);

    gl.uniformMatrix4fv(cubeUniform_mvMatrix, false, mvMatrix);
    gl.drawElements(gl.TRIANGLES, 36, gl.UNSIGNED_SHORT, 0);

    // Draw third cube
    mat4.identity(mvMatrix);
    mat4.translate(mvMatrix, mvMatrix, [(canvas.yRotAnim - 120.0) / 120.0,
                                        (canvas.xRotAnim -  60.0) / 60.0,
                                        -10.0]);
    mat4.rotate(mvMatrix, mvMatrix, degToRad(canvas.yRotAnim) * degToRad(canvas.xRotAnim),
                [1, 0, 0]);
    mat4.rotate(mvMatrix, mvMatrix, degToRad(canvas.zRotAnim), [0, 0, 1]);

    gl.uniformMatrix4fv(cubeUniform_mvMatrix, false, mvMatrix);
    gl.drawElements(gl.TRIANGLES, 36, gl.UNSIGNED_SHORT, 0);

    gl.disableVertexAttribArray(cubeAttribute_vertexPosition);
    gl.disableVertexAttribArray(cubeAttribute_vertexColor);
    gl.disableVertexAttribArray(cubeAttribute_textureCoord);

}

function initBuffers()
{
    log("    initBuffers ENTER...");

    cubeVertexPositionBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVertexPositionBuffer);
    gl.bufferData(gl.ARRAY_BUFFER,
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
                                          -1.0,  1.0, -1.0]),
                  gl.STATIC_DRAW);

    cubeVertexIndexBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, cubeVertexIndexBuffer);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,
                  Arrays.newUint16Array([0,  1,  2,      0,  2,  3,    // front
                                         4,  5,  6,      4,  6,  7,    // back
                                         8,  9,  10,     8,  10, 11,   // top
                                         12, 13, 14,     12, 14, 15,   // bottom
                                         16, 17, 18,     16, 18, 19,   // right
                                         20, 21, 22,     20, 22, 23]), // left
                  gl.STATIC_DRAW);

    var colors = [[0.0,  1.0,  1.0,  1.0],    // Front face: cyan
                  [1.0,  0.0,  0.0,  1.0],    // Back face: red
                  [0.0,  1.0,  0.0,  1.0],    // Top face: green
                  [0.0,  0.0,  1.0,  1.0],    // Bottom face: blue
                  [1.0,  1.0,  0.0,  1.0],    // Right face: yellow
                  [1.0,  0.0,  1.0,  1.0]];   // Left face: purple

    var generatedColors = [];
    for (var j = 0; j < 6; j++) {
        var c = colors[j];

        for (var i = 0; i < 4; i++) {
            generatedColors = generatedColors.concat(c);
        }
    }

    cubeVertexColorBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVertexColorBuffer);
    gl.bufferData(gl.ARRAY_BUFFER,
                  Arrays.newFloat32Array(generatedColors),
                  gl.STATIC_DRAW);

    cubeVerticesTextureCoordBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVerticesTextureCoordBuffer);
    var textureCoordinates = [// Front
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
                              1.0,  1.0];
    gl.bufferData(gl.ARRAY_BUFFER,
                  Arrays.newFloat32Array(textureCoordinates),
                  gl.STATIC_DRAW);

    bkgVertexPositionBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, bkgVertexPositionBuffer);
    gl.bufferData(gl.ARRAY_BUFFER,
                  Arrays.newFloat32Array([-1.0, -1.0,
                                          1.0, -1.0,
                                          -1.0,  1.0,
                                          -1.0,  1.0,
                                          1.0, -1.0,
                                          1.0,  1.0]),
                  gl.STATIC_DRAW);

    bkgVerticesTextureCoordBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, bkgVerticesTextureCoordBuffer);
    gl.bufferData(gl.ARRAY_BUFFER,
                  Arrays.newFloat32Array([0.0, 0.0,
                                          1.0, 0.0,
                                          0.0, 1.0,
                                          0.0, 1.0,
                                          1.0, 0.0,
                                          1.0, 1.0]),
                  gl.STATIC_DRAW);
    log("    ...initBuffers EXIT")
}

function initShaders()
{
    // CUBE SHADER

    log("    initShaders ENTER...")
    var cubeVertexShader = getShader(gl,
                                     "attribute highp vec3 aVertexPosition;       \
                                      attribute mediump vec4 aVertexColor;        \
                                      attribute highp vec2 aTextureCoord;         \
                                                                                  \
                                      uniform mat4 uMVMatrix;                     \
                                      uniform mat4 uPMatrix;                      \
                                                                                  \
                                      varying mediump vec4 vColor;                \
                                      varying highp vec2 vTextureCoord;           \
                                      varying highp vec4 vPos;                    \
                                                                                  \
                                      void main(void) {                           \
                                          vPos = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);             \
                                          gl_Position = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);      \
                                          vColor = aVertexColor;                  \
                                          vTextureCoord = aTextureCoord;          \
                                      }", gl.VERTEX_SHADER);
    var cubeFragmentShader = getShader(gl,
                                       " \
                                        uniform highp float uTime;              \
                                                                                \
                                        varying mediump vec4 vColor;            \
                                        varying highp vec2 vTextureCoord;       \
                                        varying highp vec4 vPos;                \
                                                                                \
                                        uniform sampler2D uSampler;             \
                                                                                \
                                        void main(void) {                       \
                                            highp vec4 volScale = vec4(3.0, 3.0, 3.0, 2.0);                 \
                                            highp vec4 volume = vPos * volScale - volScale/2.0;             \
                                                                                                            \
                                            highp vec2 scaling = vec2(20.0,20.0);                           \
                                            highp vec2 coord = vTextureCoord * scaling - scaling/2.0;       \
                                            highp float value = sin(volume.x+coord.x+uTime);                \
                                            value += sin((volume.y+coord.y+uTime)/1.5);                     \
                                            value += sin((volume.z+coord.y+uTime)/1.5);                     \
                                                                                                            \
                                            coord += scaling/2.0 * vec2(sin(uTime/3.0), cos(uTime/2.0));    \
                                            value += sin(sqrt(coord.x*coord.x + coord.y*coord.y + 1.0) + uTime); \
                                            value = value/2.0;                                              \
                                            value = sin(3.14*value);                                          \
                                            highp vec4 col = vec4(value, value, value, 1.0);                \
                                            highp vec4 textureColor = texture2D(uSampler, vec2(vTextureCoord.s, vTextureCoord.t));   \
                                            gl_FragColor = vec4(col.rgb * vColor.rgb * textureColor.rgb, textureColor.a);            \
                                         }", gl.FRAGMENT_SHADER);

    //! [0]
    cubeShaderProgram = gl.createProgram();
    gl.attachShader(cubeShaderProgram, cubeVertexShader);
    gl.attachShader(cubeShaderProgram, cubeFragmentShader);
    gl.linkProgram(cubeShaderProgram);
    //! [0]

    if (!gl.getProgramParameter(cubeShaderProgram, gl.LINK_STATUS)) {
        console.log("Could not initialise shaders");
        console.log(gl.getProgramInfoLog(cubeShaderProgram));
    }

    //! [2]
    gl.useProgram(cubeShaderProgram);

    cubeAttribute_vertexPosition = gl.getAttribLocation(cubeShaderProgram, "aVertexPosition");
    cubeAttribute_vertexColor = gl.getAttribLocation(cubeShaderProgram, "aVertexColor");
    cubeAttribute_textureCoord = gl.getAttribLocation(cubeShaderProgram, "aTextureCoord");

    cubeUniform_time = gl.getUniformLocation(cubeShaderProgram, "uTime");
    cubeUniform_pMatrix = gl.getUniformLocation(cubeShaderProgram, "uPMatrix");
    cubeUniform_mvMatrix = gl.getUniformLocation(cubeShaderProgram, "uMVMatrix");
    cubeUniform_textureSampler = gl.getUniformLocation(cubeShaderProgram, "uSampler")

    //! [2]
    // BACKGROUND SHADER
    var bkgVertexShader = getShader(gl,
                                    "attribute highp vec2 aVertexPosition; \
                                     attribute highp vec2 aTextureCoord;   \
                                                                           \
                                     varying highp vec2 vTextureCoord;     \
                                                                           \
                                     void main(void) {                     \
                                         gl_Position = vec4(aVertexPosition, 0.0, 1.0); \
                                         vTextureCoord = aTextureCoord;                 \
                                     }", gl.VERTEX_SHADER);
    var bkgFragmentShader = getShader(gl,
                                      "uniform highp float uTime;                  \
                                       varying highp vec2 vTextureCoord;           \
                                                                                   \
                                       void main(void) {                           \
                                           highp vec2 position = vTextureCoord.xy; \
                                                                                   \
                                           highp float centerX = position.x - 0.5 + sin(uTime / 2.0) * 0.1; \
                                           highp float centerY = position.y - 0.5 + cos(uTime / 2.0) * 0.1; \
                                                                                                            \
                                           highp float x = log(sqrt(centerX*centerX + centerY*centerY));    \
                                           highp float y = atan(centerX, centerY);                          \
                                                                                                            \
                                           highp float color = cos(x * cos(uTime / 30.0) * 80.0) + cos(x * cos(uTime / 30.0) * 10.0) + \
                                                               cos(y * cos(uTime / 20.0) * 40.0) + cos(y * sin(uTime / 50.0) * 40.0);  \
                                           color *= 0.5;                                                                               \
                                                                                                                                       \
                                           gl_FragColor = vec4(color * sin(uTime / 10.0) * 0.5, color * (1.0 - sin(uTime / 10.0)) * 0.5, sin( color + uTime / 3.0 ) * 0.3, 1.0 ); \
                                       }", gl.FRAGMENT_SHADER);
    var bkgFragmentShaderBW = getShader(gl,
                                        "uniform highp float uTime;        \
                                         varying highp vec2 vTextureCoord; \
                                                                           \
                                         void main(void) {                 \
                                             highp vec2 position = vTextureCoord.xy; \
                                                                                     \
                                             highp float centerX = position.x - 0.5 + sin(uTime / 3.0) * 0.1; \
                                             highp float centerY = position.y - 0.5 + cos(uTime / 3.0) * 0.1; \
                                                                                                              \
                                             highp float x = log(sqrt(centerX*centerX + centerY*centerY));    \
                                             highp float y = atan(centerX, centerY);                          \
                                                                                                              \
                                             highp float color = cos(x * cos(uTime / 30.0) * 80.0) + cos(x * cos(uTime / 30.0) * 10.0); \
                                             color += cos(y * cos(uTime / 20.0) * 40.0) + cos(y * sin(uTime / 50.0) * 40.0);            \
                                             color *= 0.2;                                                                              \
                                             gl_FragColor = vec4(color, color, color, 1.0 );                                            \
                                         }", gl.FRAGMENT_SHADER);

    //! [1]
    bkgShaderProgram = gl.createProgram();
    gl.attachShader(bkgShaderProgram, bkgVertexShader);
    gl.attachShader(bkgShaderProgram, bkgFragmentShader);
    gl.linkProgram(bkgShaderProgram);
    //! [1]

    if (!gl.getProgramParameter(bkgShaderProgram, gl.LINK_STATUS)) {
        console.log("Could not initialize background shader");
        console.log(gl.getProgramInfoLog(bkgShaderProgram));
    }
    //! [3]

    gl.useProgram(bkgShaderProgram);

    bkgAttribute_vertexPosition = gl.getAttribLocation(bkgShaderProgram, "aVertexPosition");
    bkgAttribute_textureCoord = gl.getAttribLocation(bkgShaderProgram, "aTextureCoord");

    bkgUniform_time = gl.getUniformLocation(bkgShaderProgram, "uTime");
    gl.bindTexture(gl.TEXTURE_2D, 0);
    //! [3]

    log("    ...initShaders EXIT");
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
