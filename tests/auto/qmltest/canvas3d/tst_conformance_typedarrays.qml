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

import "tst_conformance_typedarrays.js" as Content

// Covers the following WebGL conformance TypedArray tests:
// array-buffer-crash.html
// array-buffer-view-crash.html
// array-unit-tests.html
// array-large-array-tests.html
// data-view-crash.html
// data-view-test.html

// Doesn't cover the following TypedArray tests:
// typed-arrays-in-workers.html

Item {
    id: top
    height: 300
    width: 300

    Canvas3D {
        onInitGL: Content.initGL(this, true, true)
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

    // array-buffer-crash.html
    TestCase {
        name: "Canvas3D_conformance_array_buffer_crash"
        when: windowShown

        function test_array_buffer_no_crash() {
            verify(Content.createArray() !== null,
                   "Test ArrayBuffer.byteLength (new ArrayBuffer().byteLength)")
        }
    }

    // array-buffer-view-crash.html
    TestCase {
        name: "Canvas3D_conformance_array_buffer_view_crash"
        when: windowShown

        function test_array_buffer_view_no_crash() {
            verify(Content.createArrayView() !== null,
                   "Verify that constructing a typed array view with no arguments and fetching \
                    its length does not crash (new Uint32Array().length)")
        }
    }

    // array-unit-tests.html
    TestCase {
        // Verifies the functionality of the new array-like objects in the TypedArray spec
        name: "Canvas3D_conformance_array_unit_tests"
        when: windowShown

        function test_slice() {
            verify(Content.testSlice());
        }

        function test_array_buffer_is_view() {
            verify(Content.testArrayBufferIsViewMethod()); // Half the tests here fail. Something to check in V4VM?
        }

        function test_inheritance_hierarchy() {
            verify(Content.testInheritanceHierarchy());
        }

        function test_typed_arrays() {
            for (var i = 0; i < Content.testCases.length; i++) {
                var testCase = Content.testCases[i];
                Content.running(testCase.name);
                var type = testCase.type;
                var name = testCase.name;
                if (testCase.unsigned) {
                    verify(Content.testSetAndGet10To1(type, name));
                    verify(Content.testConstructWithArrayOfUnsignedValues(type, name));
                    verify(Content.testConstructWithTypedArrayOfUnsignedValues(type, name));
                } else {
                    verify(Content.testSetAndGetPos10ToNeg10(type, name));
                    verify(Content.testConstructWithArrayOfSignedValues(type, name));
                    verify(Content.testConstructWithTypedArrayOfSignedValues(type, name));
                }
                if (testCase.integral) {
                    verify(Content.testIntegralArrayTruncationBehavior(type, name, testCase.unsigned));
                }
                verify(Content.testGetWithOutOfRangeIndices(type, name));
                verify(Content.testOffsetsAndSizes(type, name, testCase.elementSizeInBytes));
                verify(Content.testSetFromTypedArray(type, name));
                verify(Content.negativeTestSetFromTypedArray(type, name));
                verify(Content.testSetFromArray(type, name));
                verify(Content.negativeTestSetFromArray(type, name));
                verify(Content.testSubarray(type, name));
                verify(Content.negativeTestSubarray(type, name));
                verify(Content.testSetBoundaryConditions(type,
                                                         name,
                                                         testCase.testValues,
                                                         testCase.expectedValues));
                verify(Content.testConstructionBoundaryConditions(type,
                                                                  name,
                                                                  testCase.testValues,
                                                                  testCase.expectedValues));
                //verify(Content.testConstructionWithNullBuffer(type, name)); // V4VM? Float32Array ConstructionBoundaryConditions: Construction of Float32Array with null buffer should throw exception. Something to check in V4VM?
                verify(Content.testConstructionWithOutOfRangeValues(type, name));
                //verify(Content.testConstructionWithNegativeOutOfRangeValues(type, name)); // V4VM? Float32Array ConstructionBoundaryConditions: Construction of Float32Array with negative out-of-range values should throw an exception. Something to check in V4VM?
                //verify(Content.testConstructionWithUnalignedOffset(type, name, testCase.elementSizeInBytes)); // V4VM? Construction of Int8Array with out-of-range values threw an exception. Something to check in V4VM?
                //verify(Content.testConstructionWithUnalignedLength(type, name, testCase.elementSizeInBytes)); // V4VM? Construction of Int8Array with out-of-range values threw an exception.  Something to check in V4VM?
                //verify(Content.testConstructionOfHugeArray(type, name, testCase.elementSizeInBytes)); // V4VM? Float32Array ConstructionBoundaryConditions: Construction of huge Float32Array should throw exception. Something to check in V4VM?
                verify(Content.testConstructionWithBothArrayBufferAndLength(type, name, testCase.elementSizeInBytes));
                verify(Content.testSubarrayWithOutOfRangeValues(type, name, testCase.elementSizeInBytes));
                verify(Content.testSubarrayWithDefaultValues(type, name, testCase.elementSizeInBytes));
                verify(Content.testSettingFromArrayWithOutOfRangeOffset(type, name));
                verify(Content.testSettingFromFakeArrayWithOutOfRangeLength(type, name));
                verify(Content.testSettingFromTypedArrayWithOutOfRangeOffset(type, name));
                verify(Content.negativeTestGetAndSetMethods(type, name));
                verify(Content.testNaNConversion(type, name));
            }
        }
    }

    // array-large-array-tests.html
    TestCase {
        name: "Canvas3D_conformance_array_large_array_tests"
        when: windowShown

        // Verifies allocation of large array buffers
        function test_array_large_array() {
            for (var i = 0; i < Content.testCases.length; i++) {
                var testCase = Content.testCases[i];
                Content.running(testCase.name);
                var type = testCase.type;
                var name = testCase.name;
                //verify(Content.testConstructionOfHugeArray(type, name, testCase.elementSizeInBytes)); // V4VM? Float32Array: Construction of huge Float32Array should throw exception
            }
        }
    }

    // data-view-crash.html
    TestCase {
        name: "Canvas3D_conformance_data_view_crash"
        when: windowShown

        function test_data_view_crash() {
            verify(Content.dataView(),
                   "Test that DataView does not crash with bad offset or length.")
        }
    }

    // data-view-test.html
    TestCase {
        name: "Canvas3D_conformance_data_view_test"
        when: windowShown

        function test_data_view_test_1() {
            verify(Content.runConstructorTests());
        }

        function test_data_view_test_2() {
            //verify(Content.runGetTests()); // V4VM? view.getUint8(0) should be 0. Threw exception TypeError: Property 'getUint8' of object [object Object] is not a function
        }

        function test_data_view_test_3() {
            //verify(Content.runSetTests()); // V4VM? view.setUint8(0, 0) fails
        }

        function test_data_view_test_4() {
            //verify(Content.runIndexingTests()); // V4VM? view.getUint8(0) should be 1. Threw exception TypeError: Property 'getUint8' of object [object Object] is not a function
        }
    }
}
