#include "pili_pub.hpp"

#include <iostream>
#include <cstdlib>
#include <cstring>

// librtmp
#include "rtmp.h"
#include "rtmp_sys.h"
#include "amf.h"

#include "x264.hpp"

using namespace std;

//----------------------------------

// pili publish

PiliStream::PiliStream(void):
	m_piliCtx(nullptr),
	m_piliTick(0)
{
	// once-read file buffer
        m_pFileBuf = new char[FILE_BUFFER_SIZE];
        memset(m_pFileBuf, 0, FILE_BUFFER_SIZE);
}

bool PiliStream::Connect(const char *_url)
{
	return Connect(_url, NULL);
}

bool PiliStream::Connect(const char *_url, pili_stream_state_cb _callback)
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

void PiliStream::Close()
{
	pili_stream_push_close(m_piliCtx);
	pili_release_stream_context(m_piliCtx);
}

bool PiliStream::SendH264File(const char *_pFileName)
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
		// TODO check return values
		case 0x01:
			PutSlice(&nalu);
			break;
		case 0x05:
			PutIdr(&nalu);
			break;
		case 0x06:
			PutSei(&nalu);
		case 0x07:
			PutSps(&nalu);
			break;
		case 0x08:
			PutPps(&nalu);
			break;
		default:
			break;
		}
        }

        return true;
}

void PiliStream::PutSps(NalUnit *_pNalu)
{
	//int width, height;
        //Util264::h264_decode_sps(_pNalu.data, _pNalu.size, width, height);
        m_nFrameRate = 30;

	m_piliKeyframe.sps.data = (uint8_t *)_pNalu->data;
	m_piliKeyframe.sps.length = _pNalu->size;
}

void PiliStream::PutPps(NalUnit *_pNalu)
{
	m_piliKeyframe.pps.data = (uint8_t *)_pNalu->data;
	m_piliKeyframe.pps.length = _pNalu->size;
}

void PiliStream::PutIdr(NalUnit *_pNalu)
{
	m_piliKeyframe.idr.data = (uint8_t *)_pNalu->data;
	m_piliKeyframe.idr.length = _pNalu->size;

	pili_write_h264_key_frame(m_piliCtx, m_piliKeyframe, m_piliTick);

	RunTicks();
}

void PiliStream::PutSei(NalUnit *_pNalu)
{
	m_piliKeyframe.sei.data = (uint8_t *)_pNalu->data;
	m_piliKeyframe.sei.length = _pNalu->size;
}

void PiliStream::PutSlice(NalUnit *_pNalu)
{
	// have to convert to pili stype ~~
	pili_h264_nalu_t nalu;
	nalu.data = (uint8_t *)_pNalu->data;
	nalu.length = _pNalu->size;

	pili_write_h264_slice(m_piliCtx, nalu, m_piliTick);

	RunTicks();
}

void PiliStream::RunTicks()
{
	unsigned int fps = 1000 / m_nFrameRate;
	m_piliTick += fps;
	msleep(fps);
}

