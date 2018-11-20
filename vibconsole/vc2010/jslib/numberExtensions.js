//
// Extensions to Number
//

//
// ECMAScript 6 Constants
//

//
// Number.EPSILON
// Closest representable floating point value to 0
//
if(!Number.EPSILON) {
  Object.defineProperty(Number, 'EPSILON', {
      enumerable:   false,
      configurable: false,
      writable:     false,
      value:        Math.pow(2, -52)
  });
}

//
// Number.MAX_SAFE_INTEGER
// Largest fully representable integer
//
if(!Number.MAX_SAFE_INTEGER) {
  Object.defineProperty(Number, 'MAX_SAFE_INTEGER', {
      enumerable:   false,
      configurable: false,
      writable:     false,
      value:        0x1fffffffffffff
  });
}

//
// Number.MIN_SAFE_INTEGER
// Smallest fully representable integer
//
if(!Number.MIN_SAFE_INTEGER) {
  Object.defineProperty(Number, 'MIN_SAFE_INTEGER', {
      enumerable:   false,
      configurable: false,
      writable:     false,
      value:        -0x1fffffffffffff
  });
}

//
// ECMAScript 6 Static Functions
//

//
// Number.isFinite
// Behaves the same as global isFinite function
//
if(!Number.isFinite) {
  Object.defineProperty(Number, 'isFinite', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value:        isFinite
  });
}

//
// Number.isInteger
//
if(!Number.isInteger) {
  Object.defineProperty(Number, 'isInteger', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function isInteger(it) {
        var floor    = Math.floor;
        var isObject = (typeof it === 'object' ? it !== null : typeof it === 'function');
        return !isObject && isFinite(it) && floor(it) === it;
      }
  });
}

//
// Number.isSafeInteger
//
if(!Number.isSafeInteger) {
  Object.defineProperty(Number, 'isSafeInteger', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function isSafeInteger(it) {
        var abs       = Math.abs;
        var isInteger = Number.isInteger;
        return isInteger(it) && abs(it) <= 0x1fffffffffffff;
      }
  });
}

//
// Number.isNaN
//
if(!Number.isNaN) {
  Object.defineProperty(Number, 'isNaN', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function isNaN(value) {
        return value !== value;
      }        
  });
}

//
// Number.parseFloat
// Behaves same as global parseFloat function
//
if(!Number.parseFloat) {
  Object.defineProperty(Number, 'parseFloat', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value:        parseFloat
  });
}

//
// Number.parseInt
// Different from global parseInt in that implicit octal base via strings with
// one or more leading zeroes is not allowed and will instead be parsed as a
// base 10 number.
//
if(!Number.parseInt) {
  (function (global) {
    var hex = /^[\-+]?0[xX]/;
    var globalParseInt = global.parseInt;
    var trim = String.prototype.trim || function () {
      return this.replace(/^[\s\uFEFF\xA0]+|[\s\uFEFF\xA0]+$/g, '')
    };
    Object.defineProperty(Number, 'parseInt', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function parseInt(value, radix) {
        var string = trim.call(String(value));
        return globalParseInt(string, (radix >>> 0) || (hex.test(string) ? 16 : 10));
      }
    });
  })(this);
}

