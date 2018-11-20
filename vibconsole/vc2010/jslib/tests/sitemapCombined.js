(function () {
  // Create MediaWiki API object
  var dw = Core.loadScript('jslib/mediaWikiAPI.js');
  dw.setDefaultBaseURLs('doomwiki.org', 'https');
     
  // Namespace data for areas of the site to include in the map;
  // some priorities will be boosted for pages with a high hit count
  var namespaces = [
    { id:  0, priority: '1.0', fn: 'main',   name: ''               }, // Main
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

  var GetSitemapData = function () {
    // Create a new MediaWiki JSON query for the given namespace to retrieve
    // all pages in that namespace.
    var createNamespaceQuery = function (ns) {
      var q = dw.createJSONQuery();
      var qobj = {
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
    var createImageQuery = function (pageid) {
      var q = dw.createJSONQuery();
      var qobj = {
        list:    'allimages', // query all images
        ailimit: 1000         // 1000 at a time
      };
      q.addObjectAsArguments(qobj);
      return q;
    };
     
    var getOneNamespace = function (ns) {
      var q    = createNamespaceQuery(ns);
      var json = q.executeQuery();
      ns.pages = {};
      ns.titles = [];
      while(1) {
        var pages = pathGet(json, 'response.query.pages');
        if(pages) {
          for(var pageid in pages) {
            if(pages.hasOwnProperty(pageid)) {
              ns.pages[pageid] = pages[pageid];
              ns.titles.push(ns.pages[pageid].title);
            }
          }
        }
         
        Console.println(q.toString()); // DEBUG
         
        Timer.delay(6000); // wait 6 seconds between these queries
        
        // loop until the API returns no query-continue element
        if(!(json = q.continueQueryFrom(json)))
          break;
      }
    };

    // Iterate over all the namespace objects and retrieve data for all articles
    var getAllNamespaces = function () {
      namespaces.forEach(getOneNamespace);
    };
     
    // Get data on all images at once.
    var getAllImages = function () {
      var q    = createImageQuery();
      var json = q.executeQuery();
      while(1) {
        var curImages = pathGet(json, 'response.query.allimages');
        if(curImages) {
          curImages.forEach(function(elem) {
            allimages.push(elem);
            imagemap[elem.name] = allimages.length - 1;
          });
        }
         
        Console.println(q.toString()); // DEBUG
         
        Timer.delay(6000); // wait 6 seconds between these queries
        
        // loop until the API returns no query-continue element
        if(!(json = q.continueQueryFrom(json)))
          break;
      }
    };

    // Main routine
     
    // get login credentials and log into MediaWiki
    var creds    = Core.loadJSON('s:/dwLoginCreds.json');
    var loginRes = dw.doLogin(creds.user, creds.password);
     
    if(loginRes.error) {
      Console.println(loginRes.error);
      return;
    }
     
    getAllNamespaces(); // get article info
    getAllImages();     // get image info
     
    // log out
    dw.doLogout();
  }; 

  var OutputSiteMap = function () {
    // Calculate the edit frequency using a crude estimate based only on the last
    // time the article was modified. The estimate will never be more than weekly
    // and never less than yearly.
    var calcEditFreq  = function (touched) {
      var modDate = new Pdate();
       
      // Convert ISO-8601 to JS date, and JS date to Pdate
      modDate.fromDate(Date.fromISO(touched));
       
      // Get the "time ago" string compared to the current date
      var ago = modDate.getTimeSince(new Pdate());
       
      if(ago.search(/year/g) >= 0)  // was years ago?
        return 'yearly';
      if(ago.search(/month/g) >= 0) // was months ago?
        return 'monthly';
         
      return 'weekly'; // modified recently; will probably be modified again soon.
    };
    
    // Make a pretty image name
    var prettyImageName = function (inputname) {
      var ret     = inputname.replace(/_/g, ' ');
      var lastDot = ret.lastIndexOf('.');
      if(lastDot >= 0)
        ret = ret.substr(0, lastDot);
      return dw.encodeForXML(ret);
    };
     
    // Write out a single image.
    var writeImage = function (outFile, image) {
      outFile.puts('    <image:image>\n' +
                   '      <image:loc>'     + dw.encodeForXML(image.url)            + '</image:loc>\n'     +
                   '      <image:title>'   + prettyImageName(image.name)           + '</image:title>\n'   +
                   '      <image:license>' + dw.encodeForXML(image.descriptionurl) + '</image:license>\n' +
                   '    </image:image>\n');
    };
    
    // Write XML data for a single file directory page
    var writeFileDirPage = function (outFile, dir, subdir) {
      outFile.puts('  <url>\n' +
                   '    <loc>' + dw.getEntryPointURLForXML('images/' + dir + '/' + subdir) + '</loc>\n' +
                   '    <changefreq>always</changefreq>\n' +
                   '    <priority>0.3</priority>\n' +
                   '  </url>\n');
    };
    
    // Write the page URLs for all the file directories
    var writeFileDirs = function (outFile) {
      for(var dir = 0; dir < 0x10; dir++) {
        for(var subdir = 0; subdir < 0x10; subdir++) {
          writeFileDirPage(outFile, dir.toString(16), dir.toString(16) + subdir.toString(16));
        }
      }
    };

    // Write the XML data for a single page.
    var writePageData = function (outFile, ns, pageData) {
      // get base priority; some modifications may be made below.
      var priority = ns.priority;
       
      // calculate edit frequency value
      var editFreq = calcEditFreq(pageData.touched);
       
      // main namespace tweaks
      if(ns.id == 0) {
        // downgrade articles with certain substrings
        if(pageData.title.indexOf('.') >= 0 || pageData.title.indexOf('(') >= 0 ||
           pageData.title.indexOf('MAP') == 0 ||
           (pageData.title.indexOf('E') == 0 && pageData.title.indexOf('M') == 2 &&
            pageData.title.indexOf(':') == 4))
        {
          // map articles, articles about files, and person articles mostly.
          // A few false positives, but nothing drastic.
          priority = '0.8';
        }
         
        // downgrade based on low hit count or low edit frequency
        /*
        if(pageData.counter < 1000 || editFreq == 'yearly')   // bleh
          priority = '0.7';
         
        // upgrade based on high hit count
        if(pageData.counter >= 10000)  // important main space
          priority = '0.9';
        if(pageData.counter >= 100000) // super-important main space
          priority = '1.0';
        */
      }
         
      // Talk namespace tweaks
      if(ns.id == 1) {
        if(pageData.title.indexOf('.') >= 0 || pageData.title.indexOf('(') >= 0 ||
           pageData.title.indexOf('MAP') == 0 ||
           (pageData.title.indexOf('E') == 0 && pageData.title.indexOf('M') == 2 &&
            pageData.title.indexOf(':') == 4))
        {
          // map articles, articles about files, and person articles mostly.
          priority = '0.4';
        }
      }
       
      // User/User talk namespace tweaks
      if(ns.id == 2 || ns.id == 3) {
        // anonymous user or user talk page
        if(pageData.title.split('.').length - 1 == 3)
          priority = '0.1'; 
      }
       
      // Doom Wiki namespace tweaks
      if(ns.id == 4) { 
        /*
        if(pageData.counter >= 2000)   // important Doom Wiki article
          priority = '0.7';
        if(pageData.counter >= 4000)   // super-important Doom Wiki article
          priority = '0.8';
        */
      }
         
      // subpages in ANY namespace are unimportant
      if(pageData.title.indexOf('/') >= 0) {
        if(ns.titles.indexOf((pageData.title.split('/'))[0]) >= 0)
          priority = '0.1';
      }
      
      // For File: pages, write out the index.php?curid=<pageid> URL;
      // everything else should use the canonical location URL
      /*
      var locTagValue;
      if(ns.id == 6)
        locTagValue = dw.getEntryPointURLForXML('index.php', 'curid=' + pageData.pageid);
      else
        locTagValue = dw.getCanonicalURLForXML(pageData.title);
      */
      
      outFile.puts('  <url>\n' +
                   '    <loc>'        + dw.getCanonicalURLForXML(pageData.title) + '</loc>\n' +
                   '    <lastmod>'    + pageData.touched + '</lastmod>\n'    +
                   '    <changefreq>' + editFreq         + '</changefreq>\n' +
                   '    <priority>'   + priority         + '</priority>\n');

      if(pageData.images) {
        pageData.images.forEach(function(elem) {
          var lookupTitle = dw.getCanonicalTitle(elem.title.replace("File:", ""))
          if(imagemap[lookupTitle])
            writeImage(outFile, allimages[imagemap[lookupTitle]]);
        });
      }
       
      outFile.puts('  </url>\n');
    };
     
    // Write out the sitemap XML for a single namespace's page data.
    var writeNamespaceMapFile = function (ns) {
      if(!ns.pages) // nothing to write?
        return;
       
      var outFile = new File(ns.fn + '.xml', 'w');
           
      if(outFile.isOpen()) {
        // open XML and urlset
        outFile.puts('<?xml version="1.0" encoding="UTF-8"?>\n' +
                     '<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9"\n' +
                     ' xmlns:image="http://www.google.com/schemas/sitemap-image/1.1">\n');
         
        for(var page in ns.pages) {
          if(ns.pages.hasOwnProperty(page))
            writePageData(outFile, ns, ns.pages[page]);
        }

        // FIXME: currently broken on the server...
        /*
        // Also output file directories when in in the File: namespace
        if(ns.id == 6)
          writeFileDirs(outFile);
        */
         
        // close urlset and end file
        outFile.puts('</urlset>');
         
        outFile.close();
      }    
    };
     
    // Main routine: write out all the articles to XML sitemap files
    namespaces.forEach(writeNamespaceMapFile);
  };
   
  return { getSiteMap: GetSitemapData, outputSiteMap: OutputSiteMap };
})();