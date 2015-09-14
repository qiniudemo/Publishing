//
// pub.hpp
//

#ifndef __PUB_HPP__
#define __PUB_HPP__

#include "rtmp.h"
#include "rtmp_sys.h"
#include "amf.h"

#include <cstring>

#define FILE_BUFFER_SIZE (1024 * 1024 * 15)
#define FLV_CODECID_H264 7

typedef struct _NalUnit
{
        int type;
        int size;
        char *data;
} NalUnit;

typedef struct _RtmpMetadata
{
        unsigned int nWidth;
        unsigned int nHeight;

        // define fps and bps
        unsigned int nFrameRate;
        unsigned int nVideoDataRate;

        unsigned int nSpsLen;
        char sps[1024];
        unsigned int nPpsLen;
        char pps[1024];

        // audio
        bool bHasAudio;
        unsigned int nAudioSampleRate;
        unsigned int nAudioSampleSize;
        unsigned int nAudioChannels;
        char *pAudioSpecCfg;
        unsigned int nAudioSpecCfgLen;
} RtmpMetadata;

class RtmpStream
{
public:
        RtmpStream(void);
        ~RtmpStream(void);
        // TODO define as virtual functions
        bool Connect(const char *url);
        void Close();
        bool SendH264File(const char *pFileName);

protected:
        bool SendMetadata(RtmpMetadata * pMeta);
        bool SendH264Packet(const char *data, unsigned int size, bool bIsKeyFrame, unsigned int nTimeStamp);
        bool ReadOneNaluFromBuf(NalUnit & nalu);
        bool GetNextNalUnit(unsigned int nStart, unsigned int &nDelimiter, unsigned int &nNalu);
        int SendPacket(unsigned int nPacketType, const char *data, unsigned int size, unsigned int nTimestamp);
        void PrintNalUnit(const NalUnit *pNalu);
        RTMP *m_pRtmp;
        char *m_pFileBuf;
        unsigned int m_nFileBufSize;
        unsigned int m_nCurPos;
        unsigned int m_nFrameRate;
};

static inline char * put_byte(char *output, uint8_t nVal)
{
        output[0] = nVal;
        return output + 1;
}

static inline char * put_be16(char *output, uint16_t nVal)
{
        output[1] = nVal & 0xff;
        output[0] = nVal >> 8;
        return output + 2;
}

static inline char * put_be24(char *output,uint32_t nVal)
{
        output[2] = nVal & 0xff;
        output[1] = nVal >> 8;
        output[0] = nVal >> 16;
        return output + 3;
}

static inline char * put_be32(char *output, uint32_t nVal)
{
        output[3] = nVal & 0xff;
        output[2] = nVal >> 8;
        output[1] = nVal >> 16;
        output[0] = nVal >> 24;
        return output + 4;
}

static inline char *  put_be64( char *output, uint64_t nVal )
{
        output = put_be32(output, nVal >> 32);
        output = put_be32(output, nVal );
        return output;
}

static inline char * put_amf_string( char *c, const char *str )
{
        uint16_t len = strlen(str);
        c = put_be16(c, len);
        memcpy(c, str, len);
        return c + len;
}

static inline char * put_amf_double(char *c, double d)
{ 
        *c++ = AMF_NUMBER;  /* type: Number */
        { 
                char *ci, *co;
                ci = (char *)&d;
                co = (char *)c;
                co[0] = ci[7];
                co[1] = ci[6];
                co[2] = ci[5];
                co[3] = ci[4];
                co[4] = ci[3];
                co[5] = ci[2];
                co[6] = ci[1];
                co[7] = ci[0];
        } 
        return c + 8;
}

#endif
