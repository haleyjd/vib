(function () {
  // create MediaWiki API object
  var mw = Core.loadScript('jslib/mediaWikiAPI.js');
  mw.setDefaultBaseURLs('doomwiki.org');
  
  // Query the wiki for the edit token and revision text for one page
  var queryPageData = function (page) {
    // get edit token and revision text
    var getPageQuery = mw.createPageEditQuery(mw.getCanonicalTitle('File:' + page.file));
    Console.println('Get page query: ' + getPageQuery);
    return getPageQuery.executeQuery();
  };
  
  // Edit one page from the pagelist JSON
  var editOnePage = function (page) {    
    Console.println('Going to edit page ' + page.file + ' to change to category ' + page.category);
    
    // get page data
    var pageData = queryPageData(page);
    
    try {
      var pagesObj = pageData.response.query.pages;
      for(var p in pagesObj) {
        Console.println('Page ID: ' + p);
        if(pagesObj.hasOwnProperty(p)) {
          var pp       = pagesObj[p];
          var revision = pp.revisions[0];
          var wikitext = revision['*'];
          
          wikitext = wikitext.replace('{{Screenshot-comm}}', '{{Screenshot-doom|' + page.category + '}}');
          
          // submit edited page
          var editDesc = 'Automated edit - change image categorization to ' + page.category;
          var putEditQuery = mw.prepareEditSubmit(p, wikitext, editDesc, '1', revision.timestamp, pp.starttimestamp, pp.edittoken);
          Console.println('Edit query: ' + putEditQuery.getArgumentString());
          var resp = putEditQuery.executeQuery();
          Console.println('Response: ' + resp.toSource());
          break; // only one result should be present per query
        }        
      }
    } catch(ex) {
      Core.println('Aborting: Error while editing page ' + page.file);
      throw ex;
    }
    Timer.delay(6000); // wait 6 seconds between edits
  };

  //
  // Main Routine
  //
 
  // get pagelist
  var pagelist = Core.loadJSON('s:/pagelist.json');
  
  // do login
  var creds    = Core.loadJSON('s:/dwLoginCreds.json');
  var loginRes = mw.doLogin(creds.user, creds.password);
     
  if(loginRes.error) {
    Console.println('Login error: ' + loginRes.error);
    return;
  }
  
  // For each page in the pagelist, perform an edit.
  pagelist.pages.forEach(editOnePage);
  
  // Log out
  mw.doLogout();
})();