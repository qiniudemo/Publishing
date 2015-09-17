//
// main entry
//

#include "rtsp_pub.hpp"
#include "config.hpp"
#include <iostream>
#include <string>

int usage(const char *pProgName)
{
        cout << "Usage: " << endl;
        cout << "  " << pProgName << " <rtsp://xxxx> <rtmp://xxxx>" << endl;
        cout << "  " << pProgName << " -d <path_to_config_file>" << endl;
        return -1;
}

int main(int argc, char** argv)
{
        if (argc == 3) {
                string option = argv[1];
                string value = argv[2];

                if (PubConfig::IsRtspUrl(argv[1]) && PubConfig::IsRtmpUrl(argv[2])) {
                        Rtsp2Rtmp streamManager;
                        streamManager.Add("test", argv[1], argv[2]);
                        streamManager.Run();
                } else if (option.compare(0, 2, "-d") == 0) {
                        PubConfig conf;
                        int nCount = conf.LoadConfigFile(value);
                        if (nCount <= 0) {
                                cout << "config not loaded, status: " << nCount << endl;
                                return -1;
                        } else {
                                cout << "loaded " << nCount << " entries ..." << endl;
                        }
                        conf.Run();
                } else {
                        usage(argv[0]);
                }
        } else {
                usage(argv[0]);
        }
        return -1;
}

