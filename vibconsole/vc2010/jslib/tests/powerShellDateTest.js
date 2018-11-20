(new Pstamp()).fromDate(
  Date.fromISO(
    XMLDocument.FromString(
      (new PowerShell()).addCommand('Get-Date').invoke()
    ).evalXPathExpression(
      '/ms:Objs/ms:Obj/ms:LST/ms:Obj/ms:DT', 
      { 'ms': 'http://schemas.microsoft.com/powershell/2004/04'}
    ).getNodeAt(0).getContent()
  )
).dayOfWeek();