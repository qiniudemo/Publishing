#include "pub_mon.hpp"

//
// ProcEntry
//

Policy ProcEntry::m_policy[] = 
{ 
        { POLICY_RESPAWN, "respawn=" } 
};

ostream& operator<< (ostream &_os, const ProcEntry& _entry)
{
        _os << _entry.m_command;
        return _os;
}

ProcEntry::ProcEntry(char *_pCommand):
        m_command(_pCommand),
        m_policyTable()
{
        // remove possible 'newline' characters
        vector<char> charSet = {'\n', '\r'}; 
        for_each(charSet.begin(), charSet.end(), [this](char _chRep){
                string::size_type nPos = m_command.find(_chRep);
                if (nPos != string::npos) {
                        m_command.replace(nPos, 1, " ");
                }
        });
}

void ProcEntry::AddPolicy(int _nExitCode, unsigned int _nPolicyId)
{
        m_policyTable.push_back(make_pair(_nExitCode, _nPolicyId));
}

void ProcEntry::AddPolicy(char *_pPolicy)
{
        string line = _pPolicy;
        size_t nFoundPos;
        size_t nLineSize = line.size();
        unsigned int nPolicy = sizeof(m_policy) / sizeof(Policy);

        for (unsigned int nPolicyIndex = 0; nPolicyIndex < nPolicy; nPolicyIndex++) {
                nFoundPos = 0;
                const char* &pPolicyOption = m_policy[nPolicyIndex].pOptionName;
                const int &nPolicyType = m_policy[nPolicyIndex].nType;
                while (nFoundPos < nLineSize) {
                        // find policy string such as "respawn="
                        // find next space or tab character and get substring between '=' and '\t' or '\s'
                        nFoundPos = line.find(pPolicyOption, nFoundPos);
                        if (nFoundPos != string::npos) {
                                size_t nStart = nFoundPos + strlen(pPolicyOption);
                                size_t nEnd = line.find(';', nFoundPos);
                                nEnd = ((nEnd == string::npos) ? nLineSize - 1 : nEnd);
                                nFoundPos = nEnd + 1;

                                if (nStart == nEnd) {
                                        cout << "Warning: attribute is empty for " << m_policy[nPolicyIndex].pOptionName << endl;
                                        continue;
                                }

                                int nExitCode;
                                try {
                                        nExitCode = stoi(line.substr(nStart, nEnd - nStart));
                                } catch (exception &e) {
                                        cout << "Warning: exception caught: " << e.what() << endl;
                                        cout << "Warning: maybe invalid exit code, deprecated" << endl;
                                        continue;
                                }
                                
                                // get exit code
                                AddPolicy(nExitCode, nPolicyType);
                        } else {
                                break;
                        }
                }
        }
}

void ProcEntry::Print()
{
        cout << "command: " << m_command << endl;
        cout << "policy : " << endl;
        for_each(m_policyTable.begin(), m_policyTable.end(), [](const pair<int, unsigned int> policyPair) {
                cout << "  value:" << policyPair.first << " action:" << policyPair.second << endl;
        });
}

int ProcEntry::Run()
{
        int nRetVal = OSIAPI::RunCommand(m_command.c_str());
        cout << "Process [" << m_command << "] terminated with exit code = " << nRetVal << endl;
        return nRetVal;
}

//
// PubMonitor
//

PubMonitor::PubMonitor()
{
}

PubMonitor::~PubMonitor()
{
        // cleanup entries
        for (auto it = m_procTable.begin(); it != m_procTable.end(); it++) {
                delete *it;
        }
}

// load initial configuration file
int PubMonitor::LoadConfig(const char *_pPath)
{
        if (_pPath == nullptr || strlen(_pPath) == 0) {
                return 0;
        }

        m_confPath = _pPath;

        // open confiuration file
        FILE *fp = fopen(_pPath, "r");
        if (fp == nullptr) {
                cout << "Cannot open file :" << _pPath << endl;
                return -1;
        }
        fseek(fp, 0, SEEK_SET);

        char pBuffer[CONFIG_MAX_CHAR_EACH_LINE];
        unsigned int nLine = 0, nValidLine = 0;
        ProcEntry *pEntry = nullptr;

        memset(pBuffer, 0, CONFIG_MAX_CHAR_EACH_LINE);
        while (fgets(pBuffer, CONFIG_MAX_CHAR_EACH_LINE, fp)) {
                nLine++;

                if (!IsValidLine(pBuffer)) {
                        continue;
                }
                nValidLine++;

                // command line
                if (nValidLine % 2 != 0) {
                        pEntry = new ProcEntry(pBuffer);
                        cout << "Info: loading entry [" << *pEntry << "] ..." << endl;
                }
                // policy line
                if (nValidLine % 2 == 0) {
                        if (pEntry != nullptr) {
                                pEntry->AddPolicy(pBuffer);
                                AddEntry(pEntry);
                                cout << "Info: entry [" << *pEntry << "] was added" << endl;
                        }
                        pEntry = nullptr;
                }
        }
        return nValidLine / 2;
}

bool PubMonitor::IsValidLine(char *_pLine)
{
        int nLength = strlen(_pLine);

        if (nLength == 0) {
                return false;
        }

        // comment line
        if (_pLine[0] == '#') {
                return false;
        }

        // line with spaces and tabs only
        for (int i = 0; i < nLength; i++) {
                if (_pLine[i] != ' ' && _pLine[i] != '\t') {
                        return true;
                }
        }
        return false;
}

void PubMonitor::AddEntry(ProcEntry *_pEntry)
{
        m_procTable.push_back(_pEntry);
}

void PubMonitor::PrintTable()
{
        const char *pSplit = "----------";
        for_each(m_procTable.begin(), m_procTable.end(), [pSplit](ProcEntry *pEntry){
                cout << pSplit << endl;
                pEntry->Print();
        });
        cout << pSplit << endl;
}

void PubMonitor::Run()
{
        for (auto it = m_procTable.begin(); it != m_procTable.end(); it++) {
                OSIAPI::RunThread(PubMonitor::MonitorThread, *it);
        }
        OSIAPI::WaitForAllThreads();
}

void PubMonitor::MonitorThread(void *_pParam)
{
        while (true) {
                ProcEntry *pEntry = (ProcEntry *)_pParam;
                pEntry->Run();
                cout << "sleep 5 seconds" << endl;
                OSIAPI::MakeSleep(5);
        }
}
