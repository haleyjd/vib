//
// ECMAScript 6 Map polyfill
//
if(!this.Map) {
  (function (global) {
    // Symbol is required
    if(!global.Symbol)
      Core.loadScript('jslib/symbolExtension.js');
    var Symbol = global.Symbol;
    
    var id    = 0;      
    var ID    = Symbol('id');
    var O1    = Symbol('O1');
    var LAST  = Symbol('last');
    var FIRST = Symbol('first');
    var ITER  = Symbol('iter');
    var SIZE  = Symbol('size');
    
    var isObject = function (it) {
      return it !== null && (typeof it == 'object' || typeof it == 'function');
    };
    
    var has = function (it, key) {
      return Object.prototype.hasOwnProperty.call(it, key);
    };
    
    var fastKey = function (it, create) {
      if(!isObject(it))
        return (typeof it == 'string' ? 'S' : 'P') + it;
      if(!has(it, ID)) {
        if(!create)
          return 'E';
        Object.defineProperty(it, ID, {
          enumerable:   false,
          configurable: true,
          writable:     true,
          value:        ++id
        });
      }
      return 'O' + it[ID];
    };
    
    var getEntry = function (that, key) {
      var index = fastKey(key);
      return that[O1][index];
    };
    
    var def = function (that, key, value) {
      var entry = getEntry(that, key);
      var prev, index;
      
      if(entry) {
        // change existing entry
        entry.v = value;
      } else {
        that[LAST] = entry = {
          i: index = fastKey(key, true), // <- index
          k: key,                        // <- key
          v: value,                      // <- value
          p: prev = that[LAST],          // <- previous entry
          n: undefined,                  // <- next entry
          r: false                       // <- removed
        };
        if(!that[FIRST])
          that[FIRST] = entry;
        if(prev)
          prev.n = entry;
        ++that[SIZE];
        // add to index
        that[O1][index] = entry;
      }
      return that;
    };
    
    // Iterator
    var MapIterator = function (obj, kind) {
      Object.defineProperty(this, ITER, {
        enumerable:   false,
        configurable: true,
        writable:     true,
        value:        { o: obj, k: kind }
      });
    };
    Object.defineProperty(MapIterator.prototype, 'next', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function next() {
        var iter  = this[ITER];
        var kind  = iter.k;
        var entry = iter.l;
        
        // revert to last existing entry
        while(entry && entry.r)
          entry = entry.p;
        
        // get next entry
        if(!iter.o || !(iter.l = entry = entry ? entry.n : iter.o[FIRST])) {
          // or finish the iteration
          iter.o = undefined;
          return { value: undefined, done: true };
        }
        
        // return step by kind
        if(kind == 'key')
          return { value: entry.k, done: false };
        else if(kind == 'value')
          return { value: entry.v, done: false };
        else
          return { value: [entry.k, entry.v], done: false };
      }
    });
    
    // 23.1.3.2 Constructor
    var Map = function Map() {
      if(!(this instanceof Map))
        throw new TypeError('Must be constructing an instance of Map');
      var set = function (that, name, val) {
        Object.defineProperty(that, name, {
          enumerable:   false,
          configurable: true,
          writable:     true,
          value:        val
        });
      };
      set(this, O1,    Object.create(null));
      set(this, SIZE,  0);
      set(this, LAST,  undefined);
      set(this, FIRST, undefined);
    };
    
    // 23.1.3.1 Map.prototype.clear
    Object.defineProperty(Map.prototype, 'clear', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function clear() {
        for(var that = this, data = that[O1], entry = that[FIRST]; entry; entry = entry.n) {
          entry.r = true;
          if(entry.p)
            entry.p = entry.p.n = undefined;
          delete data[entry.i];
        }
        that[FIRST] = that[LAST] = undefined;
        that[SIZE] = 0;
      }
    });
    
    // 23.1.3.3 Map.prototype.delete(key)
    Object.defineProperty(Map.prototype, 'delete', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function delete(key) {
        var that  = this;
        var entry = getEntry(that, key);
        if(entry) {
          var next = entry.n, prev = entry.p;
          delete that[O1][entry.i];
          entry.r = true;
          if(prev)
            prev.n = next;
          if(next)
            next.p = prev;
          if(that[FIRST] == entry)
            that[FIRST] = next;
          if(that[LAST] == entry)
            that[LAST] = prev;
          --that[SIZE];
        }
        return !!entry;
      }
    });
    
    // 23.1.3.4 Map.prototype.entries
    Object.defineProperty(Map.prototype, 'entries', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function entries() {
        return new MapIterator(this, 'key+value');
      }
    });
    
    // 23.1.3.5 Map.prototype.forEach(callback, thisArg = undefined)
    Object.defineProperty(Map.prototype, 'forEach', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function forEach(callback /*, that = undefined */) {
        if(typeof callback != 'function')
          throw new TypeError('callback is not a callable function');
        var fn = (arguments[1] === undefined) ? callback : callback.bind(arguments[1]);
        var entry;
        while(entry = entry ? entry.n : this[FIRST]) {
          fn(entry.v, entry.k, this);
          // revert to the last existing entry
          while(entry && entry.r)
            entry = entry.p;
        }
      }
    });
    
    // 23.1.3.6 Map.prototype.get(key)
    Object.defineProperty(Map.prototype, 'get', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function get(key) {
        var entry = getEntry(this, key);
        return entry && entry.v;
      }
    });
    
    // 23.1.3.7 Map.prototype.has(key)
    Object.defineProperty(Map.prototype, 'has', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function has(key) {
        return !!getEntry(this, key);
      }
    });
    
    // 23.1.3.8 Map.prototype.keys
    Object.defineProperty(Map.prototype, 'keys', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function keys() {
        return new MapIterator(this, 'key');
      }
    });
    
    // 23.1.3.9 Map.prototype.set(key, value)
    Object.defineProperty(Map.prototype, 'set', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function set(key, value) {
        return def(this, key === 0 ? 0 : key, value);
      }
    });
    
    // 23.1.3.10 Map.prototype.size
    Object.defineProperty(Map.prototype, 'size', {
      enumerable:   false,
      configurable: true,
      get: function size() {
        return this[SIZE];
      }
    });
    
    // 23.1.3.11 Map.prototype.values
    Object.defineProperty(Map.prototype, 'values', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function values() {
        return new MapIterator(this, 'value');
      }
    });
    
    // 23.1.3.12 Map.prototype[@@iterator]
    Map.prototype[Symbol.iterator] = Map.prototype.entries;
    
    // Assign to global
    Object.defineProperty(global, 'Map', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value:        Map
    });   
  })(this);
}