#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <fstream>
#include <Windows.h>
#include <tchar.h>

#include "ThreadSafeQueue.h"

struct SomeData
{
    ThreadSafeQueue* StartQueue;
    ThreadSafeQueue* SortedQueue;
};

void InsertIntoQueue(std::string fileName, ThreadSafeQueue& StartQueue)
{
    std::ifstream File(fileName);

    if (File.is_open())
    {
        std::string line;

        while (getline(File, line))
        {
            StartQueue.AddIntoQueue(line);
        }

        File.close();
    }
    else
    {
        std::cout << "File wasn't open" << std::endl;
    }
}

DWORD WINAPI SortQueue(LPVOID lpParam)
{
    SomeData* someData = static_cast<SomeData*>(lpParam);
    std::string line;

    while (!someData->StartQueue->RemoveFromQueue(line))
    {
        sort(line.begin(), line.end());
        someData->SortedQueue->AddIntoQueue(line);
    }

    delete someData;

    return 0;
}
const std::string TCHARToString(const TCHAR* tcharString)
{
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, tcharString, -1, nullptr, 0, nullptr, nullptr);
    std::string SortedQueue(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, tcharString, -1, &SortedQueue[0], sizeNeeded, nullptr, nullptr);
    return SortedQueue;
}

std::string GetFileName()
{
    TCHAR filename[MAX_PATH];

    OPENFILENAME ofn;
    ZeroMemory(&filename, sizeof(filename));
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = _T("Text Files\0*.txt\0Any File\0*.*\0");
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = _T("Select a file to sort");
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn))
    {
        std::cout << "You chose the file \"" << TCHARToString(filename) << "\"\n";
    }
    return TCHARToString(filename);
}

void SaveToFile(std::string fileName, ThreadSafeQueue StartQueue)
{
    std::cout << StartQueue.GetString();
    std::ofstream File(fileName);
    if (File.is_open())
    {
        File << StartQueue.GetString();
        File.close();
        std::cout << "Result was written to file" << std::endl;
    }
    else
    {
        std::cout << "Error while file writting" << std::endl;
    }
}

void CloseThreads(HANDLE* threads, int numberOfThreads)
{
    for (int i = 0; i < numberOfThreads; ++i)
    {
        if (threads != nullptr)
        {
            if (threads[i] != NULL)
            {
                CloseHandle(threads[i]);
            }
        }
    }
}

int main()
{
    int numberOfThreads;
    std::string fileName;
    LARGE_INTEGER start;
    LARGE_INTEGER end;
    LARGE_INTEGER frequency;
    ThreadSafeQueue StartQueue;
    ThreadSafeQueue SortedQueue;
    QueryPerformanceFrequency(&frequency);
    fileName = GetFileName();
    std::cout << "Enter number of threads" << std::endl;
    std::cin >> numberOfThreads;

    HANDLE* threads = new HANDLE[numberOfThreads];
    SomeData** SomeDataForThreads = new SomeData * [numberOfThreads];

    QueryPerformanceCounter(&start);
    double elapsedSeconds = 0;

    InsertIntoQueue(fileName, StartQueue);

    for (int i = 0; i < numberOfThreads; i++)
    {
        SomeDataForThreads[i] = new SomeData();
        SomeDataForThreads[i]->StartQueue = &StartQueue;
        SomeDataForThreads[i]->SortedQueue = &SortedQueue;
        threads[i] = CreateThread(NULL, 0, SortQueue, SomeDataForThreads[i], 0, NULL);
    }

    WaitForMultipleObjects(numberOfThreads, threads, TRUE, INFINITE);
    CloseThreads(threads, numberOfThreads);
    SaveToFile(fileName, SortedQueue);

    QueryPerformanceCounter(&end);
    elapsedSeconds = static_cast<double>(end.QuadPart - start.QuadPart) / frequency.QuadPart;
    std::cout << "It takes " << elapsedSeconds << " seconds" << std::endl;
    delete[] threads;
    delete[] SomeDataForThreads;

    return 0;
}