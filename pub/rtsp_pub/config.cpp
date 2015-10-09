#include "config.hpp"
#include <cstdio>

PubConfig::PubConfig()
{
        m_filePath = "";
        m_pStreamManager = new Rtsp2Rtmp;
        m_bExitOnError = false;
}

PubConfig::~PubConfig()
{
        delete m_pStreamManager;
}

int PubConfig::LoadConfigFile(const string &_path, bool _bExitOnError)
{
        // sanity check
        if (_path.empty() == true) {
                return -5;
        }

        // open confiuration file
        FILE *fp = fopen(_path.c_str(), "r");
        if (fp == nullptr) {
                cout << "Cannot open file :" << _path << endl;
                return -1;
        }
        fseek(fp, 0, SEEK_SET);

        // read each line and create stream configurations
        char pBuffer[CONFIG_MAX_CHAR_EACH_LINE];
        const char *pName, *pSrc, *pDst;
        unsigned int nLineNo = 0, nLineLoaded = 0, nLength;
        bool bStatus;
        memset(pBuffer, 0, CONFIG_MAX_CHAR_EACH_LINE);
        while (fgets(pBuffer, CONFIG_MAX_CHAR_EACH_LINE, fp)) {
                // next line
                nLineNo++;

                // do not accept one line exceeding given max length
                nLength = strlen(pBuffer);
                if (nLength >= CONFIG_MAX_CHAR_EACH_LINE) {
                        cout << "Line " << nLineNo << " is too long" << endl;
                        continue;
                }

                // commented out
                if (pBuffer[0] == '#') {
                        continue;
                }

                // no new line
                if (pBuffer[nLength - 1] == '\n') {
                        pBuffer[nLength - 1] = 0;
                }

                // get name/rtsp/rtmp
                bStatus = ParseOneLine(pBuffer, &pName, &pSrc, &pDst);
                if (bStatus == true) {
                        nLineLoaded++;
                        m_pStreamManager->Add(pName, pSrc, pDst, _bExitOnError);
                } else {
                        cout << "Error: line " << nLineNo << " invalid entry: [" << pBuffer << "]" << endl;
                }
        }
        fclose(fp);
        return nLineLoaded;
}

bool PubConfig::ParseOneLine(char *_pBuffer, const char **_pName, const char **_pSrc, const char **_pDst)
{
        bool bFound = true;
        unsigned int nParamIndex = 0;
        for(char *pCursor = _pBuffer; *pCursor != 0; pCursor++) {
                if (*pCursor == ' ' || *pCursor == '\t') {
                        *pCursor = 0;
                        bFound = true;
                        continue;
                }
                if (bFound) {
                        bFound = false;
                        switch (nParamIndex) {
                        case 0:
                                *_pName = pCursor;
                                break;
                        case 1:
                                *_pSrc = pCursor;
                                break;
                        case 2:
                                *_pDst = pCursor;
                                break;
                        default:
                                break;
                        }
                        nParamIndex++;
                        if (nParamIndex == 4) {
                                break;
                        }
                }
        }
        if (nParamIndex != 3) {
                return false;
        }
        if (IsRtspUrl(*_pSrc) == false || IsRtmpUrl(*_pDst) == false) {
                return false;
        }
        return true;
}

bool PubConfig::IsRtspUrl(const char *_pUrl)
{
        return (strncmp(_pUrl, "rtsp://", 7) == 0);
}

bool PubConfig::IsRtmpUrl(const char *_pUrl)
{
        return (strncmp(_pUrl, "rtmp://", 7) == 0);
}

void PubConfig::Run()
{
        m_pStreamManager->Run();
}

