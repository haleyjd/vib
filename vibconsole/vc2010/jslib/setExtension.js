//
// ECMAScript 6 Set polyfill
//
if(!this.Set) {
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
    var SetIterator = function (obj, kind) {
      Object.defineProperty(this, ITER, {
        enumerable:   false,
        configurable: true,
        writable:     true,
        value:        { o: obj, k: kind }
      });
    };
    Object.defineProperty(SetIterator.prototype, 'next', {
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
        if(kind == 'key' || kind == 'value')
          return { value: entry.v, done: false };
        else
          return { value: [entry.v, entry.v], done: false };
      }
    });
    
    // 23.2.1.1 Constructor
    var Set = function Set() {
      if(!(this instanceof Set))
        throw new TypeError('Must be constructing an instance of Set');
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
    
    // 23.2.3.1 Set.prototype.add 
    Object.defineProperty(Set.prototype, 'add', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function add(val) {
        if(!!getEntry(this, val))
          return this;
        else
          return def(this, val === 0 ? 0 : val, val);
      }
    });
    
    // 23.2.3.2 Set.prototype.clear
    Object.defineProperty(Set.prototype, 'clear', {
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
    
    // 23.2.3.4 Set.prototype.delete(value)
    Object.defineProperty(Set.prototype, 'delete', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function delete(val) {
        var that  = this;
        var entry = getEntry(that, val);
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
    
    // 23.2.3.5 Set.prototype.entries
    Object.defineProperty(Set.prototype, 'entries', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function entries() {
        return new SetIterator(this, 'key+value');
      }
    });
    
    // 23.2.3.6 Set.prototype.forEach(callback, thisArg = undefined)
    Object.defineProperty(Set.prototype, 'forEach', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function forEach(callback /*, that = undefined */) {
        if(typeof callback != 'function')
          throw new TypeError('callback is not a callable function');
        var fn = (arguments[1] === undefined) ? callback : callback.bind(arguments[1]);
        var entry;
        while(entry = entry ? entry.n : this[FIRST]) {
          fn(entry.v, entry.v, this);
          // revert to the last existing entry
          while(entry && entry.r)
            entry = entry.p;
        }
      }
    });
    
    // 23.2.3.7 Set.prototype.has(value)
    Object.defineProperty(Set.prototype, 'has', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function has(val) {
        return !!getEntry(this, val);
      }
    });
    
    // 23.2.3.8 Set.prototype.keys
    Object.defineProperty(Set.prototype, 'keys', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function keys() {
        return new SetIterator(this, 'key');
      }
    });
    
    // 23.2.3.9 Set.prototype.size
    Object.defineProperty(Set.prototype, 'size', {
      enumerable:   false,
      configurable: true,
      get: function size() {
        return this[SIZE];
      }
    });
    
    // 23.2.3.10 Set.prototype.values
    Object.defineProperty(Set.prototype, 'values', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value: function values() {
        return new SetIterator(this, 'value');
      }
    });
    
    // 23.2.3.11 Set.prototype[@@iterator]
    Set.prototype[Symbol.iterator] = Set.prototype.values;
    
    // Assign to global
    Object.defineProperty(global, 'Set', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value:        Set
    });   
  })(this);
}