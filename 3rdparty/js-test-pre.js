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

function areArraysEqual(_a, _b)
{
    try {
        if (_a.length !== _b.length)
            return false;
        for (var i = 0; i < _a.length; i++)
            if (_a[i] !== _b[i])
                return false;
    } catch (ex) {
        return false;
    }
    return true;
}
function isMinusZero(n)
{
    // the only way to tell 0 from -0 in JS is the fact that 1/-0 is
    // -Infinity instead of Infinity
    return n === 0 && 1/n < 0;
}
function isResultCorrect(_actual, _expected)
{
    if (_expected === 0)
        return _actual === _expected && (1/_actual) === (1/_expected);
    if (_actual === _expected)
        return true;
    if (typeof(_expected) == "number" && isNaN(_expected))
        return typeof(_actual) == "number" && isNaN(_actual);
    if (Object.prototype.toString.call(_expected) == Object.prototype.toString.call([]))
        return areArraysEqual(_actual, _expected);
    return false;
}
function stringify(v)
{
    if (v === 0 && 1/v < 0)
        return "-0";
    else return "" + v;
}
function evalAndLog(_a)
{
    if (typeof _a != "string")
        debug("WARN: tryAndLog() expects a string argument");
    // Log first in case things go horribly wrong or this causes a sync event.
    debug(_a);
    var _av;
    try {
        _av = eval(_a);
    } catch (e) {
        return false;//testFailed(_a + " threw exception " + e);
    }
    return _av;
}
function shouldBe(_a, _b)
{
    if (typeof _a != "string" || typeof _b != "string")
        debug("WARN: shouldBe() expects string arguments");
    var exception;
    var _av;
    try {
        _av = eval(_a);
    } catch (e) {
        exception = e;
    }
    var _bv = eval(_b);
    if (exception) {
        console.log(_a + " should be " + _bv + ". Threw exception " + exception);
        return false;//testFailed(_a + " should be " + _bv + ". Threw exception " + exception);
    } else if (isResultCorrect(_av, _bv)) {
        return true;//testPassed(_a + " is " + _b);
    } else if (typeof(_av) == typeof(_bv)) {
        console.log(_a + " should be " + _bv + ". Was " + stringify(_av) + ".");
        return false;//testFailed(_a + " should be " + _bv + ". Was " + stringify(_av) + ".");
    } else {
        console.log(_a + " should be " + _bv + " (of type " + typeof _bv + "). Was " + _av + " (of type " + typeof _av + ").");
        return false;//testFailed(_a + " should be " + _bv + " (of type " + typeof _bv + "). Was " + _av + " (of type " + typeof _av + ").");
    }
}
function shouldBeTrue(_a) { return shouldBe(_a, "true"); }
function shouldBeFalse(_a) { return shouldBe(_a, "false"); }
function shouldBeNaN(_a) { return shouldBe(_a, "NaN"); }
function shouldBeNull(_a) { return shouldBe(_a, "null"); }
function shouldBeEqualToString(a, b)
{
    var unevaledString = '"' + b.replace(/"/g, "\"") + '"';
    return shouldBe(a, unevaledString);
}
function shouldEvaluateTo(actual, expected) {
    // A general-purpose comparator. 'actual' should be a string to be
    // evaluated, as for shouldBe(). 'expected' may be any type and will be
    // used without being eval'ed.
    var retval = true;
    if (expected == null) {
        // Do this before the object test, since null is of type 'object'.
        retval = shouldBeNull(actual);
        if (!retval) return false;
    } else if (typeof expected == "undefined") {
        retval = shouldBeUndefined(actual);
        if (!retval) return false;
    } else if (typeof expected == "function") {
        // All this fuss is to avoid the string-arg warning from shouldBe().
        try {
            actualValue = eval(actual);
        } catch (e) {
            return false;//testFailed("Evaluating " + actual + ": Threw exception " + e);
        }
        retval = shouldBe("'" + actualValue.toString().replace(/\n/g, "") + "'",
                 "'" + expected.toString().replace(/\n/g, "") + "'");
        if (!retval) return false;
    } else if (typeof expected == "object") {
        retval = shouldBeTrue(actual + " == '" + expected + "'");
        if (!retval) return false;
    } else if (typeof expected == "string") {
        retval = shouldBe(actual, expected);
        if (!retval) return false;
    } else if (typeof expected == "boolean") {
        retval =  shouldBe("typeof " + actual, "'boolean'");
        if (!retval) return false;
        if (expected) {
            retval = shouldBeTrue(actual);
            if (!retval) return false;
        } else {
            retval = shouldBeFalse(actual);
            if (!retval) return false;
        }
    } else if (typeof expected == "number") {
        retval = shouldBe(actual, stringify(expected));
        if (!retval) return false;
    } else {
        debug(expected + " is unknown type " + typeof expected);
        retval = shouldBeTrue(actual, "'" +expected.toString() + "'");
        if (!retval) return false;
    }
    return retval;
}
function shouldBeNonZero(_a)
{
    var exception;
    var _av;
    try {
        _av = eval(_a);
    } catch (e) {
        exception = e;
    }
    if (exception)
        return false;//testFailed(_a + " should be non-zero. Threw exception " + exception);
    else if (_av != 0)
        return true;//testPassed(_a + " is non-zero.");
    else
        return false;//testFailed(_a + " should be non-zero. Was " + _av);
}
function shouldBeNonNull(_a)
{
    var exception;
    var _av;
    try {
        _av = eval(_a);
    } catch (e) {
        exception = e;
    }
    if (exception)
        return false;//testFailed(_a + " should be non-null. Threw exception " + exception);
    else if (_av != null)
        return true;//testPassed(_a + " is non-null.");
    else
        return false;//testFailed(_a + " should be non-null. Was " + _av);
}
function shouldBeUndefined(_a)
{
    var exception;
    var _av;
    try {
        _av = eval(_a);
    } catch (e) {
        exception = e;
    }
    if (exception)
        return false;//testFailed(_a + " should be undefined. Threw exception " + exception);
    else if (typeof _av == "undefined")
        return true;//testPassed(_a + " is undefined.");
    else
        return false;//testFailed(_a + " should be undefined. Was " + _av);
}
function shouldBeDefined(_a)
{
    var exception;
    var _av;
    try {
        _av = eval(_a);
    } catch (e) {
        exception = e;
    }
    if (exception)
        return false;//testFailed(_a + " should be defined. Threw exception " + exception);
    else if (_av !== undefined)
        return true;//testPassed(_a + " is defined.");
    else
        return false;//testFailed(_a + " should be defined. Was " + _av);
}
function shouldBeGreaterThanOrEqual(_a, _b) {
    if (typeof _a != "string" || typeof _b != "string")
        console.log("WARN: shouldBeGreaterThanOrEqual expects string arguments");
    var exception;
    var _av;
    try {
        _av = eval(_a);
    } catch (e) {
        exception = e;
    }
    var _bv = eval(_b);
    if (exception) {
        console.log(_a + " should be >= " + _b + ". Threw exception " + exception);
        return false;//testFailed(_a + " should be >= " + _b + ". Threw exception " + exception);
    } else if (typeof _av == "undefined" || _av < _bv) {
        console.log(_a + " should be >= " + _b + ". Was " + _av + " (of type " + typeof _av + ").");
        return false;//testFailed(_a + " should be >= " + _b + ". Was " + _av + " (of type " + typeof _av + ").");
    } else {
        return true;//testPassed(_a + " is >= " + _b);
    }
}
function shouldThrow(_a, _e)
{
    var exception;
    var _av;
    try {
        _av = eval(_a);
    } catch (e) {
        exception = e;
    }
    var _ev;
    if (_e)
        _ev = eval(_e);
    if (exception) {
        if (typeof _e == "undefined" || exception == _ev)
            return true;//testPassed(_a + " threw exception " + exception + ".");
        else
            return false;//testFailed(_a + " should throw " + (typeof _e == "undefined" ? "an exception" : _ev) + ". Threw exception " + exception + ".");
    } else if (typeof _av == "undefined")
        return false;//testFailed(_a + " should throw " + (typeof _e == "undefined" ? "an exception" : _ev) + ". Was undefined.");
    else
        return false;//testFailed(_a + " should throw " + (typeof _e == "undefined" ? "an exception" : _ev) + ". Was " + _av + ".");
}
function assertMsg(assertion, msg) {
    if (assertion) {
        return true;//testPassed(msg);
    } else {
        console.log(msg);
        return false;//testFailed(msg);
    }
}
