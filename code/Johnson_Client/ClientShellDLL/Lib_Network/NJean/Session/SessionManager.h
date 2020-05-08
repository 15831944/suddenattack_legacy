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

#include "../Network/TcpAsyncSelect.h"

#include "../../DataStructure/MemoryPoolSafty.h"

#include "PacketAssembler.h"
#include "MessageQueue.h"
#include "Timer.h"
#include "SendPacket.h"

#include "PacketBuffer.h"
#include "QueryBuffer.h"

namespace GamehiSolution{	namespace Session{		namespace Manage{

	class CSessionManager : public ITimerHandler, Network::ITcpNetworkHandler
	{
		// ����/�Ҹ���
	public:
		CSessionManager();
		~CSessionManager(void);

	public:
		enum	NetworkType
		{
			TCP_IOCP			= 0,
			TCP_ASYNCSELECT		= 1,
		};
		enum	QueryType
		{
			HTTP		= 1,
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
			unsigned long		pSessionOwner;
		};

		struct	TagPackets
		{
			CPacketPingAck	PingAck;
			CPacketAck		Ack;
		};

	private:
		TagPackets*		m_pPackets;

		Common::DataStructure::CMemoryPoolSafty<Message::CPacketAssembler>
			m_PacketAssemblerPool;		// ��Ŷ ������ ��ü �޸� Ǯ
		INT							m_nNumOfPacketAssembler;	// Assembler�� �ִ� ����


		//		CRecvPacketQueue		m_RecvPacketQueue;	// Recv ��Ŷ�� ����ȭ ��Ű�� ť
		Message::CMessageQueue		m_RecvMessageQueue;		// Recv ��Ŷ�� ����ȭ ��Ű�� ť

		// SendPacketQueue�� ���� ������ �Ѵ�. 
		Message::CMessageQueue		m_SendMessageQueue;		// Send ��Ŷ�� ����ȭ ��Ű�� ť

		Message::CMessage			m_SendMessage;			// Dequeue�� Send�� ���� �ӽ� ��Ŷ
		Message::CMessage			m_SendManagePacket;		// Manage Pakcet�� ���� �ӽ� Send ��Ŷ
		Message::CMessage			m_HttpMessage;			// Http�� ���� �ӽ� ��Ŷ
		int							m_nPacketSize;

		CRITICAL_SECTION			m_csSend;				// Send ����ȭ�� ���� ��ü
		CRITICAL_SECTION			m_csHttp;				// Http ����ȭ�� ���� ��ü

		Common::DataStructure::CMemoryPoolSafty<CSendPacket>
			m_SendPacketPool;		// Send Packet Ǯ

		Common::DataStructure::CMemoryPoolSafty<CPacketBuffer>
									m_RecvPacketBufferPool;		// Recv ��Ŷ ���� Ǯ
		Common::DataStructure::CMemoryPoolSafty<CPacketBuffer>
									m_SendPacketBufferPool;		// Send ��Ŷ ���� Ǯ

		CTimer						m_Timer;

		Network::INetwork*			m_pNetwork;

		BOOL						m_bFirstRecvMsg;
		Message::CMessage			m_OldMessage;


		// Operations
	public:
		// ��Ŷ�� �����Ѵ�.
		void	SendMessage( Message::CMessage& message );
		VOID	SendMessage( unsigned long pNetworkKey, const CHAR* pData, INT nDataSize );

		// ��Ŷ�� �����Ѵ�. // Blocking
		void	RecvMessage( Message::CMessage& outMessage );

		BOOL	IsEmptyRecvMessage()
		{
			return m_RecvMessageQueue.IsEmpty();
		}

		// Ÿ�̸Ӹ� ���۽�Ų��.
		void	StartTimer( int nTickTime, int nMaxTickCount, int nSize );

		// Ÿ�̸Ӹ� �߰��Ѵ�.
		void	AddTimer( int nTickCount, const Message::CSignal& signal );

		// �̺�Ʈ�� �߰��Ѵ�.
		void	AddSignal( const Message::CSignal& signal );

		// Accept�� Connect�� �ʱ� ������ ��ġ�� Receive�� �����ϱ� ���ؼ� �̺�Ʈ�� ȣ���Ѵ�.
		void	SetReceiveEvent( unsigned long lpNetworkKey );

		// ��Ŷ Assembler�� �ʱ�ȭ�Ѵ�.
		// ����Ǵ� Session���� ��Ŷ Ÿ���� �ٸ� ��쿡�� OnAccept, OnConnect �ÿ� �Ʒ� �Լ��� ȣ���Ѵ�.
		void	InitPacketType( 
			unsigned long							lpCompletionKey,
			Message::CPacketCheckSum::CheckSumType		checkSumType,
			Message::CPacketAssembler::PacketUnitType	packetUnitType );
		void	InitLengthFieldOfPacket(
			unsigned long lpCompletionKey,
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

		// ���� ���ҽ��� �����Ѵ�.
		void	Destroy();

		// �ش� Ű�� Service �ε����� �����´�.
		INT		GetServiceIndex( unsigned long lpCompletionKey );
		void	SetOwnerKey( unsigned long lpNetworkKey, unsigned long lpOwnerKey )
		{
			m_pNetwork->SetOwnerKey( lpNetworkKey, lpOwnerKey );
		}
		unsigned long	getOwnerKey( unsigned long lpNetworkKey )
		{
			return m_pNetwork->GetOwnerKey( lpNetworkKey );
		}

		BOOL	Start( INT nServicerIndex )
		{
			return m_pNetwork->StartListen( nServicerIndex );
		}

		BOOL	Connect( unsigned long lpOwnerKey, const sockaddr* addr, INT namelen )
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

		DWORD	GetRttTime( unsigned long lpNetworkKey )
		{
			DWORD dwRTT;
			m_pNetwork->GetRttTime( lpNetworkKey, dwRTT );
			return dwRTT;
		}

		LPCTSTR	GetLocalAddr( unsigned long lpNetworkKey )
		{
			return m_pNetwork->GetLocalAddr( lpNetworkKey );
		}

		LPCTSTR	GetPeerAddr( unsigned long lpNetworkKey )
		{
			return m_pNetwork->GetPeerAddr( lpNetworkKey );
		}

		// Implementaion
	private:
		// ITimerHandler
		virtual void	OnTimer( const Message::CSignal& signal );

		// IAsyncInetHandler
		virtual void	OnRecvInet		( unsigned long lpInetKey, unsigned long lpContext );
		virtual void	OnErrorInet		( unsigned long lpContext, DWORD nError );

		// ITcpNetworkHandler
		virtual void	OnAccept	( unsigned long lpNetworkKey );
		virtual void	OnConnect	( unsigned long lpNetworkKey );
		virtual void	OnBeginConnect( unsigned long lpNetworkKey );
		virtual void	OnReceive	( unsigned long lpNetworkKey, char* pRecvBuff, int nReceivedLenth );
		virtual void	OnSend		( unsigned long lpNetworkKey, char* pRecvBuff, int nSenedLenth );
		virtual void	OnClose		( unsigned long lpNetworkKey );

		// Network ��ü�� �Ҵ��� ��, �ش� ��ü�� ���� �̷�� Packet Assembler�� �Ҵ�
		virtual void	OnAllocNetworkObject	( unsigned long lpCompletionKey );
		// Network ��ü�� ��ȯ�� ��, �ش� ��ü�� ���� �̷�� Packet Assembler�� ��ȯ
		virtual void	OnFreeNetworkObject		( unsigned long lpCompletionKey );


		void			SendPacket	( Session::Message::CMessage& message, BOOL bFirst = FALSE );
		void			SendPacket	( unsigned long lpNetworkKey, CPacketBuffer* pPacketBuffer, BOOL bFirst = FALSE );

	private:
		static	DWORD __stdcall SendThreadStartingPoint(PVOID pvParam);
	};// class CSessionManager

}/* namespace Manage */		}/* namespace Session */	}/* namespace GamehiSolution */