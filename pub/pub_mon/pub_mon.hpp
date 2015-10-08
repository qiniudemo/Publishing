#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdlib>

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
        ProcEntry(char *pCommand);
        void AddPolicy(int nExitCode, unsigned int nPolicyId);
        void AddPolicy(char *pPolicy);
        void Print();
        int Run();
        friend ostream& operator<< (ostream &os, const ProcEntry& entry);
private:
        // policy strings
        static Policy m_policy[];

        string m_command;
        vector< pair<int, unsigned int> > m_policyTable;
};

#define CONFIG_MAX_CHAR_EACH_LINE 256
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
        bool IsValidLine(char *pLine);
        static void MonitorThread(void *pParam);

        string m_confPath;
        vector<ProcEntry *> m_procTable;
};

