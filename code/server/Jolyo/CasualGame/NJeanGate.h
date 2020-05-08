#pragma once

//#include <map>

#include "..\NJean\Session\SessionGate.h"
#include "..\DataStructure\MemoryPool.h"
#include "User.h"
#include "Room.h"
#include "Channel.h"

#include "UserMap.h"

namespace CasualGame
{
	using namespace Session;
	class CNJeanGate : public Session::Gate::CSessionGate //��ɫ��room�Ĺ�������Ϣ�ķַ�
	{
	public:
		CNJeanGate();
		virtual ~CNJeanGate();

	private:
		enum	ServicerIndex
		{
			Client	= 0,
		};

	private:
		Common::DataStructure::CMemoryPool<CUser>	m_UserPool;					// ����� ��ü���� �޸� Ǯ
		Common::DataStructure::CInnerManager<CRoom>	m_InnerManagerUnusedRoom;	// ������� �ʴ� �� ����Ʈ
		Common::DataStructure::CInnerManager<CRoom>	m_InnerManagerUsedRoom;		// ������� �� ����Ʈ
		
		CRoom*					m_aRoom;					// Room �迭
		CChannel*				m_aChannel;					// ä�� �迭
		INT						m_nAliveManaged;			// Ÿ�Ӿƿ� ó���� ���� ��ü�� ����

		BOOL*					m_baTmpRoomInnerIndex;		// m_nMaxRoomUserũ�⸸ŭ ����

		//////////////////////////////////////////////////////////////
		// SesstionGate�� Method�� �����.

		virtual BOOL	Create( Manage::CSessionManager::NetworkType networkType, Manage::CSessionManager::QueryType queryType, INT nRecvQueueSize, INT nSendQueueSize, INT nQueryQueueSize, INT nSignalQueueSize, INT nNumOfListener, INT nNumOfConnectSockets, INT nPacketSize, INT nResponseBuffer, HWND hWindow, UINT uiMessage )
		{ return FALSE; }

		VOID			CloseRequest( Application::CPeer* pPeer )
		{
			CSessionGate::CloseRequest( pPeer );	
		}
		VOID			RequestHttp( Application::CPeer* pPeer, UINT uiMessage, LPCTSTR lpszURL )
		{
			CSessionGate::RequestHttp( pPeer, uiMessage, lpszURL );
		}
		VOID			RequestHttp( Application::CPeer* pPeer, UINT uiMessage, LPCTSTR lpszAddr, LPCTSTR lpszObjectName, LPCTSTR lpszBody )
		{
			CSessionGate::RequestHttp( pPeer, uiMessage, lpszAddr, lpszObjectName, lpszBody );
		}
		VOID			SendData( Application::CPeer* pPeer, const char* pData, INT nDataLength )
		{
			CSessionGate::SendData( pPeer, pData, nDataLength );
		}


		///////////////////////////////////////////////////////////////
		// Implementation

		virtual VOID		OnSignal	( UINT uiMessage, ULONG_PTR wParam, ULONG_PTR lParam );
		virtual VOID		OnResponse	( Application::CPeer* pPeer, UINT uiMessage, DWORD dwError, INT nResult, CHAR* pData, INT nDataSize );
		virtual VOID		OnResponse	( Application::CPeer* pPeer, UINT uiMessage, BOOL bSuccess, _CommandPtr& pCmd );
		virtual VOID		OnAccept	( Application::CPeer* pPeer, INT nServicerIndex );
		virtual VOID		OnConnect	( Application::CPeer* pPeer, INT nServicerIndex );
		virtual VOID		OnClose		( Application::CPeer* pPeer, INT nServicerIndex );
		virtual VOID		OnData		( Application::CPeer* pPeer, INT nServicerIndex, CHAR* pPacket, INT nPacketLength );
		virtual VOID		OnTimeout	( Application::CPeer* pPeer );

		virtual Application::CPeer*
							AllocPeer	( INT nServicerIndex );
		virtual VOID		FreePeer	( Application::CPeer* pPeer, INT nServicerIndex );

		virtual VOID		InitListenerPacketType( 
			INT											nServicerIndex,
			Message::CPacketCheckSum::CheckSumType&		out_CheckSumType,
			Message::CPacketAssembler::PacketUnitType&	out_PacketUnitType );

		virtual VOID		InitLengthFieldOfPacket(
			INT											nServicerIndex,
			INT&										out_nLengthFieldPos,
			Message::CPacketAssembler::LengthSizeType&	out_lengthSizeType,
			Message::CPacketAssembler::LengthType&		out_lengthType,
			BOOL&										out_bLittleEndian );

		virtual BOOL		IsAliveManageObject( INT nServicerIndex );


		////////////////////////////////////////////////////////////////
		// Operations

		/////////////////////////////////////////////////////////////////////////////////////////////
		// �ñ׳� ó��
		VOID		SignalProcessChangeCaptine			( ULONG_PTR wParam, ULONG_PTR lParam );
		VOID		SignalProcessCompulseExit			( ULONG_PTR wParam, ULONG_PTR lParam );
		VOID		SignalProcessCompulseDisconnect		( ULONG_PTR wParam, ULONG_PTR lParam );


		/////////////////////////////////////////////////////////////////////////////////////////////
		// ��Ŷ ó��

		// �α��� ��û ó��
		VOID		PacketProcessLoginRequest			( CUser* pUser, CCasualPacketLoginRequest* pCasualPacket );

		// ��ȣȭ�� �α��� ��û ó��
		VOID		PacketProcessSecLoginRequest		( CUser* pUser, CCasualPacketSecLoginRequest* pCasualPacket );

		// ä�� ���� ��û ó��
		VOID		PacketProcessJoinChannelRequest		( CUser* pUser, CCasualPacketJoinChannelRequest* pCasualPacket );

		// ä�� ���� ��û ó��
		VOID		PacketProcessLeaveChannelRequest	( CUser* pUser, CCasualPacketLeaveChannelRequest* pCausalPacket );

		// �� ���� ��û ó��
		VOID		PacketProcessOpenRoomRequest		( CUser* pUser, CCasualPacketOpenRoomRequest* pCasualPacket );

		// �� ���� ��û ó��
		VOID		PacketProcessJoinRoomRequest		( CUser* pUser, CCasualPacketJoinRoomRequest* pCasualPacket );

		// �濡�� ������ ��û ó��
		VOID		PacketProcessLeaveRoomRequest		( CUser* pUser, CCasualPacketLeaveRoomRequest* pCasualPacket );

		// �濡 �ڵ� ���� ��û ó��
		VOID		PacketProcessAutoJoinRoomRequest	( CUser* pUser, CCasualPacketAutoJoinRoomRequest* pCasualPacket );

		// ����� ã�� ��û ó��
		VOID		PacketProcessFindRequest			( CUser* pUser, CCasualPacketFindRequest* pCasualPacket );

		// ä�� �޽���
		VOID		PacketProcessChat					( CUser* pUser, CCasualPacketChat* pCasualPacket );

		// ����
		VOID		PacketProcessMemo					( CUser* pUser, CCasualPacketMemo* pCasualPacket );

		// �ʴ��ϱ�
		VOID		PacketProcessInvite					( CUser* pUser, CCasualPacketInvite* pCasualPacket );

		// ���� ��û
		VOID		PacketProcessCompulseExitRequest	( CUser* pUser, CCasualPacketCompulsionExitRequest* pCasualPacket );

		// P2P���� ������ �� �� �ִ����� �����Ѵ�.
		VOID		InspectP2PCaptine				( CUser* pUser, LPCTSTR strIP );


	protected:
		////////////////////////////////////////////////////////////////////////////////////
		// Types

		// ���� Ÿ��
		enum	PocketType
		{
			User,			// ����� ����
			Room,			// �� ����
			Channel,		// ä�� ����
		};

		// ��Ŷ �����
		enum	PacketCondition
		{
			JoinChannel,		// ä�ο� ����
			OpenRoom,			// �� ����
			JoinRoom,			// �濡 ����
			AutoJoinRoom,		// �濡 �ڵ� ����
			LeaveRoom,			// �濡�� ����
			ChangeCaptine,		// ������ ���������� �˻�
			CompulseExit,		// �����ų �� �ִ��� ���θ� �˻�
		};

		////////////////////////////////////////////////////////////////////////////////////
		// Attributes

		CUserMap				m_UserMap;					// ����ڵ��� �˻��ϱ� ���� ��

		INT						m_nMaxRoomUser;				// �Ϲ� Room�� �� �� �ִ� �ִ� ����� ��
		INT						m_nMaxRoom;					// ������ �� �ִ� �ִ� Room�� ��
		INT						m_nNumOfClients;			// �ִ� ���� �������� ��
		INT						m_nMaxChannel;				// ä���� �ִ� ����

		INT						m_nPort;					// ��Ʈ ��ȣ

		TLibPackets*				m_pCasualPackets;			// Peer�鿡�� ������ ��Ŷ��


		////////////////////////////////////////////////////////////////////////////////////
		// Operations

		// ������� ������ �����Ų��.
		VOID			CloseUser( CUser* pUser )
		{
			CSessionGate::CloseRequest( pUser );
		}

		// Http ������ ��û�Ѵ�. : Get ���
		VOID			Request( CUser* pUser, UINT uiMessage, LPCTSTR lpszURL )
		{
			CSessionGate::RequestHttp( pUser, uiMessage, lpszURL );
		}
		// Http ������ ��û�Ѵ�. : Get ���
		VOID			Request( UINT uiMessage, LPCTSTR lpszURL )
		{
			CSessionGate::RequestHttp( uiMessage, lpszURL );
		}

		// Http ������ ��û�Ѵ�. : Post ���
		VOID			Request( CUser* pUser, UINT uiMessage, LPCTSTR lpszAddr, LPCTSTR lpszObjectName, LPCTSTR lpszBody )
		{
			CSessionGate::RequestHttp( pUser, uiMessage, lpszAddr, lpszObjectName, lpszBody );
		}

		// ��Ŷ�� �����Ѵ�.
		VOID			SendPacket( CUser* pUser, const CPacket* pPacket, INT nPacketLength )
		{
			CSessionGate::SendData( pUser, reinterpret_cast<const char*>(pPacket), nPacketLength );
		}

		// ���� Ref�� ��ȯ�Ѵ�.
		CRoom*		GetRoomRef						( CUser*	pUser		);
		CRoom*		GetRoomRef						( INT		nRoomIndex	);

		// ä���� Ref�� ��ȯ�Ѵ�.
		CChannel*	GetChannelRef					( CUser*	pUser		);
		CChannel*	GetChannelRef					( INT		nChannelIndex );

		// �ش� ���� ���������� ��ȯ�Ѵ�.
		BOOL		IsWaitRoom						( CRoom*	pRoom		);
		BOOL		IsWaitRoom						( INT		nRoomIndex	);

		// �ش� ä���� ���ä�������� ��ȯ�Ѵ�.
		BOOL		IsWaitChannel					( CChannel*	pChannel	);
		BOOL		IsWaitChannel					( INT		nChannel	);

		// �ش� ���� ���� ä���� Ref�� ��ȯ�Ѵ�.
		CChannel*	GetChannelRefOfRoom				( CRoom*	pRoom		);
		CChannel*	GetChannelRefOfRoom				( INT		nRoomIndex	);

		// �ش� �濡 �޽����� �߰��Ѵ�.
		VOID		SendToRoom						( CRoom* pRoom, const CUser* pExeptUser, const CPacket* pPacket, INT nPacketSize );

		// ���� ���� ��� ����ڿ��� �޽����� �����Ѵ�.
		VOID		SendToAllUser					( const CPacket* pPacket, INT nPacketSize );

		// ä�� ���� ��� ����ڿ��� �޽����� �����Ѵ�.
		VOID		SendToAllUser					( CChannel* pChannel, const CPacket* pPacket, INT nPacketSize );

		// ���� ���� ��� ���� �濡 �޽����� �����Ѵ�.
		VOID		SendToAllRoomUser				( const CPacket* pPacket, INT nPacketSize );

		// ä�� ���� ��� ���� �濡 �޽����� �����Ѵ�.
		VOID		SendToAllRoomUser				( CChannel* pChannel, const CPacket* pPacket, INT nPacketSize );

		// ä�� ���� ���ǿ� �޽����� �����Ѵ�.
		VOID		SendToWaitRoom					( CChannel* pChannel, const CUser* pExeptUser, const CPacket* pPacket, INT nPacketSize );

		// ������� ��ġ�� ��ȯ�Ѵ�.
		VOID		GetUserPlace					( CUser* pUser, CChannel* &out_pChannel, CRoom* &out_pRoom, BOOL &out_bWaitRoom );

		// �α��� ������ �����Ѵ�.
		VOID		LoginReply						( CUser* pUser, BOOL bLogined );

		// �α��� ������ �����Ѵ�.
		VOID		LoginQueryRequest				( CUser* pUser, LPCTSTR strLoginQuery );

		INT			GetUserCount					()
		{
			return m_UserPool.GetAllocatedSize();
		}

		// XML�� �Ľ��Ѵ�. : ����
		VOID		UrlParsing						( CHAR* pStr );
		CHAR		HexToChar						( CHAR* pStr );



		////////////////////////////////////////////////////////////////////////////////////
		// Operations : ����Ŭ�������� ������ �Ͽ� ���

		virtual VOID		OnLoginSuccess			( CUser* pUser );
		virtual VOID		OnOpenRoomReply			( CUser* pUser, CRoom* pRoom );
		virtual VOID		OnJoinReply				( CUser* pUser, CRoom* pRoom );
		virtual VOID		OnAutoJoinReply			( CUser* pUser, CRoom* pRoom );
		virtual VOID		OnLeaveRoom				( CUser* pUser, CRoom* pRoom, CCasualPacketCompulsionExited::Type ExitType ){}
		virtual VOID		OnChangeCaptine( CUser* pCaptineUser, CRoom* pRoom );
		
		// Pocket�� �����Ѵ�.
		virtual CPocket*	NewPocket( PocketType type )	{ return NULL; }

		// Pocket�� �����Ѵ�.
		virtual VOID		DeletePocket( PocketType type, CPocket* pPocket ){}

		// ä���� �ʱ�ȭ �Ѵ�.
		virtual VOID		InitChannel( INT nChannelIndex, INT& out_nMaxUsers, INT& out_nMaxRooms, string& out_strChannelName ){}

		virtual VOID		OnCasualSignal		( UINT uiMessage, ULONG_PTR wParam, ULONG_PTR lParam ){}
		virtual VOID		OnCasualResponse	( CUser* pUser, UINT uiMessage, DWORD dwError, INT nResult, CHAR* pData, INT nDataSize ){}
		virtual VOID		OnCasualAccept		( CUser* pUser ){}
		virtual VOID		OnCasualConnect		( CUser* pUser ){}
		virtual VOID		OnCasualClose		( CUser* pUser ){}
		virtual VOID		OnCasualData		( CUser* pUser, CPacket* pPacket, INT nPacketLength ){}
		virtual BOOL		OnCasualTimeout		( CUser* pUser )
		{
			return TRUE;
		}
		virtual VOID		OnCasualResponse	( CUser* pUser, UINT uiMessage, BOOL bSuccess, _CommandPtr& pCmd ){}

		// �α��� ���� ���� �߻���
		virtual VOID	OnResponseLoginQuery	( CUser* pUser, DWORD dwError, INT nResult, CHAR* pData, INT nDataSize );

		// CNJeanGate���� Ư�� ������ ���� Ŭ�������� ������ �� ���
//			virtual BOOL	IsCorrectCondition		( CUser* pUser, PacketCondition condition, ULONG_PTR rParam = NULL, ULONG_PTR lParam = NULL )
//			{
//				return TRUE;
//			}

		virtual BOOL	IsCorrectCondition		( SHORT &out_nResult, CUser* pUser, PacketCondition condition, ULONG_PTR rParam = NULL, ULONG_PTR lParam = NULL )
		{
			return TRUE;
		}

		// ��ȣȭ �Ǿ� ��Ʈ�����·� ���޵� �α��� ��Ŷ ����
		virtual VOID	OnPacketSecLogin		( CUser* pUser, const char* pSecData, INT nDataSize, const char* pSessionKey, INT nKeyLength ){}

		// �Ϲ� �α��� ��Ŷ ����
		virtual VOID	OnPacketLogin			( CUser* pUser, LPCTSTR strUserNo, LPCTSTR strID, LPCTSTR strNick );

		// ������ �� �� �ִ��� ���θ� ��ȯ
		virtual BOOL	CanBeCaptine			( CUser* pUser )
		{
			return pUser->CanBeGameCaptine() && pUser->CanBeP2PCaptine();
		}

		// ������ ���� ���� ������ ��
		virtual VOID	OnNoneCaptine			( CRoom* pRoom ){}

	public:
		// ���� ����
		// nMaxPacketSize			: ������ ��Ŷ�� ���� �ִ� ũ��
		// nMaxClients				: �ִ�� ���� ����� Ŭ���̾�Ʈ�� ����
		// nMaxResponsebufferSize	: Http Response�� �ִ� ũ��
		// nMaxRoomUser				: �� �濡 ������ �� �ִ� �ִ� ������� ���� : ���� ����
		// nMaxChannels				: ä���� �ִ� ����
		// nPort					: ��Ʈ ��ȣ
		virtual BOOL	Create( INT nMaxPacketSize, INT nMaxClients, INT nMaxResponseBufferSize, INT nMaxRoomUser, INT nMaxChannels, INT nPort );

		// ���� ����
		virtual BOOL	Start();

		// ���� �Ҹ�
		virtual VOID	Destroy();
	};
}