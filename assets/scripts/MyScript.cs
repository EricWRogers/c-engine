using CanisEngine;
using System;
using System.Collections.Generic;
using System.Globalization;

public class MyScript
{
    private int m_counter = 0;
    private List<int> m_numbers = new List<int>();

    MyScript()
    {
        Logger.Log("Constructor");
    }

    public void Start()
    {
        for (int i = 0; i < 1000; i++)
            m_numbers.Add(i);

        Logger.Log("This is mine now Unity!" + m_counter++);
        Logger.Warning("This is mine now Unity!" + m_counter++);
        Logger.Error("This is mine now Unity!" + m_counter++);
        Logger.Log("m_numbers.Count: " + m_numbers.Count);
    }

    private float elapsed = 0.0f;

    public void Update(float _deltaTime)
    {
        elapsed += _deltaTime;

        if (elapsed > 1.0f) // once every second
        {
            Logger.Log("1 second passed!");
            elapsed = 0.0f;
        }

        String temp = _deltaTime.ToString();

        float color = elapsed / 10.0f;
        CanisWindow.SetBackgroundColor(color, 0.0f, 0.0f, 1.0f);
    }

        public void OnDestroy()
        {
            Logger.Log("Destroyed");
        }
    }
