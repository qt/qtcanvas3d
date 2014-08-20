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
    vertexShader = compileShader("attribute vec2 a_position; \
                                  void main() { \
                                      gl_Position = vec4(a_position, 0.0, 1.0); \
                                 }", gl.VERTEX_SHADER);
    fragmentShader = compileShader("void main() { \
                                        gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0); \
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
