//
// Extensions for JS Date object
//

// Due to not having ISO-8601 date format support in ECMAScript 3, this adds it
// as a static function to Date.
if(!Date.fromISO) {
  Object.defineProperty(Date, 'fromISO', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function fromISO(s) {
      var day, tz;
      var rx = /^(\d{4}\-\d\d\-\d\d([tT][\d:\.]*)?)([zZ]|([+\-])(\d\d):(\d\d))?$/;
      var p  = rx.exec(s) || [];
      
      if(p[1]) {
        day = p[1].split(/\D/);
        if(day.length >= 7) // fraction of a second must be between 0 and 999
          day[6] = day[6].substr(0, 3);
        for(var i = 0, L = day.length; i < L; i++) {
          day[i] = parseInt(day[i], 10) || 0;
        }
        day[1] -= 1;
        day = new Date(Date.UTC.apply(Date, day));
        if(!day.getDate())
          return NaN;
        if(p[5]) {
          tz = (parseInt(p[5], 10) * 60);
          if(p[6])
            tz += parseInt(p[6], 10);
          if(p[4] == '+')
            tz *= -1;
          if(tz)
            day.setUTCMinutes(day.getUTCMinutes() + tz);          
        }
        return day;
      }
      return NaN;
    }
  });
}

if(!Date.prototype.fromPstamp) {
  Object.defineProperty(Date.prototype, 'fromPstamp', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function fromPstamp(ps) {
      this.setMonth(ps.month - 1);
      this.setDate(ps.day);
      this.setFullYear(ps.year);
      this.setHours(ps.hour);
      this.setMinutes(ps.minute);
      this.setSeconds(ps.second);
      this.setMilliseconds(ps.millisecond);
    }
  });
}

if(!Date.prototype.fromPdate) {
  Object.defineProperty(Date.prototype, 'fromPdate', {
    enumerable:   false,
    configurable: true,
    writable:     true,
    value: function fromPdate(pd, pt) {
      this.setMonth(pd.month - 1);
      this.setDate(pd.day);
      this.setFullYear(pd.year);
      
      // Time argument is optional
      if(pt){
        this.setHours(pt.hour);
        this.setMinutes(pt.minute);
        this.setSeconds(pt.second);
        this.setMilliseconds(pt.millisecond);
      }
    }
  });
}


// EOF

