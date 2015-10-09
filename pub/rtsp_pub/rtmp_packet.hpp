#ifndef __FLV_PACKET_HPP__
#define __FLV_PACKET_HPP__

// switches
//#define __STD_THREAD_SUPPORT__
//#define __PRINT_QUEUE_STATE__ // no, peformance issue

#include "pub.hpp"
#include "queue.hpp"
#include <string>
#include <vector>

#ifdef __STD_THREAD_SUPPORT__
#include <thread>
#include <atomic>
#endif

using namespace std;

#define SPS_BUFFER_SIZE 1024
#define PPS_BUFFER_SIZE 1024
#define SEI_BUFFER_SIZE 1024

typedef enum _RtmpPacketSenderStatusType
{
        UNINITIALIZED,
        INITIALIZED,
        DISCONNECTED,
        CONNECTING,
        CONNECTION_ERROR,
        RETRYING,
        CONNECTED,
        RESERVED
} RtmpPacketSenderStatusType;

#ifdef __STD_THREAD_SUPPORT__
class RtmpPacket
{
public:
        RtmpPacket(const char *pData, unsigned int nLength, unsigned int nTimestamp);
        char* GetBuffer();
        unsigned int GetSize();
        unsigned int GetTimestamp();
private:
        vector<char> m_buffer;
        unsigned int m_nTimestamp;
};
#endif

class RtmpPacketSender: private RtmpStream
{
public:
        
        RtmpPacketSender(string &url);
        ~RtmpPacketSender();
        bool Connect();
        bool Connect(bool bIsAsync);
        void Close();
        void Sps(const char *pData, unsigned int nLength);
        void Pps(const char *pData, unsigned int nLength);
        void Sei(const char *pData, unsigned int nLength);
        bool SendIdrOnly(const char *pData, unsigned int nLength);
        bool SendIdrAll(const char *pData, unsigned int nLength);
        bool SendNonIdr(const char *pData, unsigned int nLength);
        bool SendConfig();
private:
        void ResetSps();
        void ResetPps();
        void ResetSei();
        unsigned int WriteNalDataToBuffer(char *pBuffer, const char *pData, unsigned int nLength);
        unsigned int WriteNalUnitToBuffer(char *pBuffer, NalUnit *pNalu);
        void SetStatus(RtmpPacketSenderStatusType nStatus);
        RtmpPacketSenderStatusType Status();

#ifdef __STD_THREAD_SUPPORT__
        // for async use
        void SenderLoop();
        void StopAllThreads();
#endif

        // copies of keyframe units
        NalUnit m_sps;
        NalUnit m_pps;
        NalUnit m_sei;

        char m_spsData[SPS_BUFFER_SIZE];
        char m_ppsData[PPS_BUFFER_SIZE];
        char m_seiData[SEI_BUFFER_SIZE];

        unsigned int m_nStatus;
        unsigned int m_nTimestamp;
        unsigned int m_nFps;
        string m_publishUrl;

#ifdef __STD_THREAD_SUPPORT__
        // for async use
        PacketQueue<RtmpPacket> m_queue;
        bool m_bIsAsync;
        vector<thread> m_threads;
        atomic<bool> m_bExitNow;
#endif
};

#endif
