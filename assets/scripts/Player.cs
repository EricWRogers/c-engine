using Canis;
using System;
using System.Collections.Generic;

public class Player
{
    private int m_counter = 0;
    private List<int> m_numbers = new List<int>();

    public void Start()
    {
        for (int i = 0; i < 1000; i++)
            m_numbers.Add(i);
        
        Debug.Log("This is mine now Unity!" + m_counter++);
        Debug.Warning("This is mine now Unity!" + m_counter++);
        Debug.Error("This is mine now Unity!" + m_counter++);
        Debug.Log("m_numbers.Count: " + m_numbers.Count);
    }

    public void Update(float _deltaTime)
    {
        Window.SetTitle("Set From C#");
        
        Debug.Log("Update CanisEngine!" + m_counter++ + " " + _deltaTime);

        Window.SetBackgroundColor((float)m_counter/10000.0f,0.0f,0.0f,1.0f);
    }

    public void OnDestroy()
    {
        Debug.Log("Destroyed");
    }
}
