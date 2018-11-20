ihsComms = function () {
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
  
  tr.executeStatement("delete from ihs_communities");
  
  var code, state, county, community, asu, status;
  var fieldMap, fieldOpts;
  
  for(var i = 0; i < communities.size(); i++) {
    code      = communities[i]['Code'];
    state     = communities[i]['State'];
    county    = communities[i]['County'];
    community = communities[i]['Community'];
    asu       = communities[i]['ASU Code'];
    status    = communities[i]['Status'];
    
    if(status.valueOf() === 'Inactive')
      continue;
    
    fieldMap  = new Object;
    fieldOpts = new Object;
    
    fieldMap['ihs_state_id'       ] = code.substr(0, 2);
    fieldMap['ihs_county_id'      ] = code.substr(2, 2);
    fieldMap['ihs_community_id'   ] = code.substr(4, 3);
    fieldMap['item'               ] = community;
    fieldMap['ihs_area_id'        ] = asu.substr(0, 2);
    fieldMap['ihs_service_unit_id'] = asu.substr(2, 2);
    
    if(!tr.executeInsertStatement("ihs_communities", fieldMap, fieldOpts)) {
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