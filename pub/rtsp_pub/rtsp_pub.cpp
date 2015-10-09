// --------------------------
// rtsp_pub.cpp
// --------------------------

#include "rtsp_pub.hpp"

using namespace std;


//
// Data Sink
//

RtspSink::RtspSink(UsageEnvironment& _env, MediaSubsession& _subsession, char const* _pStreamId):
        MediaSink(_env),
        m_subsession(_subsession)
{
        m_streamId = _pStreamId;
        m_pReceiveBuffer = new u_int8_t[RTSP_SINK_RECEIVE_BUFFER_SIZE];
        m_bIsConfigSent = false;
}

RtspSink::~RtspSink()
{
        delete[] m_pReceiveBuffer;
}

RtspSink* RtspSink::createNew(UsageEnvironment& _env, MediaSubsession& _subsession, const char* _pStreamId)
{
        return new RtspSink(_env, _subsession, _pStreamId);
}

Boolean RtspSink::continuePlaying()
{
        if (fSource == nullptr) {
                return False;
        }
        fSource->getNextFrame(m_pReceiveBuffer, RTSP_SINK_RECEIVE_BUFFER_SIZE, OnGettingFrame, this, onSourceClosure, this);
        return True;
}

void RtspSink::OnGettingFrame(void* _pClientData, unsigned int _nFrameSize, unsigned int _nTruncatedBytes,
                              struct timeval _presentationTime, unsigned int _durationInMicroseconds)
{
        RtspSink* pSink = (RtspSink *)_pClientData;
#ifdef __PRINT_CODEC_INFO__
        cout << pSink->m_subsession.mediumName() << " " << pSink->m_subsession.codecName() << endl;
#endif
        if (strcmp(pSink->m_subsession.mediumName(), "video") == 0) {
                if (strcmp(pSink->m_subsession.codecName(), "H264") == 0) {
                        pSink->GetNextH264Frame(_nFrameSize, _nTruncatedBytes, _presentationTime, _durationInMicroseconds);
                }
        } else if (strcmp(pSink->m_subsession.mediumName(), "audio") == 0) {
                if (strcmp(pSink->m_subsession.codecName(), "PCMA") == 0) {
                        pSink->GetNextPcmaFrame(_nFrameSize, _nTruncatedBytes, _presentationTime, _durationInMicroseconds);
                } else if (strcmp(pSink->m_subsession.codecName(), "PCMU") == 0) {
                        pSink->GetNextPcmuFrame(_nFrameSize, _nTruncatedBytes, _presentationTime, _durationInMicroseconds);
                }
        }
}

void RtspSink::GetNextH264Frame(unsigned int _nFrameSize, unsigned int _nTruncatedBytes,
                            struct timeval _presentationTime, unsigned int _durationInMicroseconds)
{
        bool bStatus = true;
        short nUnitType = static_cast<short> (m_pReceiveBuffer[0]) & 0x0f;
        RtspStream *pRtsp = (RtspStream *)m_subsession.miscPtr;
        RtmpPacketSender *pRtmp = (pRtsp == nullptr ? nullptr : pRtsp->pRtmpSender);

        if (pRtmp == nullptr) {
                cout << *pRtsp << "Wrong sink" << endl;
                return;
        }

        // send configuration if needed
        if (m_bIsConfigSent == false) {
                const char *pSPropSets = m_subsession.fmtp_spropparametersets();
                if (pSPropSets != nullptr) {
                        short nSPropType;
                        unsigned int nSPropRecords;
                        SPropRecord *pSPropRecords = parseSPropParameterSets(pSPropSets, nSPropRecords);
                        for (unsigned int i = 0; i < nSPropRecords; i++) {
                                nSPropType = static_cast<short> (pSPropRecords[i].sPropBytes[0]) & 0x0f;
                                switch (nSPropType) {
                                case 0x7:
                                        pRtmp->Sps((char *)pSPropRecords[i].sPropBytes, pSPropRecords[i].sPropLength);
                                        break;
                                case 0x8:
                                        pRtmp->Pps((char *)pSPropRecords[i].sPropBytes, pSPropRecords[i].sPropLength);
                                        break;
                                default:
                                        cout << *pRtsp << "SPropParam: invalid type: " << nSPropType << endl;
                                        break;
                                }
                        }
                        bStatus = pRtmp->SendConfig();
                        if (bStatus == false) {
                                cout << *pRtsp << "Configuration not sent" << endl;
                                return;
                        } else {
                                cout << *pRtsp << "Configuration sent ..." << endl;
                                m_bIsConfigSent = true;
                        }
                }
        }

        switch (nUnitType) {
        case 0x1:
                bStatus = pRtmp->SendNonIdr((char *)m_pReceiveBuffer, _nFrameSize);
                break;
        case 0x5:
                bStatus = pRtmp->SendIdrAll((char *)m_pReceiveBuffer, _nFrameSize);
                break;
        case 0x6:
                pRtmp->Sei((char *)m_pReceiveBuffer, _nFrameSize);
                break;
        case 0x7:
                pRtmp->Sps((char *)m_pReceiveBuffer, _nFrameSize);
                break;
        case 0x8:
                pRtmp->Pps((char *)m_pReceiveBuffer, _nFrameSize);

                // OK, this is just my assumption that after we fail to get configurations from
                // m_subsession.fmtp_spropparametersets() (what the hell..) we need have a backup
                // plan to send configuration, so SPS should be always followed by PPS
                if (m_bIsConfigSent == false) {
                        // try
                        cout << *pRtsp << "received PPS and try to send configuration again ..." << endl;
                        bStatus = pRtmp->SendConfig();
                        if (bStatus == false) {
                                cout << *pRtsp << "Configuration not sent" << endl;
                                return;
                        } else {
                                cout << *pRtsp << "Configuration sent ..." << endl;
                                m_bIsConfigSent = true;
                        }
                }
                break;
        default:
                cout << *pRtsp << "Error: NAL type not handled" << endl;

        }
        if (bStatus != true) {
                cout << *pRtsp << "Error: NAL type " << nUnitType << " send error" << endl;

                // when ExitOnError flag is set
                if (pRtsp->bExitOnError) {
                        if (pRtsp->pMgmtPtr != nullptr) {
                                pRtsp->pMgmtPtr->Stop();
                        } else {
                                cout << *pRtsp << "Error: Could not stop event loop, exit anyway" << endl;
                                exit(RTSP_EXIT_CONNECTION);
                        }
                }
        } else {
#ifdef __PRINT_EACH_RTSP_NALU__
                cout << *pRtsp << "OK: NAL type " << nUnitType << " frame_size:" << _nFrameSize  << endl;
#endif
        }

        continuePlaying();
}

void RtspSink::GetNextPcmaFrame(unsigned int _nFrameSize, unsigned int _nTruncatedBytes,
                                struct timeval _presentationTime, unsigned int _durationInMicroseconds)
{
        continuePlaying();
}

void RtspSink::GetNextPcmuFrame(unsigned int _nFrameSize, unsigned int _nTruncatedBytes,
                                struct timeval _presentationTime, unsigned int _durationInMicroseconds)
{
        continuePlaying();
}

//
// Stream Client State
//

StreamClientState::StreamClientState():
        iter(nullptr), session(nullptr), subsession(nullptr), streamTimerTask(nullptr), duration(0.0)
{
}

StreamClientState::~StreamClientState()
{
        delete iter;
        if (session != nullptr) {
                UsageEnvironment& env = session->envir();
                env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
                Medium::close(session);
        }
}

//
// RtspStream
//

ostream& operator<< (ostream &_os, const RTSPClient& _rtspClient)
{
        _os << "[" << _rtspClient.url() << "]";
        return _os;
}

ostream& operator<< (ostream &_os, const RtspStream& _rtspStream)
{
        _os << "[" << _rtspStream.rtspUrl.c_str() << " -> " << _rtspStream.rtmpUrl.c_str() << "]";
        return _os;
}

ostream& operator<< (ostream &_os, const MediaSubsession& _subsession)
{
        _os << _subsession.mediumName() << "/" << _subsession.codecName();
        return _os;
}

RtspStream::RtspStream(UsageEnvironment& _env, const char* _name, const char* _rtspUrl, const char* _rtmpUrl, int _verbosityLevel, 
                       const char* _applicationName, portNumBits _tunnelOverHTTPPortNum):
        RTSPClient(_env, _rtspUrl, _verbosityLevel, _applicationName, _tunnelOverHTTPPortNum, -1)
{
        name = _name;
        rtmpUrl = _rtmpUrl;
        rtspUrl = _rtspUrl;
        pRtmpSender = new RtmpPacketSender(rtmpUrl);
        bExitOnError = false;
        pMgmtPtr = nullptr;
}

RtspStream::~RtspStream(void)
{
        delete pRtmpSender;
}

RtspStream* RtspStream::createNew(UsageEnvironment& _env, const char* _name, const char* _rtspUrl, const char* _rtmpUrl, 
                                  int _verbosityLevel, const char* _applicationName, portNumBits _tunnelOverHTTPPortNum)
{
        return new RtspStream(_env, _name, _rtspUrl, _rtmpUrl,  _verbosityLevel, _applicationName, _tunnelOverHTTPPortNum);
}

void RtspStream::StartStreaming()
{
        bool bStatus;

        cout << *this<< "Connecting RTMP stream server: " << this->rtmpUrl << " ..." << endl;
        bStatus = pRtmpSender->Connect(true);
        if (bStatus != true) {
                cout << *this << "Connection failed to server: " << this->rtmpUrl << " ..." << endl;
                return;
        }

        cout << *this << "Send describe command to RTSP server: " << this->rtspUrl << " ..." << endl;
        sendDescribeCommand(RtspStream::OnAfterDescribe);
}

void RtspStream::StopStreaming()
{
        // this is tricky, if ExitOnError is set we just notify Rtsp2Rtmp(pMgmtPtr) object to stop event loop and
        // Rtsp2Rtmp::StopStreaming will be responsible for calling RtspStream::ShutdownStream for each stream
        // Notice: do never call RtspStream::ShutdownStream for the same RTSPClient* twice, otherwise core dump
        if (bExitOnError) {
                pMgmtPtr->Stop();
        } else {
                if (pRtmpSender != nullptr) {
                        pRtmpSender->Close();
                }
                RtspStream::ShutdownStream((RTSPClient *)this);
        }
}

void RtspStream::OnAfterDescribe(RTSPClient* _pRtspClient, int _nResultCode, char* _pResultString)
{
        UsageEnvironment& env = _pRtspClient->envir();
        RtspStream* pRtspStream = (RtspStream *)_pRtspClient;
        StreamClientState& scs = pRtspStream->scs;

        do {
                if (_nResultCode != 0) {
                        cout << *_pRtspClient << "Failed to get a SDP description of " << pRtspStream->name
                             <<  ": " << _pResultString << endl;
                        delete[] _pResultString;
                        break;
                }

                char *sdpDescription = _pResultString;
                cout << *_pRtspClient << "Got a SDP description:\n" << sdpDescription << endl;

                scs.session = MediaSession::createNew(env, sdpDescription);
                delete[] sdpDescription;
                if (scs.session == nullptr) {
                        cout << *_pRtspClient << "Failed to create MediaSession for " << pRtspStream->name << ":"
                             << env.getResultMsg() << endl;
                        break;
                } else if (!scs.session->hasSubsessions()) {
                        cout << *_pRtspClient << "Session has no MediaSubsessions";
                        break;
                }

                scs.iter = new MediaSubsessionIterator(*scs.session);
                RtspStream::SetupNextSubsession(_pRtspClient);
                return;
        } while(0);

        pRtspStream->StopStreaming();
}

void RtspStream::ShutdownStream(RTSPClient* _pRtspClient, int _nExitCode)
{
        UsageEnvironment& env = _pRtspClient->envir();
        StreamClientState& scs = ((RtspStream *)_pRtspClient)->scs;

        if (scs.session != nullptr) {
                Boolean someSubsessionsWereActive = False;
                MediaSubsessionIterator iter(*scs.session);
                MediaSubsession* subsession;

                while ((subsession = iter.next()) != nullptr) {
                        if (subsession->sink != nullptr) {
                                Medium::close(subsession->sink);
                                subsession->sink = nullptr;
                                if (subsession->rtcpInstance() != nullptr) {
                                        subsession->rtcpInstance()->setByeHandler(nullptr, nullptr);
                                }
                                someSubsessionsWereActive = True;
                        }
                }

                if (someSubsessionsWereActive) {
                        _pRtspClient->sendTeardownCommand(*scs.session, nullptr);
                }
        }

        cout << *_pRtspClient << "Closing the stream." << endl;
        Medium::close(_pRtspClient);
}

void RtspStream::SetupNextSubsession(RTSPClient* _pRtspClient)
{
        UsageEnvironment& env = _pRtspClient->envir();
        RtspStream* pRtspStream = (RtspStream *)_pRtspClient;
        StreamClientState& scs = ((RtspStream *)_pRtspClient)->scs;

        scs.subsession = scs.iter->next();
        if (scs.subsession != nullptr) {
                if (!scs.subsession->initiate()) {
                        cout << *_pRtspClient << "Failed to initate subsession " << *scs.subsession << " of " << pRtspStream->name
                             << ":" << env.getResultMsg() << endl;
                } else {
                        cout << *_pRtspClient << "Initialized subsession " << *scs.subsession << " of " << pRtspStream->name << " (";
                        if (scs.subsession->rtcpIsMuxed()) {
                                cout << "client port " << scs.subsession->clientPortNum();
                        } else {
                                cout << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum() + 1;
                        }
                        cout << ")" << endl;
                        pRtspStream->sendSetupCommand(*scs.subsession, RtspStream::OnAfterSetup, False, REQUEST_STREAMING_OVER_TCP);
                }
                return;
        }

        if (scs.session->absStartTime() != nullptr) {
                _pRtspClient->sendPlayCommand(*scs.session, RtspStream::OnAfterPlay, scs.session->absStartTime(), scs.session->absEndTime());
        } else {
                scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
                _pRtspClient->sendPlayCommand(*scs.session, RtspStream::OnAfterPlay);
        }
}

void RtspStream::OnAfterSetup(RTSPClient* _pRtspClient, int _nResultCode, char* _pResultString)
{
        do {
                UsageEnvironment& env = _pRtspClient->envir();
                RtspStream* pRtspStream = (RtspStream *)_pRtspClient;
                StreamClientState& scs = ((RtspStream *)_pRtspClient)->scs;

                if (_nResultCode != 0) {
                        cout  << "Failed to set up the subsession " << *scs.subsession << " of " 
                              << pRtspStream->name << ":" << _pResultString << endl;
                        break;
                }

                cout << *_pRtspClient << "Set up the subsession " << *scs.subsession << " for " << pRtspStream->name << " (";
                if (scs.subsession->rtcpIsMuxed()) {
                        cout << "client port " << scs.subsession->clientPortNum();
                } else {
                        cout << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum() + 1;
                }
                cout << ")" << endl;

                //
                // dispatch a Sink class to handle incoming data
                //
                scs.subsession->sink = RtspSink::createNew(env, *scs.subsession, _pRtspClient->url());
                if (scs.subsession->sink == nullptr) {
                        cout << "Failed to create a data sink for " << pRtspStream->name << ":" << env.getResultMsg() << endl;
                        break;
                }
                cout << "Created a data sink for subsession of " << pRtspStream->name << endl;
                scs.subsession->miscPtr = _pRtspClient;
                scs.subsession->sink->startPlaying(*(scs.subsession->readSource()), RtspStream::OnSubsessionAfterPlaying, scs.subsession);
                if (scs.subsession->rtcpInstance() != nullptr) {
                        scs.subsession->rtcpInstance()->setByeHandler(RtspStream::OnSubsessionByeHandler, scs.subsession);
                }
        } while(0);
        delete[] _pResultString;

        RtspStream::SetupNextSubsession(_pRtspClient);
}

void RtspStream::OnAfterPlay(RTSPClient* _pRtspClient, int _nResultCode, char* _pResultString)
{
        bool bSuccess = false;

        do {
                UsageEnvironment& env = _pRtspClient->envir();
                StreamClientState& scs = ((RtspStream *)_pRtspClient)->scs;
                if (_nResultCode != 0) {
                        cout << *_pRtspClient << "Failed to start playing session: " << _pResultString << endl;
                        break;
                }
                if (scs.duration > 0) {
                        unsigned int const nDelaySlop = 2;
                        scs.duration += nDelaySlop;
                        unsigned int nSecsToDelay = (unsigned int)(scs.duration * 1000000);
                        scs.streamTimerTask = env.taskScheduler()
                                                 .scheduleDelayedTask(nSecsToDelay, 
                                                                      (TaskFunc *)RtspStream::StreamTimerHandler, _pRtspClient);
                }

                cout << *_pRtspClient << "Started playing session";
                if (scs.duration > 0) {
                        cout << " (for up to " << scs.duration << " seconds)";
                }
                cout << "..." << endl;
                bSuccess = True;
        }while (0);

        delete[] _pResultString;
        if (!bSuccess) {
                RtspStream* pRtspStream = (RtspStream *)_pRtspClient;
                cout << *_pRtspClient << "In OnAfterPlay():Going to shutdown" << endl;
                pRtspStream->StopStreaming();
        }
}

void RtspStream::StreamTimerHandler(void* _pClientData)
{
        RtspStream* pRtspStream = (RtspStream *)_pClientData;
        StreamClientState& scs = pRtspStream->scs;
        scs.streamTimerTask = nullptr;
        pRtspStream->StopStreaming();
}

void RtspStream::OnSubsessionAfterPlaying(void* _pClientData)
{
        MediaSubsession* pSubsession = (MediaSubsession *)_pClientData;
        RTSPClient* pRtspClient = (RTSPClient *)(pSubsession->miscPtr);

        Medium::close(pSubsession->sink);
        pSubsession->sink = nullptr;

        MediaSession& session = pSubsession->parentSession();
        MediaSubsessionIterator iter(session);
        while ((pSubsession = iter.next()) != nullptr) {
                if (pSubsession->sink != nullptr) 
                        return;
        }
        RtspStream* pRtspStream = (RtspStream *)pRtspClient;
        pRtspStream->StopStreaming();
}

void RtspStream::OnSubsessionByeHandler(void* _pClientData)
{
        MediaSubsession* pSubsession = (MediaSubsession *)_pClientData;
        RTSPClient* pRtspClient = (RTSPClient *)pSubsession->miscPtr;
        UsageEnvironment& env = pRtspClient->envir();

        cout << *pRtspClient << "Received RTCP BYE on " << *pSubsession << " subsession" << endl;
        RtspStream::OnSubsessionAfterPlaying(pSubsession);
}

//
// Rtsp2Rtmp
//

Rtsp2Rtmp::Rtsp2Rtmp(void):
        m_pScheduler(nullptr),
        m_pEnv(nullptr),
        m_chEventLoopControl(0)
{
        m_pScheduler = BasicTaskScheduler::createNew();
        m_pEnv = BasicUsageEnvironment::createNew(*m_pScheduler);
}

Rtsp2Rtmp::~Rtsp2Rtmp(void)
{
        m_pEnv->reclaim(); 
        m_pEnv = nullptr;
        delete m_pScheduler;
        m_pScheduler = nullptr;
}

void Rtsp2Rtmp::Add(const char *_pName, const char *_pRtspUrl, const char *_pRtmpUrl, bool _bExitOnError)
{
        CreateStreamPair(_pName, _pRtspUrl, _pRtmpUrl, _bExitOnError);
}

void Rtsp2Rtmp::Run()
{
        StartStreaming();
        StartEventLoop();
        StopStreaming();
}

void Rtsp2Rtmp::Stop()
{
        m_chEventLoopControl = 1;
}

void Rtsp2Rtmp::StopStreaming()
{
        for (auto it = m_streams.begin(); it != m_streams.end(); ++it) {
                cout << **it << "Stop streaming ..." << endl;
                (*it)->StopStreaming();
        }
        cout << endl << "Stopped !" << endl;
        exit(RTSP_EXIT_CONNECTION);
}

void Rtsp2Rtmp::CreateStreamPair(const char *_pName, const char *_pRtspUrl, const char *_pRtmpUrl, bool _bExitOnError)
{
        RtspStream *pRtsp = RtspStream::createNew(*m_pEnv, _pName, _pRtspUrl, _pRtmpUrl, RTSP_CLIENT_VERBOSITY_LEVEL, "PROG_NAME");
        pRtsp->bExitOnError = _bExitOnError;
        pRtsp->pMgmtPtr = this;
        m_streams.push_back(pRtsp);
}

void Rtsp2Rtmp::StartStreaming()
{
        for (vector<RtspStream *>::iterator it = m_streams.begin(); it != m_streams.end(); ++it) {
                cout << **it << "Start streaming ..." << endl;
                (*it)->StartStreaming();
        }
}

void Rtsp2Rtmp::StartEventLoop()
{
        // start event loop from here
        m_pEnv->taskScheduler().doEventLoop(&m_chEventLoopControl);
}
