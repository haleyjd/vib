//
// Extensions to Math
//

//
// ECMAScript 6 Functions
//

//
// Math.acosh
// Hyperbolic arc cosine
//
if(!Math.acosh) {
  Object.defineProperty(Math, 'acosh', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function acosh(x) {
      return x >= 1 ? Math.log(x + Math.sqrt(x * x - 1)) : NaN;
    }
  });
}

//
// Math.asinh
// Hyperbolic arc sine
//
if(!Math.asinh) {
  Object.defineProperty(Math, 'asinh', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function asinh(x) {
      return X === -Infinity ? -Infinity : Math.log(x + Math.sqrt(x * x + 1));
    }
  });
}

//
// Math.atanh
// Hyperbolic arc tangent
//
if(!Math.atanh) {
  Object.defineProperty(Math, 'atanh', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function atanh(x) {
      return x >= -1 && x <= 1 ? Math.log((1 + x) / (1 - x)) / 2 : NaN;
    }
  });
}

//
// Math.cbrt
// Cube root
//
if(!Math.cbrt) {
  Object.defineProperty(Math, 'cbrt', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function cbrt(x) {
      return x === 0 ? x : x < 0 ? -Math.pow(-x, 1/3) : Math.pow(x, 1/3);
    }
  });
}

//
// Math.clz32
//
if(!Math.clz32) {
  Object.defineProperty(Math, 'clz32', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function clz32(x) {
      if(x === -Infinity) return 32;
      if(x < 0 || (x |= 0) < 0) return 0;
      if(!x) return 32;
      var i = 31;
      while(x >>= 1) i--;
      return i;
    }
  });
}

//
// Math.cosh
// Hyperbolic cosine
//
if(!Math.cosh) {
  Object.defineProperty(Math, 'cosh', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function cosh(x) {
      var exp = Math.exp(x);
      return exp / 2 + 0.5 / exp;
    }
  });
}

//
// Math.expm1
// Exponent minus 1
//
if(!Math.expm1) {
  Object.defineProperty(Math, 'expm1', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function expm1(x) {
      return x === 0 ? x : Math.exp(x) - 1;
    }
  });
}

//
// Math.hypot
// Hypotenuse
//
if(!Math.hypot) {
  Object.defineProperty(Math, 'hypot', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function hypot(value1, value2) {
      for(var i = 0, s = 0, args = arguments; i < args.length; i++)
        s += args[i] * args[i];
      return Math.sqrt(s);
    }
  });
}

//
// Math.imul
// Integer multiplication
//
if(!Math.imul) {
  Object.defineProperty(Math, 'imul', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function imul(x, y) {
      return (x | 0) * (y | 0) | 0;
    }
  });
}

//
// Math.log10
// Base 10 logarithm
//
if(!Math.log10) {
  Object.defineProperty(Math, 'log10', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function log10(x) {
      return Math.log(x) / Math.LN10;
    }
  });
}

//
// Math.log1p
// Natural logarithm of x plus 1
//
if(!Math.log1p) {
  Object.defineProperty(Math, 'log1p', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function log1p(x) {
      return x === 0 ? x : Math.log(1 + x);
    }
  });
}

//
// Math.log2
// Base two logarithm
//
if(!Math.log2) {
  Object.defineProperty(Math, 'log2', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function log2(x) {
      return Math.log(x) / Math.LN2;
    }
  });
}

//
// Math.sign
// Get sign of number
//
if(!Math.sign) {
  Object.defineProperty(Math, 'sign', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function sign(x) {
      return isNaN(x) ? NaN : x < 0 ? -1 : x > 0 ? 1 : +x;
    }
  });
}

//
// Math.sinh
// Hyperbolic sine
//
if(!Math.sinh) {
  Object.defineProperty(Math, 'sinh', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function sinh(x) {
      if(x === 0)
        return x;
      var exp = Math.exp(x);
      return exp / 2 - 0.5 / exp;
    }
  });
}

//
// Math.tanh
// Hyperbolic tangent
//
if(!Math.tanh) {
  Object.defineProperty(Math, 'tanh', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function tanh(x) {
      if(x === 0)
        return x;
      if(x < 0) {
        var exp = Math.exp(2*x);
        return (exp - 1) / (exp + 1);
      } else {
        var exp = Math.exp(-2*x);
        return (1 - exp) / (1 + exp);
      }
    }
  });
}

//
// Math.trunc
// Truncate float to integer
//
if(!Math.trunc) {
  Object.defineProperty(Math, 'trunc', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function trunc(x) {
      return x === 0 ? x : x < 0 ? Math.ceil(x) : Math.floor(x);
    }
  });
}
