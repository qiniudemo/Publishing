//
// main entry
//

#include "rtsp_pub.hpp"
#include <iostream>

int main(int argc, char** argv)
{
	if (argc != 4) {
		cout << "Usage: " << endl;
		cout << "  " << argv[0] << " <name> <rtsp://xxxx> <rtmp://xxxx>" << endl;
		cout << "  " << argv[0] << " -d <path_to_config_file>" << endl;
		return 1;
	}
        Rtsp2Rtmp streamManager;
	streamManager.CreateStreamPair(argv[1], argv[2], argv[3]);
        return 0;
}

