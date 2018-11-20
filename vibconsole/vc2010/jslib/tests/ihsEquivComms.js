equivComms = function () {
  var db = new PrometheusDB();
  
  if(!db.connect("live")) {
    Console.println("Error: could not connect to database");
    return;
  }
  
  var communities = LazyVecMap.FromCSV("w_community.txt");
  
  if(communities.size() == 0) {
    Console.println("Nothing in the CSV file??");
    return;
  }
  
  var tr = new PrometheusTransaction();  
  if(!tr.stdTransaction(db)) {
    Console.println("Transaction open failure!");
    return;
  }
  
  var count, code, state, county, community, asu, status;
  var fieldMap, fieldOpts;
  
  for(var i = 0; i < communities.size(); i++) {
    code      = communities[i]['Code'];
    community = communities[i]['Community'];
    status    = communities[i]['Status'];
    
    if(status.valueOf() === 'Inactive')
      continue;
      
    count = tr.getOneField(
      "select count(*) from ihs_equiv_communities_by_city " +
      "where city = '" + community + "' and " +
      "ihs_state_id = '" + code.substr(0, 2) + "' and " +
      "ihs_county_id = '" + code.substr(2, 2) + "' and " +
      "ihs_community_id = '" + code.substr(4, 3) + "'");
      
    if(count > 0)
      continue;
      
    state = tr.getOneField(
      "select state_id from ihs_equiv_states where ihs_state_id = '" + code.substr(0, 2) + "'");
    
    fieldMap  = new Object;
    fieldOpts = new Object;
    
    fieldMap['state_id'        ] = state;
    fieldMap['city'            ] = community;
    fieldMap['ihs_state_id'    ] = code.substr(0, 2);
    fieldMap['ihs_county_id'   ] = code.substr(2, 2);
    fieldMap['ihs_community_id'] = code.substr(4, 3);
    
    if(!tr.executeInsertStatement("ihs_equiv_communities_by_city", fieldMap, fieldOpts)) {
      Console.println("Insert failure!");
      tr.rollback();
      return;
    }    
  }

  if(!tr.commit()) {
    Console.println("WARNING: commit failure");
    tr.rollback();
  }
  
  db.disconnect();
};