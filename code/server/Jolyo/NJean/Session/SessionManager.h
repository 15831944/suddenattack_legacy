/*****************************************************************************************************************
	CSessionManager

	�ۼ��� : ������
	�ۼ��� : 2004�� 3�� 4 ~ 5��

	��  �� :
		IOCP�� �̿��� Server�� ���
		OnReceive �̺�Ʈ�� �޽��� ������ �ƴ�, ������ ��Ŷ ������ ȣ��
	��  �� :
		- CIocp
		1. Create�� ����Ͽ� Iocp Server�� �����Ѵ�.
		2. Bind�� Listen�� �Ѵ�. �� ���� �⺻ ���� ���α׷��� �����ϴ�.
		3. Start�� ���ؼ� Iocp Server�� �����Ų��.
		4. Iocp Server�� ���� �ڵ�κ��� OnAccept, OnReceive, OnSend, OnClose �̺�Ʈ�� �߻���Ų��.
		5. Send�� ���ؼ� Iocp Server�� ����Ǿ� �ִ� Ŭ���̾�Ʈ���� �޽����� �����Ѵ�.
		6. CloseSocket�� ���ؼ� Iocp Server�� ����Ǿ� �ִ� Ư�� ������ �ݴ´�.
		7. ��� ����� �ڿ��� Destroy�� ���� ���� ���ҽ��� ���Ÿ� �Ѵ�.

		- CSessionManager
		1. CIocp�� 3�� Start�� �ϱ����� ��Ŷ�� ���õ� �ʱ�ȭ�� ���ش�.
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
		// ����/�Ҹ���
	public:
		CSessionManager();
		~CSessionManager(void);

	public:
		enum	NetworkType			//����ײ�ķ�ʽ
		{
			TCP_IOCP			= 0,
			TCP_ASYNCSELECT		= 1,
		};
		enum	QueryType			//Query��ʽ
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
									m_PacketAssemblerPool;		// ������Ϣ�Ļ����
		INT							m_nNumOfPacketAssembler;	// �������ĳ������,


		//		CRecvPacketQueue		m_RecvPacketQueue;	// Recv ��Ŷ�� ����ȭ ��Ű�� ť
		Message::CMessageQueue		m_RecvMessageQueue;		// Recv ��Ŷ�� ����ȭ ��Ű�� ť

		// SendPacketQueue�� ���� ������ �Ѵ�. 
		Message::CMessageQueue		m_SendMessageQueue;		// Send ��Ŷ�� ����ȭ ��Ű�� ť

		// Query Queue
		Message::CMessageQueue		m_QueryMessageQueue;	// http��Ϣ
		// Response Query Queue
		Message::CMessageQueue		m_ResponseMessageQueue;	// Response û���õ�

		Message::CMessageQueue		m_DbReqeustMessageQueue;	// Db Request �޽��� ť
		Message::CMessageQueue		m_DbResponseMessageQueue;	// Db Response �޽��� ť
		Message::CMessageQueue		m_DbConnMessageQueue;		// Db Connection �޽��� ť

		Message::CMessage			m_SendMessage;			// Dequeue�� Send�� ���� �ӽ� ��Ŷ
		Message::CMessage			m_SendManagePacket;		// Manage Pakcet�� ���� �ӽ� Send ��Ŷ
		Message::CMessage			m_HttpMessage;			// Http�� ���� �ӽ� ��Ŷ
		int							m_nPacketSize;

		CRITICAL_SECTION			m_csSend;				// Send ����ȭ�� ���� ��ü
		CRITICAL_SECTION			m_csHttp;				// Http ����ȭ�� ���� ��ü

		Common::DataStructure::CMemoryPoolSafty<CSendPacket>
			m_SendPacketPool;		// ����Send�ڴ��

		Common::DataStructure::CListEx<CPacketBuffer*>*
			m_pPacketBufferList;	//��ǰsocket��Ӧ����Ҫ���͵�һ����Ϣ��

		Common::DataStructure::CMemoryPoolSafty< Common::DataStructure::CListEx<CPacketBuffer*> >
									m_PacketBufferListPool;		// ��Ŷ ���� ����Ʈ Ǯ
		ULONG_PTR					m_lpRecentNetworkKey;

		Common::DataStructure::CMemoryPoolSafty<CPacketBuffer>
									m_RecvPacketBufferPool;		// Recv ��Ŷ ���� Ǯ
		Common::DataStructure::CMemoryPoolSafty<CPacketBuffer>
									m_SendPacketBufferPool;		// Send ����

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
		// ��Ŷ�� �����Ѵ�.
		VOID	SendMessage( Message::CMessage& message );
		VOID	SendMessage( ULONG_PTR pNetworkKey, const CHAR* pData, INT nDataSize, DWORD dwHandle );
		VOID	SendMessageFlush();

		// ��Ŷ�� �����Ѵ�. // Blocking
		void	RecvMessage( Message::CMessage& outMessage );

		BOOL	IsEmptyRecvMessage()
		{
			return m_RecvMessageQueue.IsEmpty();
		}

		// Ÿ�̸Ӹ� ���۽�Ų��.
		void		StartTimer( int nTickTime, int nMaxTickCount, int nSize );

		// Ÿ�̸Ӹ� �߰��Ѵ�.
		ULONG_PTR	AddTimer( int nTickCount, const Message::CSignal& signal );

		// �̺�Ʈ�� �߰��Ѵ�.
		void	AddSignal( const Message::CSignal& signal );

		// Accept�� Connect�� �ʱ� ������ ��ġ�� Receive�� �����ϱ� ���ؼ� �̺�Ʈ�� ȣ���Ѵ�.
		void	SetReceiveEvent( ULONG_PTR lpNetworkKey );

		// ��Ŷ Assembler�� �ʱ�ȭ�Ѵ�.
		// ����Ǵ� Session���� ��Ŷ Ÿ���� �ٸ� ��쿡�� OnAccept, OnConnect �ÿ� �Ʒ� �Լ��� ȣ���Ѵ�.
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

		// ������ : CIocpServer
	public:
		// ���� ���ҽ��� �����Ѵ�.
		BOOL	Create( NetworkType networkType, QueryType queryType, INT nRecvQueueSize, INT nSendQueueSize, INT nQueryQueueSize, INT nEventQueueSize, INT nNumOfListener, INT nNumOfConnectSockets, INT nPacketSize = 256, HWND hWindow = NULL, UINT uiMessage = 0 );
		BOOL	CreateListener( INT nListenIndex, INT nNumOfCommSockets, INT nAcceptPoolSize, BOOL bAcceptRecvEvent = FALSE );
		BOOL	CreateEnd();


		INT		CreateDatabase( INT nDbConnectionCount, LPCTSTR lpszConnection );

		CDbObject*	AllocDbObject();
		void		FreeDbObject( CDbObject* pDb );

		// ���� ���ҽ��� �����Ѵ�.
		void	Destroy();

		// �ش� Ű�� Service �ε����� �����´�.
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

		// connect��acceptʱע��
		virtual void	OnAllocNetworkObject	( ULONG_PTR lpCompletionKey );
		// �����ر�ʱ����
		virtual void	OnFreeNetworkObject		( ULONG_PTR lpCompletionKey );
		//�����̻߳�pin�ļ�ʱ��Ϣʱ����
		void			SendPacket	( Session::Message::CMessage& message );
		void			SendPacket	( ULONG_PTR lpNetworkKey, CPacketBuffer* pPacketBuffer );

	private:
		static	DWORD __stdcall SendThreadStartingPoint(PVOID pvParam);
		static	DWORD __stdcall QueryThreadStartingPoint1(PVOID pvParam);
		static	DWORD __stdcall QueryThreadStartingPoint2(PVOID pvParam);

		static	DWORD __stdcall DbThreadStartingPoint(PVOID pvParam);
	};// class CSessionManager

}/* namespace Manage */		}/* namespace Session */