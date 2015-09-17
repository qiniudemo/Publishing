#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include "rtsp_pub.hpp"
#include <string>

using namespace std;

#define CONFIG_MAX_CHAR_EACH_LINE 1024

class PubConfig
{
public:
        PubConfig();
        ~PubConfig();
        int LoadConfigFile(const string &path);
        static bool IsRtspUrl(const char *_pUrl);
        static bool IsRtmpUrl(const char *_pUrl);
        void Run();
private:
        bool ParseOneLine(char *pBuffer, const char **pName, const char **pSrc, const char **pDst);
        string m_filePath;
        Rtsp2Rtmp *m_pStreamManager;
};

#endif

