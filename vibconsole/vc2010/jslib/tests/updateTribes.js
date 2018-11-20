updateTribes = function () 
{
  var db = new PrometheusDB();
  
  if(!db.connect("ndw")) 
  {
    Console.println("Error: could not connect to database");
    return;
  }
  
  var tribes = LazyVecMap.FromCSV("s:/w_tribe.csv");
  
  if(tribes.size() == 0) 
  {
    Console.println("Nothing in the CSV file??");
    return;
  }
  
  var tr = new PrometheusTransaction();  
  if(!tr.stdTransaction(db)) 
  {
    Console.println("Transaction open failure!");
    return;
  }
  
  var tribeExists = function (code, tr)
  {
    var count = tr.getOneField('select count(id) from tribe_types where code = ' + code);
    return count > 0;
  };
  
  var code, tribe, active, shortItem;
  var fieldMap, fieldOpts;
  
  for(var i = 0; i < tribes.size(); i++) 
  {
    code   = tribes[i]['Code'];
    tribe  = tribes[i]['Tribe'];
    active = tribes[i]['Active Flag'];
    
    if(active === 'Inactive')
      continue;
    
    if(tribeExists(code, tr))
      continue;
    
    // Make the short item pretty  
    shortItem = Utils.PrettyString(tribe);    
    
    // Remove parenthesized info
    if(shortItem.lastIndexOf('(') >= 0)
      shortItem = shortItem.substr(0, shortItem.lastIndexOf('('));
    
    // Fix state abbreviations and short words
    shortItem = shortItem.replace(
      /\b(A[klrz]|C[aot]|D[ce]|Fl|Ga|Hi|I[adln]|K[sy]|La|M[adeinost]|N[cdehjmvy]|O[hkr]|P[ar]|Ri|S[cd]|T[nx]|Ut|V[ait]|W[aivy])\b/g, 
      function (ss) { return ss.toUpperCase(); }
    );
    shortItem = shortItem.replace(
      /\b(And|Of|On|The)\b/g, 
      function (ss) { if(ss == 'And') return '&'; else return ss.toLowerCase(); }
    );
    
    // special cases
    if(shortItem.search('ME-Wuk') >= 0)
      shortItem = shortItem.replace('ME-Wuk', 'Me-Wuk');      
    if(code == 131 || code == 742)
      shortItem = shortItem.replace(/\bIN\b/g, 'in');
    if(code == 303 || code == 334)
      shortItem = shortItem.replace(/bLA\b/g, 'La');
    if(code == 406)
      shortItem = shortItem.replace(/\bDE\b/g, 'de');
    if(code == 420)
      shortItem = 'Big Valley Band of Pomo Indians';
      
    // remove dead spaces or commas at end
    shortItem = Utils.StripSurrounding(shortItem, ' ');
    shortItem = Utils.StripTrailing(shortItem, ',');
    
    fieldMap  = new Object;
    fieldOpts = new Object;
        
    fieldMap['item'      ] = tribe;
    fieldMap['valid'     ] = '1';
    fieldMap['code'      ] = code;
    fieldMap['short_item'] = shortItem;
    
    fieldOpts['short_item'] = SQLOptions.sql_no_uppercasing;
    
    if(!tr.executeInsertStatement("tribe_types", fieldMap, fieldOpts)) 
    {
      Console.println("Insert failure!");
      tr.rollback();
      return;
    }    
  }

  if(!tr.commit()) 
  {
    Console.println("WARNING: commit failure");
    tr.rollback();
  }
  
  db.disconnect();
};