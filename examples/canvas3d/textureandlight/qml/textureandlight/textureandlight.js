//! [0]
Qt.include("glMatrix-0.9.5.min.js")
//! [0]

//
// Draws a cube that has the Qt logo as decal texture on each face.
// A simple per vertex lighting equation is used to emulate light landing on the rotating cube.
//

var gl;
var cubeTexture = 0;
var vertexPositionAttribute;
var textureCoordAttribute;
var vertexNormalAttribute;
var vertexColorAttribute;
var mvMatrix = mat4.create();
var pMatrix  = mat4.create();
var nMatrix  = mat4.create();
var pMatrixUniform;
var mvMatrixUniform;
var nUniform;
var width = 0;
var height = 0;
var canvas3d;

function initGL(canvas, textureLoader) {
    canvas3d = canvas;
    //! [1]
    // Get the OpenGL context object that represents the API we call
    gl = canvas.getContext("canvas3d", {depth:true, antialias:true});
    //! [1]

    //! [2]
    // Setup the OpenGL state
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.DEPTH_LESS);
    gl.enable(gl.CULL_FACE);
    gl.cullFace(gl.BACK);
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.clearDepth(1.0);
    gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
    //! [2]

    // Set viewport
    gl.viewport(0, 0, canvas.width, canvas.height);

    // Initialize the shader program
    initShaders();

    // Initialize vertex and color buffers
    initBuffers();

    // Load the texture
    textureLoader.loadTexture("qtlogo.png");
}

//! [8]
function textureLoaded(textureImage) {
    if (textureImage.imageState == TextureImage.LOADING_FINISHED && cubeTexture  == 0) {
        if (canvas3d.logAllCalls)
            console.log("    processing "+textureImage.source);
        // Create the Texture3D object
        cubeTexture = gl.createTexture();
        // Bind it
        gl.bindTexture(gl.TEXTURE_2D, cubeTexture);
        // Set the properties
        gl.texImage2D(gl.TEXTURE_2D,    // target
                      0,                // level
                      gl.RGBA,          // internalformat
                      gl.RGBA,          // format
                      gl.UNSIGNED_BYTE, // type
                      textureImage);    // pixels
        // Set texture filtering parameters
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST);
        // Generate mipmap
        gl.generateMipmap(gl.TEXTURE_2D);
    }
}
//! [8]

function degToRad(degrees) {
    return degrees * Math.PI / 180;
}

function renderGL(canvas) {
    //! [9]
    // Check for resize
    var pixelRatio = canvas.devicePixelRatio;
    var currentWidth = canvas.width * pixelRatio;
    var currentHeight = canvas.height * pixelRatio;
    if (currentWidth !== width || currentHeight !== height ) {
        width = currentWidth;
        height = currentHeight;
        mat4.perspective(pMatrix, degToRad(45), width / height, 0.1, 500.0);
        gl.uniformMatrix4fva(pMatrixUniform, false, pMatrix);
    }
    //! [9]

    //! [10]
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    //! [10]

    //! [11]
    mat4.identity(mvMatrix);
    mat4.translate(mvMatrix, mvMatrix, [(canvas.yRotAnim - 120.0) / 120.0,
                                        (canvas.xRotAnim -  60.0) / 50.0,
                                        -10.0]);
    mat4.rotate(mvMatrix, mvMatrix, degToRad(canvas.xRotAnim), [0, 1, 0]);
    mat4.rotate(mvMatrix, mvMatrix, degToRad(canvas.yRotAnim), [1, 0, 0]);
    mat4.rotate(mvMatrix, mvMatrix, degToRad(canvas.zRotAnim), [0, 0, 1]);
    gl.uniformMatrix4fva(mvMatrixUniform, false, mvMatrix);
    //! [11]

    //! [12]
    mat4.invert(nMatrix, mvMatrix);
    mat4.transpose(nMatrix, nMatrix);
    gl.uniformMatrix4fva(nUniform, false, nMatrix);
    //! [12]

    //! [13]
    gl.drawElements(gl.TRIANGLES, 36, gl.UNSIGNED_SHORT, 0);
    //! [13]
}

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
    gl.vertexAttribPointer(vertexPositionAttribute, 3, gl.FLOAT, false, 0, 0);

    if (canvas3d.logAllCalls)
        console.log("        cubeVertexIndexBuffer");
    //! [7]
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
    //! [7]

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
    for (var j = 0; j < 6; j++) {
        var c = colors[j];

        for (var i = 0; i < 4; i++) {
            generatedColors = generatedColors.concat(c);
        }
    }
    gl.bufferData(gl.ARRAY_BUFFER, Arrays.newFloat32Array(generatedColors), gl.STATIC_DRAW);
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
    gl.vertexAttribPointer(textureCoordAttribute, 2, gl.FLOAT, false, 0, 0);


    var cubeVerticesNormalBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVerticesNormalBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, Arrays.newFloat32Array([
                                                              // Front
                                                              0.0,  0.0,  1.0,
                                                              0.0,  0.0,  1.0,
                                                              0.0,  0.0,  1.0,
                                                              0.0,  0.0,  1.0,

                                                              // Back
                                                              0.0,  0.0, -1.0,
                                                              0.0,  0.0, -1.0,
                                                              0.0,  0.0, -1.0,
                                                              0.0,  0.0, -1.0,

                                                              // Top
                                                              0.0,  1.0,  0.0,
                                                              0.0,  1.0,  0.0,
                                                              0.0,  1.0,  0.0,
                                                              0.0,  1.0,  0.0,

                                                              // Bottom
                                                              0.0, -1.0,  0.0,
                                                              0.0, -1.0,  0.0,
                                                              0.0, -1.0,  0.0,
                                                              0.0, -1.0,  0.0,

                                                              // Right
                                                              1.0,  0.0,  0.0,
                                                              1.0,  0.0,  0.0,
                                                              1.0,  0.0,  0.0,
                                                              1.0,  0.0,  0.0,

                                                              // Left
                                                              -1.0,  0.0,  0.0,
                                                              -1.0,  0.0,  0.0,
                                                              -1.0,  0.0,  0.0,
                                                              -1.0,  0.0,  0.0
                                                          ]), gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexNormalAttribute, 3, gl.FLOAT, false, 0, 0);
}

function initShaders()
{
    //! [3]
    var vertexShader = getShader(gl,
                                 "attribute highp vec3 aVertexNormal;
                                  attribute highp vec3 aVertexPosition;
                                  attribute mediump vec4 aVertexColor;
                                  attribute highp vec2 aTextureCoord;

                                  uniform highp mat4 uNormalMatrix;
                                  uniform mat4 uMVMatrix;
                                  uniform mat4 uPMatrix;

                                  varying mediump vec4 vColor;
                                  varying highp vec2 vTextureCoord;
                                  varying highp vec3 vLighting;

                                  void main(void) {
                                      gl_Position = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);
                                      vColor = aVertexColor;
                                      vTextureCoord = aTextureCoord;
                                      highp vec3 ambientLight = vec3(0.5, 0.5, 0.5);
                                      highp vec3 directionalLightColor = vec3(0.75, 0.75, 0.75);
                                      highp vec3 directionalVector = vec3(0.85, 0.8, 0.75);
                                      highp vec4 transformedNormal = uNormalMatrix * vec4(aVertexNormal, 1.0);
                                      highp float directional = max(dot(transformedNormal.xyz, directionalVector), 0.0);
                                      vLighting = ambientLight + (directionalLightColor * directional);
                                  }", gl.VERTEX_SHADER);
    //! [3]
    //! [4]
    var fragmentShader = getShader(gl,
                                   "varying mediump vec4 vColor;
                                    varying highp vec2 vTextureCoord;
                                    varying highp vec3 vLighting;

                                    uniform sampler2D uSampler;

                                    void main(void) {
                                        mediump vec4 texelColor = texture2D(uSampler, vec2(vTextureCoord.s, vTextureCoord.t));
                                        mediump vec3 blendColor = mix(vColor.rgb, texelColor.rgb, texelColor.a);
                                        gl_FragColor = vec4(blendColor * vLighting, 1.0);
                                    }", gl.FRAGMENT_SHADER);
    //! [4]
    //! [5]
    // Create the Program3D for shader
    var shaderProgram = gl.createProgram();

    // Attach the shader sources to the shader program
    gl.attachShader(shaderProgram, vertexShader);
    gl.attachShader(shaderProgram, fragmentShader);

    // Link the program
    gl.linkProgram(shaderProgram);

    // Check the linking status
    if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
        console.log("Could not initialise shaders");
        console.log(gl.getProgramInfoLog(shaderProgram));
    }

    // Take the shader program into use
    gl.useProgram(shaderProgram);
    //! [5]

    //! [6]
    // Look up where the vertex data needs to go
    vertexPositionAttribute = gl.getAttribLocation(shaderProgram, "aVertexPosition");
    gl.enableVertexAttribArray(vertexPositionAttribute);
    vertexColorAttribute = gl.getAttribLocation(shaderProgram, "aVertexColor");
    gl.enableVertexAttribArray(vertexColorAttribute);
    textureCoordAttribute = gl.getAttribLocation(shaderProgram, "aTextureCoord");
    gl.enableVertexAttribArray(textureCoordAttribute);
    vertexNormalAttribute = gl.getAttribLocation(shaderProgram, "aVertexNormal");
    gl.enableVertexAttribArray(vertexNormalAttribute);

    // Get the uniform locations
    pMatrixUniform = gl.getUniformLocation(shaderProgram, "uPMatrix");
    mvMatrixUniform = gl.getUniformLocation(shaderProgram, "uMVMatrix");
    nUniform = gl.getUniformLocation(shaderProgram, "uNormalMatrix");

    // Setup texture sampler uniform
    var textureSamplerUniform = gl.getUniformLocation(shaderProgram, "uSampler")
    gl.activeTexture(gl.TEXTURE0);
    gl.uniform1i(textureSamplerUniform, 0);
    gl.bindTexture(gl.TEXTURE_2D, 0);
    //! [6]
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
