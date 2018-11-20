//
// Extensions to String
//

//
// String.fromCodePoint - ECMAScript 6
//
if(!String.fromCodePoint) {
  (function () {
    var stringFromCharCode = String.fromCharCode;
    var floor = Math.floor;
    const MAX_SIZE = 0x4000;
    Object.defineProperty(String, 'fromCodePoint', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function fromCodePoint() {
        var codeUnits = [];
        var highSurrogate, lowSurrogate;
        var index = -1;
        var length = arguments.length;
        if(!length)
          return '';
        var result = '';
        while(++index < length) {
          var codePoint = Number(arguments[index]);
          if(!isFinite(codePoint) || codePoint < 0 || codePoint > 0x10FFFF || floor(codePoint) != codePoint)
            throw new RangeError('Invalid code point: ' + codePoint);
          if(codePoint <= 0xFFFF) // BMP codepoint
            codeUnits.push(codePoint);
          else { // astral codepoint, split into surrogates
            codePoint -= 0x10000;
            highSurrogate = (codePoint >> 10) + 0xD800;
            lowSurrogate  = (codePoint % 0x400) + 0xDC00;
            codeUnits.push(highSurrogate, lowSurrogate);
          }
          if(index + 1 == length || codeUnits.length > MAX_SIZE) {
            result += stringFromCharCode.apply(null, codeUnits);
            codeUnits.length = 0;
          }
        }
        return result;
      }
    });
  })();
}

//
// String.prototype.codePointAt - ECMAScript 6
//
if(!String.prototype.codePointAt) {
  Object.defineProperty(String.prototype, 'codePointAt', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function codePointAt(position) {
      if(this == null)
        throw new TypeError('Invalid object for codePointAt');
      var string = String(this);
      var size   = string.length;
      var index  = position ? Number(position) : 0;
      if(index != index) // better 'isNaN'
        index = 0;
      if(index < 0 || index >= size)
        return;
      var first = string.charCodeAt(index);
      var second;
      // check if it is the start of a surrogate pair
      if(first >= 0xD800 && first <= 0xDBFF && size > index + 1) { // high surrogate
        second = string.charCodeAt(index + 1);
        if(second >= 0xDC00 && second <= 0xDFFF) // low surrogate
          return (first - 0xD800) * 0x400 + second - 0xDC00 + 0x10000;
      }
      return first;
    }
  });
}

//
// String.prototype.contains - ECMAScript 6 proposal
//
if(!String.prototype.contains) {
  Object.defineProperty(String.prototype, 'contains', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function contains(value, pos) {
      var string = this;
      if(pos)
        string = string.slice(pos);
      return string.indexOf(value.toString()) !== -1;
    }
  });
}

//
// String.prototype.endsWith - ECMAScript 6 extension
//
if(!String.prototype.endsWith) {
  Object.defineProperty(String.prototype, 'endsWith', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function endsWith(searchString, position) {
      var subjectString = this.toString();
      if(position === undefined || position > subjectString.length)
        position = subjectString.length;
      position -= searchString.length;
      var lastIndex = subjectString.indexOf(searchString, position);
      return lastIndex !== -1 && lastIndex === position;
    }
  });
}

//
// String.prototype.includes - ECMAScript 6 extension
//
if(!String.prototype.includes) {
  Object.defineProperty(String.prototype, 'includes', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function includes() {
      'use strict';
      return String.prototype.indexOf.apply(this, arguments) !== -1;
    }
  });
}

(function () {
  var RequireObjectCoercible = function (O) {
    if(O === null || typeof O === 'undefined')
      throw new TypeError('"this" value must not be null or undefined');
    return O;
  };
  var MAX_SAFE_INTEGER = Number.MAX_SAFE_INTEGER || Math.pow(2, 53) - 1;
  var ToLength = function (argument) {
    var len = Number(argument);
    if(Number.isNaN(len) || len <= 0)
      return 0;
    if(len > MAX_SAFE_INTEGER)
      return MAX_SAFE_INTEGER;
    return len;
  };    

  //
  // String.prototype.padStart
  //
  // ECMAScript 7 proposal
  //
  if(!String.prototype.padStart) {
    Object.defineProperty(String.prototype, 'padStart', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function padStart(maxLength, fillString) {
        var O = RequireObjectCoercible(this);
        var S = String(O);
        var intMaxLength = ToLength(maxLength);
        var stringLength = ToLength(S.length);
        if(intMaxLength <= stringLength)
          return S;
        var filler = (typeof fillString === 'undefined' ? '' : String(fillString));
        if(filler === '')
          filler = ' ';
        var fillLen = intMaxLength - stringLength;
        while(filler.length < fillLen) {
          var fLen = filler.length;
          var remainingCodeUnits = fillLen - fLen;
          if(fLen > remainingCodeUnits)
            filler += filler.slice(0, remainingCodeUnits);
          else
            filler += filler;
        }
        var truncatedStringFiller = filler.slice(0, fillLen);
        return truncatedStringFiller + S;
      }        
    });
  }
  
  if(!String.prototype.padEnd) {
    Object.defineProperty(String.prototype, 'padEnd', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function padEnd(maxLength, fillString) {
        var O = RequireObjectCoercible(this);
        var S = String(O);
        var intMaxLength = ToLength(maxLength);
        var stringLength = ToLength(S.length);
        if(intMaxLength <= stringLength)
          return S;
        var filler = (typeof fillString === 'undefined' ? '' : String(fillString));
        if(filler === '')
          filler = ' ';
        var fillLen = intMaxLength - stringLength;
        while(filler.length < fillLen) {
          var fLen = filler.length;
          var remainingCodeUnits = fillLen - fLen;
          if(fLen > remainingCodeUnits)
            filler += filler.slice(0, remainingCodeUnits);
          else
            filler += filler;
        }
        var truncatedStringFiller = filler.slice(0, fillLen);
        return S + truncatedStringFiller;
      }
    });
  }
})();

//
// String.prototype.repeat - ECMAScript 6 extension
//
if(!String.prototype.repeat) {
  Object.defineProperty(String.prototype, 'repeat', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function repeat(count) {
      'use strict';
      if(this === null || this === undefined)
        throw new TypeError('String.prototype.repeat called with null or undefined');
      var str = '' + this;
      count = +count;
      if(count != count)
        count = 0;
      if(count < 0)
        throw new RangeError('repeat count must be non-negative');
      if(count == Infinity)
        throw new RangeError('repeat count must be less than infinity');
      count = Math.floor(count);
      if(str.length == 0 || count == 0)
        return '';
      if(str.length * count >= 1 << 28)
        throw new RangeError('repeat count must not overflow maximum string size');
      var rpt = '';
      while(1) {
        if((count & 1) == 1)
          rpt += str;
        count >>>= 1;
        if(count == 0)
          break;
        str += str;
      }
      return rpt;
    }
  });
}

//
// String.prototype.startsWith - ECMAScript 6 extension
//
if(!String.prototype.startsWith) {
  Object.defineProperty(String.prototype, 'startsWith', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function startsWith(searchString, position) {
      position = position || 0;
      return this.lastIndexOf(searchString, position) === position;
    }
  });
}

//
// String.prototype.trim
//
if(!String.prototype.trim) {
  Object.defineProperty(String.prototype, 'trim', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function trim() {
      return this.replace(/^[\s\uFEFF\xA0]+|[\s\uFEFF\xA0]+$/g, '');
    }
  });
}

//
// ECMAScript 6 iterator protocol
//
if(Symbol && Symbol.iterator && !String.prototype[Symbol.iterator]) {
  (function () {
    var strCharCodeAt    = String.prototype.charCodeAt;
    var strFromCharCode  = String.fromCharCode;
    var objToString      = Object.prototype.toString;
    var ToObject = function (val) {
      if(val === null || val === undefined)
        throw new TypeError('Attempt to iterate on null or undefined');
      return Object(val);
    };
    var AssertString = function (obj) {
      if(objToString.call(obj) != '[object String]')
        throw new TypeError('A String Iterator can only be created for a string');
    };
    var AssertProperty = function (obj, key) {
      if(!obj.hasOwnProperty(key))
        throw new TypeError('Incompatible object for StringIterator');
    };
   
    var StringIterator = function (obj) {
      AssertString(obj);
      this.iteratedObject = ToObject(obj);
      this.index = 0;
    };

    StringIterator.prototype.next = function () {
      var self, object, index;
      
      self = ToObject(this);
      AssertProperty(self, 'iteratedObject');
      AssertProperty(self, 'index');
      
      if(self.iteratedObject === undefined)
        return { value: undefined, done: true };
      
      object = self.iteratedObject;
      index  = self.index;
      
      if(index >= (object.length || 0)) {
        self.iteratedObject = undefined;
        return { value: undefined, done: true };
      }
      
      var resultString;
      var first = strCharCodeAt.apply(object, [index]);
      if(first < 0xD800 || first > 0xDBFF || index + 1 === object.length)
        resultString = strFromCharCode.apply(null, [first]);
      else {
        var second = strCharCodeAt.apply(object, [index+1]);
        if(second < 0xDC00 || second > 0xDFFF)
          resultString = strFromCharCode.apply(null, [first]);
        else
          resultString = strFromCharCode.apply(null, [first, second]);
      }
      self.index += resultString.length;
     
      return { value: resultString, done: false };
    };
  
    String.prototype[Symbol.iterator] = function () {
      return new StringIterator(this);
    };
  })();
}
