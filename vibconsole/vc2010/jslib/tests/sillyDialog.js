(function () {
   var mb  = Win.MB;
   var res = Win.MessageBox("Are you there?", "JS Dialog", 
                            mb.YESNO|mb.ICONQUESTION); 
   if(res == Win.ID.YES) 
      Console.println("You're there!"); 
   else 
      Console.println("You're not there?!");
})();