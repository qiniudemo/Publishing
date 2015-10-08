#include "api.hpp"

#if defined(__WIN32__) || defined(_WIN32)
        vector<HANDLE> OSIAPI::m_threads;
#else
        vector<int> OSIAPI::m_threads;
#endif


int OSIAPI::RunCommand(const char *_pCommand)
{
        int nRetVal = system(_pCommand);
#if !defined(__WIN32__) && !defined(_WIN32)
        nRetVal >>= 8;
#endif
        return nRetVal;
}

#if defined(__WIN32__) || defined(_WIN32)
int OSIAPI::RunThread(void (*_pFunction)(void*), void *_pParameter)
{
        HANDLE hThread;
        DWORD dwThreadId;

        hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)_pFunction, _pParameter, 0, &dwThreadId);
        OSIAPI::m_threads.push_back(hThread);
        return 0;
}

int OSIAPI::WaitForAllThreads()
{
        // wait until all threads have terminated
        WaitForMultipleObjects(OSIAPI::m_threads.size(), OSIAPI::m_threads.data(), TRUE, INFINITE);
        for (auto it = OSIAPI::m_threads.begin(); it != OSIAPI::m_threads.end(); it++) {
                CloseHandle(*it);
                OSIAPI::m_threads.erase(it);
        }
        return 0;
}
#else
int OSIAPI::RunThread(void (*_pFunction)(void*), void *_pParameter)
{
        return 0;
}

int OSIAPI::WaitForAllThreads()
{
        return 0;
}
#endif

void OSIAPI::MakeSleep(unsigned int nSeconds)
{
#if defined(__WIN32__) || defined(_WIN32)
        Sleep(nSeconds * 1000);
#else
        sleep(nSeconds);
#endif
}
