(function () {
  // MediaWiki Module object
  return ({
    // Base URL for any entry point
    baseEntryURL: '',
    
    // Base URL of the MediaWiki website API entry point
    apiBaseURL: '',
    
    // Base URL of the MediaWiki website HTML frontend
    webBaseURL: '',
    
    // Utility to set API and web base URLs based on the typical MediaWiki
    // defaults. Just send in your domain name.
    setDefaultBaseURLs: function (domainName, protocol) {
      var protoStr = (protocol ? protocol : 'http');
      this.baseEntryURL = protoStr + '://' + domainName + '/w';
      this.apiBaseURL   = protoStr + '://' + domainName + '/w/api.php';
      this.webBaseURL   = protoStr + '://' + domainName + '/wiki';
    },
    
    // Return the canonical form of an article title; it is not
    // encoded for URI or XML. Spaces will be replaced with 
    // underscores.
    getCanonicalTitle: function (articleTitle) {
      return articleTitle.replace(/ /g, '_');
    },
    
    // Generate a canonical frontend URL for an article title.
    // Encoded as URI only.
    getCanonicalURL: function (articleTitle) {
      // article titles with question marks require special treatment, since encodeURI does
      // not handle them, expecting them to only occur as the beginning of a query string.
      // Google also does not properly parse URLs with single quotes in them, even though
      // they are allowed via the sitemap schema to be directly XML-encoded.
      var encoded = encodeURI(this.webBaseURL + '/' + this.getCanonicalTitle(articleTitle));
      var matches = {
        "'": "%27",
        "?": "%3F"
      };
      return encoded.replace(/'|\?/g, function (m) { return matches[m]; });
    },
    
    encodeForXML: function (url) {
      // ampersands must be replaced first
      var encoded = url.replace(/&/g, "&amp;");
      var matches = {
        "'": "&apos;",
        '"': "&quot;",
        ">": "&gt;",
        "<": "&lt;"
      };
      return encoded.replace(/'|"|>|</g, function (m) { return matches[m]; });
    },
    
    // Given an article title, generate a canonical frontend URL for it
    // and encode it for XML
    getCanonicalURLForXML: function (articleTitle) {
      return this.encodeForXML(this.getCanonicalURL(articleTitle));
    },
    
    // Get a URL for any php entry point
    getEntryPointURL: function (entryPoint, parameters) {
      var url = this.baseEntryURL + '/' + entryPoint;
      if(parameters)
        url += '?' + parameters;
      return url;
    },
    
    // Get an XML-encoded URL for any php entry point
    getEntryPointURLForXML: function (entryPoint, parameters) {
      return this.encodeForXML(this.getEntryPointURL(entryPoint, parameters));
    },

    // Create a JSON Query Object
    createJSONQuery: function () {
      var mwObject = this; // reference to parent object
      return ({
        args: {
          'action': new String('query'), // action is query by default
          'format': new String('json'),  // format of query is JSON
        },
        
        setAction: function (actionName) {
          this.args['action'] = new String(actionName);
        },
        
        // Returns true or false to indicate if action has been set to a value
        // that requires use of HTTP POST rather than GET (right now login,
        // logout, and edit are supported).
        actionRequiresPost: function () {
          return (this.args['action'].valueOf() === 'login'  || 
                  this.args['action'].valueOf() === 'logout' ||
                  this.args['action'].valueOf() === 'edit');
        },
        
        // Add a key=value style argument to the query.
        addArgument: function (name, value) {
          this.args[name] = new String(value);
        },
        
        // Add a key=value|value style argument to the query; 
        // any number of values can be added.
        addMultiArgument: function (name, value) {
          if(!this.args[name] || !(this.args[name] instanceof Array))
            this.args[name] = new Array();
          
          if(value instanceof Array) {
            var dest = this.args[name];
            value.forEach(function (elem) {
              dest.push(new String(elem));
            });
          }
          else
            this.args[name].push(new String(value));
        },
        
        // Add a flag (unvalued) argument to the query.
        addFlag: function (name) {
          this.args[name] = null; // defined, but unvalued
        },

        // Add all properties of an object as arguments of the query;
        // Array properties will be added as multi-valued arguments,
        // null properties will be added as flags, and any other type of
        // property will be interpreted as a string key=value style argument.
        addObjectAsArguments: function (obj) {
          for(var i in obj) {
            if(obj.hasOwnProperty(i)) {
              if(obj[i] instanceof Array)
                this.addMultiArgument(i, obj[i]);
              else if(obj[i] === null)
                this.addFlag(i);
              else
                this.addArgument(i, obj[i]);
            }
          }
        },
        
        // Remove an argument from the query.
        removeArgument: function (name) {
          if(this.args.hasOwnProperty(name) &&       // not from Object
             name != 'action' && name != 'format' && // not action or format
             !(this.args[name] instanceof Function)) // not a method
          {
            delete this.args[name]; // remove the property
          }
        },
        
        // Return all the arguments as an array
        getArgumentArray: function () {
          var arr = [];
          for(var i in this.args) {
            if(this.args.hasOwnProperty(i)) {
              if(this.args[i] instanceof String) { // valued option
                arr.push(i + '=' + this.args[i]);
              }
              else if(this.args[i] instanceof Array) { // multivalued option
                arr.push(i + '=' + this.args[i].join('|'));
              }
              else if(this.args[i] === null) { // flag option
                arr.push(i);
              }
            }
          }
          return arr;
        },
        
        // Return all the arguments as a string
        getArgumentString: function () {
          return this.getArgumentArray().join('&');
        },
        
        // Return the URL representation of the query with all of its present
        // arguments, which are accumulated as properties of this query object.
        toString: function () {
          return encodeURI(mwObject.apiBaseURL + '?' + this.getArgumentString());
        },
        
        // Execute the query using its current set of parameters. A JSONRequest
        // object will be returned. If the query was successful, the evaluated
        // JSON will be in the response property of the returned object. The raw
        // text will be in the responseText property.
        executeQuery: function () {
          var request = new JSONRequest();
          if(this.actionRequiresPost())
            return request.getJSON(mwObject.apiBaseURL, this.getArgumentString());
          else
            return request.getJSON(this.toString());
        },
        
        // Continue a query of a specific type from the given continuation key.
        // Pass in the previous JSON request object; the new one will be returned
        // if the query can be continued. Otherwise, null is returned.
        continueQueryFrom: function (json) {
          var continueValue = null;
          try {
            continueValue = json.response['continue'];
          } catch(ex) {
            return null; // cannot continue the query this way.
          }

          if(!continueValue)
            return null; // still cannot continue the query.

          this.addObjectAsArguments(continueValue);
          return this.executeQuery();
          
          /*
          // OLD MediaWiki 1.24 query continuation mechanism
          var continueValue = null;
          try {
            continueValue = json.response['query-continue'][queryType][continueKey];
          } catch(ex) {
            return null; // cannot continue the query this way.
          }
          
          if(!continueValue)
            return null; // still cannot continue the query.
            
          this.addArgument(continueKey, continueValue);
          return this.executeQuery();
          */
        }        
      });
    }, // end createJSONQuery
    
    // Returns a JSONQuery object setup to retrieve the edit token, timestamp, and
    // revision info, including wikitext, in order to start a page edit.
    createPageEditQuery: function (title) {
      var getPageQuery = this.createJSONQuery();
      getPageQuery.addObjectAsArguments({
        'prop':    'info|revisions',
        'intoken': 'edit',
        'rvprop':  'timestamp|content',
        'titles':  title
      });
      return getPageQuery;
    }, // end createPageEditQuery
    
    // Returns a JSONQuery object setup to perform a HTTP POST edit action,
    // suitable for edits of less than 8000 characters (greater requires 
    // a multipart MIME, which is not yet supported). Do not URL-encode any
    // of the input parameters, as that is taken care of here.
    // * pageid: ID # of the page to edit
    // * wikitext: Your edited wikitext
    // * summary: Edit summary text
    // * bot: If a passed a non-null defined value, this will be flagged as a bot edit
    // * basetimestamp: timestamp taken from the last revision of the page being edited
    // * starttimestamp: timestamp returned in the page object when querying for edit
    // * token: Edit token returned in the page object when querying for edit
    prepareEditSubmit: function (pageid, wikitext, summary, bot, basetimestamp, starttimestamp, token) {
      var putEditQuery = this.createJSONQuery();
      putEditQuery.setAction('edit');
      if(bot)
        putEditQuery.addArgument('bot', '1');
      putEditQuery.addObjectAsArguments({
        'pageid':         pageid,
        'text':           encodeURIComponent(wikitext),
        'summary':        encodeURIComponent(summary),
        'basetimestamp':  basetimestamp,
        'starttimestamp': starttimestamp,
        'token':          encodeURIComponent(token)
      });
      return putEditQuery;
    }, // end prepareEditSubmit
    
    // Login to MediaWiki as a registered user
    doLogin: function (user, password) {
      var json;
      var login;
      var token;
      var query = this.createJSONQuery();
      query.setAction('query');
      query.addObjectAsArguments({ meta: 'tokens', type: 'login' });
      
      // get token
      json  = query.executeQuery();
      token = pathGet(json, 'response.query.tokens.logintoken');
      if(!token)
        return { error: 'No server response' };

      // FIXME / TODO: this should really be getting handled at a lower layer (+ must be encoded in POST arguments)
      var matches = {
        "+": "%2B"
      };
      token = token.replace(/\+/g, function (m) { return matches[m]; });
      
      // do login
      query = this.createJSONQuery();
      query.setAction('login');
      query.addObjectAsArguments({ lgname: user, lgpassword: password, lgtoken: token });
      json  = query.executeQuery();
      login = pathGet(json, 'response.login');

      if(!login)
        return { error: 'No server response' };
      if(!login.result)
        return { error: 'No login result' };
      if(login.result != 'Success')
        return { error: 'Error in result: ' + login.result };

      // Login OK! Return all login data in case needed.
      return json.response;

      /*
      // OLD 1.26 and prior login code:
      var json;
      var login;
      var query = this.createJSONQuery();
      query.setAction('login');
      query.addObjectAsArguments({ lgname: user, lgpassword: password });
      
      // Challenge
      json  = query.executeQuery();
      login = pathGet(json, 'response.login');
      if(!login)
        return { error: 'No server response' };
      if(!login.result)
        return { error: 'No challenge result' };
      if(login.result != 'NeedToken')
        return { error: 'Error in result: ' + login.result };
      if(!login.token)
        return { error: 'No login token returned' };

      // Response
      query.addArgument('lgtoken', login.token);
      json  = query.executeQuery();
      login = pathGet(json, 'response.login');
      if(!login)
        return { error: 'No server response' };
      if(!login.result)
        return { error: 'No login result' };
      if(login.result != 'Success')
        return { error: 'Error in result: ' + login.result };
      
      // Login OK! Return all login data in case needed.
      return json.response;
      */
    }, // end doLogin    
    
    // Log out of MediaWiki
    doLogout: function () {
      var query = this.createJSONQuery();
      query.setAction('logout');
      return query.executeQuery(); // response is usually empty.
    } // end doLogout
  }); // end MediaWiki API object
})();