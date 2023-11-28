#pragma once

#include <queue>
#include <string>
#include <Windows.h>

class ThreadSafeQueue
{
private:
    CRITICAL_SECTION crtSection;
    std::queue<std::string> Queue;
public:
    ThreadSafeQueue()
    {
        InitializeCriticalSection(&crtSection);
    }

    ~ThreadSafeQueue()
    {
        DeleteCriticalSection(&crtSection);
    }

    void AddIntoQueue(std::string& value)
    {
        EnterCriticalSection(&crtSection);
        Queue.push(value);
        LeaveCriticalSection(&crtSection);
    }

    bool RemoveFromQueue(std::string& value)
    {
        bool isEmpty = true;

        EnterCriticalSection(&crtSection);
        if (!Queue.empty())
        {
            value = Queue.front();
            Queue.pop();
            isEmpty = false;
        }
        LeaveCriticalSection(&crtSection);
        return isEmpty;
    }

    int getCount()
    {
        return Queue.size();
    }

    bool IsEmpty()
    {
        return Queue.empty();
    }

    std::string GetString()
    {
        std::string result;
        std::queue<std::string> tempQueue = Queue;

        while (!tempQueue.empty())
        {
            result.append(tempQueue.front() + "\n");
            tempQueue.pop();
        }

        return result;
    }


};