//
// Extensions to Array
//

//
// Array.from - ECMAScript.next working standard function
//
if(!Array.from) {
  (function () {
    var isCallable = function (fn) {
      return typeof fn == 'function';
    };
    var isIterable = function(obj) {
      // Arrays are more efficiently treated as array-like objects,
      // since our implementation of iterators is non-native.
      if(Symbol && Symbol.iterator && obj[Symbol.iterator]) {
        var toStr = Object.prototype.toString.call(obj);
        return (toStr !== '[object Array]')
      }
      return false;
    };
    var toInteger = function (value) {
      var number = Number(value);
      if(isNaN(number))
        return 0;
      if(number == 0 || !isFinite(number))
        return number;
      return (number > 0 ? 1 : -1) * Math.floor(Math.abs(number));
    };
    var maxSafeInteger = Math.pow(2, 53) - 1;
    var toLength = function (value) {
      var len = toInteger(value);
      return Math.min(Math.max(len, 0), maxSafeInteger);
    };
    var addValue = function(A, T, k, kValue, mapFn) {
      if(mapFn)
        A[k] = typeof T == 'undefined' ? mapFn(kValue, k) : mapFn.call(T, kValue, k);
      else
        A[k] = kValue;
    };
    var from = function from(arrayLike) {
      var C = this;
      if(arrayLike == null)
        throw new TypeError('`Array.from` requires an array-like object, not `null` or `undefined`');
      var items = Object(arrayLike);
      var mapping = arguments.length > 1;
      
      var mapFn, T;
      if(arguments.length > 1) {
        mapFn = arguments[1];
        if(!isCallable(mapFn))
          throw new TypeError('When provided, the second argument to `Array.from` must be a function');
        if(arguments.length > 2)
          T = arguments[2];
      }
      
      var A = isCallable(C) ? Object(new C(len)) : new Array(len);
      var k = 0;
      if(isIterable(items)) { 
        // Object supports ECMAScript 6 iterator protocol
        if(!isCallable(items[Symbol.iterator]))
          throw new TypeError('When provided, the value of `Symbol.iterator` must be a function');
        var itr = items[Symbol.iterator]();
        var iitr;
        while(!(iitr = itr.next()).done) {
          addValue(A, T, k, iitr.value, mapFn);
          ++k;
        }
        A.length = k;
      } else {
        // Assume array-like object
        var len = toLength(items.length);
        while(k < len) {
          addValue(A, T, k, items[k], mapFn);
          ++k;
        }
        A.length = len;
      }
      return A;
    };
    Object.defineProperty(Array, 'from', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value:        from
    });
  })();
}

//
// Array.isArray - ECMAScript 5.1 extension
//
if(!Array.isArray) {
  Object.defineProperty(Array, 'isArray', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function isArray(arg) {
      return Object.prototype.toString.call(arg) === '[object Array]';
    }
  });
}

//
// Array.of - ECMAScript 6 extension
//
if(!Array.of) {
  Object.defineProperty(Array, 'of', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function of() {
      return Array.prototype.slice.call(arguments);
    }
  });
}

//
// Array.prototype.copyWithin - ECMAScript 6 specification
//
if(!Array.prototype.copyWithin) {
  Object.defineProperty(Array.prototype, 'copyWithin', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function copyWithin(target, start /*, end*/) {
      if(this === null || this === undefined)
        throw new TypeError('Array.prototype.copyWithin called on null or undefined');
      var O        = Object(this);
      var len      = O.length >>> 0;
      var reltarg  = target >> 0;
      var to       = reltarg < 0 ? Math.max(len + reltarg, 0) : Math.min(reltarg, len);
      var relstart = start >> 0;
      var from     = relstart < 0 ? Math.max(len + relstart, 0) : Math.min(relstart, len);
      var end      = arguments[2];
      var relend   = end === undefined ? len : end >> 0;
      var fin      = relend < 0 ? Math.max(len + relend, 0) : Math.min(relend, len);
      var count    = Math.min(fin - from, len - to);
      var dir      = 1;
      
      if(from < to && to < (from + count)) {
        dir   = -1;
        from += count - 1;
        to   += count - 1;
      }
      
      while(count > 0) {
        if(from in O)
          O[to] = O[from];
        else
          delete O[to];
        from += dir;
        to   += dir;
        --count;
      }
      
      return O;
    }
  });
}

//
// Array.prototype.fill - ECMAScript 6 specification
//
if(!Array.prototype.fill) {
  Object.defineProperty(Array.prototype, 'fill', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function fill(value) {
      if(this === null || this === undefined)
        throw new TypeError('Array.prototype.fill called on null or undefined');
      var O        = Object(this);
      var len      = O.length >>> 0;
      var start    = arguments[1];
      var relstart = start >> 0;
      var k        = relstart < 0 ? Math.max(len + relstart, 0) : Math.min(relstart, len);
      var end      = arguments[2];
      var relend   = end === undefined ? len : end >> 0;
      var fin      = relend < 0 ? Math.max(len + relend, 0) : Math.min(relend, len);
      
      while(k < fin) {
        O[k] = value;
        ++k;
      }
      
      return O;
    }
  });
}

//
// Array.prototype.find - ECMAScript 6 specification
//
if(!Array.prototype.find) {
  Object.defineProperty(Array.prototype, 'find', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function find(predicate) {
      if(this == null)
        throw new TypeError('Array.prototype.find called on null or undefined');
      if(typeof predicate !== 'function')
        throw new TypeError('predicate must be a function');
      var list    = Object(this);
      var length  = list.length >>> 0;
      var thisArg = arguments[1];
      var value;
    
      for(var i = 0; i < length; i++) {
        value = list[i];
        if(predicate.call(thisArg, value, i, list))
          return value;
      }
      return undefined;
    }
  });
}

//
// Array.prototype.findIndex - ECMAScript 6 extension
//
if(!Array.prototype.findIndex) {
  Object.defineProperty(Array.prototype, 'findIndex', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function findIndex(predicate) {
      if(this == null)
        throw new TypeError('Array.prototype.findIndex called on null or undefined');
      if(typeof predicate !== 'function')
        throw new TypeError('predicate must be a function');
      var list    = Object(this);
      var length  = list.length >>> 0;
      var thisArg = arguments[1];
      var value;
      
      for(var i = 0; i < length; i++) {
        value = list[i];
        if(predicate.call(thisArg, value, i, list))
          return i;
      }
      return -1;
    }
  });
}

//
// Array.prototype.includes - ECMAScript 7 proposal
//
if(!Array.prototype.includes) {
  Object.defineProperty(Array.prototype, 'includes', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function includes(searchElement /*, fromIndex*/) {
      'use strict';
      var O   = Object(this);
      var len = parseInt(O.length) || 0;
      if(len === 0)
        return false;
      var n   = parseInt(arguments[1]) || 0;
      var k;
      if(n >= 0)
        k = n;
      else {
        k = len + n;
        if(k < 0)
          k = 0;
      }
      var currentElement;
      while(k < len) {
        currentElement = O[k];
        if(searchElement === currentElement ||
           (searchElement !== searchElement && currentElement !== currentElement)) {
          return true;
        }
        k++;
      }
      return false;
    }
  });
}

//
// Array.prototype.entries
// Array.prototype.keys
// Array.prototype.values
// Array.prototype[Symbol.iterator]
//
// ECMAScript 6 Array Iterator protocol
//
(function () {
  var ToObject = function (val) {
    if(val === null || val === undefined)
      throw new TypeError('Attempt to iterate on null or undefined');
    return Object(val);
  };
  var AssertProperty = function (obj, key) {
    if(!obj.hasOwnProperty(key))
      throw new TypeError('Incompatible object for ArrayIterator');
  };
   
  var ArrayIterator = function (obj, kind) {
    this.iteratedObject = ToObject(obj);
    this.index = 0;
    this.kind = kind;
  };
  ArrayIterator.prototype.next = function () {
    var self, object, index, kind;
      
    self = ToObject(this);
    AssertProperty(self, 'iteratedObject');
    AssertProperty(self, 'index');
    AssertProperty(self, 'kind');
      
    if(self.iteratedObject === undefined)
      return { value: undefined, done: true };
      
    object = self.iteratedObject;
    index  = self.index;
    kind   = self.kind;
      
    if(index >= (object.length || 0)) {
      self.iteratedObject = undefined;
      return { value: undefined, done: true };
    }
      
    self.index += 1;
      
    if(kind === 'key')
      return { value: index, done: false };
    else if(kind === 'value')
      return { value: object[index], done: false };
    else if(kind === 'key+value')
      return { value: [index, object[index]], done: false };
      
    throw new TypeError('Invalid ArrayIterator kind');
  };
   
  if(!Array.prototype.keys) {
    Object.defineProperty(Array.prototype, 'keys', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function keys() {
        return new ArrayIterator(this, 'key');
      }
    });
  }
   
  if(!Array.prototype.values) {
    Object.defineProperty(Array.prototype, 'values', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function values() {
        return new ArrayIterator(this, 'value');
      }
    });
  }
   
  if(!Array.prototype.entries) {
    Object.defineProperty(Array.prototype, 'entries', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function entries() {
        return new ArrayIterator(this, 'key+value');
      }
    });
  }
   
  if(Symbol && Symbol.iterator && !Array.prototype[Symbol.iterator]) {
    Array.prototype[Symbol.iterator] = Array.prototype.values;
  }
})();

