//
// Extensions to Object
//

//
// Bind natively defined extensions
//

//
// Object.defineProperty - ECMAScript 5 extension
//
if(!Object.defineProperty) {
  Extensions.objectDefineProperty(Object, 'defineProperty', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function defineProperty(O, prop, desc) {
      return Extensions.objectDefineProperty(O, prop, desc); // insert inception sfx here
    }
  });
}

//
// Object.getOwnPropertyDescriptor - ECMAScript 5 extension
//
if(!Object.getOwnPropertyDescriptor) {
  Object.defineProperty(Object, 'getOwnPropertyDescriptor', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function getOwnPropertyDescriptor(obj, prop) {
      var O   = typeof obj === 'object' ? obj : Object(obj); // NB: ECMAScript 6 compliant behavior here.
      var get = function ()  { return O[prop]; }
      var set = function (v) { O[prop] = v;    }
      if(O.hasOwnProperty(prop))
        return Extensions.objectGetOwnPropertyDescriptorInt(O, prop, get, set);
    }
  });
}

//
// Object.getOwnPropertyNames - ECMAScript 5.1 extension
// NB: This required addition of a custom API function to JSAPI 1.8.0 as it 
// normally does not expose any method for iterating on an object's 
// non-enumerable properties. This API is called JS_EnumerateAll.
//
if(!Object.getOwnPropertyNames) {
  Object.defineProperty(Object, 'getOwnPropertyNames', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function getOwnPropertyNames(obj) {
      if(obj === null || obj === undefined)
        throw new TypeError("can't convert " + obj + " to object");
      var O   = typeof obj === 'object' ? obj : Object(obj); // NB: ECMAScript 6 compliant behavior here.
      return Extensions.objectGetOwnPropertyNamesInt(O);
    }
  });
}

if(!Object.getPrototypeOf) {
  Object.defineProperty(Object, 'getPrototypeOf', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function getPrototypeOf(obj) { return obj.__proto__; }
  });
}

//
// Object.is - ECMAScript 6 extension
//
if(!Object.is) {
  Object.defineProperty(Object, 'is', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function is(v1, v2) {
      if(v1 === 0 && v2 === 0) // +0 vs -0
        return 1 / v1 === 1 / v2;
      if(v1 !== v1)            // NaN
        return v2 !== v2;
      return v1 === v2;
    }
  });
}

//
// Object.keys - ECMAScript 5.1 extension
//
if(!Object.keys) {
  Object.defineProperty(Object, 'keys', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: (function () {
      'use strict';
      var hasOwnProperty = Object.prototype.hasOwnProperty;
      return function keys(obj) {
        if(obj === null || obj === undefined)
          throw new TypeError("can't convert " + obj + " to object");
        var O = typeof obj === 'object' ? obj : Object(obj); // ES6 behavior here
        var result = [], prop, i;
         
        for(prop in O) {
          if(hasOwnProperty.call(O, prop))
            result.push(prop);
        }
         
        return result;
      };
    })()
  });
}

//
// Object.setPrototypeOf - ECMAScript 6 extension
//
if(!Object.setPrototypeOf) {
  Object.defineProperty(Object, 'setPrototypeOf', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function setPrototypeOf(obj, proto) {
      if(typeof obj !== 'object')
        throw new TypeError('Cannot set prototype of a non-object value');
      if(typeof proto !== 'object')
        throw new TypeError('A prototype must be an object or null');
      obj.__proto__ = proto;
      return obj;
    }
  });
}

//=============================================================================
//
// Functions with dependencies follow:
//

//
// Object.assign - ECMAScript 6 extension
// REQUIRES: Object.keys, Object.getOwnPropertyDescriptor
// NB: This implementation is deliberately neglecting Symbol properties,
//     because copying them between objects as such seems to be a misfeature
//     of the current ECMAScript standard which makes no logical sense. It
//     violates the concept of Symbols being used to protect private data and
//     negates any pre- or post-condition assumptions that can be made from an
//     Object possessing a given Symbol property.
//
if(!Object.assign) {
  Object.defineProperty(Object, 'assign', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: (function () {
      var ToObject = function (o) { return typeof o === 'object' ? o : Object(o); }
      return function assign(target, firstSource) {
        'use strict';
        if(target === undefined || target === null)
          throw new TypeError('Cannot convert first argument to object');
        var to = ToObject(target);
        for(var i = 1; i < arguments.length; i++) {
          var nextSource = arguments[i];
          if(nextSource === undefined || nextSource === null)
            continue;
          var keysArray = Object.keys(ToObject(nextSource));
          for(var nextIndex = 0, len = keysArray.length; nextIndex < len; nextIndex++) {
            var nextKey = keysArray[nextIndex];
            var desc    = Object.getOwnPropertyDescriptor(nextSource, nextKey);
            if(desc !== undefined && desc.enumerable)
              to[nextKey] = nextSource[nextKey];
          }
        }
        return to;
      };
    })()
  });
}

//
// Object.defineProperties - ECMAScript 5 extension
// REQUIRES: Object.defineProperty, Object.keys
//
if(!Object.defineProperties) {
  Object.defineProperty(Object, 'defineProperties', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: (function () { 
      var isCallable = function (v) { return typeof v === 'function'; };
      var hasProperty = function (obj, prop) {
        return Object.prototype.hasOwnProperty.call(obj, prop);
      };
      var convertToDescriptor = function (desc) {
        if(typeof desc !== 'object' || desc === null)
          throw new TypeError('Bad descriptor object');
            
        var d = {};
            
        if(hasProperty(desc, 'enumerable'))
          d.enumerable = !!desc.enumerable;
        if(hasProperty(desc, 'configurable'))
          d.configurable = !!desc.configurable;
        if(hasProperty(desc, 'value'))
          d.value = desc.value;
        if(hasProperty(desc, 'writable'))
          d.writable = !!desc.writable;
        if(hasProperty(desc, 'get')) {
          if(!isCallable(desc.get) && typeof desc.get !== 'undefined')
            throw new TypeError('Getter is not a callable function');
          d.get = desc.get;
        }
        if(hasProperty(desc, 'set')) {
          if(!isCallable(desc.set) && typeof desc.set !== 'undefined')
            throw new TypeError('Setter is not a callable function');
          d.set = desc.set;
        }
            
        if(('get' in d || 'set' in d) && ('value' in d || 'writable' in d))
          throw new TypeError('Invalid mixed-state descriptor');
            
        return d;            
      };
      return function defineProperties(obj, props) {
        if(typeof obj !== 'object' || obj === null)
          throw new TypeError('Object.defineProperties called on non-object value or null');
        var p     = typeof props === 'object' ? props : Object(props);
        var keys  = Object.keys(p);
        var descs = [];
         
        for(var i = 0; i < keys.length; i++)
          descs.push([keys[i], convertToDescriptor(p[keys[i]])]);
        for(var i = 0; i < descs.length; i++)
          Object.defineProperty(obj, descs[i][0], descs[i][1]);
      };
    })()
  });
}

//
// Object.create - ECMAScript 5.1 extension
// REQUIRES: Object.defineProperties
//
if(!Object.create) {
  Object.defineProperty(Object, 'create', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: (function () {
      var createEmpty = function () { return { __proto__: null }; };
      var tmp         = function () {};
      return function create(proto, properties) {
        var obj;
        if(proto === null)
          obj = createEmpty();
        else {
          if(typeof proto !== 'object' && typeof proto !== 'function')
            throw new TypeError('Object prototype may only be an Object or null');
          tmp.prototype = proto;
          obj = new tmp();
          tmp.prototype = null;
          obj.__proto__ = proto;
        }
        if(properties !== void 0)
          Object.defineProperties(obj, properties);
        return obj;
      };
    })()
  });
}

