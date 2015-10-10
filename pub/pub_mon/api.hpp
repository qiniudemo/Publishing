#ifndef __API_HPP__
#define __API_HPP__

#if defined(__WIN32__) || defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <iostream>
#include <vector>
#include <cstdlib>

using namespace std;

class OSIAPI
{
public:
        OSIAPI() = delete;
        static int RunCommand(const char *pCommand);
        static void MakeSleep(unsigned int nSeconds);

        // threads
public:
        static int RunThread(void (*pFunction)(void *), void *nParameter);
        static int WaitForAllThreads();
private:
#if defined(__WIN32__) || defined(_WIN32)
        static vector<HANDLE> m_threads;
#else
        static vector<int> m_threads;
#endif
};

#endif
