dialogLoadTest = function () {
  var dlg = Win.CreateDialog("s:/traymenu-203dev/release/traymenu.exe", "RUNDIALOG",
    function (uMsg, wParam, lParam) {
      switch(uMsg) 
      {
      case Win.WM.COMMAND:
        switch(wParam) 
        {
        case Win.ID.OK:
        case Win.ID.CANCEL:
          this.destroy();
          return true;
        default:
          break;
        }
        break;
      default: 
        break;
      }
      return false;
    });
  
  while(dlg.valid()) {
    Win.PumpEvents();
  }  
};