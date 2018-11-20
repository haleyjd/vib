//
// Extensions to RegExp
//

//
// RegExp.prototype.flags - ECMAScript 6
//
if(!RegExp.prototype.flags) {
  Object.defineProperty(RegExp.prototype, 'flags', {
    configurable: true,
    get: function() {
      return this.toString().match(/[gimuy]*$/)[0];
    }
  });
}
