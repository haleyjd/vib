//
// JSONRequest Class
//
// Implements an object somewhat similar to XMLHttpRequest, but intended for
// retrieval and evaluation of JSON from network URIs. IO is blocking.
//

if(!this.JSONRequest) {
  (function (global) {
    var JSONRequest = function JSONRequest() {
      // response is the evaluated result
      this.response     = null;

      // responseText is the raw response text
      this.responseText = '';
    };

    // Call getJSON to populate the results. If you want to perform a HTTP POST,
    // provide the POST parameters separately as the second argument.
    JSONRequest.prototype.getJSON = function getJSON(url, post) {
      var cf = new CURLFile();
      try {
        if(post)
          cf.openPost(url, post);
        else
          cf.open(url);
            
        this.responseText = cf.read().toUCString();
        this.response = Core.evalUntrustedString('(' + this.responseText + ')');
      } catch(ex) {
        // TODO: some kind of error reporting.
      } finally {
        cf.close();
      }
      return this;
    };  

    Object.defineProperty(global, 'JSONRequest', {
      enumerable:   false,
      configurable: true,
      writable:     true,
      value:        JSONRequest
    });
  })(this);
}

