#ifndef __FLV_PACKET_HPP__
#define __FLV_PACKET_HPP__

#include "pub.hpp"
#include <string>

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


class RtmpPacketSender: private RtmpStream
{
public:
        
        RtmpPacketSender(string &url);
        ~RtmpPacketSender();
        bool Connect();
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
};

#endif
