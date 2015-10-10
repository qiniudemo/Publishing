#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cstring>

#include "api.hpp"

using namespace std;

typedef enum _PType {
        POLICY_RESPAWN
} PType;

typedef struct _Policy
{
        PType nType;
        const char *pOptionName;
} Policy;

class ProcEntry
{
public:
        ProcEntry(const char *pCommand);
        void AddPolicy(int nExitCode, unsigned int nPolicyId);
        void AddPolicy(const char *pPolicy);
        void Print();
        int Run();
        friend ostream& operator<< (ostream &os, const ProcEntry& entry);
private:
        // policy strings
        static Policy m_policy[];

        string m_command;
        vector< pair<int, unsigned int> > m_policyTable;
};

class PubMonitor
{
public:
        PubMonitor();
        ~PubMonitor();
        int LoadConfig(const char *pPath);
        void Run();
        void PrintTable();
private:
        void AddEntry(ProcEntry *pEntry);
        bool IsValidLine(const char *pLine);
        static void MonitorThread(void *pParam);

        string m_confPath;
        vector<ProcEntry *> m_procTable;
};

