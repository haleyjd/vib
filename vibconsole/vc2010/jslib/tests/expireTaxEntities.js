(function () {
  var sqlQuery = 
    "select TE.ID, count(VT.ID) " +
    "from tax_entities TE " +
    "left join VISIT_TASKS VT on VT.TAX_ENTITY_ID = TE.ID and VT.DATE_OF_TASK >= '1/1/2014' " +
    "where exists (" +
    "  select * from COB_TASKS COB " +
    "  inner join SERVICES_PROVIDED SP on COB.SERVICES_PROVIDED_ID = SP.ID and SP.TAX_ENTITY_ID = TE.ID " +
    ") and not exists ( " +
    "  select * from COB_TASKS COB " +
    "  inner join SERVICES_PROVIDED SP on COB.SERVICES_PROVIDED_ID = SP.ID and SP.TAX_ENTITY_ID = TE.ID " +
    "  where COB.DATE_OF_TASK >= '1/1/2014' " +
    ") and upper(TE.NAME) not like '%ABSENTEE%' " + 
    "group by TE.ID " +
    "having count(VT.ID) = 0";
  
  var db = new PrometheusDB();
  var tr = new PrometheusTransaction();
  
  // connect to the database
  if(!db.connect("live")) {
    Console.println("Error: could not connect to database");
    return;
  }
  if(!tr.stdTransaction(db)) {
    Console.println("Transaction open failure!");
    return;
  }
  
  var teCount = 0;
  var taxEntities = tr.sqlToVecMap(sqlQuery);
  for(var i = 0; i < taxEntities.size(); i++) {
    var taxEntity = taxEntities[i];
    var teToUpdate = {
      id:                taxEntity["id"],
      current_id:        "2",
      last_updated_by:   "330",
      last_update_stamp: "current_timestamp"
    };
    var teOpts = {
      last_update_stamp: SQLOptions.sql_no_uppercasing|SQLOptions.sql_no_quoting
    };
    tr.executeUpdateStatement("tax_entities", teToUpdate, teOpts);
    
    var spUpdateSQL = 
      "update services_provided SP set SP.current_id = 2, SP.last_update_stamp = current_timestamp, " + 
      "SP.last_updated_by = 330 where SP.tax_entity_id = " + taxEntity["id"];
    tr.executeStatement(spUpdateSQL);
    ++teCount;
  }
   
  if(!tr.commit())
    Console.println("Warning: commit failure!");
  else
    Console.println("Expired " + teCount + " tax entities.");
})();