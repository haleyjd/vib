(function () {
   var staticValue = 0;
   
   function foo () {
      Console.println("foo!");
   }
   
   function bar(x) {
      return x * 2;
   }
   
   function baz() {
      return staticValue++;
   }
   
   return {
      foo: foo,
      bar: bar,
      baz: baz
   };
})();