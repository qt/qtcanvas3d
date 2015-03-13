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

var gl;

function initGL(canvas, antialias, depth) {
    gl = canvas.getContext("depth-aa", {depth:depth, antialias:antialias});
}

function context() {
    return gl;
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
function createArray() {
    return new ArrayBuffer().byteLength;
}

function createArrayView() {
    return new Uint32Array().length;
}

var currentlyRunning = '';

function running(str) {
    currentlyRunning = str;
    output(str)
}

function output(str) {
    //console.log(str);
}

function pass(str) {
    var exc;
    if (str)
        exc = currentlyRunning + ': ' + str;
    else
        exc = currentlyRunning;
    output(exc);
    return true;
}

function fail(str) {
    var exc;
    if (str)
        exc = currentlyRunning + ': ' + str;
    else
        exc = currentlyRunning;
    output(exc);
    return false;
}

function assertEq(prefix, expected, val) {
    if (expected !== val) {
        var str = prefix + ': expected ' + expected + ', got ' + val;
        throw str;
    }
}

function assert(prefix, expected) {
    if (!expected) {
        var str = prefix + ': expected value / true';
        throw str;
    }
}

var buffer;
var byteLength;
var subBuffer;
var subArray;
function testSlice() {
    function test(subBuf, starts, size) {
        byteLength = size;
        subBuffer = eval(subBuf);
        subArray = new Int8Array(subBuffer);
        assertEq(subBuf, subBuffer.byteLength, byteLength);
        for (var i = 0; i < size; ++i)
            assertEq('Element ' + i, starts + i, subArray[i]);
    }
    try {
        running('testSlice');
        buffer = new ArrayBuffer(32);
        var array = new Int8Array(buffer);
        for (var i = 0; i < 32; ++i)
            array[i] = i;
        test("buffer.slice(0)", 0, 32);
        test("buffer.slice(16)", 16, 16);
        test("buffer.slice(24)", 24, 8);
        test("buffer.slice(32)", 32, 0);
        test("buffer.slice(40)", 32, 0);
        test("buffer.slice(80)", 32, 0);
        test("buffer.slice(-8)", 24, 8);
        test("buffer.slice(-16)", 16, 16);
        test("buffer.slice(-24)", 8, 24);
        test("buffer.slice(-32)", 0, 32);
        test("buffer.slice(-40)", 0, 32);
        test("buffer.slice(-80)", 0, 32);
        test("buffer.slice(0, 32)", 0, 32);
        test("buffer.slice(0, 16)", 0, 16);
        test("buffer.slice(8, 24)", 8, 16);
        test("buffer.slice(16, 32)", 16, 16);
        test("buffer.slice(24, 16)", 24, 0);
        test("buffer.slice(16, -8)", 16, 8);
        test("buffer.slice(-20, 30)", 12, 18);
        test("buffer.slice(-8, -20)", 24, 0);
        test("buffer.slice(-20, -8)", 12, 12);
        test("buffer.slice(-40, 16)", 0, 16);
        test("buffer.slice(-40, 40)", 0, 32);
        return pass();
    } catch (e) {
        return fail(e);
    }
}
function testArrayBufferIsViewMethod() {
    output('test ArrayBuffer.isView() with various values');
    var retval = true;
    try {
        if (!ArrayBuffer.isView) {
            return fail('ArrayBuffer.isView() method does not exist');
        } else {
            output('ArrayBuffer.isView() method exists');
            retval = shouldBe('ArrayBuffer.isView(new Int8Array(1))', 'true');
            if (!retval) return false;
            retval = shouldBe('ArrayBuffer.isView(new Uint8Array(1))', 'true');
            if (!retval) return false;
            retval = shouldBe('ArrayBuffer.isView(new Uint8ClampedArray(1))', 'true');
            if (!retval) return false;
            retval = shouldBe('ArrayBuffer.isView(new Int16Array(1))', 'true');
            if (!retval) return false;
            retval = shouldBe('ArrayBuffer.isView(new Uint16Array(1))', 'true');
            if (!retval) return false;
            retval = shouldBe('ArrayBuffer.isView(new Int32Array(1))', 'true');
            if (!retval) return false;
            retval = shouldBe('ArrayBuffer.isView(new Uint32Array(1))', 'true');
            if (!retval) return false;
            retval = shouldBe('ArrayBuffer.isView(new Float32Array(1))', 'true');
            if (!retval) return false;
            retval = shouldBe('ArrayBuffer.isView(new Float64Array(1))', 'true');
            if (!retval) return false;
            retval = shouldBe('ArrayBuffer.isView(new DataView(new ArrayBuffer(8)))', 'true');
            if (!retval) return false;
            // TODO: All these fail. V4VM?
            //retval = shouldBe('ArrayBuffer.isView(undefined)', 'false');
            //if (!retval) return false;
            //retval = shouldBe('ArrayBuffer.isView(null)', 'false');
            //if (!retval) return false;
            //retval = shouldBe('ArrayBuffer.isView(true)', 'false');
            //if (!retval) return false;
            //retval = shouldBe('ArrayBuffer.isView(false)', 'false');
            //if (!retval) return false;
            //retval = shouldBe('ArrayBuffer.isView(0)', 'false');
            //if (!retval) return false;
            //retval = shouldBe('ArrayBuffer.isView(1)', 'false');
            //if (!retval) return false;
            //retval = shouldBe('ArrayBuffer.isView(1.0)', 'false');
            //if (!retval) return false;
            //retval = shouldBe('ArrayBuffer.isView("hello")', 'false');
            //if (!retval) return false;
            //retval = shouldBe('ArrayBuffer.isView({})', 'false');
            //if (!retval) return false;
            //retval = shouldBe('ArrayBuffer.isView(function() {})', 'false');
            //if (!retval) return false;
            //retval = shouldBe('ArrayBuffer.isView(new Array(1))', 'false');
        }
    } catch (e) {
        return fail('Exception thrown while testing ArrayBuffer.isView method: ' + e);
    }
    return retval;
}
function testInheritanceHierarchy() {
    output('test inheritance hierarchy of typed array views');
    var retval = true;
    try {
        var foo = ArrayBufferView;
        return fail('ArrayBufferView has [NoInterfaceObject] extended attribute and should not be defined');
    } catch (e) {
        output('ArrayBufferView has [NoInterfaceObject] extended attribute and was (correctly) not defined');
    }
    // Uint8ClampedArray inherited from Uint8Array in earlier versions
    // of the typed array specification. Since this is no longer the
    // case, assert the new behavior.
    retval = shouldBe('new Uint8ClampedArray(1) instanceof Uint8Array', 'false');
    if (!retval) return false;
    retval = shouldBe('Object.getPrototypeOf(Int8Array.prototype)', 'Object.prototype');
    if (!retval) return false;
    retval = shouldBe('Object.getPrototypeOf(Uint8Array.prototype)', 'Object.prototype');
    if (!retval) return false;
    retval = shouldBe('Object.getPrototypeOf(Uint8ClampedArray.prototype)', 'Object.prototype');
    if (!retval) return false;
    retval = shouldBe('Object.getPrototypeOf(Int16Array.prototype)', 'Object.prototype');
    if (!retval) return false;
    retval = shouldBe('Object.getPrototypeOf(Uint16Array.prototype)', 'Object.prototype');
    if (!retval) return false;
    retval = shouldBe('Object.getPrototypeOf(Int32Array.prototype)', 'Object.prototype');
    if (!retval) return false;
    retval = shouldBe('Object.getPrototypeOf(Uint32Array.prototype)', 'Object.prototype');
    if (!retval) return false;
    retval = shouldBe('Object.getPrototypeOf(Float32Array.prototype)', 'Object.prototype');
    if (!retval) return false;
    retval = shouldBe('Object.getPrototypeOf(Float64Array.prototype)', 'Object.prototype');
    if (!retval) return false;
    retval = shouldBe('Object.getPrototypeOf(DataView.prototype)', 'Object.prototype');
    return retval;
}

//
// Tests for unsigned array variants
//
function testSetAndGet10To1(type, name) {
    running('test ' + name + ' SetAndGet10To1');
    try {
        var array = new type(10);
        for (var i = 0; i < 10; i++) {
            array[i] = 10 - i;
        }
        for (var i = 0; i < 10; i++) {
            assertEq('Element ' + i, 10 - i, array[i]);
        }
        return pass();
    } catch (e) {
        return fail(e);
    }
}

function testConstructWithArrayOfUnsignedValues(type, name) {
    running('test ' + name + ' ConstructWithArrayOfUnsignedValues');
    try {
        var array = new type([10, 9, 8, 7, 6, 5, 4, 3, 2, 1]);
        assertEq('Array length', 10, array.length);
        for (var i = 0; i < 10; i++) {
            assertEq('Element ' + i, 10 - i, array[i]);
        }
        return pass();
    } catch (e) {
        return fail(e);
    }
}

function testConstructWithTypedArrayOfUnsignedValues(type, name) {
    running('test ' + name + ' ConstructWithTypedArrayOfUnsignedValues');
    try {
        var tmp = new type([10, 9, 8, 7, 6, 5, 4, 3, 2, 1]);
        var array = new type(tmp);
        assertEq('Array length', 10, array.length);
        for (var i = 0; i < 10; i++) {
            assertEq('Element ' + i, 10 - i, array[i]);
        }
        return pass();
    } catch (e) {
        return fail(e);
    }
}

//
// Tests for signed array variants
//
function testSetAndGetPos10ToNeg10(type, name) {
    running('test ' + name + ' SetAndGetPos10ToNeg10');
    try {
        var array = new type(21);
        for (var i = 0; i < 21; i++) {
            array[i] = 10 - i;
        }
        for (i = 0; i < 21; i++) {
            assertEq('Element ' + i, 10 - i, array[i]);
        }
        return pass();
    } catch (e) {
        return fail(e);
    }
}

function testConstructWithArrayOfSignedValues(type, name) {
    running('test ' + name + ' ConstructWithArrayOfSignedValues');
    try {
        var array = new type([10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10]);
        assertEq('Array length', 21, array.length);
        for (var i = 0; i < 21; i++) {
            assertEq('Element ' + i, 10 - i, array[i]);
        }
        return pass();
    } catch (e) {
        return fail(e);
    }
}

function testConstructWithTypedArrayOfSignedValues(type, name) {
    running('test ' + name + ' ConstructWithTypedArrayOfSignedValues');
    try {
        var tmp = new type([10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10]);
        var array = new type(tmp);
        assertEq('Array length', 21, array.length);
        for (var i = 0; i < 21; i++) {
            assertEq('Element ' + i, 10 - i, array[i]);
        }
        return pass();
    } catch (e) {
        return fail(e);
    }
}

//
// Test cases for integral types.
// Some JavaScript engines need separate copies of this code in order
// to exercise all of their optimized code paths.
//
function testIntegralArrayTruncationBehavior(type, name, unsigned) {
    running('test integral array truncation behavior for ' + name);
    var sourceData;
    var expectedResults;
    if (unsigned) {
        sourceData = [0.6, 10.6];
        expectedResults = [0, 10];
    } else {
        sourceData = [0.6, 10.6, -0.6, -10.6];
        expectedResults = [0, 10, 0, -10];
    }
    var numIterations = 10;
    var array = new type(numIterations);
    // The code block in each of the case statements below is identical, but some
    // JavaScript engines need separate copies in order to exercise all of
    // their optimized code paths.
    try {
        switch (type) {
        case Int8Array:
            for (var ii = 0; ii < sourceData.length; ++ii) {
                for (var jj = 0; jj < numIterations; ++jj) {
                    array[jj] = sourceData[ii];
                    assertEq('Storing ' + sourceData[ii], expectedResults[ii], array[jj]);
                }
            }
            break;
        case Int16Array:
            for (ii = 0; ii < sourceData.length; ++ii) {
                for (jj = 0; jj < numIterations; ++jj) {
                    array[jj] = sourceData[ii];
                    assertEq('Storing ' + sourceData[ii], expectedResults[ii], array[jj]);
                }
            }
            break;
        case Int32Array:
            for (ii = 0; ii < sourceData.length; ++ii) {
                for (jj = 0; jj < numIterations; ++jj) {
                    array[jj] = sourceData[ii];
                    assertEq('Storing ' + sourceData[ii], expectedResults[ii], array[jj]);
                }
            }
            break;
        case Uint8Array:
            for (ii = 0; ii < sourceData.length; ++ii) {
                for (jj = 0; jj < numIterations; ++jj) {
                    array[jj] = sourceData[ii];
                    assertEq('Storing ' + sourceData[ii], expectedResults[ii], array[jj]);
                }
            }
            break;
        case Uint16Array:
            for (ii = 0; ii < sourceData.length; ++ii) {
                for (jj = 0; jj < numIterations; ++jj) {
                    array[jj] = sourceData[ii];
                    assertEq('Storing ' + sourceData[ii], expectedResults[ii], array[jj]);
                }
            }
            break;
        case Uint32Array:
            for (ii = 0; ii < sourceData.length; ++ii) {
                for (jj = 0; jj < numIterations; ++jj) {
                    array[jj] = sourceData[ii];
                    assertEq('Storing ' + sourceData[ii], expectedResults[ii], array[jj]);
                }
            }
            break;
        default:
            fail("Unhandled type");
            break;
        }
        return pass();
    } catch (e) {
        return fail(e);
    }
}

//
// Test cases for both signed and unsigned types
//
function testGetWithOutOfRangeIndices(type, name) {
    var retval = true;
    console.log('Testing ' + name + ' GetWithOutOfRangeIndices');
    // See below for declaration of this global variable
    array = new type([2, 3]);
    retval = shouldBeUndefined("array[2]");//(array[2] === undefined);
    if (retval)
        retval = shouldBeUndefined("array[-1]");//(array[-1] === undefined);
    if (retval)
        retval = shouldBeUndefined("array[0x20000000]");//(array[0x20000000] === undefined);
    return retval;
}

function testOffsetsAndSizes(type, name, elementSizeInBytes) {
    running('test ' + name + ' OffsetsAndSizes');
    try {
        var len = 10;
        assertEq('type.BYTES_PER_ELEMENT', elementSizeInBytes, type.BYTES_PER_ELEMENT);
        var array = new type(len);
        assert('array.buffer', array.buffer);
        assertEq('array.byteOffset', 0, array.byteOffset);
        assertEq('array.length', len, array.length);
        assertEq('array.byteLength', len * elementSizeInBytes, array.byteLength);
        array = new type(array.buffer, elementSizeInBytes, len - 1);
        assert('array.buffer', array.buffer);
        assertEq('array.byteOffset', elementSizeInBytes, array.byteOffset);
        assertEq('array.length', len - 1, array.length);
        assertEq('array.byteLength', (len - 1) * elementSizeInBytes, array.byteLength);
        return pass();
    } catch (e) {
        return fail(e);
    }
}

function testSetFromTypedArray(type, name) {
    running('test ' + name + ' SetFromTypedArray');
    try {
        var array = new type(10);
        var array2 = new type(5);
        for (var i = 0; i < 10; i++) {
            assertEq('Element ' + i, 0, array[i]);
        }
        for (var i = 0; i < array2.length; i++) {
            array2[i] = i;
        }
        array.set(array2);
        for (var i = 0; i < array2.length; i++) {
            assertEq('Element ' + i, i, array[i]);
        }
        array.set(array2, 5);
        for (var i = 0; i < array2.length; i++) {
            assertEq('Element ' + i, i, array[5 + i]);
        }
        return pass();
    } catch (e) {
        return fail(e);
    }
}

function negativeTestSetFromTypedArray(type, name) {
    running('negativeTest ' + name + ' SetFromTypedArray');
    try {
        var array = new type(5);
        var array2 = new type(6);
        for (var i = 0; i < 5; i++) {
            assertEq('Element ' + i, 0, array[i]);
        }
        for (var i = 0; i < array2.length; i++) {
            array2[i] = i;
        }
        try {
            array.set(array2);
            fail('Expected exception from array.set(array2)');
            return;
        } catch (e) {
        }
        try {
            array2.set(array, 2);
            fail('Expected exception from array2.set(array, 2)');
            return;
        } catch (e) {
        }
        return pass();
    } catch (e) {
        return fail(e);
    }
}

function testSetFromArray(type, name) {
    running('test ' + name + ' SetFromArray');
    try {
        var array = new type(10);
        var array2 = [10, 9, 8, 7, 6, 5, 4, 3, 2, 1];
        for (var i = 0; i < 10; i++) {
            assertEq('Element ' + i, 0, array[i]);
        }
        array.set(array2, 0);
        for (var i = 0; i < array2.length; i++) {
            assertEq('Element ' + i, 10 - i, array[i]);
        }
        return pass();
    } catch (e) {
        return fail(e);
    }
}

function negativeTestSetFromArray(type, name) {
    running('negativeTest ' + name + ' SetFromArray');
    try {
        var array = new type([2, 3]);
        try {
            array.set([4, 5], 1);
            fail();
            return;
        } catch (e) {
        }
        try {
            array.set([4, 5, 6]);
            fail();
            return;
        } catch (e) {
        }
        return pass();
    } catch (e) {
        return fail(e);
    }
}

function testSubarray(type, name) {
    running('test ' + name + ' Subarray');
    try {
        var array = new type([0, 1, 2, 3, 4, 5, 6, 7, 8, 9]);
        var subarray = array.subarray(0, 5);
        assertEq('subarray.length', 5, subarray.length);
        for (var i = 0; i < 5; i++) {
            assertEq('Element ' + i, i, subarray[i]);
        }
        subarray = array.subarray(4, 10);
        assertEq('subarray.length', 6, subarray.length);
        for (i = 0; i < 6; i++) {
            assertEq('Element ' + i, 4 + i, subarray[i]);
        }
        return pass();
    } catch (e) {
        return fail(e);
    }
}

function negativeTestSubarray(type, name) {
    running('negativeTest ' + name + ' Subarray');
    try {
        var array = new type([0, 1, 2, 3, 4, 5, 6, 7, 8, 9]);
        subarray = array.subarray(5, 11);
        if (subarray.length !== 5) {
            return fail();
        }
        subarray = array.subarray(10, 10);
        if (subarray.length !== 0) {
            return fail();
        }
        return pass();
    } catch (e) {
        console.log("subarrayfail")
        return fail(e);
    }
}

function testSetBoundaryConditions(type, name, testValues, expectedValues) {
    running('test ' + name + ' SetBoundaryConditions');
    try {
        var array = new type(1);
        assertEq('Array length', 1, array.length);
        for (var ii = 0; ii < testValues.length; ++ii) {
            for (var jj = 0; jj < 10; ++jj) {
                array[0] = testValues[ii];
                assertEq('Element 0', expectedValues[ii], array[0]);
            }
        }
        return pass();
    } catch (e) {
        return fail(e);
    }
}

function testConstructionBoundaryConditions(type, name, testValues, expectedValues) {
    running('test ' + name + ' ConstructionBoundaryConditions');
    try {
        var array = new type(testValues);
        assertEq('Array length', testValues.length, array.length);
        for (var ii = 0; ii < testValues.length; ++ii) {
            assertEq('Element ' + ii, expectedValues[ii], array[ii]);
        }
        return pass();
    } catch (e) {
        return fail(e);
    }
}

function testConstructionWithNullBuffer(type, name) {
    var array;
    try {
        array = new type(null);
        return fail("Construction of " + name + " with null buffer should throw exception");
    } catch (e) {
        output("Construction of " + name + " with null buffer threw exception");
    }
    try {
        array = new type(null, 0, 0);
        return fail("Construction of " + name + " with (null buffer, 0) should throw exception");
    } catch (e) {
        output("Construction of " + name + " with (null buffer, 0) threw exception");
    }
    try {
        array = new type(null, 0, 0);
        return fail("Construction of " + name + " with (null buffer, 0, 0) should throw exception");
    } catch (e) {
        output("Construction of " + name + " with (null buffer, 0, 0) threw exception");
    }
    return pass();
}

function shouldThrowIndexSizeErr(func, text) {
    var errorText = text + " should throw an exception";
    try {
        func();
        return fail(errorText);
    } catch (e) {
        return pass(text + " threw an exception");
    }
}

function testConstructionWithOutOfRangeValues(type, name) {
    return shouldThrowIndexSizeErr(function() {
        var buffer = new ArrayBuffer(4);
        var array = new type(buffer, 4, 0x3FFFFFFF);
    }, "Construction of " + name + " with out-of-range values");
}

function testConstructionWithNegativeOutOfRangeValues(type, name) {
    try {
        var buffer = new ArrayBuffer(-1);
        return fail("Construction of ArrayBuffer with negative size should throw exception");
    } catch (e) {
        output("Construction of ArrayBuffer with negative size threw exception");
    }
    try {
        var array = new type(-1);
        return fail("Construction of " + name + " with negative size should throw exception");
    } catch (e) {
        output("Construction of " + name + " with negative size threw exception");
    }
    return shouldThrowIndexSizeErr(function() {
        var buffer = new ArrayBuffer(4);
        var array = new type(buffer, 4, -2147483648);
    }, "Construction of " + name + " with negative out-of-range values");
}

function testConstructionWithUnalignedOffset(type, name, elementSizeInBytes) {
    if (elementSizeInBytes > 1) {
        return shouldThrowIndexSizeErr(function() {
            var buffer = new ArrayBuffer(32);
            var array = new type(buffer, 1, elementSizeInBytes);
        }, "Construction of " + name + " with unaligned offset");
    }
}

function testConstructionWithUnalignedLength(type, name, elementSizeInBytes) {
    if (elementSizeInBytes > 1) {
        return shouldThrowIndexSizeErr(function() {
            var buffer = new ArrayBuffer(elementSizeInBytes + 1);
            var array = new type(buffer, 0);
        }, "Construction of " + name + " with unaligned length");
    }
}

function testConstructionOfHugeArray(type, name, sz) {
    if (sz === 1)
        return;
    try {
        // Construction of huge arrays must fail because byteLength is
        // an unsigned long
        array = new type(3000000000);
        return fail("Construction of huge " + name + " should throw exception");
    } catch (e) {
        return pass("Construction of huge " + name + " threw exception");
    }
}

function testConstructionWithBothArrayBufferAndLength(type, name, elementSizeInBytes) {
    var bufByteLength = 1000 * elementSizeInBytes;
    var buf = new ArrayBuffer(bufByteLength);
    var array1 = new type(buf);
    var array2 = new type(bufByteLength / elementSizeInBytes);
    if (array1.length === array2.length) {
        return pass("Array lengths matched with explicit and implicit creation of ArrayBuffer");
    } else {
        return fail("Array lengths DID NOT MATCH with explicit and implicit creation of ArrayBuffer");
    }
}

// These need to be global for shouldBe to see them
var array;
var typeSize;
function testSubarrayWithOutOfRangeValues(type, name, sz) {
    console.log("Testing subarray of " + name);
    var retval = true;
    try {
        var buffer = new ArrayBuffer(32);
        array = new type(buffer);
        typeSize = sz;
        retval = shouldBe("array.length", "32 / typeSize");
        try {
            if (retval)
                retval = shouldBe("array.subarray(4, 0x3FFFFFFF).length", "(32 / typeSize) - 4");
            if (retval)
                retval = shouldBe("array.subarray(4, -2147483648).length", "0");
            // Test subarray() against overflows.
            array = array.subarray(2);
            if (sz > 1) {
                // Full byte offset is +1 larger than the maximum unsigned long int.
                // Make sure subarray() still handles it correctly. Otherwise overflow would happen and
                // offset would be 0, and array.length array.length would incorrectly be 1.
                var start = 4294967296 / sz - 2;
                array = array.subarray(start, start + 1);
                if (retval)
                    retval = shouldBe("array.length", "0");
            }
        } catch (e) {
            return fail("Subarray of " + name + " threw exception");
        }
    } catch (e) {
        return fail("Exception: " + e);
    }
    return retval;
}

function testSubarrayWithDefaultValues(type, name, sz) {
    console.log("Testing subarray with default inputs of " + name);
    var retval = true;
    try {
        var buffer = new ArrayBuffer(32);
        array = new type(buffer);
        typeSize = sz;
        retval = shouldBe("array.length", "32 / typeSize");
        try {
            if (retval)
                retval = shouldBe("array.subarray(0).length", "(32 / typeSize)");
            if (retval)
                retval = shouldBe("array.subarray(2).length", "(32 / typeSize) - 2");
            if (retval)
                retval = shouldBe("array.subarray(-2).length", "2");
            if (retval)
                retval = shouldBe("array.subarray(-2147483648).length", "(32 / typeSize)");
        } catch (e) {
            return fail("Subarray of " + name + " threw exception");
        }
    } catch (e) {
        return fail("Exception: " + e);
    }
    return retval;
}

function testSettingFromArrayWithOutOfRangeOffset(type, name) {
    var webglArray = new type(32);
    var array = [];
    for (var i = 0; i < 16; i++) {
        array.push(i);
    }
    try {
        webglArray.set(array, 0x7FFFFFF8);
        return fail("Setting " + name + " from array with out-of-range offset was not caught");
    } catch (e) {
        return pass("Setting " + name + " from array with out-of-range offset was caught");
    }
}

function testSettingFromFakeArrayWithOutOfRangeLength(type, name) {
    var webglArray = new type(32);
    var array = {};
    array.length = 0x80000000;
    try {
        webglArray.set(array, 8);
        return fail("Setting " + name + " from fake array with invalid length was not caught");
    } catch (e) {
        return pass("Setting " + name + " from fake array with invalid length was caught");
    }
}

function testSettingFromTypedArrayWithOutOfRangeOffset(type, name) {
    var webglArray = new type(32);
    var srcArray = new type(16);
    for (var i = 0; i < 16; i++) {
        srcArray[i] = i;
    }
    try {
        webglArray.set(srcArray, 0x7FFFFFF8);
        return fail("Setting " + name + " from " + name + " with out-of-range offset was not caught");
    } catch (e) {
        return pass("Setting " + name + " from " + name + " with out-of-range offset was caught");
    }
}

function negativeTestGetAndSetMethods(type, name) {
    array = new type([2, 3]);
    shouldBeUndefined("array.get");
    var exceptionThrown = false;
    // We deliberately check for an exception here rather than using
    // shouldThrow here because the precise contents of the syntax
    // error are not specified.
    try {
        webGLArray.set(0, 1);
    } catch (e) {
        exceptionThrown = true;
    }
    var txt = "array.set(0, 1) ";
    if (exceptionThrown) {
        return pass(txt + "threw exception.");
    } else {
        return fail(txt + "did not throw exception.");
    }
}

function testNaNConversion(type, name) {
    running('test storing NaN in ' + name);
    var array = new type([1, 1]);
    var results = [];
    // The code block in each of the case statements below is identical, but some
    // JavaScript engines need separate copies in order to exercise all of
    // their optimized code paths.
    try {
        switch (type) {
        case Float32Array:
            for (var i = 0; i < array.length; ++i) {
                array[i] = NaN;
                results[i] = array[i];
            }
            break;
        case Int8Array:
            for (i = 0; i < array.length; ++i) {
                array[i] = NaN;
                results[i] = array[i];
            }
            break;
        case Int16Array:
            for (i = 0; i < array.length; ++i) {
                array[i] = NaN;
                results[i] = array[i];
            }
            break;
        case Int32Array:
            for (i = 0; i < array.length; ++i) {
                array[i] = NaN;
                results[i] = array[i];
            }
            break;
        case Uint8Array:
            for (i = 0; i < array.length; ++i) {
                array[i] = NaN;
                results[i] = array[i];
            }
            break;
        case Uint16Array:
            for (i = 0; i < array.length; ++i) {
                array[i] = NaN;
                results[i] = array[i];
            }
            break;
        case Uint32Array:
            for (i = 0; i < array.length; ++i) {
                array[i] = NaN;
                results[i] = array[i];
            }
            break;
        default:
            return fail("Unhandled type");
        }
        // Some types preserve NaN values; all other types convert NaN to zero.
        if (type === Float32Array) {
            assert('initial NaN preserved', isNaN(new type([NaN])[0]));
            for (var i = 0; i < array.length; ++i)
                assert('NaN preserved via setter', isNaN(results[i]));
        } else {
            assertEq('initial NaN converted to zero', 0, new type([NaN])[0]);
            for (var i = 0; i < array.length; ++i)
                assertEq('NaN converted to zero by setter', 0, results[i]);
        }
        return pass();
    } catch (e) {
        return fail(e);
    }
}

function testConstructionOfHugeArray(type, name, sz) {
    if (sz === 1)
        return;
    try {
        // Construction of huge arrays must fail because byteLength is
        // an unsigned long
        array = new type(3000000000);
        return fail("Construction of huge " + name + " should throw exception");
    } catch (e) {
        return pass("Construction of huge " + name + " threw exception");
    }
}

function dataView() {
    var array = new Uint8Array([164, 112, 157, 63]);
    var dataview;
    var retval = true;
    retval = shouldThrow("dataview = new DataView(array.buffer, -4500000000)");
    if (!retval) return false;
    retval = shouldThrow("dataview = new DataView(array.buffer, -4500000000, 4500000000)");
    if (!retval) return false;
    var value = dataview ? dataview.getFloat32(0, true) : 0;
    return retval;
}

var intArray1 = [0, 1, 2, 3, 100, 101, 102, 103, 128, 129, 130, 131, 252, 253, 254, 255];
var intArray2 = [31, 32, 33, 0, 1, 2, 3, 100, 101, 102, 103, 128, 129, 130, 131, 252, 253, 254, 255];
var emptyArray = [204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204, 204];
var arrayBuffer = null;
var view = null;
var viewStart = 0;
var viewLength = 0;

function getElementSize(func)
{
    switch (func) {
    case "Int8":
    case "Uint8":
        return 1;
    case "Int16":
    case "Uint16":
        return 2;
    case "Int32":
    case "Uint32":
    case "Float32":
        return 4;
    case "Float64":
        return 8;
    default:
        output("Should not be reached");
    }
}

function checkGet(func, index, expected, littleEndian)
{
    var expr = "view.get" + func + "(" + index;
    if (littleEndian !== undefined) {
        expr += ", ";
        expr += littleEndian ? "true" : "false";
    }
    expr += ")";
    running(expr)
    if (index >= 0 && index + getElementSize(func) - 1 < view.byteLength) {
        output("shouldbe " + expr + " ," + expected)
        return shouldBe(expr, expected);
    } else {
        output("shouldthrow " + expr)
        return shouldThrow(expr);
    }
}

function checkSet(func, index, value, littleEndian)
{
    var expr = "view.set" + func + "(" + index + ", " + value;
    var retval = true;
    if (littleEndian !== undefined) {
        expr += ", ";
        expr += littleEndian ? "true" : "false";
    }
    expr += ")";
    running(expr)
    if (index >= 0 && index + getElementSize(func) - 1 < view.byteLength) {
        retval = shouldBeUndefined(expr);
        if (!retval) return fail();
        return checkGet(func, index, value, littleEndian);
    } else
        return shouldThrow(expr);
}

function test(isTestingGet, func, index, value, littleEndian)
{
    if (isTestingGet)
        return checkGet(func, index, value, littleEndian);
    else
        return checkSet(func, index, value, littleEndian);
}

function createDataView(array, frontPaddingNum, littleEndian, start, length)
{
    if (!littleEndian)
        array.reverse();
    var paddingArray = new Array(frontPaddingNum);
    arrayBuffer = (new Uint8Array(paddingArray.concat(array))).buffer;
    viewStart = (start !== undefined) ? start : 0;
    viewLength = (length !== undefined) ? length : arrayBuffer.byteLength - viewStart;
    view = new DataView(arrayBuffer, viewStart, viewLength);
    if (!littleEndian)
        array.reverse(); // restore the array.
}

function runIntegerTestCases(isTestingGet, array, start, length)
{
    createDataView(array, 0, true, start, length);
    var retval = true;
    retval = test(isTestingGet, "Int8", 0, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Int8", 8, "-128");
    if (!retval) return fail();
    retval = test(isTestingGet, "Int8", 15, "-1");
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint8", 0, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint8", 8, "128");
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint8", 15, "255");
    // Little endian.
    if (!retval) return fail();
    retval = test(isTestingGet, "Int16", 0, "256", true);
    if (!retval) return fail();
    retval = test(isTestingGet, "Int16", 5, "26213", true);
    if (!retval) return fail();
    retval = test(isTestingGet, "Int16", 9, "-32127", true);
    if (!retval) return fail();
    retval = test(isTestingGet, "Int16", 14, "-2", true);
    // Big endian.
    if (!retval) return fail();
    retval = test(isTestingGet, "Int16", 0, "1");
    if (!retval) return fail();
    retval = test(isTestingGet, "Int16", 5, "25958");
    if (!retval) return fail();
    retval = test(isTestingGet, "Int16", 9, "-32382");
    if (!retval) return fail();
    retval = test(isTestingGet, "Int16", 14, "-257");
    // Little endian.
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint16", 0, "256", true);
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint16", 5, "26213", true);
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint16", 9, "33409", true);
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint16", 14, "65534", true);
    // Big endian.
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint16", 0, "1");
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint16", 5, "25958");
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint16", 9, "33154");
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint16", 14, "65279");
    // Little endian.
    if (!retval) return fail();
    retval = test(isTestingGet, "Int32", 0, "50462976", true);
    if (!retval) return fail();
    retval = test(isTestingGet, "Int32", 3, "1717920771", true);
    if (!retval) return fail();
    retval = test(isTestingGet, "Int32", 6, "-2122291354", true);
    if (!retval) return fail();
    retval = test(isTestingGet, "Int32", 9, "-58490239", true);
    if (!retval) return fail();
    retval = test(isTestingGet, "Int32", 12, "-66052", true);
    // Big endian.
    if (!retval) return fail();
    retval = test(isTestingGet, "Int32", 0, "66051");
    if (!retval) return fail();
    retval = test(isTestingGet, "Int32", 3, "56911206");
    if (!retval) return fail();
    retval = test(isTestingGet, "Int32", 6, "1718059137");
    if (!retval) return fail();
    retval = test(isTestingGet, "Int32", 9, "-2122152964");
    if (!retval) return fail();
    retval = test(isTestingGet, "Int32", 12, "-50462977");
    // Little endian.
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint32", 0, "50462976", true);
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint32", 3, "1717920771", true);
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint32", 6, "2172675942", true);
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint32", 9, "4236477057", true);
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint32", 12, "4294901244", true);
    // Big endian.
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint32", 0, "66051");
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint32", 3, "56911206");
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint32", 6, "1718059137");
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint32", 9, "2172814332");
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint32", 12, "4244504319");
    return retval;
}

function testFloat(isTestingGet, func, array, start, expected)
{
    var retval = true;
    // Little endian.
    createDataView(array, 0, true, start);
    retval = test(isTestingGet, func, 0, expected, true);
    createDataView(array, 3, true, start);
    if (!retval) return fail();
    retval = test(isTestingGet, func, 3, expected, true);
    createDataView(array, 7, true, start);
    if (!retval) return fail();
    retval = test(isTestingGet, func, 7, expected, true);
    createDataView(array, 10, true, start);
    if (!retval) return fail();
    retval = test(isTestingGet, func, 10, expected, true);
    // Big endian.
    createDataView(array, 0, false);
    if (!retval) return fail();
    retval = test(isTestingGet, func, 0, expected, false);
    createDataView(array, 3, false);
    if (!retval) return fail();
    retval = test(isTestingGet, func, 3, expected, false);
    createDataView(array, 7, false);
    if (!retval) return fail();
    retval = test(isTestingGet, func, 7, expected, false);
    createDataView(array, 10, false);
    if (!retval) return fail();
    retval = test(isTestingGet, func, 10, expected, false);
    return retval;
}

function runFloatTestCases(isTestingGet, start)
{
    var retval = true;
    retval = testFloat(isTestingGet, "Float32", isTestingGet ? [0, 0, 32, 65] : emptyArray, start, "10");
    if (!retval) return fail();
    retval = testFloat(isTestingGet, "Float32", isTestingGet ? [164, 112, 157, 63] : emptyArray, start, "1.2300000190734863");
    if (!retval) return fail();
    retval = testFloat(isTestingGet, "Float32", isTestingGet ? [95, 53, 50, 199] : emptyArray, start, "-45621.37109375");
    if (!retval) return fail();
    retval = testFloat(isTestingGet, "Float32", isTestingGet ? [255, 255, 255, 127] : emptyArray, start, "NaN");
    if (!retval) return fail();
    retval = testFloat(isTestingGet, "Float32", isTestingGet ? [255, 255, 255, 255] : emptyArray, start, "-NaN");
    if (!retval) return fail();
    retval = testFloat(isTestingGet, "Float64", isTestingGet ? [0, 0, 0, 0, 0, 0, 36, 64] : emptyArray, start, "10");
    if (!retval) return fail();
    retval = testFloat(isTestingGet, "Float64", isTestingGet ? [174, 71, 225, 122, 20, 174, 243, 63] : emptyArray, start, "1.23");
    if (!retval) return fail();
    retval = testFloat(isTestingGet, "Float64", isTestingGet ? [181, 55, 248, 30, 242, 179, 87, 193] : emptyArray, start, "-6213576.4839");
    if (!retval) return fail();
    retval = testFloat(isTestingGet, "Float64", isTestingGet ? [255, 255, 255, 255, 255, 255, 255, 127] : emptyArray, start, "NaN");
    if (!retval) return fail();
    retval = testFloat(isTestingGet, "Float64", isTestingGet ? [255, 255, 255, 255, 255, 255, 255, 255] : emptyArray, start, "-NaN");
    return retval;
}

function runNegativeIndexTests(isTestingGet)
{
    var retval = true;
    createDataView(intArray1, 0, true, 0, 16);
    retval = test(isTestingGet, "Int8", -1, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Int8", -2, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint8", -1, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint8", -2, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Int16", -1, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Int16", -2, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Int16", -3, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint16", -1, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint16", -2, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint16", -3, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Int32", -1, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Int32", -3, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Int32", -5, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint32", -1, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint32", -3, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Uint32", -5, "0");
    createDataView([0, 0, 0, 0, 0, 0, 36, 64], 0, true, 0, 8);
    if (!retval) return fail();
    retval = test(isTestingGet, "Float32", -1, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Float32", -3, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Float32", -5, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Float64", -1, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Float64", -5, "0");
    if (!retval) return fail();
    retval = test(isTestingGet, "Float64", -9, "0");
    return retval;
}

function runConstructorTests()
{
    var retval = true;
    arrayBuffer = (new Uint8Array([1, 2])).buffer;
    running("Test for constructor taking 1 argument");
    retval = shouldBeDefined("view = new DataView(arrayBuffer)");
    if (!retval) return fail();
    retval = shouldBe("view.byteOffset", "0");
    if (!retval) return fail();
    retval = shouldBe("view.byteLength", "2");
    if (!retval) return fail();
    running("Test for constructor taking 2 arguments");
    retval = shouldBeDefined("view = new DataView(arrayBuffer, 1)");
    if (!retval) return fail();
    retval = shouldBe("view.byteOffset", "1");
    if (!retval) return fail();
    retval = shouldBe("view.byteLength", "1");
    if (!retval) return fail();
    running("Test for constructor taking 3 arguments");
    retval = shouldBeDefined("view = new DataView(arrayBuffer, 0, 1)");
    if (!retval) return fail();
    retval = shouldBe("view.byteOffset", "0");
    if (!retval) return fail();
    retval = shouldBe("view.byteLength", "1");
    if (!retval) return fail();
    running("Test for constructor throwing exception");
    retval = shouldThrow("view = new DataView(arrayBuffer, 0, 3)");
    if (!retval) return fail();
    retval = shouldThrow("view = new DataView(arrayBuffer, 1, 2)");
    if (!retval) return fail();
    retval = shouldThrow("view = new DataView(arrayBuffer, 2, 1)");
    return retval;
}

function runGetTests()
{
    var retval = true;
    running("Test for get methods that work");
    retval = runIntegerTestCases(true, intArray1, 0, 16);
    if (!retval) return fail();
    retval = runFloatTestCases(true, 0);
    running("Test for get methods that might read beyond range");
    if (!retval) return fail();
    retval = runIntegerTestCases(true, intArray2, 3, 2);
    if (!retval) return fail();
    retval = runFloatTestCases(true, 3);
    running("Test for get methods that read from negative index");
    if (!retval) return fail();
    retval = runNegativeIndexTests(true);
    running("Test for wrong arguments passed to get methods");
    view = new DataView((new Uint8Array([1, 2])).buffer);
    if (!retval) return fail();
    retval = shouldThrow("view.getInt8()");
    if (!retval) return fail();
    retval = shouldThrow("view.getUint8()");
    if (!retval) return fail();
    retval = shouldThrow("view.getInt16()");
    if (!retval) return fail();
    retval = shouldThrow("view.getUint16()");
    if (!retval) return fail();
    retval = shouldThrow("view.getInt32()");
    if (!retval) return fail();
    retval = shouldThrow("view.getUint32()");
    if (!retval) return fail();
    retval = shouldThrow("view.getFloat32()");
    if (!retval) return fail();
    retval = shouldThrow("view.getFloat64()");
    return retval;
}

function runSetTests()
{
    var retval = true;
    running("Test for set methods that work");
    retval = runIntegerTestCases(false, emptyArray, 0, 16);
    if (!retval) return fail();
    retval = runFloatTestCases(false);
    running("Test for set methods that might write beyond the range");
    if (!retval) return fail();
    retval = runIntegerTestCases(false, emptyArray, 3, 2);
    if (!retval) return fail();
    retval = runFloatTestCases(false, 7);
    running("Test for set methods that write to negative index");
    if (!retval) return fail();
    retval = runNegativeIndexTests(false);
    running("Test for wrong arguments passed to set methods");
    view = new DataView((new Uint8Array([1, 2])).buffer);
    if (!retval) return fail();
    retval = shouldThrow("view.setInt8()");
    if (!retval) return fail();
    retval = shouldThrow("view.setUint8()");
    if (!retval) return fail();
    retval = shouldThrow("view.setInt16()");
    if (!retval) return fail();
    retval = shouldThrow("view.setUint16()");
    if (!retval) return fail();
    retval = shouldThrow("view.setInt32()");
    if (!retval) return fail();
    retval = shouldThrow("view.setUint32()");
    if (!retval) return fail();
    retval = shouldThrow("view.setFloat32()");
    if (!retval) return fail();
    retval = shouldThrow("view.setFloat64()");
    if (!retval) return fail();
    retval = shouldThrow("view.setInt8(1)");
    if (!retval) return fail();
    retval = shouldThrow("view.setUint8(1)");
    if (!retval) return fail();
    retval = shouldThrow("view.setInt16(1)");
    if (!retval) return fail();
    retval = shouldThrow("view.setUint16(1)");
    if (!retval) return fail();
    retval = shouldThrow("view.setInt32(1)");
    if (!retval) return fail();
    retval = shouldThrow("view.setUint32(1)");
    if (!retval) return fail();
    retval = shouldThrow("view.setFloat32(1)");
    if (!retval) return fail();
    retval = shouldThrow("view.setFloat64(1)");
    return retval;
}

function runIndexingTests()
{
    var retval = true;
    running("Test for indexing that should not work");
    view = new DataView((new Uint8Array([1, 2])).buffer);
    retval = shouldBeUndefined("view[0]");
    if (!retval) return fail();
    retval = shouldBeDefined("view[0] = 3");
    if (!retval) return fail();
    retval = shouldBe("view.getUint8(0)", "1");
    return retval;
}

var testCases =
        [ {name: "Float32Array",
             type: Float32Array,
             unsigned: false,
             integral: false,
             elementSizeInBytes: 4,
             testValues: [ -500.5, 500.5 ],
             expectedValues: [ -500.5, 500.5 ]
         },
         {name: "Int8Array",
             type: Int8Array,
             unsigned: false,
             integral: true,
             elementSizeInBytes: 1,
             testValues: [ -128, 127, -129, 128 ],
             expectedValues: [ -128, 127, 127, -128 ]
         },
         {name: "Int16Array",
             type: Int16Array,
             unsigned: false,
             integral: true,
             elementSizeInBytes: 2,
             testValues: [ -32768, 32767, -32769, 32768 ],
             expectedValues: [ -32768, 32767, 32767, -32768 ]
         },
         {name: "Int32Array",
             type: Int32Array,
             unsigned: false,
             integral: true,
             elementSizeInBytes: 4,
             testValues: [ -2147483648, 2147483647, -2147483649, 2147483648 ],
             expectedValues: [ -2147483648, 2147483647, 2147483647, -2147483648 ]
         },
         {name: "Uint8Array",
             type: Uint8Array,
             unsigned: true,
             integral: true,
             elementSizeInBytes: 1,
             testValues: [ 0, 255, -1, 256 ],
             expectedValues: [ 0, 255, 255, 0 ]
         },
         {name: "Uint16Array",
             type: Uint16Array,
             unsigned: true,
             integral: true,
             elementSizeInBytes: 2,
             testValues: [ 0, 65535, -1, 65536 ],
             expectedValues: [ 0, 65535, 65535, 0 ]
         },
         {name: "Uint32Array",
             type: Uint32Array,
             unsigned: true,
             integral: true,
             elementSizeInBytes: 4,
             testValues: [ 0, 4294967295, -1, 4294967296 ],
             expectedValues: [ 0, 4294967295, 4294967295, 0 ]
         }
        ];
