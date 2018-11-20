// Test that our sandboxing cannot pollute the global namespace
function Polluter()
{
   Console.println("Hi!");
}

var x = function () { return 1337; };

(function () { return {}; })();

// EOF

