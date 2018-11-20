(function () {
  var user;
  var pwd;
  var api;
  
  var buildPostString = function (obj) {
    var postStr = '';
    for(var prop in obj) {
      if(obj.hasOwnProperty(prop))
        postStr += prop + '=' + encodeURIComponent(obj[prop]) + '&';
    }
    return postStr;
  };
  
  var login = function (conn) {
    var loginPage = conn.readURL('http://tickets.astribe.com/login');
    try {
      var doc = XMLDocument.FromHTMLString(loginPage);
      var xp  = doc.evalXPathExpression("//input[@name='authenticity_token']");
      var tok = xp.getNodes()[0].hasProp('value').getValue();
      var loginArgs = {
        'authenticity_token': tok,
        '_pickaxe': '⸕',
        'user[email]': user,
        'user[password]': pwd        
      };
      conn.postURL('http://tickets.astribe.com/login', buildPostString(loginArgs));
    } catch(ex) {
      Console.println(ex.toString());
      Console.println('Failed to login to Spiceworks');
      return false;
    }
    return true;
  };
  
  var query = function (conn, url) {
    var obj = null;
    var str = conn.readURL(url);
    try {
      obj = Core.evalUntrustedString('(' + str + ')');
    } catch(ex) {
      Console.println(ex.toString());
    }
    return obj;
  };
 
  // Main routine
  Console.print('Enter your Spiceworks username: ');
  user = Console.getline();
  Console.print('Enter your Spiceworks password: ');
  pwd  = Console.getline();
  Console.print('Enter API to execute: ');
  api  = Console.getline();
  
  var obj;
  var conn = new CURLConnection();
  if(login(conn)) {
    obj = query(conn, 'http://tickets.astribe.com/api/' + api);
    if(!obj)
      Console.println('No response to query');
  }
  conn.close();

  return obj;  
})();