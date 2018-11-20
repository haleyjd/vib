// Create MediaWiki API object
var dw = Core.loadScript('jslib/mediaWikiAPI.js');
dw.setDefaultBaseURLs('doomwiki.org');
  
// Namespace data for areas of the site to include in the map;
// some priorities will be boosted for pages with a high hit count
var namespaces =
[
  { id:  0, priority: '0.8', fn: 'main',   name: ''               }, // Main
  { id:  1, priority: '0.5', fn: 'talk',   name: 'Talk'           }, 
  { id:  2, priority: '0.7', fn: 'user',   name: 'User'           },
  { id:  3, priority: '0.4', fn: 'utalk',  name: 'User talk'      },    
  { id:  4, priority: '0.6', fn: 'dwiki',  name: 'Doom Wiki'      },
  { id:  5, priority: '0.2', fn: 'dwtalk', name: 'Doom Wiki talk' },
  { id:  6, priority: '0.5', fn: 'file',   name: 'File'           },
  { id:  7, priority: '0.3', fn: 'ftalk',  name: 'File talk'      },
  { id: 12, priority: '0.5', fn: 'help',   name: 'Help'           },    
  { id: 13, priority: '0.2', fn: 'htalk',  name: 'Help talk'      },    
  { id: 14, priority: '0.4', fn: 'cat',    name: 'Category'       },
  { id: 15, priority: '0.2', fn: 'ctalk',  name: 'Category talk'  }
];

// Data on all images on the wiki  
var allimages = [];

// Map from image title to the index in allimages, for fast lookup
var imagemap  = {};

var GetSitemapData = function ()
{
  // Create a new MediaWiki JSON query for the given namespace to retrieve
  // all pages in that namespace.
  var createNamespaceQuery = function (ns)
  {
    var q = dw.createJSONQuery();
    var qobj = 
    {
      generator:      'allpages',         // generate all pages...      
      gapnamespace:   ns.id,              // ...in target namespace
      gapfilterredir: 'nonredirects',     // ...that are not redirects
      gaplimit:       500,                // ...get 500 at a time
      prop:           ['info', 'images'], // for each, get page info and images
      imlimit:        5000                // ...and pull the max amount of images
    };
    q.addObjectAsArguments(qobj);
    return q;
  };
  
  // Create a new MediaWiki JSON query for images
  var createImageQuery = function (pageid)
  {
    var q = dw.createJSONQuery();
    var qobj =
    {
      list:    'allimages', // query all images
      ailimit: 1000         // 1000 at a time
    };
    q.addObjectAsArguments(qobj);
    return q;
  };
  
  var getOneNamespace = function (ns)
  {
    var q    = createNamespaceQuery(ns);
    var json = q.executeQuery();
    ns.pages = {};
    ns.titles = [];
    while(1)
    {
      var pages = pathGet(json, 'response.query.pages');
      if(pages)
      {
        for(var pageid in pages)
        {
          if(pages.hasOwnProperty(pageid))
          {
            ns.pages[pageid] = pages[pageid];
            ns.titles.push(ns.pages[pageid].title);
          }
        }
      }
      
      Console.println(q.toString()); // DEBUG
      
      Timer.delay(6000); // wait 6 seconds between these queries
      
      // loop until the API returns no query-continue element
      if(!(json = q.continueQueryFrom(json, 'allpages', 'gapcontinue')))
        break;
    }
  };

  // Iterate over all the namespace objects and retrieve data for all articles
  var getAllNamespaces = function ()
  {
    namespaces.forEach(getOneNamespace);
  };
  
  // Get data on all images at once.
  var getAllImages = function ()
  {
    var q    = createImageQuery();
    var json = q.executeQuery();
    while(1)
    {
      var curImages = pathGet(json, 'response.query.allimages');
      if(curImages)
      {
        curImages.forEach(function(elem) {
          allimages.push(elem);
          imagemap[elem.name] = allimages.length - 1;
        });
      }
      
      Console.println(q.toString()); // DEBUG
      
      Timer.delay(6000); // wait 6 seconds between these queries
      
      // loop until the API returns no query-continue element
      if(!(json = q.continueQueryFrom(json, 'allimages', 'aicontinue')))
        break;
    }
  };

  // Main routine
  
  // get login credentials and log into MediaWiki
  var creds    = Core.loadJSON('s:/dwLoginCreds.json');
  var loginRes = dw.doLogin(creds.user, creds.password);
  
  if(loginRes.error)
  {
    Console.println(loginRes.error);
    return;
  }
  
  getAllNamespaces(); // get article info
  getAllImages();     // get image info
  
  // log out
  dw.doLogout();
}; 
