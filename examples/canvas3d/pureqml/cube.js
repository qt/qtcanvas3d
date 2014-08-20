Qt.include("glMatrix-0.9.5.min.js")

//
// Draws a cube that has different colors assigned to the vertices.
// Each face of the cube has the linear interpolation of the corner colors.
//

var gl;
var vertexPositionAttrLoc;
var shaderProgram;
var cubeVertexPositionBuffer;
var cubeVertexIndexBuffer;
var cubeVertexColorBuffer;
var vertexShader;
var fragmentShader;
var vertexColorAttrLoc;
var mvMatrix = mat4.create();
var pMatrix = mat4.create();
var pMatrixUniformLoc;
var mvMatrixUniformLoc;

var canvas3d;

function log(message) {
    if (canvas3d.logAllCalls)
        console.log(message)
}

function initGL(canvas) {
    canvas3d = canvas
    try {
        // Get the context object that represents the 3D API
        gl = canvas3d.getContext("canvas3d", {depth:true, antialias:true});

        // Setup the OpenGL state
        gl.enable(gl.DEPTH_TEST);
        gl.depthMask(true);

        log("canvas width:"+canvas.width+" height:"+canvas.height+" devicePixelRatio:"+canvas.devicePixelRatio);
        gl.viewport(0, 0, canvas.width * canvas.devicePixelRatio, canvas.height * canvas.devicePixelRatio);

        // Initialize vertex and color buffers
        initBuffers();

        // Initialize the shader program
        initShaders();
    } catch(e) {
        console.log("initGL FAILURE!");
        console.log(""+e);
        console.log(""+e.message);
    }
}

function degToRad(degrees) {
    return degrees * Math.PI / 180;
}

function renderGL() {
    log("Render Enter *******")
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    mat4.perspective(pMatrix, degToRad(45), canvas3d.width / canvas3d.height, 0.1, 100.0);
    mat4.identity(mvMatrix);
    mat4.translate(mvMatrix, mvMatrix, [0.0, 0.0, -5.0]);
    //! [0]
    mat4.rotate(mvMatrix, mvMatrix, degToRad(canvas3d.xRotAnim), [0, 1, 0]);
    mat4.rotate(mvMatrix, mvMatrix, degToRad(canvas3d.yRotAnim), [1, 0, 0]);
    //! [0]

    gl.useProgram(shaderProgram);

    gl.uniformMatrix4fva(pMatrixUniformLoc, false, pMatrix);
    gl.uniformMatrix4fva(mvMatrixUniformLoc, false, mvMatrix);

    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVertexPositionBuffer);
    gl.enableVertexAttribArray(vertexPositionAttrLoc);
    gl.vertexAttribPointer(vertexPositionAttrLoc, 3, gl.FLOAT, false, 0, 0);

    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVertexColorBuffer);
    gl.enableVertexAttribArray(vertexColorAttrLoc);
    gl.vertexAttribPointer(vertexColorAttrLoc, 4, gl.FLOAT, false, 0, 0);

    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, cubeVertexIndexBuffer);
    gl.drawElements(gl.TRIANGLES, 36, gl.UNSIGNED_SHORT, 0);
    log("Render Exit *******")
}

function initBuffers() {
    // Create a cubeVertexPositionBuffer and put a single clipspace rectangle in
    // it (2 triangles)
    cubeVertexPositionBuffer = gl.createBuffer();
    cubeVertexPositionBuffer.name = "cubeVertexPositionBuffer";
    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVertexPositionBuffer);
    gl.bufferData(gl.ARRAY_BUFFER,
                  Arrays.newFloat32Array([ // front
                                          -1.0, -1.0,  1.0,
                                          1.0, -1.0,  1.0,
                                          1.0,  1.0,  1.0,
                                          -1.0,  1.0,  1.0,
                                          // back
                                          -1.0, -1.0, -1.0,
                                          1.0, -1.0, -1.0,
                                          1.0,  1.0, -1.0,
                                          -1.0,  1.0, -1.0]),
                  gl.STATIC_DRAW);

    cubeVertexIndexBuffer = gl.createBuffer();
    cubeVertexIndexBuffer.name = "cubeVertexIndexBuffer";
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, cubeVertexIndexBuffer);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,
                  Arrays.newUint16Array([// front
                                         0, 1, 2,
                                         2, 3, 0,
                                         // top
                                         3, 2, 6,
                                         6, 7, 3,
                                         // back
                                         7, 6, 5,
                                         5, 4, 7,
                                         // bottom
                                         4, 5, 1,
                                         1, 0, 4,
                                         // left
                                         4, 0, 3,
                                         3, 7, 4,
                                         // right
                                         1, 5, 6,
                                         6, 2, 1]),
                  gl.STATIC_DRAW);

    cubeVertexColorBuffer = gl.createBuffer();
    cubeVertexColorBuffer.name = "cubeVertexColorBuffer";
    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVertexColorBuffer);
    gl.bufferData(gl.ARRAY_BUFFER,
                  Arrays.newFloat32Array([// front
                                          0.000,  1.000,  0.000,
                                          0.000,  0.000,  1.000,
                                          1.000,  1.000,  1.000,
                                          1.000,  0.000,  0.000,
                                          // back
                                          1.000,  0.000,  1.000,
                                          1.000,  1.000,  0.000,
                                          0.000,  0.000,  0.000,
                                          0.000,  1.000,  1.000]),
                  gl.STATIC_DRAW);
}

function initShaders() {
    log("    Initializing shaders...");

    vertexShader = getShader(gl, "attribute highp vec3 aVertexPosition; \
                                  attribute highp vec4 aVertexColor; \
                                  uniform highp mat4 uMVMatrix; \
                                  uniform highp mat4 uPMatrix; \
                                  varying highp vec4 vColor; \
                                  void main(void) { \
                                      gl_Position = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0); \
                                      vColor = aVertexColor; \
                                  }", gl.VERTEX_SHADER);
    fragmentShader = getShader(gl, "varying highp vec4 vColor; \
                                    void main(void) { \
                                        gl_FragColor = vColor; \
                                    }", gl.FRAGMENT_SHADER);

    shaderProgram = gl.createProgram();
    shaderProgram.name = "shaderProgram";
    gl.attachShader(shaderProgram, vertexShader);
    gl.attachShader(shaderProgram, fragmentShader);
    gl.linkProgram(shaderProgram);

    if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
        console.log("Could not initialise shaders");
        console.log(gl.getProgramInfoLog(shaderProgram));
    }

    gl.useProgram(shaderProgram);

    // look up where the vertex data needs to go.
    vertexPositionAttrLoc = gl.getAttribLocation(shaderProgram, "aVertexPosition");
    gl.enableVertexAttribArray(vertexPositionAttrLoc);
    vertexColorAttrLoc = gl.getAttribLocation(shaderProgram, "aVertexColor");
    gl.enableVertexAttribArray(vertexColorAttrLoc);

    pMatrixUniformLoc  = gl.getUniformLocation(shaderProgram, "uPMatrix");
    pMatrixUniformLoc.name = "pMatrixUniformLoc";
    mvMatrixUniformLoc = gl.getUniformLocation(shaderProgram, "uMVMatrix");
    mvMatrixUniformLoc.name = "mvMatrixUniformLoc";
    log("    ...done")
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
