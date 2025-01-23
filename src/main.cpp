#include <vector>
#include <stdio.h>
#include <iostream>

#include "core/String.hpp"

struct Task {
    bool complete;
    String name;
};

int main()
{
    int* nums = nullptr;
    List::Init(&nums, 1, sizeof(int));
    for(int i = 0; i < 100; i++)
        List::Add(&nums, &i);

    String message("Hello, World!");
    String p = ".";

    printf("Before assignment\n");
    p = message;
    printf("After assignment\n");

    printf("Message: %s\n", message.CString());
    printf("P: %s\n", p.CString());

    message = message + " From Eric" + p;

    printf("h\n");

    printf("Message: %s\n", message.CString());

    printf("Start\n");

    Task* tasks = nullptr;
    List::Init(&tasks, 1, sizeof(Task));

    for (int i = 0; i < 10; i++)
    {
        
        Task t = {};
        t.complete = false;
        t.name = "task name";

        List::Add(&tasks, &t);
    }

    for (int i = 0; i < List::GetCount(&tasks); i++)
    {
        Task* task = &tasks[i];
        printf("%s\n", task->name.CString());
    }

    printf("End\n");

    return 0;
}
