//
// rtsp2rtmp
//

// switches
//#define __PRINT_EACH_RTSP_NALU__
//#define __PRINT_CODEC_INFO__

#ifndef __RTSP_PUB_HPP__
#define __RTSP_PUB_HPP__

#include "pub.hpp"
#include "rtmp_packet.hpp"

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

#include <iostream>
#include <vector>

using namespace std;

// when RtspStream::bExitOnError is set, process will bail out with
// specified exit codes defined
#define RTSP_EXIT_NORMAL      0
#define RTSP_EXIT_USAGE       1
#define RTSP_EXIT_CONNECTION  2

//
// define a state class to current save stream info
//

class StreamClientState 
{
public:
        StreamClientState();
        virtual ~StreamClientState();
        friend ostream& operator<< (ostream &os, const MediaSubsession& subsession);
public:
        MediaSubsessionIterator* iter;
        MediaSession* session;
        MediaSubsession* subsession;
        TaskToken streamTimerTask;
        double duration;
};


//
// RtspStream
//

// forward declaration
class Rtsp2Rtmp;

#define RTSP_CLIENT_VERBOSITY_LEVEL 0
#define REQUEST_STREAMING_OVER_TCP False

class RtspStream: public RTSPClient 
{
public:
        static RtspStream* createNew(UsageEnvironment& env, const char *name, const char* rtspUrl, const char* rtmpUrl, 
                                     int verbosityLevel = 0, const char* applicationName = nullptr, 
                                     portNumBits tunnelOverHTTPPortNum = 0);
        void StartStreaming(void);
        void StopStreaming(void);
        friend ostream& operator<< (ostream &os, const RTSPClient& rtspClient);
        friend ostream& operator<< (ostream &os, const RtspStream& rtspStream);
protected:
        RtspStream(UsageEnvironment& env, const char *name, const char* rtspUrl, const char *rtmpUrl, int verbosityLevel, const char* applicationName, 
                   portNumBits tunnelOverHTTPPortNum);
        virtual ~RtspStream(void);
private:
        static void OnAfterDescribe(RTSPClient* pRtspClient, int nResultCode, char* pResultString);
        static void ShutdownStream(RTSPClient* pRtspClient, int nExitCode = 1);
        static void SetupNextSubsession(RTSPClient* pRtspClient);
        static void OnAfterSetup(RTSPClient* pRtspClient, int nResultCode, char* pResultString);
        static void OnAfterPlay(RTSPClient* pRtspClient, int nResultCode, char* pResultString);
        static void OnSubsessionAfterPlaying(void* pClientData);
        static void OnSubsessionByeHandler(void* pClientData);
        static void StreamTimerHandler(void* pClientData);
public:
        string name;
        string rtmpUrl;
        string rtspUrl;
        StreamClientState scs;
        RtmpPacketSender *pRtmpSender;
        bool bExitOnError;
        Rtsp2Rtmp *pMgmtPtr;
};


//
// RtspSink
//

#define RTSP_SINK_RECEIVE_BUFFER_SIZE 1000000

class RtspSink: public MediaSink 
{
public:
        static RtspSink* createNew(UsageEnvironment& env, MediaSubsession& subsession, const char* streamId = nullptr);
private:
        RtspSink(UsageEnvironment& env, MediaSubsession& subsession, const char* streamId);
        virtual ~RtspSink();
        static void OnGettingFrame(void* pClientData, unsigned int nFrameSize, unsigned int nTruncatedBytes,
                                   struct timeval presentationTime, unsigned int durationInMicroseconds);
        void GetNextH264Frame(unsigned int nFrameSize, unsigned int nTruncatedBytes,
                              struct timeval presentationTime, unsigned int durationInMicroseconds);
        void GetNextPcmaFrame(unsigned int nFrameSize, unsigned int nTruncatedBytes,
                              struct timeval presentationTime, unsigned int durationInMicroseconds);
        void GetNextPcmuFrame(unsigned int nFrameSize, unsigned int nTruncatedBytes,
                              struct timeval presentationTime, unsigned int durationInMicroseconds);
private:
        virtual Boolean continuePlaying();
private:
        u_int8_t* m_pReceiveBuffer;
        MediaSubsession& m_subsession;
        string m_streamId;
        bool m_bIsConfigSent;
};


//
// RTSP/RTMP stream manager
//

class Rtsp2Rtmp
{
public:
        Rtsp2Rtmp(void);
        ~Rtsp2Rtmp(void);
        void Add(const char *pName, const char *pRtspUrl, const char *pRtmpUrl, bool bExitOnError = false);
        void Run();
        void Stop();
private:
        void CreateStreamPair(const char *pName, const char *pRtspUrl, const char *pRtmpUrl, bool bExitOnError = false);
        void StartStreaming();
        void StartEventLoop();
        void StopStreaming();
private:
        vector<RtspStream*> m_streams;
        TaskScheduler* m_pScheduler;
        UsageEnvironment* m_pEnv;
        char m_chEventLoopControl;
};

#endif
