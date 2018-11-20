importICD10 = function () {
  
  // global variables
  var pcsTypeID;
  var cmTypeID;
  
  // initialization
  var initGlobals = function (tr) {    
    pcsTypeID = tr.getOneField("select id from code_types where item = 'ICD-10 PCS'");
    cmTypeID  = tr.getOneField("select id from code_types where item = 'ICD-10 CM'");
  };
  
  // insert one code to the database
  var insertCode = function (tr, codeType, codeNum, codeDesc) {
    // get unique ID for new record
    var newID = tr.getNextId("codes");
    if(Utils.IsBlankOrZero(newID))
      return false;
    
    var fieldMap  = new Object;
    var fieldOpts = new Object;

    // build insert map    
    fieldMap['id'               ] = newID;
    fieldMap['code_id'          ] = codeType; 
    fieldMap['code'             ] = codeNum; 
    fieldMap['description'      ] = codeDesc.substr(0, 1023);
    fieldMap['current_id'       ] = '4';   // current 
    fieldMap['last_updated_by'  ] = '330'; // me
    fieldMap['clinic_cost'      ] = '0.0';
    fieldMap['medicare_cost'    ] = '0.0';
    fieldMap['medicaid_cost'    ] = '0.0';
    fieldMap['last_update_stamp'] = (new Pstamp()).toString(); // right now
    
    // do it.
    return tr.executeInsertStatement("codes", fieldMap, fieldOpts);    
  };
  
  // test if a CM code is within a numeric range inclusive
  var codeInRange = function (code, substart, sublen, rangestart, rangeend) {
    var codeNum = (new Number(code.substr(substart, sublen))).valueOf();
    
    return (codeNum >= rangestart && codeNum <= rangeend);
  };
  
  // test if a CM code is in a set in the form of a passed-in array of numbers
  var codeInSet = function (code, codeSet) {
    var codeNum = (new Number(code.substr(1, 2))).valueOf();
    
    return (codeSet.indexOf(codeNum) != -1);
  };
  
  // Detect special case three-letter CM codes that need to be extended to 7 
  // characters (ie. T66.XXXA, V99.XXXS) and return the suffixes to the codes 
  // and descriptions which need to be generated as an array of objects. 
  // Returns an empty array if the code does not fall into this category of codes.
  var detectCMShortCode = function (code) {
    if(code.length != 3)
      return [];
      
    switch(code.substr(0, 1)) 
    {
    case 'T':
      if(codeInSet(code, [66, 68])) {
        return [
          { code: 'A', desc: ', initial encounter' },
          { code: 'D', desc: ', subsequent encounter' },
          { code: 'S', desc: ', sequela' }];
      }
      break;
    case 'V':
      if(codeInSet(code, [99])) {
        return [
          { code: 'A', desc: ', initial encounter' },
          { code: 'D', desc: ', subsequent encounter' },
          { code: 'S', desc: ', sequela' }];
      }
      break;
    case 'W':
      if(codeInSet(code, [3,4,6,7,8,11,12,14,15,19, 25,28,35,38,39,51,52,57,60,
                          64,65,67,69,73,74,85,92,99])) 
      {
        return [
          { code: 'A', desc: ', initial encounter' },
          { code: 'D', desc: ', subsequent encounter' },
          { code: 'S', desc: ', sequela' }];
      }
      break;
    case 'X':
      if(codeInSet(code, [4,5,12,16,17,18,19,30,31,32,34,35,38,52,58,72,75,76,
                          79,80,93,97]))
      {
        return [
          { code: 'A', desc: ', initial encounter' },
          { code: 'D', desc: ', subsequent encounter' },
          { code: 'S', desc: ', sequela' }];
      }
      break;
    case 'Y':
      if(codeInSet(code, [0,1,22,25,26,29,30,31,32,33])) {
        return [
          { code: 'A', desc: ', initial encounter' },
          { code: 'D', desc: ', subsequent encounter' },
          { code: 'S', desc: ', sequela' }];
      }
      break;
    }
    
    return [];
  };
  
  // Detect special case four-or-more character CM codes that need to be 
  // extended to 7 characters (ie. M48.4XXA) and return the suffixes to the 
  // codes and descriptions which need to be generated as an array of objects.
  // Returns an empty array if the code does not fall into this category of codes.
  var detectCMSpecialRange = function (code) {
    if(code.length < 4) // doesn't apply to codes without a '.' in them
      return [];
      
    switch(code.substr(0, 1))
    {
    case 'M':
      if(code.substr(0, 4) == 'M1A.') {
        return [
          { code: '0', desc: ', without tophus' }, 
          { code: '1', desc: ', with tophus'    }];
      }
      else if(codeInRange(code, 1, 4, 48.4, 48.5)) {
        return [
          { code: 'A', desc: ', initial encounter' },
          { code: 'D', desc: ', subsequent encounter, routine healing' },
          { code: 'G', desc: ', subsequent encounter, delayed healing' },
          { code: 'S', desc: ', sequela' }];
      }
      else if(codeInRange(code, 1, 2, 80,   80  ) ||
              codeInRange(code, 1, 4, 84.3, 84.6)) {
        return [
          { code: 'A', desc: ', initial encounter' },
          { code: 'D', desc: ', subsequent encounter, routine healing' },
          { code: 'G', desc: ', subsequent encounter, delayed healing' },
          { code: 'K', desc: ', subsequent encounter, nonunion' },
          { code: 'P', desc: ', subsequent encounter, malunion' },
          { code: 'S', desc: ', sequela' }];
      }
      break;
    case 'O':
      if(codeInRange(code, 1, 2, 31, 32) ||
         codeInRange(code, 1, 4, 33.4, 33.7) ||
         codeInRange(code, 1, 2, 35, 41) ||
         codeInRange(code, 1, 4, 60.1, 60.2) ||
         codeInRange(code, 1, 2, 64, 64) ||
         codeInRange(code, 1, 2, 69, 69)) {
        return [
          { code: '0', desc: ', NA or unspecified' },
          { code: '1', desc: ', fetus 1' },
          { code: '2', desc: ', fetus 2' },
          { code: '3', desc: ', fetus 3' },
          { code: '4', desc: ', fetus 4' },
          { code: '5', desc: ', fetus 5' },
          { code: '9', desc: ', other fetus' }];
      }  
      break;
    case 'R':
      if(codeInRange(code, 1, 5, 40.21, 40.23)) {
        return [
          { code: '0', desc: ', unspecified time' },
          { code: '1', desc: ', in the field [EMT/ambulance]' },
          { code: '2', desc: ', at arrival to ER' },
          { code: '3', desc: ', at hospital admission' },
          { code: '4', desc: ', 24 hrs or more after hospital admission' }];
      }
      break;
    case 'S':
      if(codeInRange(code, 1, 2, 0, 78)) {
        return [
          { code: 'A', desc: ', initial encounter' },
          { code: 'D', desc: ', subsequent encounter' },
          { code: 'S', desc: ', sequela' }];
      }
      else if(codeInRange(code, 1, 4, 79.0, 79.1)) {
        return [
          { code: 'A', desc: ', initial encounter for closed fracture' },
          { code: 'D', desc: ', subsequent encounter, routine healing' },
          { code: 'G', desc: ', subsequent encounter, delayed healing' },
          { code: 'K', desc: ', subsequent encounter, nonunion' },
          { code: 'P', desc: ', subsequent encounter, malunion' },
          { code: 'S', desc: ', sequela' }];
      }
      else if(codeInRange(code, 1, 4, 79.8, 79.9) ||
              codeInRange(code, 1, 2, 80,   81  ) ||
              codeInRange(code, 1, 2, 83,   88  ) ||
              codeInRange(code, 1, 4, 89.8, 89.9) ||
              codeInRange(code, 1, 2, 90,   99  )) {
        return [
          { code: 'A', desc: ', initial encounter' },
          { code: 'D', desc: ', subsequent encounter' },
          { code: 'S', desc: ', sequela' }];
      }
      else if(codeInRange(code, 1, 2, 82, 82)) {
        return [
          { code: 'A', desc: ', initial encounter for closed fracture' },
          { code: 'B', desc: ', initial encounter for open fracture type I/II' },
          { code: 'C', desc: ', initial encounter for open fracture type IIIA/B/C' },
          { code: 'D', desc: ', subsequent encounter for closed fracture, routine healing' },
          { code: 'E', desc: ', subsequent encounter for open fracture type I/II, routine healing' },
          { code: 'F', desc: ', subsequent encounter for open fracture type IIIA/B/C, routine healing' },
          { code: 'G', desc: ', subsequent encounter for closed fracture, delayed healing' },
          { code: 'H', desc: ', subsequent encounter for open fracture type I/II, delayed healing' },
          { code: 'J', desc: ', subsequent encounter for open fracture type IIIA/B/C, delayed healing' },
          { code: 'K', desc: ', subsequent encounter for closed fracture, nonunion' },
          { code: 'M', desc: ', subsequent encounter for open fracture type I/II, nonunion' },
          { code: 'N', desc: ', subsequent encounter for open fracture type IIIA/B/C, nonunion' },
          { code: 'P', desc: ', subsequent encounter for closed fracture, malunion' },
          { code: 'Q', desc: ', subsequent encounter for open fracture type I/II, malunion' },
          { code: 'R', desc: ', subsequent encounter for open fracture type IIIA/B/C, malunion' },
          { code: 'S', desc: ', sequela' }];
      }
      else if(codeInRange(code, 1, 4, 89.0, 89.3)) {
        return [
          { code: 'A', desc: ', initial encounter' },
          { code: 'D', desc: ', subsequent encounter, routine healing' },
          { code: 'G', desc: ', subsequent encounter, delayed healing' },
          { code: 'K', desc: ', subsequent encounter, nonunion' },
          { code: 'P', desc: ', subsequent encounter, malunion' },
          { code: 'S', desc: ', sequela' }];
      }
      break;
    case 'T':
      if(codeInRange(code, 1, 2, 15, 28) ||
         codeInRange(code, 1, 2, 33, 85) ||
         codeInRange(code, 1, 2, 88, 88)) {
        return [
          { code: 'A', desc: ', initial encounter' },
          { code: 'D', desc: ', subsequent encounter' },
          { code: 'S', desc: ', sequela' }];
      }  
      break;
    case 'V':
    case 'W':
    case 'X':
      if(codeInRange(code, 1, 2, 0, 99)) {
        return [
          { code: 'A', desc: ', initial encounter' },
          { code: 'D', desc: ', subsequent encounter' },
          { code: 'S', desc: ', sequela' }];
      }
      break;
    case 'Y':
      if(codeInRange(code, 1, 2, 0, 4) || codeInRange(code, 1, 2, 8, 38)) {
        return [
          { code: 'A', desc: ', initial encounter' },
          { code: 'D', desc: ', subsequent encounter' },
          { code: 'S', desc: ', sequela' }];
      }
      break;
    default:
      break;
    }    
    
    return [];
  };
  
  // pad an ICD-10 CM code out to 7 characters with 'X' chars
  var padCode = function (code) {
    return code + Array(7 + 1 - code.length).join('X');
  };
  
  // Import the ICD-10 CM codeset
  var importCM = function (tr) {
    var cm = LazyVecMap.FromCSV("datafiles/ICD10/Tabular.xml.csv");
    
    if(cm.size() == 0) {
      Console.println("Nothing in the CM file??");
      return;
    }
    
    var codeNum, codeDesc;
    var specArray;
    
    for(var i = 0; i < cm.size(); i++) {
      codeNum  = cm[i]['Code'];
      codeDesc = cm[i]['Description'];
      
      // detect special cases for three letter codes
      specArray = detectCMShortCode(codeNum);
      
      if(specArray.length > 0) {
        // insert the code with every possible subcode
        specArray.forEach(function (elem) {
          var descConcat = codeDesc + elem.desc;
          var numConcat  = codeNum  + '.'; // add a dot
          numConcat = padCode(numConcat);  // pad with X
          numConcat += elem.code;          // add the subcode
          
          if(!insertCode(tr, cmTypeID, numConcat, descConcat)) {
            Console.println("Insertion of code " + numConcat + " faled for CM codeset!");
            throw new TypeError; // abort
          }
        });
      }
      
      // detect special cases for seven letter code generation
      specArray = detectCMSpecialRange(codeNum);
      
      if(specArray.length > 0) {
        // insert the code with every possible subcode
        specArray.forEach(function (elem) {
          var descConcat = codeDesc + elem.desc;
          var numConcat  = codeNum;
          
          if(numConcat.length < 7)
            numConcat = padCode(numConcat);          
          numConcat += elem.code;
          
          if(!insertCode(tr, cmTypeID, numConcat, descConcat)) {
            Console.println("Insertion of code " + numConcat + " failed for CM codeset!");
            throw new TypeError; // abort
          }
        });
      }
      
      // insert the normal code (even if it has subcodes)
      if(!insertCode(tr, cmTypeID, codeNum, codeDesc)) {
        Console.println("Insertion of code " + codeNum + " failed for CM codeset!");
        throw new TypeError; // STOP!
      }
    }
  };
  
  // Import the ICD-10 PCS codeset
  var importPCS = function (tr) {
    var pcs = LazyVecMap.FromCSV("datafiles/ICD10/icd10pcs_tabular_2014.xml.csv");
    
    if(pcs.size() == 0) {
      Console.println("Nothing in the PCS file??");
      return;
    }
    
    var codeNum, codeDesc;
    
    for(var i = 0; i < pcs.size(); i++) {
      codeNum  = pcs[i]['Code'];
      codeDesc = pcs[i]['Description'];
      
      if(!insertCode(tr, pcsTypeID, codeNum, codeDesc)) {
        Console.println("Insertion of code " + codeNum + " failed for PCS codeset!");
        throw new TypeError; // STOP!
      }
    }
  };   
   
  //
  // Main Program
  //
  
  var db = new PrometheusDB();
  
  // connect to the database
  if(!db.connect("ndw")) {
    Console.println("Error: could not connect to database");
    return;
  }

  // open transaction for insertions
  var tr = new PrometheusTransaction();  
  if(!tr.stdTransaction(db)) {
    Console.println("Transaction open failure!");
    return;
  }
  
  // do initialization
  initGlobals(tr);

  // import the codesets
  try {
    importPCS(tr);
    Core.GC();
    
    importCM(tr);
    Core.GC();
    
    // commit the transaction
    if(!tr.commit()) {
      Console.println("WARNING: commit failure!");
      tr.rollback();
    }
  } 
  catch(err) {
    Console.println("Caught exception in main routine, exiting");    
    tr.rollback();
  }
  
  // close the connection
  db.disconnect();
};