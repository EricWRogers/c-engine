using System;
using System.Runtime.InteropServices;

public class MyScript
{
    [DllImport("CanisEngine")]
    public static extern void Log(string msg);

    public static void Start()
    {
        Log("This is mine now Unity!");
    }

    public static void Update()
    {
        Log("Update CanisEngine!");
    }
}
