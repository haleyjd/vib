// 
// Module Sandboxing Test 2
//

var x = function () { return 1337; };
var y = 1492;
var z = function () { return this.publicY; };

// Expose independent global vars x, y, z inside the returned object;
// they should not be visible in the global namespace, and, they should
// live as long as a reference is maintained to the module.
(function () { return { publicX: x, publicY: y, publicZ: z }; })();

// EOF

