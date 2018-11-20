//
// Test ability to use bind() to create lambda-like closures
//
// Depends on Function.prototype.bind extension.
//

//
// TestClassA can have any sort of object registered with it, and it will
// call the registered function and print its result.
//
TestClassA = function() {
   this.callback = null;
};

TestClassA.prototype.doCallback = function() {
   Console.println("My callback returns ", this.callback());
};

//
// TestClassB is a sample subscriber for TestClassA's calback ability.
// * registerWith will register a callback function with TestClassA that
//   uses bind() to remember the instance to which it belongs. 
// * registerWithAlt will create a closure that does the same thing.
// 
// The two operations should be logically equivalent, though one is bound
// to be faster than the other - the question is, which? :P
//
TestClassB = function (bval) {
   this.b = bval;
};

TestClassB.prototype.getB = function () { return this.b; }

TestClassB.prototype.registerWith = function (a) {
   a.callback = this.getB.bind(this);
};

// Alternative not using bind (uses a closure)
TestClassB.prototype.registerWithAlt = function (a) {
   a.callback = 
     (function (that) { return function () { return that.b; }; })(this);
};

a  = new TestClassA();
b1 = new TestClassB(1);
b2 = new TestClassB(2);

// Test using bind
b1.registerWith(a);
a.doCallback();

b2.registerWith(a);
a.doCallback();


// Test using plain closures
b1.registerWithAlt(a);
a.doCallback();

b2.registerWithAlt(a);
a.doCallback();

// EOF


