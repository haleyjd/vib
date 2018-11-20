(function (target, filename) {
   function test1() {
      Console.println("This is test 1");
   }
   
   function test2(str) {
      Console.println(str);
   }
   
   function test3() {
      Console.print("Input a message: ");
      Console.println(Console.getline());
   }
   
   // Interfaces idea test: (works, but wanted?)
   //target.interfaces = target.interfaces || [];
   //target.interfaces.push({ func: arguments.callee, file: filename });
   
   target.interfaces = target.interfaces || [];
   target.interfaces.push(filename);
   
   target.test1 = test1;
   target.test2 = test2;
   target.test3 = test3;
   
   return target;

})(mixin, arguments[0]);