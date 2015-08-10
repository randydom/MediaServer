#pragma once

#ifndef RTSPCLIENT_H
#define RTSPCLIENT_H

//#include "VideoNetSDK.h"

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "RTSPCommon.h"
#include "DigestAuthentication.hh"
//#include "Log.h"

#define RTPDATALEN          (65536 + 512)
#define RAWDATALEN          (1024*1024*4)
#define RTSPCOMMANDLEN      65536//1536
#define RECVRTSPRESPONSELEN 65536//40000VideoNetSDK.h
#define RTSP_TCP            0
#define RTSP_UDP            1

#ifndef WIN32
#ifndef __stdcall
#define __stdcall __attribute__((__stdcall__))
#endif
#endif

//切换成功后，新的视频摄像机的域名信息回调函数原型
typedef void (__stdcall *VIDEO_SWITCH_CAMERA_UPDATE_CALLBACK)(unsigned long dwOldCameraID,
	unsigned long dwNewCameraID, unsigned char *NewCameraName, unsigned long user);

//extern CLog m_log;

class CRTSPClient
{
public:
	CRTSPClient(void);
public:
	virtual ~CRTSPClient(void);

public:
	enum ResponseType{
		Failed = -1,
		ResponseSUCCESS = 0,
		NOEnoughData = 1,
		NeedToResendCommand = 2,
		NeedToRedirectCommand = -301,
		FullChannelResponse = -563
	};

	enum CommandName{
		NULLCOMMAND,
		OPTOINSCOMMAND,
		DESCRIBECOMMAND,
		SETUPCOMMAND,
		PLAYCOMMAND,
		PLAYCOMMANDED,
		TEARDOWNCOMMAND,
	};

	enum CommandStatus{
		FREE_STATUS,
		WAITFOR_RESPONSE,
		SUCCESS_RESPONSE,
		FAILED_RESPONSE,
	};

	typedef struct _RTPExtensionData {
		int dwDataType;
		int dwtimeTamp;
	} RTPExtensionData;

public:
	virtual int startRTSPRequest(char* ip ,int port ,const char *url, const char* Range_clock = "-", float scale = 1.0) = 0;
	int stopRTSPRequest();

	int ChangePlayCommandInPlay(const char * url, float scale = 1.0, const char* Range_clock = "-");
	int ChangePlayToPause(const char * url);
	int ChangePauseToPlay(const char * url, float scale = 1.0, const char* Range_clock = "-");
	int SwitchCameraInPlay(const char * url, int ioldCamera, int inewCamera);
	int CameraPTZCtrlInPlay(const char * url, int cmd, int param1, int param2);

	int GetSaveFileInfo(char* ip, int port, const char * url, const char* Range_clock, char* resultStr);

	int GetCurCameraID(){ return m_cameraid; };
	std::string GetCurCameraName(){ return m_cameraname; };
	void setUsernameAndPassword(char* username, char* password){
		if(strlen(username) != 0){ 	sprintf(m_user_agent, "VNMPNetSDK V1.0 01 %d", m_sClientID); }
		fCurrentAuthenticator.setUsernameAndPassword(username, password); 
	}

	static bool parseRTSPURL(char const* url, char*& username, char*& password,
		char*& address, int& portNum, char*& urlSuffix);

	char * GetLocationUrl(){ return m_baseUrl; };
	std::list<int>& GetCameraList(){ return m_cameraIdList; };

protected:
	virtual int sendSetupCommand(const char * url) = 0;
	virtual void handleRecvVideoDataThread() = 0;
	int RecvPlayResponse();

protected:
	int setRTSPClientType(int NetType);
	int sendSomeCommand(const char * url);

	int sendtoSvr(char *buf, int len);
	int connectRtspSrv(char* ip, int port);
	int recvRTSPResponse();
	int HandleIncomingData();

	int sendOptionsCommand(const char * url);
	int sendDescribeCommand(const char * url, const char* contentType = "sdp", const char* Range_clock = "-");

	int sendPlayCommand(const char * url,float scale = 1.0, const char* Range_clock = "-");
	int sendTeardownCommand(const char * url);
	
	int sendSet_ParameterCommand(const char * url, int cmd, int param1, int param2);
	int sendGet_ParameterCommand( const char * url );

	int sendPauseCommand( const char * url );

    int handleResponseBytes(int newBytesRead);
	void resetResponseBuffer();

	Boolean parseResponseCode(char const* line, 
		unsigned& responseCode, char const*& responseString);
	Boolean checkForHeader(char const* line, char const* headerName,
		unsigned headerNameLength, char const*& headerParams) ;
	Boolean handleSETUPResponse(char const* transportParamsStr);

	Boolean handleSDPResponse(char * sdpBodyStr);
	int handleIncomingRequest();
	int handleANNOUNCERequest();
	int sendANNOUNCEResponse(int icseq);

	Boolean handleAuthenticationFailure(char const* wwwAuthenticateParamsStr);
	char* createAuthenticatorString(char const* cmd, char const* url);

protected:
	char* fResponseBuffer;
	unsigned fResponseBytesAlreadySeen, fResponseBufferBytesLeft;
	static unsigned responseBufferSize;

	Authenticator fCurrentAuthenticator;

	boost::asio::io_service io_service_;
	boost::asio::ip::tcp::socket rtsp_socket_;
	boost::thread m_recvThread_;

	char* m_pSaveFileInfo;

	char m_url[256];
	char m_baseUrl[256];

	int m_icseq;
	char m_user_agent[256];
	char m_SessionID[256];
	int m_iNetType;
	int m_iCommandStatus;

	int m_serverRTPUDPPort;
	char m_ServerAddressStr[256];

	bool m_brun;

	int m_cameraid;
	std::string m_cameraname;

	int m_commandStatusInPlay;
	int m_responseCodeInPlay;

	static int m_sClientID;
	std::list<int> m_cameraIdList;

public:
	static VIDEO_SWITCH_CAMERA_UPDATE_CALLBACK s_UpdateCameraNameCallBack;
	static unsigned long s_UpdateCameraCallBackUser;
};

#endif // RTSPCLIENT_H