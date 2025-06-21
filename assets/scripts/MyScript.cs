using Canis;

public class MyScript
{
    public static void Start()
    {
        Debug.Log("This is mine now Unity!");
        Debug.Warning("This is mine now Unity!");
        Debug.Error("This is mine now Unity!");
    }

    public static void Update()
    {
        Debug.FatalError("Update CanisEngine!");
    }
}
