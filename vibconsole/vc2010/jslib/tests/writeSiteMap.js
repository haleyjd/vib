var OutputSiteMap = function ()
{
  // Calculate the edit frequency using a crude estimate based only on the last
  // time the article was modified. The estimate will never be more than weekly
  // and never less than yearly.
  var calcEditFreq  = function (touched)
  {
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
  
  // Write out a single image.
  var writeImage = function (outFile, image)
  {
    outFile.puts('    <image:image>\n' +
                 '      <image:loc>'     + dw.encodeForXML(image.url)            + '</image:loc>\n'     +
                 '      <image:title>'   + dw.encodeForXML(image.name)           + '</image:title>\n'   +
                 '      <image:license>' + dw.encodeForXML(image.descriptionurl) + '</image:license>\n' +
                 '    </image:image>\n');
  };
  
  // Write the XML data for a single page.
  var writePageData = function (outFile, ns, pageData)
  {
    // get base priority; some modifications may be made below.
    var priority = ns.priority;
    
    // calculate edit frequency value
    var editFreq = calcEditFreq(pageData.touched);
    
    // main namespace tweaks
    if(ns.id == 0)
    {
      // downgrade articles with certain substrings
      if(pageData.title.indexOf('.') >= 0 || pageData.title.indexOf('(') >= 0 ||
         pageData.title.indexOf('MAP') == 0 ||
         (pageData.title.indexOf('E') == 0 && pageData.title.indexOf('M') == 2 &&
          pageData.title.indexOf(':') == 4))
      {
        // map articles, articles about files, and person articles mostly.
        // A few false positives, but nothing drastic.
        priority = '0.7';
      }
      
      // downgrade based on low hit count or low edit frequency
      if(pageData.counter < 1000 || editFreq == 'yearly')   // bleh
        priority = '0.7';
      
      // upgrade based on high hit count
      if(pageData.counter >= 10000)  // important main space
        priority = '0.9';
      if(pageData.counter >= 100000) // super-important main space
        priority = '1.0';
    }
      
    // Talk namespace tweaks
    if(ns.id == 1)
    {
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
    if(ns.id == 2 || ns.id == 3)
    {
      // anonymous user or user talk page
      if(pageData.title.split('.').length - 1 == 3)
        priority = '0.1'; 
    }
    
    // Doom Wiki namespace tweaks
    if(ns.id == 4)
    { 
      if(pageData.counter >= 2000)   // important Doom Wiki article
        priority = '0.7';
      if(pageData.counter >= 4000)   // super-important Doom Wiki article
        priority = '0.8';
    }
      
    // subpages in ANY namespace are unimportant
    if(pageData.title.indexOf('/') >= 0)
    {
      if(ns.titles.indexOf((pageData.title.split('/'))[0]) >= 0)
        priority = '0.1';
    }
    
    outFile.puts('  <url>\n' +
                 '    <loc>'        + dw.getCanonicalURLForXML(pageData.title) + '</loc>\n'        +
                 '    <lastmod>'    + pageData.touched                         + '</lastmod>\n'    +
                 '    <changefreq>' + editFreq                                 + '</changefreq>\n' +
                 '    <priority>'   + priority                                 + '</priority>\n');

    if(pageData.images)
    {
      pageData.images.forEach(function(elem) {
        var lookupTitle = dw.getCanonicalTitle(elem.title.replace("File:", ""))
        if(imagemap[lookupTitle])
          writeImage(outFile, allimages[imagemap[lookupTitle]]);
      });
    }
    
    outFile.puts('  </url>\n');
  };
  
  // Write out the sitemap XML for a single namespace's page data.
  var writeNamespaceMapFile = function (ns)
  {
    if(!ns.pages) // nothing to write?
      return;
    
    var outFile = new File(ns.fn + '.xml', 'w');
        
    if(outFile.isOpen())
    {
      // open XML and urlset
      outFile.puts('<?xml version="1.0" encoding="UTF-8"?>\n' +
                   '<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9"\n' +
                   ' xmlns:image="http://www.google.com/schemas/sitemap-image/1.1">\n');
      
      for(var page in ns.pages)
      {
        if(ns.pages.hasOwnProperty(page))
          writePageData(outFile, ns, ns.pages[page]);
      }
      
      // close urlset and end file
      outFile.puts('</urlset>');
      
      outFile.close();
    }    
  };
  
  // Main routine: write out all the articles to XML sitemap files
  namespaces.forEach(writeNamespaceMapFile);
};