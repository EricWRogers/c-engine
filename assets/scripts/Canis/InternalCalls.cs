using System;
using System.Runtime.InteropServices;

namespace Canis
{
    public static class Debug
    {
		[DllImport("CanisEngine", EntryPoint = "CSharpLayer_FatalError", CallingConvention = CallingConvention.Cdecl)]
        public static extern void FatalError(string _message);
		[DllImport("CanisEngine", EntryPoint = "CSharpLayer_Error", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Error(string _message);
		[DllImport("CanisEngine", EntryPoint = "CSharpLayer_Warning", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Warning(string _message);
        [DllImport("CanisEngine", EntryPoint = "CSharpLayer_Log", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Log(string _message);
    }

	public static class Window
    {
		[DllImport("CanisEngine", EntryPoint = "CSharpLayer_SetTitle", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetTitle(string _message);
		//[DllImport("CanisEngine", EntryPoint = "CSharpLayer_SetWindowSize", CallingConvention = CallingConvention.Cdecl)]
        //public static extern void SetSize(int _width, int _height);
    }
}

