/*****************************************************************************************************************
	CSessionManager

	작성자 : 강진협
	작성일 : 2004년 3월 4 ~ 5일

	설  명 :
		IOCP를 이용한 Server를 상속
		OnReceive 이벤트를 메시지 단위가 아닌, 지정한 패킷 단위로 호출
	사  용 :
		- CIocp
		1. Create를 사용하여 Iocp Server를 생성한다.
		2. Bind와 Listen을 한다. 이 값은 기본 소켓 프로그램과 동일하다.
		3. Start를 통해서 Iocp Server를 수행시킨다.
		4. Iocp Server는 소켓 핸들로부터 OnAccept, OnReceive, OnSend, OnClose 이벤트를 발생시킨다.
		5. Send를 통해서 Iocp Server에 연결되어 있는 클라이언트에게 메시지를 전달한다.
		6. CloseSocket을 통해서 Iocp Server에 연결되어 있는 특정 소켓을 닫는다.
		7. 모두 사용한 뒤에는 Destroy를 통해 내부 리소스를 제거를 한다.

		- CSessionManager
		1. CIocp의 3번 Start를 하기전에 패킷에 관련된 초기화를 해준다.
*****************************************************************************************************************/

#pragma once

//#include <Windows.h>
#ifndef CLIENT_MODULE
	#include "..\Network\TcpIocp.h"
#else
	#include "..\Network\TcpAsyncSelect.h"
#endif
#include "..\Network\Network.h"

#include "..\..\DataStructure\MemoryPoolSafty.h"

#include "PacketAssembler.h"
#include "MessageQueue.h"
#include "Timer.h"
#include "SendPacket.h"

#include "PacketBuffer.h"
#include "QueryBuffer.h"

#include "..\Query\IQueryHandler.h"

#ifndef CLIENT_MODULE
	#include "..\Query\AsyncInet.h"
	#include "../Query/SyncInet.h"
#endif


namespace Session{		namespace Manage{

	class CSessionManager : public ITimerHandler, Query::IQueryHandler, Network::ITcpNetworkHandler
	{
		// 생성/소멸자
	public:
		CSessionManager();
		~CSessionManager(void);

	public:
		enum	NetworkType			//貢쭹뒀꿔돨렘駕
		{
			TCP_IOCP			= 0,
			TCP_ASYNCSELECT		= 1,
		};
		enum	QueryType			//Query렘駕
		{
			HTTP		= 1,
			ADO,
		};
		enum	AssembleType
		{
			LengthBase	= 0,
			CharBase	= 1,
		};

	private:
		struct	TagNetworkOwner
		{
			AssembleType	assembleType;
			Common::DataStructure::CMemoryPoolSafty<Message::CPacketAssembler>*
							pPacketAssembler;
			ULONG_PTR		pSessionOwner;
		};

		struct	TagPackets
		{
			CPacketPingAck	PingAck;
			CPacketAck		Ack;
		};

	private:
		struct DbArgument
		{
			CSessionManager * pThis;
			INT nIndex;
		};


		TagPackets*		m_pPackets;

		Common::DataStructure::CMemoryPoolSafty<Message::CPacketAssembler>
									m_PacketAssemblerPool;		// 랙箇句口돨뻠녑넥
		INT							m_nNumOfPacketAssembler;	// 륩蛟포돨넓潼�雍�,


		//		CRecvPacketQueue		m_RecvPacketQueue;	// Recv 패킷를 직렬화 시키는 큐
		Message::CMessageQueue		m_RecvMessageQueue;		// Recv 패킷를 직렬화 시키는 큐

		// SendPacketQueue는 따로 만들어야 한다. 
		Message::CMessageQueue		m_SendMessageQueue;		// Send 패킷를 직렬화 시키는 큐

		// Query Queue
		Message::CMessageQueue		m_QueryMessageQueue;	// http句口
		// Response Query Queue
		Message::CMessageQueue		m_ResponseMessageQueue;	// Response 청唐痰돕

		Message::CMessageQueue		m_DbReqeustMessageQueue;	// Db Request 메시지 큐
		Message::CMessageQueue		m_DbResponseMessageQueue;	// Db Response 메시지 큐
		Message::CMessageQueue		m_DbConnMessageQueue;		// Db Connection 메시지 큐

		Message::CMessage			m_SendMessage;			// Dequeue시 Send를 위한 임시 패킷
		Message::CMessage			m_SendManagePacket;		// Manage Pakcet을 위한 임시 Send 패킷
		Message::CMessage			m_HttpMessage;			// Http를 위한 임시 패킷
		int							m_nPacketSize;

		CRITICAL_SECTION			m_csSend;				// Send 동기화를 위한 객체
		CRITICAL_SECTION			m_csHttp;				// Http 동기화를 위한 객체

		Common::DataStructure::CMemoryPoolSafty<CSendPacket>
			m_SendPacketPool;		// 구痰Send코닸넥

		Common::DataStructure::CListEx<CPacketBuffer*>*
			m_pPacketBufferList;	//뎠품socket뚤壇돨矜狼랙箇돨寧莉句口관

		Common::DataStructure::CMemoryPoolSafty< Common::DataStructure::CListEx<CPacketBuffer*> >
									m_PacketBufferListPool;		// 패킷 버퍼 리스트 풀
		ULONG_PTR					m_lpRecentNetworkKey;

		Common::DataStructure::CMemoryPoolSafty<CPacketBuffer>
									m_RecvPacketBufferPool;		// Recv 패킷 버퍼 풀
		Common::DataStructure::CMemoryPoolSafty<CPacketBuffer>
									m_SendPacketBufferPool;		// Send 뻠녑

		Common::DataStructure::CMemoryPoolSafty<CDbObject>
									m_DbObjectPool;
		CDbConnObject*				m_paDbConn;
		struct TagDbConn
		{
			_bstr_t		clsServer;
			_bstr_t		clsID;
			_bstr_t		clsPass;
		}m_dbConnInfo;

		CTimer						m_Timer;

		Network::INetwork*			m_pNetwork;

		BOOL						m_bFirstRecvMsg;
		Message::CMessage			m_OldMessage;


//		int		m_nDebug;
//		int		m_nDebugAssem;

#ifndef CLIENT_MODULE
		Query::HTTP::CAsyncInet		m_InetHttp;
		Query::HTTP::CSyncInet		m_SyncInet1;
		Query::HTTP::CSyncInet		m_SyncInet2;
#endif

		// Operations
	public:
		// 패킷을 전송한다.
		VOID	SendMessage( Message::CMessage& message );
		VOID	SendMessage( ULONG_PTR pNetworkKey, const CHAR* pData, INT nDataSize, DWORD dwHandle );
		VOID	SendMessageFlush();

		// 패킷을 수신한다. // Blocking
		void	RecvMessage( Message::CMessage& outMessage );

		BOOL	IsEmptyRecvMessage()
		{
			return m_RecvMessageQueue.IsEmpty();
		}

		// 타이머를 동작시킨다.
		void		StartTimer( int nTickTime, int nMaxTickCount, int nSize );

		// 타이머를 추가한다.
		ULONG_PTR	AddTimer( int nTickCount, const Message::CSignal& signal );

		// 이벤트를 추가한다.
		void	AddSignal( const Message::CSignal& signal );

		// Accept및 Connect의 초기 설정을 마치면 Receive를 수신하기 위해서 이벤트를 호출한다.
		void	SetReceiveEvent( ULONG_PTR lpNetworkKey );

		// 패킷 Assembler를 초기화한다.
		// 연결되는 Session별로 패킷 타입이 다를 경우에는 OnAccept, OnConnect 시에 아래 함수를 호출한다.
		void	InitPacketType( 
			ULONG_PTR							lpCompletionKey,
			Message::CPacketCheckSum::CheckSumType		checkSumType,
			Message::CPacketAssembler::PacketUnitType	packetUnitType );
		void	InitLengthFieldOfPacket(
			ULONG_PTR lpCompletionKey,
			int nLengthFieldPos,
			Message::CPacketAssembler::LengthSizeType lengthSizeType,
			Message::CPacketAssembler::LengthType lengthType = Message::CPacketAssembler::LengthType::Total,
			BOOL bLittleEndian = TRUE );

		// 재정의 : CIocpServer
	public:
		// 내부 리소스를 생성한다.
		BOOL	Create( NetworkType networkType, QueryType queryType, INT nRecvQueueSize, INT nSendQueueSize, INT nQueryQueueSize, INT nEventQueueSize, INT nNumOfListener, INT nNumOfConnectSockets, INT nPacketSize = 256, HWND hWindow = NULL, UINT uiMessage = 0 );
		BOOL	CreateListener( INT nListenIndex, INT nNumOfCommSockets, INT nAcceptPoolSize, BOOL bAcceptRecvEvent = FALSE );
		BOOL	CreateEnd();


		INT		CreateDatabase( INT nDbConnectionCount, LPCTSTR lpszConnection );

		CDbObject*	AllocDbObject();
		void		FreeDbObject( CDbObject* pDb );

		// 내부 리소스를 제거한다.
		void	Destroy();

		// 해당 키의 Service 인덱스를 가져온다.
		INT		GetServiceIndex( ULONG_PTR lpCompletionKey );
		void	SetOwnerKey( ULONG_PTR lpNetworkKey, ULONG_PTR lpOwnerKey )
		{
			m_pNetwork->SetOwnerKey( lpNetworkKey, lpOwnerKey );
		}
		ULONG_PTR	getOwnerKey( ULONG_PTR lpNetworkKey )
		{
			return m_pNetwork->GetOwnerKey( lpNetworkKey );
		}

		BOOL	Start( INT nServicerIndex )
		{
			return m_pNetwork->StartListen( nServicerIndex );
		}

		BOOL	Connect( ULONG_PTR lpOwnerKey, const sockaddr* addr, INT namelen )
		{
			return m_pNetwork->Connect( lpOwnerKey, addr, namelen );
		}

		BOOL	Bind( INT nServerIndex, INT nPort, u_long hostAddr = INADDR_ANY, short family = AF_INET, INT nListenQueue = 5 )
		{
			return m_pNetwork->Bind( nServerIndex, nPort, hostAddr, family, nListenQueue );
		}

		void	FirePacket( DWORD wParam, DWORD lParam )
		{
			m_pNetwork->FirePacket( wParam, lParam );
		}

		DWORD	GetRttTime( ULONG_PTR lpNetworkKey )
		{
			DWORD dwRTT;
			m_pNetwork->GetRttTime( lpNetworkKey, dwRTT );
			return dwRTT;
		}

		LPCTSTR	GetLocalAddr( ULONG_PTR lpNetworkKey )
		{
			return m_pNetwork->GetLocalAddr( lpNetworkKey );
		}

		LPCTSTR	GetPeerAddr( ULONG_PTR lpNetworkKey )
		{
			return m_pNetwork->GetPeerAddr( lpNetworkKey );
		}

		// Implementaion
	private:
		// ITimerHandler
		virtual void	OnTimer( const Message::CSignal& signal );

		// IAsyncInetHandler
		virtual void	OnRecvInet		( ULONG_PTR lpInetKey, ULONG_PTR lpContext );
		virtual void	OnErrorInet		( ULONG_PTR lpContext, DWORD nError );

		// ITcpNetworkHandler
		virtual void	OnAccept	( ULONG_PTR lpNetworkKey );
		virtual void	OnConnect	( ULONG_PTR lpNetworkKey );
		virtual void	OnReceive	( ULONG_PTR lpNetworkKey, char* pRecvBuff, int nReceivedLenth );
		virtual void	OnSend		( ULONG_PTR lpNetworkKey, char* pRecvBuff, int nSenedLenth );
		virtual void	OnClose		( ULONG_PTR lpNetworkKey );

		// connect뵨accept珂鬧꿍
		virtual void	OnAllocNetworkObject	( ULONG_PTR lpCompletionKey );
		// 댄轎샀밑균珂딧痰
		virtual void	OnFreeNetworkObject		( ULONG_PTR lpCompletionKey );
		//랙箇窟넋샀pin돨섟珂句口珂딧痰
		void			SendPacket	( Session::Message::CMessage& message );
		void			SendPacket	( ULONG_PTR lpNetworkKey, CPacketBuffer* pPacketBuffer );

	private:
		static	DWORD __stdcall SendThreadStartingPoint(PVOID pvParam);
		static	DWORD __stdcall QueryThreadStartingPoint1(PVOID pvParam);
		static	DWORD __stdcall QueryThreadStartingPoint2(PVOID pvParam);

		static	DWORD __stdcall DbThreadStartingPoint(PVOID pvParam);
	};// class CSessionManager

}/* namespace Manage */		}/* namespace Session */