(function () {
   
   // This function takes an object and will compile all of its enumerable
   // properties into functions of the same name prefixed with "f".
   function functionalTransform(o) {
      for(var prop in o) {
         if(o.hasOwnProperty(prop) && typeof o[prop] !== 'function')
            o['f'+prop] = new Function(o[prop]);
      }
   }
   
   return functionalTransform;
})();