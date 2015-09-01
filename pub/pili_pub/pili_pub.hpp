#include "pub.hpp"

extern "C" {
#include "push.h"
}


class PiliStream: public RtmpStream
{
public:
        PiliStream(void);
        bool Connect(const char *url);
	bool Connect(const char *_url, pili_stream_state_cb _callback);
        void Close();
        bool SendH264File(const char *pFileName);

private:
        void PutPps(NalUnit *pNalu);
        void PutSps(NalUnit *pNalu);
        void PutSei(NalUnit *pNalu);
        void PutIdr(NalUnit *pNalu);
        void PutSlice(NalUnit *pNalu);
        void RunTicks();
        pili_stream_context_p m_piliCtx;
        pili_h264_key_frame_t m_piliKeyframe;
        unsigned int m_piliTick;
        unsigned int m_piliFps;
};
