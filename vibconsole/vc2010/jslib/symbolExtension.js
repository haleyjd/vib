//
// Symbol
//
// Best possible approximation of ECMAScript 6 Symbol type; not perfect, but highly functional.
//
if(!this.Symbol) {
  (function (global) {
    // unique symbol generator
    var sid = 0;
    var randStr = function () {
      return (Utils ? Utils.GenerateUUID() : (++sid + Math.random()).toString(36));
    };
    var uid = function (key) {
      return 'Symbol(' + key + ')_' + randStr();
    };
    
    // global symbol map
    var globalMap = Object.create(null);
    
    // internal value and key property symbols
    var VALUE = uid('value');
    var KEY   = uid('key');
    var REF   = uid('ref');
    
    // regexp which matches Symbol names
    var symRegExp = /^Symbol\(.*\)_[0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$/i;
    
    // original value of Object.getOwnPropertyNames
    var origGetOwnPropertyNames = Object.getOwnPropertyNames;
    
    // setter which creates a closure
    var setter = function (key) {
      return function(value) {
        // Store the received value and mark it as non-enumerable
        Object.defineProperty(this, key, {
          enumerable:   false,
          configurable: true,
          writable:     true,
          value:        value
        });
      };
    };
    
    var createSymbolInternal = function (internalID) {
      var newSymbol = Object.create(Symbol.prototype);
      Object.defineProperty(newSymbol, VALUE, {
        enumerable:   false,
        configurable: false,
        writable:     false,
        value:        internalID
      });
      // Create a dummy property on Object.prototype, so that when a Symbol
      // is used to assign a property, it will become non-enumerable.
      if(!Object.prototype[internalID]) {
        Object.defineProperty(Object.prototype, internalID, {
          enumerable:   false,
          configurable: true,
          get: function () {},
          set: setter(internalID)
        });
      }
      // Define a reference on this Symbol to a natively implemented 
      // reference counter which deletes the Object.prototype dummy
      // properties once there are no longer any references to a
      // related Symbol object (this is accomplished via SpiderMonkey's
      // JSFinalizeOp callback in NativeSymbolRef's JSClass)
      Object.defineProperty(newSymbol, REF, {
        enumerable:   false,
        configurable: false,
        writable:     false,
        value: new NativeSymbolRef(internalID, Object.prototype)
      });
      return newSymbol;
    };
    
    //
    // Symbol
    //
    function Symbol(name) {
      if(this instanceof Symbol && !this[VALUE])
        throw new TypeError('Symbol is not a constructor');
      return createSymbolInternal(uid(name));
    }
    
    //
    // Symbol.prototype.toString
    //
    // This is supposed to return "Symbol(name)", which is only descriptive
    // and cannot be used to obtain the value of the property, but we do not
    // have native support for lookup of properties by anything but names or
    // indices, so this WILL actually return the internal value of the symbol.
    // You're not supposed to use it for anything evil, by contract :P
    //
    Object.defineProperty(Symbol.prototype, 'toString', { 
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function toString() {
        return this[VALUE];
      }
    });
    
    //
    // Symbol.prototype.valueOf - same deal as toString
    //
    Object.defineProperty(Symbol.prototype, 'valueOf', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function valueOf() {
        return this[VALUE];
      }
    });
    
    //
    // Symbol.for
    //
    // Add a Symbol to the global registry.
    //
    Object.defineProperty(Symbol, 'for', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function for(key) {
        var sym;
        if(!(sym = globalMap[key])) {
          sym = globalMap[key] = Symbol(key);
          Object.defineProperty(sym, KEY, {
            enumerable:   false,
            configurable: false,
            writable:     false,
            value:        key
          });
        }
        return sym;
      }
    });
    
    //
    // Symbol.keyFor
    //
    // Retrieve a Symbol stored in the global registry by name.
    //
    Object.defineProperty(Symbol, 'keyFor', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function keyFor(sym) {
        return sym[KEY];
      }
    });
    
    //
    // Symbol.iterator
    //
    // Create the "well-known" Symbol for the ECMAScript 6 iterator protocol.
    //
    Object.defineProperty(Symbol, 'iterator', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value:        Symbol('iterator')
    });
    
    //
    // Expose global object Symbol
    //
    Object.defineProperty(global, 'Symbol', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value:        Symbol
    });
    
    //
    // Overrides
    //
    
    //
    // Object.getOwnPropertyNames:
    //   Shall not return names of Symbol properties.
    //
    if(origGetOwnPropertyNames) {
      let testFunc = function (elem) { return !symRegExp.test(elem); }; // NOT symbols
      Object.getOwnPropertyNames = function getOwnPropertyNames(obj) {
        return origGetOwnPropertyNames(obj).filter(testFunc);
      };
    }
    
    //
    // Additional exports
    //
    
    //
    // Object.getOwnPropertySymbols
    //   NB: Non-compliant to the extent the objects returned will not 
    //   compare equal to the original Symbol objects constructed to add
    //   the properties to the object - this is impossible to accomplish
    //   without weak maps, as otherwise there would be a severe garbage
    //   collection issue with Symbol instances.
    //
    if(origGetOwnPropertyNames && !Object.getOwnPropertySymbols) {
      let testFunc = function (elem) { return symRegExp.test(elem); }; // ONLY symbols
      Object.defineProperty(Object, 'getOwnPropertySymbols', {
        enumerable:   false,
        configurable: true,
        writable:     true,
        value: function getOwnPropertySymbols(obj) {
          var symNames = origGetOwnPropertyNames(obj).filter(testFunc);
          var symbols  = [];
          symNames.forEach(function (symName) {
            symbols.push(createSymbolInternal(symName));
          });
          return symbols;
        }
      });
    }
  })(this);
}
