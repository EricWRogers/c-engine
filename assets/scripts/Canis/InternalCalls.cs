using System;
using System.Runtime.InteropServices;

namespace Canis
{
    public static class Debug
    {
        [DllImport("CanisEngine")]
        public static extern void Log(string msg);
    }
}

