//
// Auto Exec
//
// vibconsole will run this script at startup
//

Console.println("Running autoexec.js");

//
// Global stuff
//

// Make a shortcut to Core.exit
var exit = Core.exit;

//
// Globals
//

var pathGet = function (obj, path) {
   if(obj && path.split('.').every(function (part) { return !!(obj = obj[part]); }))
      return obj;
};

// 
// Config options
//

Console.resize(150, 60); // Make it bigger.

// EOF

