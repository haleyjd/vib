Console.println('Before');

var p1 = new Promise(function(resolve, reject) {
  Console.println('Promise started');
  if(Math.random() < 0.5)
    resolve(1);
  else
    reject(0);
});

p1.then(function (val) {
  Console.println('Promise fulfilled: ' + val);
}).catch(function (reason) {
  Console.println('Promise rejected: ' + reason);
});

Console.println('After');