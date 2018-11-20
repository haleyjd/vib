//
// Extensions to Function
//

//
// Function.prototype.bind - ECMAScript 5 extension
//
if(!Function.prototype.bind) {
  Object.defineProperty(Function.prototype, 'bind', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function bind(oThis) {
      if (typeof this !== "function") {
        // closest thing possible to the ECMAScript 5 internal IsCallable function
        throw new TypeError("Function.prototype.bind - what is trying to be bound is not callable");
      }
      var aArgs   = Array.prototype.slice.call(arguments, 1);
      var fToBind = this;
      var fNOP    = function () {};
      var fBound  = function () {
        return fToBind.apply(this instanceof fNOP && oThis ? this : oThis,
                             aArgs.concat(Array.prototype.slice.call(arguments)));
      };
         
      fNOP.prototype = this.prototype;
      fBound.prototype = new fNOP();
         
      return fBound;
    }
  });
}

//
// Function.prototype.curry - Non-standard extension
//
if(!Function.prototype.curry) {
  Object.defineProperty(Function.prototype, 'curry', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function curry() {
      var method = this, args = Array.prototype.slice.call(arguments);
      return function () {
        return method.apply(this, args.concat(Array.prototype.slice.call(arguments)));
      };
    }
  });
}

//
// Function.prototype.curryRight - Non-standard extension
//
if(!Function.prototype.curryRight) {
  Object.defineProperty(Function.prototype, 'curryRight', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function curryRight() {
      var method = this, args = Array.prototype.slice.call(arguments);
      return function () {
        return method.apply(this, Array.prototype.slice.call(arguments).concat(args));
      };
    }
  });
}

