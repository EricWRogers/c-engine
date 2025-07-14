using CanisEngine;
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
        
        Logger.Log("This is mine now Unity!" + m_counter++);
        Logger.Warning("This is mine now Unity!" + m_counter++);
        Logger.Error("This is mine now Unity!" + m_counter++);
        Logger.Log("m_numbers.Count: " + m_numbers.Count);
    }

    public void Update(float _deltaTime)
    {
        CanisWindow.SetTitle("Set From C#");

        Logger.Log("Player Update CanisEngine!" + m_counter++);

        CanisWindow.SetBackgroundColor((float)m_counter / 10000.0f, 0.0f, 0.0f, 1.0f);
    }

    public void OnDestroy()
    {
        Logger.Log("Destroyed");
    }
}
