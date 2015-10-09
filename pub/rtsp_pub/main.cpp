//
// main entry
//

#include "rtsp_pub.hpp"
#include "config.hpp"
#include <cstring>
#include <iostream>
#include <string>

//
// def for option strings
//
static const char *KEYVAL_OPT_LIST[] =
{
        "-d"
};

static const char *SWITCH_OPT_LIST[] =
{
        "-e"
};

//
// global static variables
//
static string gProgName = "";
static bool gbFlagExitOnError = false;
static string gConfigPath = "";
static string gRtspUrl = "";
static string gRtmpUrl = "";

//
// forward declaration
//
static int do_action();
static int usage(const char *pProgName);
static bool array_find_str(const char **pArray, const char *pOption);
static bool test_kv(const char *_pOption);
static bool test_switch(const char *_pOption);
static void parse_kv(const char *_pOption, const char *_pValue);
static void parse_switch(const char *_pOption);


static int usage(const char *_pProgName)
{
        cout << "Usage: " << endl;
        cout << "  " << _pProgName << " <rtsp://xxxx> <rtmp://xxxx>" << endl;
        cout << "  " << _pProgName << " -d <path_to_config_file>" << endl;
        exit(1);
}

static bool array_find_str(const char **_pArray, const char *_pOption)
{
        int nLen = sizeof(_pArray) / sizeof(char *);
        for (int i = 0; i < nLen; i++) {
                if (strcmp(_pOption, _pArray[i]) == 0) {
                        return true;
                }
        }
        return false;
}

static bool test_kv(const char *_pOption)
{
        // url is not option but it is a valid param
        if (PubConfig::IsRtspUrl(_pOption)) {
                return true;
        }
        return array_find_str(KEYVAL_OPT_LIST, _pOption);
}

static bool test_switch(const char *_pOption)
{
        return array_find_str(SWITCH_OPT_LIST, _pOption);
}

static void parse_kv(const char *_pOption, const char *_pValue)
{
        if (PubConfig::IsRtspUrl(_pOption)) {
                if (PubConfig::IsRtmpUrl(_pValue)) {
                        gRtspUrl = _pOption;
                        gRtmpUrl = _pValue;
                } else {
                        cout << "RTMP url is not valid" << endl;
                }
        }

        if (strcmp(_pOption, "-d") == 0) {
                gConfigPath = _pValue;
        }
}

static void parse_switch(const char *_pOption)
{
        if (strcmp(_pOption, "-e")) {
                gbFlagExitOnError = true;
        }
}

int main(int argc, char** argv)
{
        gProgName = argv[0];
        if (argc >= 3) {
                for (int i = 1; i < argc; i++) {
                        char *pOption = argv[i];
                        char *pValue = nullptr;

                        if (test_kv(pOption)) {
                                i++;
                                pValue = argv[i];
                                parse_kv(pOption, pValue);
                        } else if (test_switch(pOption)) {
                                parse_switch(pOption);
                        } else {
                                cout << "Error: option " << pOption << " invalid" << endl;
                                usage(gProgName.c_str());
                        }
                }

                int nStatus = do_action();
                switch (nStatus) {
                case -1:
                        exit(1);
                        break;
                default:
                        exit(0);
                }
        } else {
                usage(gProgName.c_str());
        }

        return 0;
}

static int do_action()
{
        if (!gRtspUrl.empty() && !gRtmpUrl.empty()) {
                Rtsp2Rtmp streamManager;
                streamManager.Add("test", gRtspUrl.c_str(), gRtmpUrl.c_str(), gbFlagExitOnError);
                streamManager.Run();
        } else if (!gConfigPath.empty()) {
                PubConfig conf;
                int nCount = conf.LoadConfigFile(gConfigPath, gbFlagExitOnError);
                if (nCount <= 0) {
                      cout << "config not loaded, status: " << nCount << endl;
                      return -1;
                } else {
                      cout << "loaded " << nCount << " entries ..." << endl;
                }
                conf.Run();
        } else {
                usage(gProgName.c_str());
        }
        return 0;
}
