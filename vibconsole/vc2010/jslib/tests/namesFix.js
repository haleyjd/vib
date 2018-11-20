namesFix = function () {
  var db = new PrometheusDB();
  
  if(!db.connect("live")) {
    Console.println("Error: could not connect to database");
    return;
  }
  
  var peopleIDs = LazyVecMap.FromCSV("namefixids.csv");
  
  if(peopleIDs.size() == 0) {
    Console.println("Nothing in the CSV file??");
    return;
  }
  
  for(var i = 0; i < peopleIDs.size(); i++) {
    var pid = peopleIDs[i]["id"];
    
    var tr = new PrometheusTransaction();
    
    if(!tr.stdTransaction(db)) {
      Console.println("Transaction open failure!");
      break;
    }
 
    // fetch the name records
    var getNamesSQL = 
      "select id, last_updated_by, first_name, middle_name, last_name " +
      "from person_names where current_id = 4 and person_id = " + pid;
    var namesvecmap = tr.sqlToVecMap(getNamesSQL);

    // looking for people with exactly two names 
    if(namesvecmap.size() != 2) {
      Console.println("Person " + pid + " has too many or not enough names, skipping");
      tr.rollback();
      continue;
    }
    
    // is exactly one of them last edited by HL7Daemon?
    var hl7count = 0;
    for(var nameidx = 0; nameidx < namesvecmap.size(); nameidx++) {
       if(namesvecmap[nameidx]["last_updated_by"] == "400")
         ++hl7count;
    }
    
    if(hl7count != 1) {
      Console.println("Wrong # of records edited by HL7Daemon for person " + pid);
      tr.rollback();
      continue; // don't try to do this one.
    }

    // check if the two names are roughly equal
    var fn1, fn2, mn1, mn2, ln1, ln2;
    fn1  = Utils.UppercaseString(namesvecmap[0]["first_name"]);
    fn2  = Utils.UppercaseString(namesvecmap[1]["first_name"]);
    mn1  = Utils.UppercaseString(namesvecmap[0]["middle_name"]);
    mn2  = Utils.UppercaseString(namesvecmap[1]["middle_name"]);
    ln1  = Utils.UppercaseString(namesvecmap[0]["last_name"]);
    ln2  = Utils.UppercaseString(namesvecmap[1]["last_name"]);
    
    var whichToKeep;
    
    if(fn1 != fn2 || ln1 != ln2) {
      Console.println("First/last names for person " + pid + " don't exactly match, skipping");
      tr.rollback();
      continue;
    } 
      
    if(mn1 != mn2) {
      if(mn1 == '' || mn2 == '' || mn1[0] != mn2[0]) {
        Console.println("Middle names for person " + pid + " don't match, skipping");
        tr.rollback();
        continue;
      }
    }
    
    // middle name matches fully or by prefix
    // figure out which record to keep and which to throw away
    if(namesvecmap[0]["last_updated_by"] == "400")
      whichToKeep = 1;
    else
      whichToKeep = 0;
   
    /*
    Console.println("For person " + pid + " I will keep record " + whichToKeep +
                    " which has id #" + namesvecmap[whichToKeep]["id"]); 
    Console.println(namesvecmap[whichToKeep].toCommaString());
    Console.println("I will delete " + namesvecmap[whichToKeep ^ 1]["id"]);
    Console.println(namesvecmap[whichToKeep ^ 1].toCommaString());
    */
    
    Console.println("For " + pid + ": keeping " + namesvecmap[whichToKeep]["id"] +
                    "; deleting " + namesvecmap[whichToKeep ^ 1]["id"]);
    
    var updateSQL = 
      "update person_names set last_updated_by = 330, last_update_stamp = '1/24/2013 16:30:00' " + 
      "where id = " + namesvecmap[whichToKeep]["id"];
    
    if(!tr.executeStatement(updateSQL)) { // it should now be people.person_names_id
      Console.println("Failed executing update statement!");
      tr.rollback();
      break; // STOP!
    }
    
    // Don't really delete it. Mark it deleted. After verifying nothing went awry,
    // I'll be able to find all of these with a query and THEN delete them.
    var deleteSQL =
      "update person_names set current_id = 0, last_update_stamp = '1/24/2013 16:30:00' " +
      "where id = " + namesvecmap[whichToKeep ^ 1]["id"];
      
    if(!tr.executeStatement(deleteSQL)) {
      Console.println("Failed executing delete statement!");
      tr.rollback();
      break; // STOP!
    }
    
    if(!tr.commit()) {
      Console.println("WARNING: commit failure processing person " + pid);
      break;
    }
  }
  
  db.disconnect();
};