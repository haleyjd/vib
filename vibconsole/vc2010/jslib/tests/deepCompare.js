deepCompare = function deepCompare(o1, o2) { 
  if(typeof o1 !== typeof o2) 
    return false; // not same type
  if(typeof o1 === 'function') 
    return true; // ignore functions
  else if(typeof o1 !== 'object') 
    return Object.is(o1, o2); // object of a built-in type
  else { 
    if(!(o1 || o2)) 
      return true; // both null objects
    else if(!(o1 && o2)) 
      return false; // one null but the other is not
    if(o1['equals'] && typeof o1['equals'] === 'function')
      return o1['equals'].call(o1, o2); // allow equals() method
    var keys1 = Object.keys(o1);
    var keys2 = Object.keys(o2);
    if(!keys2.every(function (x) { return o1.hasOwnProperty(x); }))
      return false; // not all keys in o2 are in o1
    return keys1.every(function (x) {
      if(o2.hasOwnProperty(x)) { 
        return deepCompare(o1[x], o2[x]); // compare properties
      } else {
        return false; // not all keys in o1 are in o2
      }
    });
  } 
  return false; 
}