require = function (path) { 
   var t = Utils.LoadTextFile('jslib/tests/' + path + '.js'); 
   var f = new Function('var module = arguments[0]; ' + t.toString()); 
   var module = {}; 
   module.exports = Object.create(null); 
   f(module);
   return module.exports; 
};