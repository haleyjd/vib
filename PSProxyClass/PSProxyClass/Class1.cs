using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;
using System.Collections.ObjectModel;
using System.Management.Automation;
using System.Web.Script.Serialization;

namespace PowerShellProxy
{
    public class PSProxyClass : IDisposable
    {
        private PowerShell ps;

        public PSProxyClass()
        {
            ps = PowerShell.Create();
        }

        public void Dispose()
        {
            ps.Dispose();
        }

        public void addArgument(Object obj)
        {
            ps.AddArgument(obj);
        }

        public void addCommand(String str)
        {
            ps.AddCommand(str);
        }

        public void addParameter(String str)
        {
            ps.AddParameter(str);
        }

        public void addParameterWithValue(String str, Object obj)
        {
            ps.AddParameter(str, obj);
        }

        public void addScript(String str)
        {
            ps.AddScript(str);
        }

        public String invoke()
        {
            Collection<PSObject> res = ps.Invoke();
            return PSSerializer.Serialize(res);
        }
    }
}
