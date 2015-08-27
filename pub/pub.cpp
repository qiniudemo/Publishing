//
//  pub.cpp - RTMP publish sample
//

#include <cstring>
#include <iostream>
#include <cstdlib>

// librtmp
#include "rtmp.h"
#include "rtmp_sys.h"
#include "amf.h"

#include "pub.hpp"
#include "x264.hpp"

using namespace std;

//----------------------------------

// pili publish

RtmpStream::RtmpStream(const char *_piliUser):
	m_piliCtx(nullptr),
	m_piliTick(0)
{
	// once-read file buffer
        m_pFileBuf = new char[FILE_BUFFER_SIZE];
        memset(m_pFileBuf, 0, FILE_BUFFER_SIZE);

	cout << "[debug] User " << _piliUser << " puts" << endl;
}

bool RtmpStream::PiliConnect(const char *_url, pili_stream_state_cb _callback)
{
	int nStatus;

        m_piliCtx = pili_create_stream_context();
	m_piliTick = 0;
        pili_init_stream_context(m_piliCtx, PILI_STREAM_DROP_FRAME_POLICY_RANDOM, PILI_STREAM_BUFFER_TIME_INTERVAL_DEFAULT,
                                 _callback);
	nStatus = pili_stream_push_open(m_piliCtx, _url);
	if (nStatus != 0) {
		cout << "pili_stream_push_open: nStatus=" << nStatus << endl;
		return false;
	}
	cout << "Connected !" << endl;
	return true;
}

void RtmpStream::PiliDisconnect()
{
	pili_stream_push_close(m_piliCtx);
	pili_release_stream_context(m_piliCtx);
}

bool RtmpStream::PiliSendH264File(const char *_pFileName)
{
        if (_pFileName == NULL) {
                return false;
        }
        FILE *fp = fopen(_pFileName, "rb");
        if (!fp) {
                cout << "Error: could not open file" << endl;
                return false;
        }
        fseek(fp, 0, SEEK_SET);
        m_nFileBufSize = fread(m_pFileBuf, sizeof(char), FILE_BUFFER_SIZE, fp);
        if (m_nFileBufSize >= FILE_BUFFER_SIZE) {
                cout << "Warning: file is truncated" << endl;
        }
        fclose(fp);

	// initialize PILI last keyframe
	m_piliKeyframe.sps.length = -1;
	m_piliKeyframe.pps.length = -1;
	m_piliKeyframe.sei.length = -1;
	m_piliKeyframe.idr.length = -1;

        NalUnit nalu;
        while (ReadOneNaluFromBuf(nalu)) {
		switch (nalu.type) {
		case 0x01:
			PiliPutSlice(&nalu);
			break;
		case 0x05:
			PiliPutIdr(&nalu);
			break;
		case 0x06:
			PiliPutSei(&nalu);
		case 0x07:
			PiliPutSps(&nalu);
			break;
		case 0x08:
			PiliPutPps(&nalu);
			break;
		default:
			break;
		}
        }

        return true;
}

void RtmpStream::PiliPutSps(NalUnit *_pNalu)
{
	//int width, height;
        //Util264::h264_decode_sps(_pNalu.data, _pNalu.size, width, height);
        m_piliFps = 30;

	m_piliKeyframe.sps.data = (uint8_t *)_pNalu->data;
	m_piliKeyframe.sps.length = _pNalu->size;
}

void RtmpStream::PiliPutPps(NalUnit *_pNalu)
{
	m_piliKeyframe.pps.data = (uint8_t *)_pNalu->data;
	m_piliKeyframe.pps.length = _pNalu->size;
}

void RtmpStream::PiliPutIdr(NalUnit *_pNalu)
{
	m_piliKeyframe.idr.data = (uint8_t *)_pNalu->data;
	m_piliKeyframe.idr.length = _pNalu->size;

	pili_write_h264_key_frame(m_piliCtx, m_piliKeyframe, m_piliTick);

	PiliTicks();
}

void RtmpStream::PiliPutSei(NalUnit *_pNalu)
{
	m_piliKeyframe.sei.data = (uint8_t *)_pNalu->data;
	m_piliKeyframe.sei.length = _pNalu->size;
}

void RtmpStream::PiliPutSlice(NalUnit *_pNalu)
{
	// have to convert to pili stype ~~
	pili_h264_nalu_t nalu;
	nalu.data = (uint8_t *)_pNalu->data;
	nalu.length = _pNalu->size;

	pili_write_h264_slice(m_piliCtx, nalu, m_piliTick);

	PiliTicks();
}

void RtmpStream::PiliTicks()
{
	unsigned int fps = 1000 / m_piliFps;
	m_piliTick += fps;
	msleep(fps);
}

//----------------------------------

RtmpStream::RtmpStream(void):
        m_pRtmp(nullptr),
        m_nFileBufSize(0),
        m_nCurPos(0)
{
        m_pFileBuf = new char[FILE_BUFFER_SIZE];
        memset(m_pFileBuf, 0, FILE_BUFFER_SIZE);
        m_pRtmp = RTMP_Alloc();
        RTMP_Init(m_pRtmp);
}

RtmpStream::~RtmpStream(void)
{
        Close();
        delete[] m_pFileBuf;
}

bool RtmpStream::Connect(const char *_url)
{
        if (RTMP_SetupURL(m_pRtmp, (char *)_url) < 0) {
                return false;
        }
        RTMP_EnableWrite(m_pRtmp);
        if (RTMP_Connect(m_pRtmp, nullptr) < 0) {
                return false;
        }
        if (RTMP_ConnectStream(m_pRtmp, 0) < 0) {
                return false;
        }
        return true;
}

void RtmpStream::Close()
{
        if (m_pRtmp) {
                RTMP_Close(m_pRtmp);
                RTMP_Free(m_pRtmp);
                m_pRtmp = NULL;
        }
}

int RtmpStream::SendPacket(unsigned int _nPacketType, char *_data, unsigned int _size, unsigned int _nTimestamp)
{
        if (m_pRtmp == nullptr) {
                return 0;
        }

        RTMPPacket packet;
        RTMPPacket_Reset(&packet);
        RTMPPacket_Alloc(&packet, _size);

        packet.m_packetType = _nPacketType;
        packet.m_nChannel = 0x04;
        packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
        packet.m_nTimeStamp = _nTimestamp;
        packet.m_nInfoField2 = m_pRtmp->m_stream_id;
        packet.m_nBodySize = _size;
        memcpy(packet.m_body, _data, _size);

        int nRet = RTMP_SendPacket(m_pRtmp, &packet, 0);
        RTMPPacket_Free(&packet);
        return nRet;
}

bool RtmpStream::SendMetadata(RtmpMetadata *_pMeta)
{
        if (_pMeta == nullptr) {
                return false;
        }
        
        char body[1024] = {0};
        char *p = (char *)body;
        
        p = put_byte(p, AMF_STRING);
        p = put_amf_string(p, "@setDataFrame");

        p = put_byte(p, AMF_STRING);
        p = put_amf_string(p, "onMetaData");
        
        p = put_byte(p, AMF_OBJECT);
        p = put_amf_string(p, "copyright");
        p = put_byte(p, AMF_STRING);
        p = put_amf_string(p, "pub");

        p = put_amf_string(p, "width");
        p = put_amf_double(p, _pMeta->nWidth);

        p = put_amf_string(p, "height");
        p = put_amf_double(p, _pMeta->nHeight);

        p = put_amf_string(p, "framerate" );
        p = put_amf_double(p, _pMeta->nFrameRate);

        p = put_amf_string(p, "videocodecid" );
        p = put_amf_double(p, FLV_CODECID_H264 );

        p = put_amf_string(p, "" );
        p = put_byte(p, AMF_OBJECT_END  );

        int index = p - body;

        SendPacket(RTMP_PACKET_TYPE_INFO, (char *)body, index ,0);

        int i = 0;
        body[i++] = 0x17; // 1:keyframe  7:AVC
        body[i++] = 0x00; // AVC sequence header

        body[i++] = 0x00;
        body[i++] = 0x00;
        body[i++] = 0x00; // fill in 0;

        // AVCDecoderConfigurationRecord.
        body[i++] = 0x01; // configurationVersion
        body[i++] = _pMeta->sps[1]; // AVCProfileIndication
        body[i++] = _pMeta->sps[2]; // profile_compatibility
        body[i++] = _pMeta->sps[3]; // AVCLevelIndication
        body[i++] = 0xff; // lengthSizeMinusOne

        // sps nums
        body[i++] = 0xE1; //&0x1f
        // sps data length
        body[i++] = _pMeta->nSpsLen >> 8;
        body[i++] = _pMeta->nSpsLen & 0xff;
        // sps data
        memcpy(&body[i], _pMeta->sps, _pMeta->nSpsLen);
        i= i + _pMeta->nSpsLen;

        // pps nums
        body[i++] = 0x01; //&0x1f
        // pps data length
        body[i++] = _pMeta->nPpsLen >> 8;
        body[i++] = _pMeta->nPpsLen & 0xff;
        // sps data
        memcpy(&body[i], _pMeta->pps, _pMeta->nPpsLen);
        i = i + _pMeta->nPpsLen;

        return SendPacket(RTMP_PACKET_TYPE_VIDEO, (char *)body, i, 0);
}

bool RtmpStream::SendH264Packet(char *_data, unsigned int _size, bool _bIsKeyFrame, unsigned int _nTimeStamp)
{
        if (_data == nullptr && _size < 11) {
                return false;
        }

        char *body = new char[_size + 9];

        int i = 0;
        if (_bIsKeyFrame) {
                body[i++] = 0x17; // 1:Iframe 7:AVC
        } else {
                body[i++] = 0x27; // 2:Pframe 7:AVC
        }
        body[i++] = 0x01; // AVC NALU
        body[i++] = 0x00;        
        body[i++] = 0x00;
        body[i++] = 0x00;

        // NALU size
        body[i++] = _size >> 24;
        body[i++] = _size >> 16;
        body[i++] = _size >> 8;
        body[i++] = _size & 0xff;

        // NALU data
        memcpy(&body[i], _data, _size);

        bool bRet = SendPacket(RTMP_PACKET_TYPE_VIDEO, body, i + _size, _nTimeStamp);
        delete[] body;
        return bRet;
}

bool RtmpStream::SendH264File(const char *_pFileName)
{
        if (_pFileName == NULL) {
                return false;
        }
        FILE *fp = fopen(_pFileName, "rb");
        if (!fp) {
                cout << "Error: could not open file" << endl;
                return false;
        }
        fseek(fp, 0, SEEK_SET);
        m_nFileBufSize = fread(m_pFileBuf, sizeof(char), FILE_BUFFER_SIZE, fp);
        if (m_nFileBufSize >= FILE_BUFFER_SIZE) {
                cout << "Warning: file is truncated" << endl;
        }
        fclose(fp);

        RtmpMetadata meta;
        memset(&meta, 0, sizeof(RtmpMetadata));

        NalUnit nalu;
        // read sps
        if (ReadOneNaluFromBuf(nalu) == false) {
		return false;
	}
        meta.nSpsLen = nalu.size;
        memcpy(meta.sps, nalu.data, nalu.size);

        // read pps
        if (ReadOneNaluFromBuf(nalu) == false) {
		return false;
	}
        meta.nPpsLen = nalu.size;
        memcpy(meta.pps, nalu.data, nalu.size);

        int width, height;
        Util264::h264_decode_sps(meta.sps, meta.nSpsLen, width, height);
        meta.nWidth = width;
        meta.nHeight = height;
        meta.nFrameRate = 30;

	cout << "[debug] width=" << width << " height=" << height << endl;

        // send metadata
        SendMetadata(&meta);

        unsigned int tick = 0;
	unsigned int n = 0;
        while (ReadOneNaluFromBuf(nalu)) {
		// notice : for a normal h264 stream, we have 1 sps
		// and 1 pps as initial NALUs, so skip others
		if (nalu.type == 0x06 || nalu.type == 0x07 || nalu.type == 0x08) {
			cout << " == skip type " << nalu.type << " ==" << endl;
			continue;
		}

		bool bKeyFrame = (nalu.type == 0x05) ? true : false;

                // send h264 frames
                SendH264Packet(nalu.data, nalu.size, bKeyFrame, tick);
                msleep(1000 / meta.nFrameRate);
                tick += 1000 / meta.nFrameRate;
        }

        return true;
}

bool RtmpStream::GetNextNalUnit(unsigned int _nStart, unsigned int &_nDelimiter, unsigned int &_nNalu)
{
	unsigned int nSeqZeroByte = 0;
	unsigned int i = _nStart;
	for (; i < m_nFileBufSize; i++) {
		if (m_pFileBuf[i] == 0x00) {
			nSeqZeroByte++;
		} else if (m_pFileBuf[i] == 0x01) {
			if (nSeqZeroByte >= 2 && nSeqZeroByte <= 3) {
				// 00 00 01 & 00 00 00 01
				_nDelimiter = i - nSeqZeroByte;
				_nNalu = i + 1;
				if (_nNalu >= m_nFileBufSize) {
					cout << "[warning] last NALU payload not valid" << endl;
				}
				return true;
			} else if (nSeqZeroByte > 3) {
				cout << "[error] missing 4-byte seq "<< nSeqZeroByte << "@" << i << endl;
			}
			nSeqZeroByte = 0;
		} else {
			nSeqZeroByte = 0;
		}
	}
	return false;
}

bool RtmpStream::ReadOneNaluFromBuf(NalUnit &nalu)
{
	unsigned int nDelimiterPos, nNaluPos;
	while (GetNextNalUnit(m_nCurPos, nDelimiterPos, nNaluPos)) {
		unsigned int nThisNalu = nNaluPos;
		if (GetNextNalUnit(nThisNalu, nDelimiterPos, nNaluPos) != true) {
			nDelimiterPos = m_nFileBufSize;
		}
		nalu.size = nDelimiterPos - nThisNalu;
		nalu.type = m_pFileBuf[nThisNalu] & 0x01f;
		nalu.data = &m_pFileBuf[nThisNalu];

		cout << "[debug] pos=" << m_nCurPos << " nalu.type="
		     << nalu.type << " nalu.size=" << nalu.size << endl;

		m_nCurPos = nDelimiterPos;
                return true;
	}
        cout << "[debug] end of publish" << endl;
        return false;
}

void RtmpStream::PrintNalUnit(const NalUnit *_pNalu)
{
	cout << "[NALU] pos=" << m_nCurPos << " type=" << _pNalu->type
	     << " size=" << _pNalu->size << endl;
}
