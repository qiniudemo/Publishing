//
// main entry
//

#include "rtsp_pub.hpp"

int main(int argc, char** argv)
{
        Rtsp2Rtmp streamManager;
        streamManager.CreateStreamPair("v1", "rtsp://10.10.10.103/av0_0", "rtmp://vcww0t.publish.z1.pili.qiniup.com/linqibin/test?key=ddf7b516");
        return 0;
}

