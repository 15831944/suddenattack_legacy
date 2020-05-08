#include "StdAfx.h"

#include "NJeanGate.h"
#include "CasualPacket.h"

#include "Des.h"

#include <time.h>

#define USING_USER_NO


using namespace CasualGame;
using namespace Application;
using namespace Session::Message;
using namespace Session::Manage;
using namespace Session::Gate;
using namespace Common::DataStructure;

CNJeanGate::CNJeanGate()
:	m_aRoom	( NULL	)
{
//	CLog::SetOutputLevel( CLog::LogLevel::None, CLog::LogLevel::Always, CLog::LogLevel::Always );
//	CLog::OpenFile( "c:\\jolyosorry_log.txt" );
}

CNJeanGate::~CNJeanGate()
{
//	CLog::CloseFile();
}

BOOL CNJeanGate::Create( INT nMaxPacketSize, INT nMaxClients, INT nMaxResponseBufferSize, INT nMaxRoomUser, INT nMaxChannels, INT nPort )
{
	INT		nTickTime;				// Tick �߻� Ÿ�� (ms ����)
	INT		nMaxTick;				// ����� Ÿ�̸��� �ִ� Tick (�ݵ�� Max���� ���� �ʾƵ� �ȴ�. �ٸ� �� ���� �Ѵ� ������ŭ�� ������尡 �ִ�.
	INT		nTimerSignalQueueSize;	// Ÿ�̸Ӱ� �ѹ� �߻���Ű�� �ñ׳��� ���� : Create���� ������ �ִ� �ñ׳��� ������ �Ѿ�� �ȵȴ�.
	INT		nAliveTickError;		// Alive ó���� ���� Timeout Tick���� ���� ��� ����
	INT		nAliveTimeoutCount;		// nAliveTimeoutCount * nAliveTickError = Timeout Tick

	nTickTime				= 1000;
	nMaxTick				= 10;
	nTimerSignalQueueSize	= 500;
	nAliveTickError			= 3;
	nAliveTimeoutCount		= 10;		// 30�� ������ Ping üũ
	m_nPort	= nPort;


//	INT		nMaxPacketSize			= 8192;

	CSessionManager::NetworkType
			networkType				= CSessionManager::NetworkType::TCP_IOCP;
	CSessionManager::QueryType
			queryType				= CSessionManager::QueryType::HTTP;
	INT		nRecvQueueSize			= 1000;
	INT		nSendQueueSize			= 3000;
	INT		nQueryQueueSize			= 1000;
	INT		nSignalQueueSize		= 50000;
	INT		nNumOfListener			= 1;
	INT		nPacketSize				= 256;

	BOOL bRet = CSessionGate::Create( nTickTime, nMaxTick, nTimerSignalQueueSize, nAliveTickError, nAliveTimeoutCount, m_nAliveManaged, networkType, queryType, nRecvQueueSize, nSendQueueSize, nQueryQueueSize, nSignalQueueSize,
									  nNumOfListener, nMaxClients, nPacketSize,	nMaxResponseBufferSize );
	if( !bRet ) return FALSE;

	//
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�. : n���� Listener�� Create ������ �ڿ� CreateEnd�� ȣ���Ѵ�.
	//


	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// �ۼ��� : ������
	// �ۼ��� : 2004�� 5�� 21��
	// ��  �� : Client Listener ����
	//		1. NJean�� ����Ǵ� ��Ʈ��ũ�� 1���� Listen�̸� ServicerIndex::Client�� �ε����� ������.
	//

	INT		nServicerIndex;			// ����Ǵ� ���� �ε���
	INT		nNumOfCommSockets;		// �ش� ���񼭸� ���� ����� �� �ִ� Ŭ���̾�Ʈ�� ����
	INT		nAcceptPoolSize;		// �ش� ������ Accept Pool ũ��

	// Listener ����
	nServicerIndex		= ServicerIndex::Client;
	nNumOfCommSockets	= nMaxClients;
	nAcceptPoolSize		= 4;
	bRet = CreateListener( nServicerIndex, nNumOfCommSockets, nAcceptPoolSize );
	if( !bRet ) return FALSE;

	// �ش� Listener�� Client���� �޸� Ǯ ����
	bRet = m_UserPool.Create( nNumOfCommSockets );
	CUser** paUser = new CUser*[nNumOfCommSockets];
	for( INT i = 0; i < nNumOfCommSockets; i++ )
	{
		paUser[i]				= m_UserPool.Alloc();
		paUser[i]->Create		( nMaxPacketSize );
		paUser[i]->SetPocket	( NewPocket( PocketType::User ) );
	}
	for( INT i = 0; i < nNumOfCommSockets; i++ )
	{
		m_UserPool.Free( paUser[i] );
	}
	delete[] paUser;

	if( !bRet ) return FALSE;

	m_nNumOfClients		= nNumOfCommSockets;
	m_nAliveManaged		= nNumOfCommSockets;

	// Listener ������ ��
	bRet = CreateEnd();
	if( !bRet ) return FALSE;
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////




	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// �ۼ��� : ������
	// �ۼ��� : 2004�� 5�� 22��
	// ��  �� : Room�� ���õ� ��ü �ʱ�ȭ
	//
	m_nMaxRoom			= m_nNumOfClients;
	m_nMaxRoomUser		= nMaxRoomUser;

	m_baTmpRoomInnerIndex	= new BOOL[m_nMaxRoomUser];

	m_InnerManagerUnusedRoom	.Init();
	m_InnerManagerUsedRoom		.Init();


	// ��ü Room�� �ѹ��� ã�� ���� InnerIndex ���
	m_aRoom = new CRoom[m_nMaxRoom];
	for( INT i = 0; i < m_nMaxRoom; i++ )
	{
		m_aRoom[i].SetRoomIndex	( i );
		m_aRoom[i].SetPocket	( NewPocket( PocketType::Room ) );
	}


	//
	// ä�� ���� : �ӽ÷� ä�� 2��, ä�δ� Room 20���� �Ѵ�. ����� 200���� �Ѵ�.
	//
	INT		nRooms;	// ä�δ� ���� ����
	INT		nUsers; // ä�δ� ����� ����
	string	strChannelName;

	m_nMaxChannel	= nMaxChannels + 1;		// 0 ����û�н���channel�Ľ�ɫ�Ĺ���

	m_aChannel = new CChannel[m_nMaxChannel];
	for( INT i = 1; i < m_nMaxChannel; i++ )
	{
		// ���� Ŭ�������� nUsers, nRooms, strChannelName�� �����´�.
		InitChannel( i, nUsers, nRooms, strChannelName );

		m_aChannel[i].Init( nUsers, nRooms, strChannelName, i );

		m_aChannel[i].SetPocket	( NewPocket( PocketType::Channel ) );

		m_aRoom[i-1].Init( i );
		m_aRoom[i-1].Open( nUsers, "����" );
		m_aChannel[i].SetWaitRoom( &m_aRoom[i-1] );
	}
	m_aChannel[0].Init( m_nNumOfClients, 0, "û��channel", 0 );

//	for( INT i = m_nMaxChannel - 1; i < m_nMaxRoom; i++ )
	for( INT i = m_nMaxRoom - 1; i >= m_nMaxChannel - 1; i-- )//������ Ϊʲô����m_nMaxRoom
	{
		m_InnerManagerUnusedRoom.Add( m_aRoom[i].GetInnerObjectForGame() );
	}


	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////


	// ��Ŷ�� ����
	m_pCasualPackets	= new TLibPackets;
	
	

	return TRUE;
}

BOOL CNJeanGate::Start()
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// �ۼ��� : ������
	// �ۼ��� : 2004�� 5�� 21��
	// ��  �� : Client Listener Accept ����
	//
	INT		nServicerIndex;			// ����Ǵ� ���� �ε���

	nServicerIndex		= ServicerIndex::Client;
	StartListen( nServicerIndex, m_nPort );
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	INT		nAliveTickError;		// Alive ó���� ���� Timeout Tick���� ���� ��� ����

	nAliveTickError			= 3;
	
	// Start�� blocking �Լ��̴�.
	// Start�� ��� ������ �޽����� ó���ϴ� �κ��� ��� �ִ�.
	// ���� return FALSE�� ����ٸ�, Start �ʱ�ȭ ������ ����̴�.
	return CSessionGate::Start( nAliveTickError );
}

VOID CNJeanGate::Destroy()
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// �ۼ��� : ������
	// �ۼ��� : 2004�� 5�� 21��
	// ��  �� : User Pool�� �����Ѵ�.
	//
	m_UserPool.Destroy();
	/*
	if( m_aRoom )
	{
		for( INT i = 0; i < m_nMaxRoom; i++ )
		{
			m_aRoom[i].Destroy();
		}
		delete[] m_aRoom;
	}
	*/
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	CSessionGate::Destroy();
}


CPeer* CNJeanGate::AllocPeer( INT nServicerIndex )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.

	CPeer* pPeer;

	switch( nServicerIndex )
	{
	case ServicerIndex::Client :
		if( m_UserPool.GetAllocatedSize() < m_nNumOfClients )
			pPeer	= m_UserPool.Alloc();
		else
			pPeer	= NULL;
		break;

	default :
		pPeer	= NULL;
		break;
	}

	return pPeer;
}

VOID CNJeanGate::FreePeer( CPeer* pPeer, INT nServicerIndex )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.

	switch( nServicerIndex )
	{
	case ServicerIndex::Client :
		m_UserPool.Free( (CUser*)pPeer );
		break;

	default :
		break;
	}
}

VOID CNJeanGate::InitListenerPacketType(
		INT									nServicerIndex,
		CPacketCheckSum::CheckSumType&		out_CheckSumType,
		CPacketAssembler::PacketUnitType&	out_PacketUnitType )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.

	switch( nServicerIndex )
	{
	case ServicerIndex::Client :
		out_CheckSumType	= CPacketCheckSum::CheckSumType::TwoByteZoroOne;
		out_PacketUnitType	= CPacketAssembler::PacketUnitType::Length;
		break;

	default :
		break;
	}
}

VOID CNJeanGate::InitLengthFieldOfPacket(
		INT									nServicerIndex,
		INT&								out_nLengthFieldPos,
		CPacketAssembler::LengthSizeType&	out_lengthSizeType,
		CPacketAssembler::LengthType&		out_lengthType,
		BOOL&								out_bLittleEndian )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.

	switch( nServicerIndex )
	{
	case ServicerIndex::Client :
		out_nLengthFieldPos		= CPacket::GetLengthPos();
		out_lengthSizeType		= CPacketAssembler::LengthSizeType::TwoByte;
		out_lengthType			= CPacketAssembler::LengthType::Total;
		out_bLittleEndian		= TRUE;
		break;

	default :
		break;
	}
}

BOOL CNJeanGate::IsAliveManageObject( INT nServicerIndex )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.

	switch( nServicerIndex )
	{
	case ServicerIndex::Client :
		return TRUE;

	default :
		return FALSE;
	}
}



VOID CNJeanGate::OnSignal( UINT uiMessage, ULONG_PTR wParam, ULONG_PTR lParam )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	switch( uiMessage )
	{
	case CCasualCommon::CasualSignalMessage::ChangeCaptineSignal :
		SignalProcessChangeCaptine( wParam, lParam );
		break;

	case CCasualCommon::CasualSignalMessage::CompulseExitSignal :
		SignalProcessCompulseExit( wParam, lParam );
		break;

	default:
		OnCasualSignal( uiMessage, wParam, lParam );
	}
}

VOID CNJeanGate::OnResponse( CPeer* pPeer, UINT uiMessage, BOOL bSuccess, _CommandPtr& pCmd )
{
	OnCasualResponse( static_cast<CUser*>( pPeer ), uiMessage, bSuccess, pCmd );
}


VOID CNJeanGate::OnResponse( CPeer* pPeer, UINT uiMessage, DWORD dwError, INT nResult, CHAR* pData, INT nDataSize )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	switch( uiMessage )
	{
	case CCasualCommon::CasualQueryMessage::LoginQuery :
		OnResponseLoginQuery( static_cast<CUser*>( pPeer ), dwError, nResult, pData, nDataSize );
		break;

		/*
	case CCasualCommon::CasualQueryMessage::TestQuery :
		memcpy( m_pCasualPackets->TestReply.Data, pData, nDataSize );
		m_pCasualPackets->TestReply.Data[nDataSize] = NULL;
		m_pCasualPackets->TestReply.SetSize( nDataSize + 1 );
		SendData( pPeer, (CHAR*)&m_pCasualPackets->TestReply, m_pCasualPackets->TestReply.GetLength() );
		break;
		*/

	default :
		OnCasualResponse( static_cast<CUser*>( pPeer ), uiMessage, dwError, nResult, pData, nDataSize );
	}
}

VOID CNJeanGate::OnResponseLoginQuery( CUser* pUser, DWORD dwError, INT nResult, CHAR* pData, INT nDataSize )
{
	LoginReply( pUser, TRUE );
}


VOID CNJeanGate::OnAccept( CPeer* pPeer, INT nServicerIndex )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	switch( nServicerIndex )
	{
	case ServicerIndex::Client :
		break;

	default:
		return;
	}

	CUser* pUser = (CUser*) pPeer;

	pUser->SetLogined		( FALSE	);		// �α��� ���� �ƴ�
	pUser->SetService		( FALSE	);		// ���� ���� �ƴ�
	pUser->SetMapped		( FALSE );		// �ʿ� ��ϵ��� ����
	pUser->SetRoomIndex		( -1	);		// �濡 �������� �ƴ�
	pUser->SetChannelIndex	( -1	);		// ä�ο� �������� �ƴ�

	OnCasualAccept( pUser );
}

VOID CNJeanGate::OnConnect( CPeer* pPeer, INT nServicerIndex )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	OnCasualConnect( (CUser*)pPeer );
}

VOID CNJeanGate::OnClose( CPeer* pPeer, INT nServicerIndex )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	switch( nServicerIndex )
	{
	case ServicerIndex::Client :
		break;

	default:
		return;
	}

	CUser* pUser = (CUser*) pPeer;

	OnCasualClose( pUser );

	//
	// ����ڰ� ���� ���� �ƴϸ� �濡 ����������, �α��� ������ ��ϵ����� �ʴ´�.
	//
	if( !pUser->IsService() || pUser->GetChannelIndex() < 0 )
		return;

	//
	// ����ڰ� ���ä�ο� ������
	//
	if( pUser->GetChannelIndex() == 0 )
	{
		m_aChannel[0].Leave( pUser );
		if( pUser->IsMapped() )
		{
#ifdef USING_USER_NO
            m_UserMap.EraseFromUserNo( pUser );
#else
			m_UserMap.EraseFromID( pUser );
#endif
			pUser->SetMapped( FALSE );
		}
		return;
	}


	//
	// ä�ο� �������� ���
	//

	// �����ϴ� ����ڰ� ���� ��� ä���� Ref�� �����´�.
	CRoom*		pRoom		= &m_aRoom[pUser->GetRoomIndex()];
	CChannel*	pChannel	= &m_aChannel[pUser->GetChannelIndex()];

	// ä�ο��� ������.
	pChannel->Leave( pUser );

	// �Ϲ� �濡 Join ���� ���
	if( !pChannel->IsWaitRoom( pRoom ) )
	{
		// �����ϴ� �濡 ���� �ִ� ����ڵ鿡�� ���� ������ ����
		m_pCasualPackets->RemoveRoomUser.SetProperty(
			pUser->GetID().c_str(),
			static_cast<INT>( pUser->GetID().length() )
			);

		SendToRoom( pRoom, pUser, &m_pCasualPackets->RemoveRoomUser, m_pCasualPackets->RemoveRoomUser.GetLength() );

		// �濡�� ������. �� ��, ���� ��� ������ Unused ��ü�� ��ȯ�Ѵ�.
		if( !pRoom->Leave( pUser ) )
		{
			int a = 3;
		}
		pRoom->FreeEmptyInnerIndex( pUser->GetRoomInnerIndex() );

		if( pRoom->IsEmpty() )
		{
			m_InnerManagerUsedRoom		.Remove	( pRoom->GetInnerObjectForGame() );
//			m_InnerManagerUnusedRoom	.Push	( pRoom->GetInnerObjectForGame() );

			// ������ ��� ����ڿ��� ���� �������� �����Ѵ�.
			m_pCasualPackets->RemoveRoom.SetProperty(
				pRoom->GetRoomIndex()
				);

			SendToRoom( pChannel->GetWaitRoom(), NULL, &m_pCasualPackets->RemoveRoom, m_pCasualPackets->RemoveRoom.GetLength() );

			// ä�ο��� �ش� ���� �����Ѵ�.
			pChannel->RemoveRoom( pRoom );
		}
		else
		{
			// ������ ��� ����ڿ��� ������ ������ �����Ѵ�.
			m_pCasualPackets->RoomInfoChange.SetProperty(
				pRoom->GetRoomIndex(),
				pRoom->GetMaxUser(),
				pRoom->GetNumOfUsers(),
				pRoom->IsPassword(),
				pRoom->GetOption()
				);

			SendToRoom( pChannel->GetWaitRoom(), NULL, &m_pCasualPackets->RoomInfoChange, m_pCasualPackets->RoomInfoChange.GetLength() );

			// �ڽ��� �����̾����� ������ ���� �̱����� �ñ׳��� ������.
			if( pUser->IsCaptine() && !pRoom->IsEmpty() )
			{
				// ������ ���� �������� ���� 
				SHORT	nResult = 0;
				if( IsCorrectCondition( nResult, pUser, PacketCondition::ChangeCaptine, reinterpret_cast<ULONG_PTR>( pRoom ) ) )
				{
					// ������ ���� �̱����� �ñ׳��� ������.
					AddSignal( CCasualCommon::CasualSignalMessage::ChangeCaptineSignal, pUser->GetRoomIndex(), 0 );
				}
				else
				{
					// ���� ���� ��� ����ڸ� �����Ų��.
					CUser* pRoomUser = pRoom->GetFirstUser();
					while( pRoomUser )
					{
						// ���� �ñ׳��� ������.
						AddSignal(
							CCasualCommon::CasualSignalMessage::CompulseExitSignal,							// ����޽���
							reinterpret_cast<ULONG_PTR>( pRoomUser ),										// ����Ǵ� �����
							static_cast<ULONG_PTR>( CCasualPacketCompulsionExited::Type::CloseRoom )	// ���� Ÿ��
							);
						pRoomUser = pRoom->GetNextUser();
					}
				}
			}

			if( pUser->IsCaptine() )
				pUser->SetCaptine( FALSE );
		}

	}
	// ���ǿ� Join ���� ���
	else
	{
		// ���ǿ��� ������.
		pRoom->Leave( pUser );

		// ���ǿ� ���� �ִ� ����ڵ鿡�� ���� ������ ����
		m_pCasualPackets->RemoveUser.SetProperty(
			pUser->GetID().c_str(),
			static_cast<INT>( pUser->GetID().length() )
			);

		SendToRoom(
			pRoom,
			NULL,
			&m_pCasualPackets->RemoveUser,
			static_cast<INT>( m_pCasualPackets->RemoveUser.GetLength() ) );
	}

	if( pUser->IsMapped() )
	{
#ifdef USING_USER_NO
		m_UserMap.EraseFromUserNo( pUser );
#else
		m_UserMap.EraseFromID( pUser );
#endif
		pUser->SetMapped( FALSE );
	}
}

VOID CNJeanGate::OnData( CPeer* pPeer, INT nServicerIndex, CHAR* pPacket, INT nPacketLength )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	CPacket* pCasualPacket = (CPacket*) pPacket;
	CUser* pUser;
	pUser = static_cast<CUser*>( pPeer );

	switch( nServicerIndex )
	{
	case ServicerIndex::Client :
		switch( pCasualPacket->GetCommand() )
		{
		case CCasualCommon::CasualPacketCommand::LoginRequest :
			// ���� ��Ŷ���� ĳ����
			CCasualPacketLoginRequest* pLoginRequest;
			pLoginRequest = (CCasualPacketLoginRequest*) pCasualPacket;

			// ���� ��Ŷ ���̿� �´��� �˻�
			if( nPacketLength != pLoginRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ��Ŷ ó��
			PacketProcessLoginRequest( pUser, pLoginRequest );
			break;

		case CCasualCommon::CasualPacketCommand::SecLoginRequest :
			// ���� ��Ŷ���� ĳ����
			CCasualPacketSecLoginRequest* pSecLoginRequest;
			pSecLoginRequest = (CCasualPacketSecLoginRequest*) pCasualPacket;

			// ���� ��Ŷ ���̿� �´��� �˻�
			if( nPacketLength != pSecLoginRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ��Ŷ ó��
			PacketProcessSecLoginRequest( pUser, pSecLoginRequest );
			break;

		case CCasualCommon::CasualPacketCommand::JoinChannelRequest :
			// ���� ��Ŷ���� ĳ����
			CCasualPacketJoinChannelRequest* pJoinChannelRequest;
			pJoinChannelRequest = (CCasualPacketJoinChannelRequest*) pCasualPacket;

			// ���� ��Ŷ ���̿� �´��� �˻�
			if( nPacketLength != pJoinChannelRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ���� ������ �˻�
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ��Ŷ ó��
			PacketProcessJoinChannelRequest( pUser, pJoinChannelRequest );
			break;

		case CCasualCommon::CasualPacketCommand::LeaveChannelRequest :
			// ���� ��Ŷ���� ĳ����
			CCasualPacketLeaveChannelRequest* pLeaveChannelRequest;
			pLeaveChannelRequest = (CCasualPacketLeaveChannelRequest*) pCasualPacket;

			// ���� ��Ŷ ���̿� �´��� �˻�
			if( nPacketLength != pLeaveChannelRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ���� ������ �˻�
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ��Ŷ ó��
			PacketProcessLeaveChannelRequest( pUser, pLeaveChannelRequest );
			break;

		case CCasualCommon::CasualPacketCommand::OpenRoomRequest :
			// ���� ��Ŷ���� ĳ����
			CCasualPacketOpenRoomRequest* pOpenRoomRequest;
			pOpenRoomRequest = (CCasualPacketOpenRoomRequest*) pCasualPacket;

			// ���� ��Ŷ ���̿� �´��� �˻�
			if( nPacketLength != pOpenRoomRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ���� ������ �˻�
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ��Ŷ ó��
			PacketProcessOpenRoomRequest( pUser, pOpenRoomRequest );
			break;

		case CCasualCommon::CasualPacketCommand::JoinRoomRequest :
			// ���� ��Ŷ���� ĳ����
			CCasualPacketJoinRoomRequest* pJoinRoomRequest;
			pJoinRoomRequest = (CCasualPacketJoinRoomRequest*) pCasualPacket;

			// ���� ��Ŷ ���̿� �´��� �˻�
			if( nPacketLength != pJoinRoomRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ���� ������ �˻�
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ��Ŷ ó��
			PacketProcessJoinRoomRequest( pUser, pJoinRoomRequest );
			break;

		case CCasualCommon::CasualPacketCommand::LeaveRoomRequest :
			// ���� ��Ŷ���� ĳ����
			CCasualPacketLeaveRoomRequest* pLeaveRoomRequest;
			pLeaveRoomRequest = (CCasualPacketLeaveRoomRequest*) pCasualPacket;

			// ���� ��Ŷ ���̿� �´��� �˻�
			if( nPacketLength != pLeaveRoomRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ���� ������ �˻�
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ��Ŷ ó��
			PacketProcessLeaveRoomRequest( pUser, pLeaveRoomRequest );
			break;

		case CCasualCommon::CasualPacketCommand::AutoJoinRoomRequest :
			// ���� ��Ŷ���� ĳ����
			CCasualPacketAutoJoinRoomRequest* pAutoJoinRoomRequest;
			pAutoJoinRoomRequest = (CCasualPacketAutoJoinRoomRequest*) pCasualPacket;

			// ���� ��Ŷ ���̿� �´��� �˻�
			if( nPacketLength != pAutoJoinRoomRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ���� ������ �˻�
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ��Ŷ ó��
			PacketProcessAutoJoinRoomRequest( pUser, pAutoJoinRoomRequest );
			break;

		case CCasualCommon::CasualPacketCommand::FindRequest :

			::OutputDebugString( "FindRequest\n" );

			// ���� ��Ŷ���� ĳ����
			CCasualPacketFindRequest* pFindRequest;
			pFindRequest = (CCasualPacketFindRequest*) pCasualPacket;

			// ���� ��Ŷ ���̿� �´��� �˻�
			if( nPacketLength != pFindRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ���� ������ �˻�
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ��Ŷ ó��
			PacketProcessFindRequest( pUser, pFindRequest );
			break;

		case CCasualCommon::CasualPacketCommand::Chat :
			// ���� ��Ŷ���� ĳ����
			CCasualPacketChat* pChat;
			pChat = (CCasualPacketChat*) pCasualPacket;

			// ���� ��Ŷ ���̿� �´��� �˻�
			if( nPacketLength != pChat->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ���� ������ �˻�
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ��Ŷ ó��
			PacketProcessChat( pUser, pChat );
			break;

		case CCasualCommon::CasualPacketCommand::Memo :
			// ���� ��Ŷ���� ĳ����
			CCasualPacketMemo* pMemo;
			pMemo = (CCasualPacketMemo*) pCasualPacket;

			// ���� ��Ŷ ���̿� �´��� �˻�
			if( nPacketLength != pMemo->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ���� ������ �˻�
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ��Ŷ ó��
			PacketProcessMemo( pUser, pMemo );
			break;

		case CCasualCommon::CasualPacketCommand::Invite :
			// ���� ��Ŷ���� ĳ����
			CCasualPacketInvite* pInvite;
			pInvite = (CCasualPacketInvite*) pCasualPacket;

			// ���� ��Ŷ ���̿� �´��� �˻�
			if( nPacketLength != pInvite->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ���� ������ �˻�
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ��Ŷ ó��
			PacketProcessInvite( pUser, pInvite );
			break;

		case CCasualCommon::CasualPacketCommand::CompulseExitRequest :
			// ���� ��Ŷ���� ĳ����
			CCasualPacketCompulsionExitRequest* pCompulseExitRequest;
			pCompulseExitRequest = (CCasualPacketCompulsionExitRequest*) pCasualPacket;

			// ���� ��Ŷ ���̿� �´��� �˻�
			if( nPacketLength != pCompulseExitRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ���� ������ �˻�
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// ��Ŷ ó��
			PacketProcessCompulseExitRequest( pUser, pCompulseExitRequest );
			break;

			/*
		case CCasualCommon::CasualPacketCommand::TestRequest :
			// ���� ��Ŷ���� ĳ����
			CCasualPacketTestRequest* pTestRequest;
			pTestRequest = (CCasualPacketTestRequest*) pCasualPacket;

			RequestHttp(
				pUser,
				CCasualCommon::CasualQueryMessage::TestQuery,
				"http://daum.net/"
				);
			break;
			*/

		default :
			OnCasualData( pUser, pCasualPacket, nPacketLength );
//			CloseRequest( pPeer );
			break;
		}
		break;

	default :
		// ���� ������ �˻�
		if( !pUser->IsService() )
		{
			//û�е�½�ɹ�
			CloseRequest( pPeer );
			break;
		}
		OnCasualData( static_cast<CUser*>( pPeer ), reinterpret_cast<CPacket*>( pPacket ), nPacketLength );

		break;
	}
}

VOID CNJeanGate::OnTimeout( CPeer* pPeer )
{
	if( OnCasualTimeout( static_cast<CUser*>( pPeer ) ) )
		TimeoutDisconnectRequest( pPeer );
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Util Fuctions

// ���� Ref�� ��ȯ�Ѵ�.
CRoom* CNJeanGate::GetRoomRef( CUser* pUser)
{
	if( pUser->GetChannelIndex() <= 0 || pUser->GetChannelIndex() >= m_nMaxChannel )
	{
		return NULL;
	}

	return &m_aRoom[ pUser->GetRoomIndex() ];
}

// ���� Ref�� ��ȯ�Ѵ�.
CRoom* CNJeanGate::GetRoomRef( INT nRoomIndex )
{
	if( nRoomIndex < 0 || nRoomIndex >= m_nMaxRoom )
		return NULL;

	return &m_aRoom[ nRoomIndex ];
}

// ä���� Ref�� ��ȯ�Ѵ�.
CChannel* CNJeanGate::GetChannelRef( CUser* pUser )
{
	if( pUser->GetChannelIndex() <= 0 || pUser->GetChannelIndex() >= m_nMaxChannel )
	{
		return NULL;
	}

	return &m_aChannel	[ pUser->GetChannelIndex() ];
}

// ä���� Ref�� ��ȯ�Ѵ�.
CChannel* CNJeanGate::GetChannelRef( INT nChannelIndex )
{
	if( nChannelIndex <= 0 || nChannelIndex >= m_nMaxChannel )
	{
		return NULL;
	}

	return &m_aChannel	[ nChannelIndex	];
}

// �ش� ���� ���������� ��ȯ�Ѵ�.
BOOL CNJeanGate::IsWaitRoom ( CRoom* pRoom )
{
	CChannel* pChannel;
	if( pRoom->GetChannelIndex() <= 0 || pRoom->GetChannelIndex() >= m_nMaxChannel )
	{
		return FALSE;
	}
	
	pChannel	= &m_aChannel[ pRoom->GetChannelIndex() ];

	return pChannel->IsWaitRoom( pRoom );
}

// �ش� ���� ���������� ��ȯ�Ѵ�.
BOOL CNJeanGate::IsWaitRoom ( INT nRoomIndex )
{
	CRoom*		pRoom;
	CChannel*	pChannel;

	if( nRoomIndex < 0 || nRoomIndex >= m_nMaxRoom )
		return FALSE;
	pRoom		= &m_aRoom[ nRoomIndex ];

	if( pRoom->GetChannelIndex() <= 0 || pRoom->GetChannelIndex() >= m_nMaxChannel )
	{
		return FALSE;
	}
	pChannel	= &m_aChannel[ pRoom->GetChannelIndex() ];

	return pChannel->IsWaitRoom( pRoom );
}

// �ش� ä���� ���ä�������� ��ȯ�Ѵ�.
BOOL CNJeanGate::IsWaitChannel ( CChannel* pChannel )
{
	return pChannel->GetChannelIndex() == 0;
}

BOOL CNJeanGate::IsWaitChannel ( INT nChannel )
{
	return nChannel == 0;
}

// �ش� ���� ���� ä���� Ref�� ��ȯ�Ѵ�.
CChannel* CNJeanGate::GetChannelRefOfRoom ( CRoom* pRoom )
{
	if( pRoom->GetChannelIndex() <= 0 || pRoom->GetChannelIndex() >= m_nMaxChannel )
	{
		return NULL;
	}
	
	return &m_aChannel[ pRoom->GetChannelIndex() ];
}

CChannel* CNJeanGate::GetChannelRefOfRoom ( INT nRoomIndex )
{
	CRoom*		pRoom;

	if( nRoomIndex < 0 || nRoomIndex >= m_nMaxRoom )
		return FALSE;
	pRoom		= &m_aRoom[ nRoomIndex ];

	if( pRoom->GetChannelIndex() <= 0 || pRoom->GetChannelIndex() >= m_nMaxChannel )
	{
		return FALSE;
	}
	
	return &m_aChannel[ pRoom->GetChannelIndex() ];
}

VOID CNJeanGate::UrlParsing(CHAR* pStr)
{
	const INT STR_MAX = CPacket::PredefineLength::GetQuerySize;

	CHAR str[STR_MAX];
	::memcpy( str, pStr, STR_MAX );
	INT nLen = static_cast<INT>( strlen( str ) );
	INT nCount = 0;
	for ( INT i = 0 ; i < nLen ; i++, nCount++ )
	{
		if ( i >= STR_MAX ) { pStr[nCount] = 0; return; }

		CHAR ch;
		if ( str[i] == '%' )
		{
			CHAR temp[2] = { str[i+1], str[i+2] };
			ch = HexToChar(temp);
			i += 2;
		}
		else if ( str[i] == '+' ) 
		{
			ch = ' ';
		}
		else
		{
			ch = str[i];
		}
		pStr[nCount] = ch;
		if ( ch == 13 )
		{
			nCount++;
			pStr[nCount] = 10;
		}
	}
	pStr[nCount] = 0;
}

CHAR CNJeanGate::HexToChar(CHAR* pStr)
{
	CHAR Hex[2];
	Hex[0] = pStr[0];
	Hex[1] = pStr[1];
	if ( Hex[0] > 57 ) 
		Hex[0] = Hex[0] - 87;
	else
		Hex[0] = Hex[0] - 48;

	if ( Hex[1] > 57 )
		Hex[1] = Hex[1] - 87;
	else
		Hex[1] = Hex[1] - 48;

	UCHAR Dec;
	Dec = (Hex[0]<<4) + Hex[1];

	return Dec;
}


// �ش� �濡 �޽����� �߰��Ѵ�.
VOID CNJeanGate::SendToRoom( CRoom* pRoom, const CUser* pExeptUser, const CPacket* pPacket, INT nPacketSize )
{
	CUser* pUser = pRoom->GetFirstUser();
	while( pUser )
	{
		if( pUser != pExeptUser )
			SendData( pUser, reinterpret_cast<const CHAR*>(pPacket), nPacketSize );
		pUser = pRoom->GetNextUser();
	}
}

// P2P���� ������ �� �� �ִ����� �����Ѵ�.
inline VOID CNJeanGate::InspectP2PCaptine( CUser* pUser, LPCTSTR strIP )
{
	if( ::strcmp( GetPeerAddr( pUser ), strIP ) == 0 )
		pUser->SetP2PCaptine( TRUE );
	else
		pUser->SetP2PCaptine( FALSE );
}

// ���� ���� ��� ����ڿ��� �޽����� �����Ѵ�.
VOID CNJeanGate::SendToAllUser( const CPacket* pPacket, INT nPacketSize )
{
	CUser* pUser;

	// 0�� ��� ä���� ����ڿ��Դ� �޽����� ������ �ʴ´�.
	for( INT i = 1; i < m_nMaxChannel; i++ )
	{
		pUser = m_aChannel[i].GetFirstUser();
		while( pUser )
		{
			SendData( pUser, reinterpret_cast<const CHAR*>(pPacket), nPacketSize );
			pUser = m_aChannel[i].GetNextUser();
		}
	}
}

// ä�� ���� ���ǿ� �޽����� �����Ѵ�.
VOID CNJeanGate::SendToWaitRoom( CChannel* pChannel, const CUser* pExeptUser, const CPacket* pPacket, INT nPacketSize )
{
	CRoom* pWaitRoom = pChannel->GetWaitRoom();
	SendToRoom( pWaitRoom, pExeptUser, pPacket, nPacketSize );
}

// ä�� ���� ��� ����ڿ��� �޽����� �����Ѵ�.
VOID CNJeanGate::SendToAllUser( CChannel* pChannel, const CPacket* pPacket, INT nPacketSize )
{
	CUser* pUser;

	pUser = pChannel->GetFirstUser();
	while( pUser )
	{
		SendData( pUser, reinterpret_cast<const CHAR*>(pPacket), nPacketSize );
		pUser = pChannel->GetNextUser();
	}
}

// ���� ���� ��� ���� �濡 �޽����� �����Ѵ�.
VOID CNJeanGate::SendToAllRoomUser( const CPacket* pPacket, INT nPacketSize )
{
	CRoom*	pRoom;
	CUser*	pUser;

	// 0�� ��� ä���� ����ڿ��Դ� �޽����� ������ �ʴ´�.
	for( INT i = 1; i < m_nMaxChannel; i++ )
	{
		pRoom = m_aChannel[i].GetFirstRoom();
		while( pRoom )
		{
			pUser = pRoom->GetFirstUser();
			while( pUser )
			{
				SendData( pUser, reinterpret_cast<const CHAR*>(pPacket), nPacketSize );
				pUser = pRoom->GetNextUser();
			}
			pRoom = m_aChannel[i].GetNextRoom();
		}
	}
}

// ä�� ���� ��� ���� �濡 �޽����� �����Ѵ�.
VOID CNJeanGate::SendToAllRoomUser( CChannel* pChannel, const CPacket* pPacket, INT nPacketSize )
{
	CRoom*	pRoom;
	CUser*	pUser;

	pRoom = pChannel->GetFirstRoom();
	while( pRoom )
	{
		pUser = pRoom->GetFirstUser();
		while( pUser )
		{
			SendData( pUser, reinterpret_cast<const CHAR*>(pPacket), nPacketSize );
			pUser = pRoom->GetNextUser();
		}
		pRoom = pChannel->GetNextRoom();
	}
}

// ������� ��ġ�� ��ȯ�Ѵ�.
VOID CNJeanGate::GetUserPlace( CUser* pUser, CChannel* &out_pChannel, CRoom* &out_pRoom, BOOL &out_bWaitRoom )
{
	if( pUser->GetChannelIndex() <= 0 || pUser->GetChannelIndex() >= m_nMaxChannel )
	{
		out_pChannel	= NULL;
		out_pRoom		= NULL;
		out_bWaitRoom	= FALSE;
		return;
	}
	out_pChannel	= &m_aChannel	[ pUser->GetChannelIndex()	];
	out_pRoom		= &m_aRoom		[ pUser->GetRoomIndex()		];
	out_bWaitRoom	= out_pChannel->IsWaitRoom( out_pRoom );
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Response ó��
inline VOID CNJeanGate::LoginReply( CUser* pUser, BOOL bLogined )
{
	if( !bLogined )
	{
		// Ŭ���̾�Ʈ�� �α��� ���и� �뺸�� ���� ����
		m_pCasualPackets->LoginReply.SetProperty( CCasualPacketLoginReply::LoginResult::Fail );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->LoginReply ), m_pCasualPackets->LoginReply.GetLength() );

		CloseRequest( pUser );
		return;
	}

//	pUser->SetUserProperty( lpszUserProperty );

	CUser* pFindUser;

#ifdef USING_USER_NO
	pFindUser = m_UserMap.FindFromUserNo( pUser->GetUserNo() );

#ifdef __TEST__
	if( pFindUser )
	{
		pFindUser = m_UserMap.FindFromID( pUser->GetID() );
	}
#endif

#else
	pFindUser = m_UserMap.FindFromID( pUser->GetID() );
#endif
		
	// ����� ������ ���� ���
	if( !pFindUser )
	{
		// �ʿ� ���
		m_UserMap.Insert( pUser->GetUserNo(), pUser->GetID(), pUser->GetNick(), pUser );

		// �α��� ���� ���
		pUser->SetLogined( TRUE );

		// ���� ���� ���
		pUser->SetService( TRUE );

		// �ʿ� ��ϵ�
		pUser->SetMapped( TRUE );

		// �⺻ ä�ο� ���
		m_aChannel[0].Join( pUser );
		pUser->SetChannelIndex( 0 );

		// Ŭ���̾�Ʈ�� �α��� ������ �뺸
		m_pCasualPackets->LoginReply.SetProperty( CCasualPacketLoginReply::LoginResult::Success );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->LoginReply ), m_pCasualPackets->LoginReply.GetLength() );

		OnLoginSuccess( pUser );
	}
	else
	{
		// �α��� ���� ���
		pUser->SetLogined( TRUE );

		// Ŭ���̾�Ʈ�� �ٸ������� �������� �뺸
		m_pCasualPackets->LoginReply.SetProperty( CCasualPacketLoginReply::LoginResult::Joined );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->LoginReply ), m_pCasualPackets->LoginReply.GetLength() );
	}
}

VOID CNJeanGate::LoginQueryRequest( CUser* pUser, LPCTSTR strLoginQuery )
{
	/*
	for( int i = 0; i < 10; i++ )
	{

		RequestHttp(
			pUser,
			111,
			strLoginQuery
			);
	}
	*/

	RequestHttp(
		pUser,
		CCasualCommon::CasualQueryMessage::LoginQuery,
		strLoginQuery
		);
}


/////////////////////////////////////////////////////////////////////////////////////////////
// �ñ׳� ó��

inline VOID CNJeanGate::SignalProcessChangeCaptine( ULONG_PTR wParam, ULONG_PTR lParam )
{
	INT nRoomIndex		= (INT) wParam;

	if( nRoomIndex < 0 || nRoomIndex >= m_nMaxRoom )
		return;

	CRoom* pRoom = &m_aRoom[nRoomIndex];

	if( pRoom->IsEmpty() )
		return;

	CUser* pCaptine;
	pCaptine = pRoom->GetFirstUser();
	while( pCaptine )
	{
		if( CanBeCaptine( pCaptine ) )
		{
			pCaptine->SetCaptine( TRUE );
			break;
		}
		pCaptine = pRoom->GetNextUser();
	}
	pRoom->SetCaptine( pCaptine );

	if( !pCaptine )
	{
		OnNoneCaptine( pRoom );
		return;
	}

	OnChangeCaptine( pCaptine, pRoom );
}

VOID CNJeanGate::OnChangeCaptine( CUser* pCaptine, CRoom* pRoom )
{
	// ���� ���� ����鿡�� ���� ���� ��Ŷ�� �����Ѵ�.
	m_pCasualPackets->RoomCaptineChange.SetProperty(
		pCaptine->GetID().c_str(),
		static_cast<INT>( pCaptine->GetID().length() )
		);
	SendToRoom( pRoom, NULL, &m_pCasualPackets->RoomCaptineChange, m_pCasualPackets->RoomCaptineChange.GetLength() );
}

inline VOID CNJeanGate::SignalProcessCompulseExit( ULONG_PTR wParam, ULONG_PTR lParam )
{
	CUser*		pUser;		// �����ϴ� �����
	CRoom*		pRoom;		// ����ڰ� ���ߴ� ��
	CChannel*	pChannel;	// ����ڰ� ���ߴ� ä��
	
	CCasualPacketCompulsionExited::Type
				type;	// ���� ���� Ÿ�� : �Ϲ� ���嵵 ���� �ñ׳η� ó�� �ϹǷ� lParam���� ������ �Ѵ�.

	pUser	= reinterpret_cast	<CUser*>								( wParam );
	type	= static_cast		<CCasualPacketCompulsionExited::Type>	( lParam );

	if( pUser->GetRoomIndex() < 0 || pUser->GetChannelIndex() <= 0 )
	{
		return;
	}

	pRoom		= &m_aRoom		[ pUser->GetRoomIndex()		];
	pChannel	= &m_aChannel	[ pUser->GetChannelIndex()	];

	if( pChannel->IsWaitRoom( pRoom ) )
		return;

	OnLeaveRoom( pUser, pRoom, type );


	// ���� ���� ������ �����Ѵ�.
	if( type != CCasualPacketCompulsionExited::Type::Mine )
	{
		m_pCasualPackets->CompulsionExited.SetProperty(
			type );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->CompulsionExited ), m_pCasualPackets->CompulsionExited.GetLength() );
	}


	// �濡�� ������. �� ��, ���� ��� ������ Unused ��ü�� ��ȯ�Ѵ�.
	if( !pRoom->Leave( pUser ) )
	{
		int a = 3;
	}

	pRoom->FreeEmptyInnerIndex( pUser->GetRoomInnerIndex() );
	if( pRoom->IsEmpty() )
	{
		m_InnerManagerUsedRoom		.Remove	( pRoom->GetInnerObjectForGame() );
//		m_InnerManagerUnusedRoom	.Push	( pRoom->GetInnerObjectForGame() );

		// ä�ο��� �ش� ���� �����Ѵ�.
		pChannel->RemoveRoom( pRoom );
	}
	else
	{
		// �ڽ��� �����̾�����
		if( pUser->IsCaptine() )
		{
			// ������ ���� �������� ���� 
			SHORT nResult = 0;
			if( IsCorrectCondition( nResult, pUser, PacketCondition::ChangeCaptine, reinterpret_cast<ULONG_PTR>( pRoom ) ) )
			{
				// ������ ���� �̱����� �ñ׳��� ������.
				AddSignal( CCasualCommon::CasualSignalMessage::ChangeCaptineSignal, pUser->GetRoomIndex(), 0 );
			}
			else
			{
				// ���� ���� ��� ����ڸ� �����Ų��.
				CUser* pRoomUser = pRoom->GetFirstUser();
				while( pRoomUser )
				{
					// ���� �ñ׳��� ������.
					AddSignal(
						CCasualCommon::CasualSignalMessage::CompulseExitSignal,							// ����޽���
						reinterpret_cast<ULONG_PTR>( pRoomUser ),										// ����Ǵ� �����
						static_cast<ULONG_PTR>( CCasualPacketCompulsionExited::Type::CloseRoom )	// ���� Ÿ��
						);
					pRoomUser = pRoom->GetNextUser();
				}
			}
		}
	}
	pUser->SetCaptine( FALSE );

	// ���ǿ� ������ �Ѵ�.
	CRoom* pWaitRoom = pChannel->GetWaitRoom();
	pWaitRoom->Join( pUser );
	pUser->SetRoomIndex( pWaitRoom->GetRoomIndex() );

	// �濡�� ������.
	m_pCasualPackets->LeaveRoomReply.SetProperty( CCasualPacketLeaveRoomReply::Result::Success );
	SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->LeaveRoomReply ), m_pCasualPackets->LeaveRoomReply.GetLength() );

	// ä�� ������ �뺸�Ѵ�. : ���� ����� �����ϰ� ���
	m_pCasualPackets->JoinChannelReply.SetProperty(
		CCasualPacketJoinChannelReply::Result::Success,
		pWaitRoom->GetNumOfUsers() - 1,
		pChannel->GetNumOfOpenRooms()
		);
	SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinChannelReply ), m_pCasualPackets->JoinChannelReply.GetLength() );



	//
	// Step 6 : �� ����Ʈ�� ���� : Step 4,5�� ����
	//

	CRoom* pOpenRoom = pChannel->GetFirstRoom();
	while( pOpenRoom )
	{
		m_pCasualPackets->RoomInfo.SetProperty(	pOpenRoom->GetRoomIndex(),
												pOpenRoom->GetMaxUser(),
												pOpenRoom->GetNumOfUsers(),
												pOpenRoom->GetRoomName().c_str(),
												static_cast<INT>( pOpenRoom->GetRoomName().length() ),
												pOpenRoom->IsPassword(),
												pOpenRoom->GetOption(),
												0 );

		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->RoomInfo ), m_pCasualPackets->RoomInfo.GetLength() );

		pOpenRoom = pChannel->GetNextRoom();
	}



	//
	// Step 4 : ���� ����ڵ鿡�� ����� ������ ����
	//
	m_pCasualPackets->RemoveRoomUser.SetProperty(
		pUser->GetID().c_str(),
		static_cast<INT>( pUser->GetID().length() )
		);
	SendToRoom( pRoom, NULL, &m_pCasualPackets->RemoveRoomUser, m_pCasualPackets->RemoveRoomUser.GetLength() );


	//
	// Step 5 : ������ ����ڵ���� ���� ��ȯ
	//

	// ���� ���������, ���� ���ŵǾ����� �뺸
	if( pRoom->IsEmpty() )
	{
		m_pCasualPackets->RemoveRoom.SetProperty(
			pRoom->GetRoomIndex()
			);

		// ���� ���� ����ڿ��Դ� �� ���� ������ ����
		SendToRoom( pWaitRoom, pUser, &m_pCasualPackets->RemoveRoom, m_pCasualPackets->RemoveRoom.GetLength() );

		m_pCasualPackets->UserInfo.SetProperty(
			pUser->GetGrade(),	
			pUser->GetID().c_str(),
			static_cast<INT>( pUser->GetID().length() ),
			pUser->GetNick().c_str(),
			static_cast<INT>( pUser->GetNick().length() )
			);

		// ���� ���� ����ڿ��Դ� ���� ������� ���� ������ ����
		SendToRoom( pWaitRoom, pUser, &m_pCasualPackets->UserInfo, m_pCasualPackets->UserInfo.GetLength() );

		CUser* pWaitUser;
		pWaitUser = pWaitRoom->GetFirstUser();
		while( pWaitUser )
		{
			if( pWaitUser != pUser )
			{
				// ���� ����ڿ��Դ� ���� ���� ������� ������ ����
				m_pCasualPackets->UserInfo.SetProperty(
					pWaitUser->GetGrade(),
					pWaitUser->GetID().c_str(),
					static_cast<INT>( pWaitUser->GetID().length() ),
					pWaitUser->GetNick().c_str(),
					static_cast<INT>( pWaitUser->GetNick().length() )
					);
				SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->UserInfo ), m_pCasualPackets->UserInfo.GetLength() );
			}
			pWaitUser = pWaitRoom->GetNextUser();
		}// while( pWaitUser )

	}// if( pRoom->IsEmpty() )

	// ���� ����������, �������� ����Ǿ����� �뺸
	else
	{
		m_pCasualPackets->RoomInfoChange.SetProperty(
			pRoom->GetRoomIndex(),
			pRoom->GetMaxUser(),
			pRoom->GetNumOfUsers(),
			pRoom->IsPassword(),
			pRoom->GetOption()
			);

		// ���� ���� ����ڿ��Դ� �� ���� ������ ����
		SendToRoom( pWaitRoom, pUser, &m_pCasualPackets->RoomInfoChange, m_pCasualPackets->RoomInfoChange.GetLength() );

		m_pCasualPackets->UserInfo.SetProperty(
			pUser->GetGrade(),
			pUser->GetID().c_str(),
			static_cast<INT>( pUser->GetID().length() ),
			pUser->GetNick().c_str(),
			static_cast<INT>( pUser->GetNick().length() )
			);

		// ���� ���� ����ڿ��Դ� ���� ������� ���� ������ ����
		SendToRoom( pWaitRoom, pUser, &m_pCasualPackets->UserInfo, m_pCasualPackets->UserInfo.GetLength() );

		CUser* pWaitUser;
		pWaitUser = pWaitRoom->GetFirstUser();
		while( pWaitUser )
		{
			if( pWaitUser != pUser )
			{
				// ���� ����ڿ��Դ� ���� ���� ������� ������ ����
				m_pCasualPackets->UserInfo.SetProperty(
					pWaitUser->GetGrade(),
					pWaitUser->GetID().c_str(),
					static_cast<INT>( pWaitUser->GetID().length() ),
					pWaitUser->GetNick().c_str(),
					static_cast<INT>( pWaitUser->GetNick().length() )
					);
				SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->UserInfo ), m_pCasualPackets->UserInfo.GetLength() );
			}
			pWaitUser = pWaitRoom->GetNextUser();
		}
	}


}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// ��Ŷ ó��
//

VOID CNJeanGate::OnPacketLogin( CUser* pUser, LPCTSTR strUserNo, LPCTSTR strID, LPCTSTR strNick )
{
	pUser->SetUserInfo( strUserNo,
						strID,
						strNick	);

	pUser->SetCaptine( FALSE );
//	pUser->SetP2PCaptine( FALSE );
	pUser->SetGameCaptine( TRUE );

	if( pUser->GetIP().compare( GetPeerAddr( pUser ) ) == 0 ) //�������ѵ����ܲ�һ����
		pUser->SetP2PCaptine( TRUE );
	else
		pUser->SetP2PCaptine( FALSE );

//	pUser->SetP2PCaptine( TRUE );
}

// �α��� ��û ó��
inline VOID CNJeanGate::PacketProcessLoginRequest( CUser* pUser, CCasualPacketLoginRequest* pCasualPacket )
{
	pUser->SetAddress( pCasualPacket->GetIpAddr(), pCasualPacket->GetPort() );

	
	OnPacketLogin(
		pUser,
		pCasualPacket->GetUserNo(),
		pCasualPacket->GetID(),
		pCasualPacket->GetNick()
		);
}

inline VOID CNJeanGate::PacketProcessSecLoginRequest( CUser* pUser, CCasualPacketSecLoginRequest* pCasualPacket )
{
	pUser->SetAddress( pCasualPacket->GetIpAddr(), pCasualPacket->GetPort() );

	// ��ȣȭ Ǯ�� ó�� �߰� ����

	// ��ȣȭ : Ȳ������ �ҽ� ī��
	CUserSCDApp cDataEn;

	char	szResult[4096];
	char	szOutput[4096];
	char	szKey[100];

	// szKey �����ϱ� : ���� ��¥ : ex) 20020101
	__time64_t ltime;
	struct tm *today;
	::_time64( &ltime );
	today = ::_localtime64( &ltime );
	::strftime( szKey, 100, "%Y%m%d", today );

	const char*	szInput	= pCasualPacket->GetSecData();
	int			nLen	= pCasualPacket->GetSecSize();

	::memset(szResult , 0 , 4096);

	CUserSCDApp::Decoder( (unsigned char *)szInput , (unsigned char *)szResult , nLen );

	long nBytesRead = cDataEn.uToDedes( (unsigned char *)szResult, (unsigned char *)szKey
									  , (unsigned char *)szOutput, nLen / 2 , 2 ) ;

	szOutput[nBytesRead] = NULL;

	OnPacketSecLogin( pUser, szOutput, pCasualPacket->GetRealLength(), pCasualPacket->GetSessionKey(), CCasualCommon::CasualStringLength::SessionKey  );
}

// ä�� ���� ��û ó��
inline VOID CNJeanGate::PacketProcessJoinChannelRequest( CUser* pUser, CCasualPacketJoinChannelRequest* pCasualPacket )
{
	CRoom*		pWaitRoom;	// ����ڰ� ���� ä���� ����
	CChannel*	pChannel;	// ����ڰ� ���� ä��

	//
	// Stpe 1 : ���� �˻�
	//

	// ä�� ������ �Ѵ� ���
	if( pCasualPacket->GetChannelIndex() <= 0 ||
		pCasualPacket->GetChannelIndex() >= m_nMaxChannel )
	{
		m_pCasualPackets->JoinChannelReply.SetProperty(
			CCasualPacketJoinChannelReply::Result::FailOutOfChannel
			);
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinChannelReply ), m_pCasualPackets->JoinChannelReply.GetLength() );
		return;
	}
	
	// �̹� �ٸ� ä�ο� ���� ���
	if( pUser->GetChannelIndex() > 0 )
	{
		m_pCasualPackets->JoinChannelReply.SetProperty(
			CCasualPacketJoinChannelReply::Result::FailExist
			);
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinChannelReply ), m_pCasualPackets->JoinChannelReply.GetLength() );
		return;
	}
	pChannel	= &m_aChannel[ pCasualPacket->GetChannelIndex() ];

	SHORT nResult = CCasualPacketJoinChannelReply::Result::FailETC;
	if( !IsCorrectCondition( nResult, pUser, PacketCondition::JoinChannel, reinterpret_cast<ULONG_PTR>( pChannel ) ) )
	{
		//��ǰ��ɫ�ܷ��½����channnel
		m_pCasualPackets->JoinChannelReply.SetProperty(
//			CCasualPacketJoinChannelReply::Result::FailETC
			nResult
			);
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinChannelReply ), m_pCasualPackets->JoinChannelReply.GetLength() );
		return;
	}


	//
	// Step 2 : ä�ο� ����
	//

	// ��m_aChannel[0]���˳���LoginReply�ɹ�ʱ������ɫ��������
	m_aChannel[0].Leave( pUser );

	// �ش� ä�ο� ����
	if( !pChannel->Join( pUser ) )
	{
		m_aChannel[0].Join( pUser );

		m_pCasualPackets->JoinChannelReply.SetProperty(
			CCasualPacketJoinChannelReply::Result::FailFull
			);
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinChannelReply ), m_pCasualPackets->JoinChannelReply.GetLength() );
		return;
	}
	pUser->SetChannelIndex( pChannel->GetChannelIndex() );


	//
	// Step 3 : ���ǿ� ����
	//

	pWaitRoom	= pChannel->GetWaitRoom();

	pWaitRoom->Join( pUser );
	pUser->SetRoomIndex( pWaitRoom->GetRoomIndex() );



	//
	// Step 4 : ���Ǿ��� ����ڵ��� ���� ��ȯ
	//

	// ������ ����ڿ��� ���� ������ ����
	m_pCasualPackets->JoinChannelReply.SetProperty(
		CCasualPacketJoinChannelReply::Result::Success,
		pWaitRoom->GetNumOfUsers() - 1,		// �ڱ� �ڽ��� ��ü ������� ���� �������� �ʴ´�.
		pChannel->GetNumOfOpenRooms()
		);
	SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinChannelReply ), m_pCasualPackets->JoinChannelReply.GetLength() );


	//
	// Step 5 : ���� ������ �����Ѵ�.
	//

	CRoom* pOpenRoom = pChannel->GetFirstRoom();
	while( pOpenRoom )
	{		
		m_pCasualPackets->RoomInfo.SetProperty(
			pOpenRoom->GetRoomIndex(),
			pOpenRoom->GetMaxUser(),
			pOpenRoom->GetNumOfUsers(),
			pOpenRoom->GetRoomName().c_str(),
			static_cast<INT>( pOpenRoom->GetRoomName().length() ),
			pOpenRoom->IsPassword(),
			pOpenRoom->GetOption()
			);

		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->RoomInfo ), m_pCasualPackets->RoomInfo.GetLength() );

		pOpenRoom = pChannel->GetNextRoom();
	}


	//
	// Step 4 : 5�� ����Ǿ���
	//

	// ���Ǿ��� ����ڿ��� ������ ������� ������ ������ ��Ŷ
	m_pCasualPackets->UserInfo.SetProperty(
		pUser->GetGrade(),
		pUser->GetID().c_str(),
		static_cast<INT>( pUser->GetID().length() ),
		pUser->GetNick().c_str(),
		static_cast<INT>( pUser->GetNick().length() )
		);

	// Room���� ����ڿ��� ������ ������� ������ ����
	SendToRoom( pWaitRoom, pUser, &m_pCasualPackets->UserInfo, m_pCasualPackets->UserInfo.GetLength() );

	CUser* pRoomUser = pWaitRoom->GetFirstUser();
	while( pRoomUser )
	{
		if( pRoomUser != pUser )
		{
			// ������ ����ڿ��� Room���� ����� ������ ����
			m_pCasualPackets->UserInfo.SetProperty(
				pRoomUser->GetGrade(),
				pRoomUser->GetID().c_str(),
				static_cast<INT>( pRoomUser->GetID().length() ),
				pRoomUser->GetNick().c_str(),
				static_cast<INT>( pRoomUser->GetNick().length() )
				);
			SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->UserInfo ), m_pCasualPackets->UserInfo.GetLength() );
		}

		pRoomUser = pWaitRoom->GetNextUser();
	}




	// ������ �ƴ��� �����Ѵ�.
	pUser->SetCaptine( FALSE );
}

// ä�� ���� ��û ó��
inline VOID CNJeanGate::PacketProcessLeaveChannelRequest( CUser* pUser, CCasualPacketLeaveChannelRequest* pCausalPacket )
{
	CRoom*		pRoom;		// ����ڰ� ���� ���� ��
	CRoom*		pWaitRoom;	// ����ڰ� ���� ä���� ����
	CChannel*	pChannel;	// ����ڰ� ���� ä��

	//
	// Stpe 1 : ���� �˻�
	//

	// ä�ο� ���� ���
	if( pUser->GetChannelIndex() <= 0 ||
		pUser->GetChannelIndex() >= m_nMaxChannel )
	{
		return;
	}

	pChannel	= &m_aChannel[pUser->GetChannelIndex()];
	pWaitRoom	= pChannel->GetWaitRoom();
	pRoom		= &m_aRoom[pUser->GetRoomIndex()];

	// ���ǿ� ���� ���� ���
	if( !pChannel->IsWaitRoom( pRoom ) )
		return;


	//
	// Step 2 : ä�ο��� ������.
	//
	pChannel->Leave( pUser );
	CCasualPacketLeaveChannelReply	leaveChannelPacket;
	SendData( pUser, reinterpret_cast<const CHAR*>( &leaveChannelPacket ), leaveChannelPacket.GetLength() );

	// ��� ä�ο� �����Ѵ�.
	m_aChannel[0].Join( pUser );
	pUser->SetChannelIndex( 0 );


	// ���ǿ��� ������.
	pWaitRoom->Leave( pUser );

	// ���ǿ� ���� �ִ� ����ڵ鿡�� ���� ������ ����
	m_pCasualPackets->RemoveUser.SetProperty(
		pUser->GetID().c_str(),
		static_cast<INT>( pUser->GetID().length() )
		);

	SendToRoom( pWaitRoom, NULL, &m_pCasualPackets->RemoveUser, m_pCasualPackets->RemoveUser.GetLength() );
}

// �� ���� ��û ó��
inline VOID CNJeanGate::PacketProcessOpenRoomRequest( CUser* pUser, CCasualPacketOpenRoomRequest* pCasualPacket )
{
	CChannel*	pChannel;	// ����ڰ� ���� ä��
	CRoom*		pWaitRoom;	// ����ڰ� ���� ����

	//
	// Step 1 : ���� �˻�
	//

	// ���� �ִ� �����ڰ�, ���� ���� ������� ū ���
	if( pCasualPacket->GetMaxUsers() > m_nMaxRoomUser )
	{
		m_pCasualPackets->OpenRoomReply.SetProperty(
			CCasualPacketOpenRoomReply::Result::FailOverUsers,
			-1 );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->OpenRoomReply ), m_pCasualPackets->OpenRoomReply.GetLength() );
		return;
	}

	// ä�ο� ������ ���� ���
	if( pUser->GetChannelIndex() <= 0 ||
		pUser->GetChannelIndex() >= m_nMaxChannel )
	{
		m_pCasualPackets->OpenRoomReply.SetProperty(
			CCasualPacketOpenRoomReply::Result::FailOutOfChannel,
			-1 );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->OpenRoomReply ), m_pCasualPackets->OpenRoomReply.GetLength() );
		return;
	}
	
	pChannel	= &m_aChannel	[ pUser->GetChannelIndex()	];
	pWaitRoom	= &m_aRoom		[ pUser->GetRoomIndex()		];

	// ���ǿ� ���� ���
	if( !pChannel->IsWaitRoom( pWaitRoom ) )
	{
		m_pCasualPackets->OpenRoomReply.SetProperty(
			CCasualPacketOpenRoomReply::Result::FailExist,
			pUser->GetRoomIndex() );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->OpenRoomReply ), m_pCasualPackets->OpenRoomReply.GetLength() );
		return;
	}

	SHORT nResult = CCasualPacketOpenRoomReply::Result::FailFull;
	if( !IsCorrectCondition( nResult, pUser, PacketCondition::OpenRoom ) )
	{
		m_pCasualPackets->OpenRoomReply.SetProperty(
//			CCasualPacketOpenRoomReply::Result::FailFull,
			nResult,
			pUser->GetRoomIndex() );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->OpenRoomReply ), m_pCasualPackets->OpenRoomReply.GetLength() );
		return;
	}

	//
	// Step 2 : �� Room �Ҵ�
	//

	BOOL		bRet;
	CRoom*		pNewRoom;

	pChannel	= &m_aChannel[ pUser->GetChannelIndex() ];

	bRet = m_InnerManagerUnusedRoom.Pop( pNewRoom );

	// �Ҵ翡 ���и� �ϸ�
	if( !bRet )
	{
		m_pCasualPackets->OpenRoomReply.SetProperty(
			CCasualPacketOpenRoomReply::Result::FailFull,
			-1 );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->OpenRoomReply ), m_pCasualPackets->OpenRoomReply.GetLength() );
		return;
	}
	pNewRoom->Init( pChannel->GetChannelIndex() );

	// ä�ο� ������ ������ ���� �߰��Ѵ�.
	bRet = pChannel->AddRoom( pNewRoom );

	// ä�ο� ���߰��� ���и� �ϸ�...
	if( !bRet )
	{
		m_InnerManagerUnusedRoom.Add( pNewRoom->GetInnerObjectForGame() );

		m_pCasualPackets->OpenRoomReply.SetProperty(
			CCasualPacketOpenRoomReply::Result::FailFull,
			-1 );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->OpenRoomReply ), m_pCasualPackets->OpenRoomReply.GetLength() );
		return;
	}

	// �Ҵ翡 ������ Room�� m_InnerManagerUsedRoom�� �����ϰ�, Room������ �����Ѵ�.
	m_InnerManagerUsedRoom.Add( pNewRoom->GetInnerObjectForGame() );
	pNewRoom->Open(
		pCasualPacket->GetMaxUsers(),
		pCasualPacket->GetRoomName()
		);



	//
	// Step 3 : ���� ����ڵ鿡�� ���� ����ڰ� ���ǿ��� �������� �뺸
	//

	// ���ǿ��� ������.
	pWaitRoom->Leave( pUser );

	// ���ǿ� ���� �ִ� ����ڵ鿡�� ���� ������ ����
	m_pCasualPackets->RemoveUser.SetProperty(
		pUser->GetID().c_str(),
		static_cast<INT>( pUser->GetID().length() )
		);

	SendToRoom( pWaitRoom, pUser, &m_pCasualPackets->RemoveUser, m_pCasualPackets->RemoveUser.GetLength() );



	//
	// Step 4 : ���� ����ڵ鿡�� ���� �� ������ �뺸
	//

	// Open�� Room�� ������ �Ѵ�.
	if( !pNewRoom->Join( pUser ) )
	{
		// ����� ���ʿ� �� ���� ����.
		return;
	}
	pUser->SetRoomIndex( pNewRoom->GetRoomIndex() );

	// ���� ������ �ڴ� 0��° �ε����� ���� ���̴�.
	pUser->SetRoomInnerIndex( pNewRoom->AllocEmptyInnerIndex() );

	// �������� �����Ѵ�.
	pUser->SetCaptine( TRUE );
	pNewRoom->SetCaptine( pUser );

	// �濡 �н����� ���� ������ �����Ѵ�.
	pNewRoom->SetPassword( pCasualPacket->IsPassword(), pCasualPacket->GetPassword() );
	
	// �ɼ��� �����Ѵ�.
	for( INT i = 0; i < CCasualCommon::CasualStringLength::RoomOption; i++ )
		pNewRoom->SetOption( i, pCasualPacket->GetOption( i ) );

	pNewRoom->SetRTT( GetRttTime( pUser ) );

	OnOpenRoomReply( pUser, pNewRoom );
	/*
	// Open ������ �����Ѵ�.
	m_pCasualPackets->OpenRoomReply.SetProperty(
		CCasualPacketOpenRoomReply::Result::Success,
		pNewRoom->GetRoomIndex() );
	SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->OpenRoomReply ), m_pCasualPackets->OpenRoomReply.GetLength() );
	*/


	// ������ ��� ����ڿ��� ������ Room������ �����Ѵ�.
	
	m_pCasualPackets->RoomInfo.SetProperty(
		pNewRoom->GetRoomIndex(),
		pNewRoom->GetMaxUser(),
		pNewRoom->GetNumOfUsers(),
		pNewRoom->GetRoomName().c_str(),
		static_cast<INT>( pNewRoom->GetRoomName().length() ),
		pNewRoom->IsPassword(),
		pNewRoom->GetOption()
		);

	SendToRoom( pWaitRoom, NULL, &m_pCasualPackets->RoomInfo, m_pCasualPackets->RoomInfo.GetLength() );

	// ���� ���� �ñ׳��� ���ؼ� ������ �����Ѵ�.
//	AddSignal( CCasualCommon::CasualSignalMessage::ChangeCaptineSignal, (DWORD) pUser->GetRoomIndex(), 0 );
}

// �� ���� ��û ó��
inline VOID CNJeanGate::PacketProcessJoinRoomRequest( CUser* pUser, CCasualPacketJoinRoomRequest* pCasualPacket )
{
	CChannel*	pChannel;	// ����ڰ� ���� ä��
	CRoom*		pWaitRoom;	// ����ڰ� ���� ����
	CRoom*		pRoom;		// ����ڰ� �����ϰ��� �ϴ� ��

	//
	// Step 1 : ���� �˻�
	//

	// ä�ο� ������ ���� ���
	if( pUser->GetChannelIndex() <= 0 ||
		pUser->GetChannelIndex() >= m_nMaxChannel )
	{
		m_pCasualPackets->JoinRoomReply.SetProperty(
			CCasualPacketJoinRoomReply::Result::FailOutOfChannel,
			pUser->GetRoomIndex() );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinRoomReply ), m_pCasualPackets->JoinRoomReply.GetLength() );
		return;
	}
	
	pChannel	= &m_aChannel	[ pUser->GetChannelIndex()	];
	pWaitRoom	= &m_aRoom		[ pUser->GetRoomIndex()		];

	// ���ǿ� ���� ���
	if( !pChannel->IsWaitRoom( pWaitRoom ) )
	{
		m_pCasualPackets->JoinRoomReply.SetProperty(
			CCasualPacketJoinRoomReply::Result::FailExist,
			pUser->GetRoomIndex() );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinRoomReply ), m_pCasualPackets->JoinRoomReply.GetLength() );
		return;
	}

	// Room Index�� ���� ���� ���
	if( pCasualPacket->GetRoomIndex() < 0 ||
		pCasualPacket->GetRoomIndex() >= m_nMaxRoom )
	{
		m_pCasualPackets->JoinRoomReply.SetProperty(
			CCasualPacketJoinRoomReply::Result::FailOutOfRoom,
			pUser->GetRoomIndex() );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinRoomReply ), m_pCasualPackets->JoinRoomReply.GetLength() );
		return;
	}
	pRoom		= &m_aRoom		[pCasualPacket->GetRoomIndex()];

	// Room�� �ٸ� ä�ο� �����ϴ� ���
	if( pRoom->GetChannelIndex() != pUser->GetChannelIndex() )
	{
		m_pCasualPackets->JoinRoomReply.SetProperty(
			CCasualPacketJoinRoomReply::Result::FailOutOfRoom,
			pUser->GetRoomIndex() );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinRoomReply ), m_pCasualPackets->JoinRoomReply.GetLength() );
		return;
	}

	// ����ڰ� �����Ϸ��� ������ ���� ���� ���
	if( pRoom->IsEmpty() )
	{
		m_pCasualPackets->JoinRoomReply.SetProperty(
			CCasualPacketJoinRoomReply::Result::FailClose,
			pUser->GetRoomIndex() );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinRoomReply ), m_pCasualPackets->JoinRoomReply.GetLength() );
		return;
	}


	//
	// Step 2 : �濡 ����
	//
	SHORT nResult = CCasualPacketJoinRoomReply::Result::FailETC;
	if( !IsCorrectCondition( nResult, pUser, PacketCondition::JoinRoom, reinterpret_cast<ULONG_PTR>( pRoom ) ) )
	{
		m_pCasualPackets->JoinRoomReply.SetProperty(
//			CCasualPacketJoinRoomReply::Result::FailETC,
			nResult,
			pUser->GetRoomIndex() );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinRoomReply ), m_pCasualPackets->JoinRoomReply.GetLength() );
		return;
	}

	if( pRoom->IsPassword() )
	{
		if( !pRoom->IsCorrectPassword( pCasualPacket->GetPass() ) )
		{
			m_pCasualPackets->JoinRoomReply.SetProperty(
				CCasualPacketJoinRoomReply::Result::FailWrongPass,
				pUser->GetRoomIndex() );
			SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinRoomReply ), m_pCasualPackets->JoinRoomReply.GetLength() );
			return;
		}
	}


	BOOL	bLeaveNotify = FALSE;

	// ���ǿ��� ������.
	pWaitRoom->Leave( pUser );

	// �濡 �����Ѵ�.
	BOOL bRet;
	bRet = pRoom->Join( pUser );

	// ���� ���� ���
	if( !bRet )
	{
		pWaitRoom->Join( pUser );
		pUser->SetRoomIndex( pWaitRoom->GetRoomIndex() );

		m_pCasualPackets->JoinRoomReply.SetProperty(
			CCasualPacketJoinRoomReply::Result::FailFull,
			pUser->GetRoomIndex() );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinRoomReply ), m_pCasualPackets->JoinRoomReply.GetLength() );
		return;
	}
	pUser->SetRoomIndex( pCasualPacket->GetRoomIndex() );


	//
	// �� �ȿ����� �ڽ��� index�� �����Ѵ�.
	//
	pUser->SetRoomInnerIndex( pRoom->AllocEmptyInnerIndex() );


	//
	// Step 3 : ���� ����ڵ鿡�� ���� ����ڰ� ���ǿ��� �������� �뺸
	//
	m_pCasualPackets->RemoveUser.SetProperty(
		pUser->GetID().c_str(),
		static_cast<INT>( pUser->GetID().length() )
		);

	SendToRoom( pWaitRoom, pUser, &m_pCasualPackets->RemoveUser, m_pCasualPackets->RemoveUser.GetLength() );


	//
	// Step 4 : ����� ����ڵ��� ���� ��ȯ
	//
	OnJoinReply( pUser, pRoom );

	//
	// Step 5 : ���� ����ڿ��� ���� ������ �ٲ������ �����Ѵ�.
	//

	m_pCasualPackets->RoomInfoChange.SetProperty(
		pRoom->GetRoomIndex(),
		pRoom->GetMaxUser(),
		pRoom->GetNumOfUsers(),
		pRoom->IsPassword(),
		pRoom->GetOption()
		);

	SendToRoom( pWaitRoom, NULL, &m_pCasualPackets->RoomInfoChange, m_pCasualPackets->RoomInfoChange.GetLength() );
}

// �濡�� ������ ��û ó��
inline VOID CNJeanGate::PacketProcessLeaveRoomRequest( CUser* pUser, CCasualPacketLeaveRoomRequest* pCasualPacket )
{
	// ���� �� �ִ��� ���ǰ˻�..
	SHORT nResult = CCasualPacketLeaveRoomReply::Result::Fail;
	if( !IsCorrectCondition( nResult, pUser, PacketCondition::LeaveRoom ) )
	{
		m_pCasualPackets->LeaveRoomReply.SetProperty(
//			CCasualPacketLeaveRoomReply::Result::Fail
			nResult
			);
		SendData(
			pUser,
			reinterpret_cast<const CHAR*>( &m_pCasualPackets->LeaveRoomReply ),
			m_pCasualPackets->LeaveRoomReply.GetLength()
			);
		return;
	}
	else
	{
		m_pCasualPackets->LeaveRoomReply.SetProperty(
			CCasualPacketLeaveRoomReply::Result::Success
			);
		SendData(
			pUser,
			reinterpret_cast<const CHAR*>( &m_pCasualPackets->LeaveRoomReply ),
			m_pCasualPackets->LeaveRoomReply.GetLength()
			);
	}

	// �濡�� ������ ����.. �ڽ��� ���� ���� ��Ű�� �Ͱ� �����ϴ�.
	AddSignal(
		CCasualCommon::CasualSignalMessage::CompulseExitSignal,
		reinterpret_cast<ULONG_PTR>( pUser ),
		static_cast<ULONG_PTR>( CCasualPacketCompulsionExited::Type::Mine )	// ���� Ÿ��
		);
}

// �濡 �ڵ� ���� ��û ó��
inline VOID CNJeanGate::PacketProcessAutoJoinRoomRequest( CUser* pUser, CCasualPacketAutoJoinRoomRequest* pCasualPacket )
{
	CChannel*	pChannel;	// ����ڰ� ���� ä��
	CRoom*		pWaitRoom;	// ����ڰ� ���� ����

	//
	// Step 1 : ���� �˻�
	//

	// ä�ο� ������ ���� ���
	if( pUser->GetChannelIndex() <= 0 ||
		pUser->GetChannelIndex() >= m_nMaxChannel )
	{
		m_pCasualPackets->AutoJoinRoomReply.SetProperty(
			CCasualPacketAutoJoinRoomReply::Result::FailOutOfChannel,
			pUser->GetRoomIndex() );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->AutoJoinRoomReply ), m_pCasualPackets->AutoJoinRoomReply.GetLength() );
		return;
	}
	
	pChannel	= &m_aChannel	[ pUser->GetChannelIndex()	];
	pWaitRoom	= &m_aRoom		[ pUser->GetRoomIndex()		];

	// ���ǿ� ���� ���
	if( !pChannel->IsWaitRoom( pWaitRoom ) )
	{
		m_pCasualPackets->AutoJoinRoomReply.SetProperty(
			CCasualPacketAutoJoinRoomReply::Result::FailExist,
			pUser->GetRoomIndex() );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->AutoJoinRoomReply ), m_pCasualPackets->AutoJoinRoomReply.GetLength() );
		return;
	}

	// ����ڰ� �ڵ������� �� �� ���� ��� : ���� Ŭ�������� ó��
	SHORT nResult = CCasualPacketAutoJoinRoomReply::Result::FailExist;
	if( !IsCorrectCondition( nResult, pUser, PacketCondition::AutoJoinRoom ) )
	{
		m_pCasualPackets->AutoJoinRoomReply.SetProperty(
//			CCasualPacketJoinRoomReply::Result::FailETC,
			nResult,
			pUser->GetRoomIndex() );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->AutoJoinRoomReply ), m_pCasualPackets->AutoJoinRoomReply.GetLength() );
		return;
	}


	CRoom*		pOpenRoom;	// �ڵ����� ������ ��

	//
	// Step 2 : �ڵ� ����
	//
	CRoom*		pTmpRoom;	// Ư���濡 ���� �ʱ� ���� ����ϴ� ����
	pTmpRoom	= pChannel->GetFirstRoom();
	pOpenRoom	= NULL;

	while( pTmpRoom )
	{
		// �濡 �� �� �� �ִ��� �˻�
		if( pTmpRoom->GetNumOfUsers() < pTmpRoom->GetMaxUser()	&&
			pTmpRoom->GetCaptine() != NULL							&& 	// ������ ������ ���� �������� ��
			!pTmpRoom->IsPassword()									// ����� ���� ��
			)
		{
			if( !IsCorrectCondition( nResult, pUser, PacketCondition::JoinRoom, reinterpret_cast<ULONG_PTR>( pTmpRoom ) ) )
			{
				pTmpRoom = pChannel->GetNextRoom();
				continue;
			}

			if( pTmpRoom->GetRoomIndex() == pCasualPacket->GetLastRoomIndex() )
			{
				pOpenRoom = pTmpRoom;		// case A??? �Ƿ�Ӧ���ڴ�break��ֱ�ӽ����ϴε�room
			}
			else
			{
				break;
			}
		}

		pTmpRoom = pChannel->GetNextRoom();
	}

	// ã���� ã�� ���� ���� �ƴϸ� ���� case A�� ���� ������ ������ ����, ������ NULL
	if( pTmpRoom )
		pOpenRoom = pTmpRoom;

	//
	// Step 3 : ���� ���н� ���� �޽��� ����
	//
	if( !pOpenRoom )
	{
		// ���� ���� ����
		m_pCasualPackets->AutoJoinRoomReply.SetProperty(
			CCasualPacketAutoJoinRoomReply::Result::FailNone
			);
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->AutoJoinRoomReply ), m_pCasualPackets->AutoJoinRoomReply.GetLength() );
		return;
	}
	nResult = CCasualPacketAutoJoinRoomReply::Result::FailExist;
	/*/
	if( !IsCorrectCondition( nResult, pUser, PacketCondition::JoinRoom, reinterpret_cast<ULONG_PTR>( pOpenRoom ) ) )
	{
		m_pCasualPackets->AutoJoinRoomReply.SetProperty(
//			CCasualPacketJoinRoomReply::Result::FailETC,
			nResult,
			pUser->GetRoomIndex() );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->AutoJoinRoomReply ), m_pCasualPackets->AutoJoinRoomReply.GetLength() );
		return;
	}
	/*/



	//
	// Step 5 : ���� ����ڿ��� ����� ���� ����
	//
	pWaitRoom->Leave( pUser );

	m_pCasualPackets->RemoveUser.SetProperty(
		pUser->GetID().c_str(),
		static_cast<INT>( pUser->GetID().length() )
		);
	SendToRoom( pWaitRoom, pUser, &m_pCasualPackets->RemoveUser, m_pCasualPackets->RemoveUser.GetLength() );

	BOOL bRet = pOpenRoom->Join( pUser );

	// ���� ���� ���
	if( !bRet )
	{
		pWaitRoom->Join( pUser );
		pUser->SetRoomIndex( pWaitRoom->GetRoomIndex() );

		m_pCasualPackets->AutoJoinRoomReply.SetProperty(
			CCasualPacketAutoJoinRoomReply::Result::FailFull,
			pUser->GetRoomIndex() );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->AutoJoinRoomReply ), m_pCasualPackets->AutoJoinRoomReply.GetLength() );
		return;
	}
	pUser->SetRoomIndex( pOpenRoom->GetRoomIndex() );


	//
	// �� �ȿ����� �ڽ��� index�� �����Ѵ�.
	//
	pUser->SetRoomInnerIndex( pOpenRoom->AllocEmptyInnerIndex() );


	//
	// Step 4 : ���� ���� ����
	//
	OnAutoJoinReply( pUser, pOpenRoom );

	//
	// Step 7 : ���� ����ڿ��� ���� ������ �ٲ������ �����Ѵ�.
	//
	m_pCasualPackets->RoomInfoChange.SetProperty(
		pOpenRoom->GetRoomIndex(),
		pOpenRoom->GetMaxUser(),
		pOpenRoom->GetNumOfUsers(),
		pOpenRoom->IsPassword(),
		pOpenRoom->GetOption()
		);
	SendToRoom( pWaitRoom, NULL, &m_pCasualPackets->RoomInfoChange, m_pCasualPackets->RoomInfoChange.GetLength() );
}

// ����� ã�� ��û ó��
inline VOID CNJeanGate::PacketProcessFindRequest( CUser* pUser, CCasualPacketFindRequest* pCasualPacket )
{
	CUser* pFindUser = NULL;
	switch( pCasualPacket->GetFindType() )
	{
	case CCasualPacketFindRequest::FindType::ID :
		pFindUser = m_UserMap.FindFromID( pCasualPacket->GetFindUser() );
		break;

	case CCasualPacketFindRequest::FindType::Nick :
		pFindUser = m_UserMap.FindFromNick( pCasualPacket->GetFindUser() );
		break;
	}


	// ����ڸ� ã�� ���� ���
	if( pFindUser == NULL )
	{
		m_pCasualPackets->FindReply.SetProperty(
			CCasualPacketFindReply::Result::Fail
			);
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->FindReply ), m_pCasualPackets->FindReply.GetLength() );
	}

	// ����ڸ� ã�� ���
	else
	{
		// ����ڰ� ��� ä�ο� �ִ����� �˻�
		if( pFindUser->GetChannelIndex() == 0 )
		{
			m_pCasualPackets->FindReply.SetProperty(
				CCasualPacketFindReply::Result::Success,
				pFindUser->GetChannelIndex(),
				pFindUser->GetRoomIndex(),
				pFindUser->GetID().c_str(),
				static_cast<INT>( pFindUser->GetID().length() ),
				pFindUser->GetNick().c_str(),
				static_cast<INT>( pFindUser->GetNick().length() ),
				TRUE
			);
		}
		else
		{
			CChannel*	pFindUserChannel	= &m_aChannel	[ pFindUser->GetChannelIndex() ];
			CRoom*		pFindUserRoom		= &m_aRoom		[ pFindUser->GetRoomIndex() ];
			BOOL		bWaitRoom;

			if( pFindUserChannel->IsWaitRoom( pFindUserRoom ) )
				bWaitRoom = TRUE;
			else
				bWaitRoom = FALSE;

			m_pCasualPackets->FindReply.SetProperty(
				CCasualPacketFindReply::Result::Success,
				pFindUser->GetChannelIndex(),
				pFindUser->GetRoomIndex(),
				pFindUser->GetID().c_str(),
				static_cast<INT>( pFindUser->GetID().length() ),
				pFindUser->GetNick().c_str(),
				static_cast<INT>( pFindUser->GetNick().length() ),
				bWaitRoom
				);
		}
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->FindReply ), m_pCasualPackets->FindReply.GetLength() );
	}
}

// ä�� �޽���
inline VOID CNJeanGate::PacketProcessChat( CUser* pUser, CCasualPacketChat* pCasualPacket )
{
	//
	// Step 1 : ���� �˻�
	//

	INT nRoomIndex = pUser->GetRoomIndex();

	if( nRoomIndex < 0 || pUser->GetChannelIndex() <= 0 )
		return;

	//
	// Step 2 :  ���� ����ڰ� ���� ���� ��� ����ڿ��� �����Ѵ�.
	//
	SendToRoom( &m_aRoom[nRoomIndex], pUser, pCasualPacket, pCasualPacket->GetLength() );
}


// ����
inline VOID CNJeanGate::PacketProcessMemo( CUser* pUser, CCasualPacketMemo* pCasualPacket )
{
	CUser* pFindUser = NULL;
	switch( pCasualPacket->GetFindType() )
	{
	case CCasualPacketMemo::FindType::ID :
		pFindUser = m_UserMap.FindFromID( pCasualPacket->GetReceiver() );
		break;

	case CCasualPacketMemo::FindType::Nick :
		pFindUser = m_UserMap.FindFromNick( pCasualPacket->GetReceiver() );
		break;
	}

	// ����ڸ� ã�� ���� ���
	if( pFindUser == NULL )
	{
		m_pCasualPackets->MemoReply.SetProperty(
			pCasualPacket->GetFindType(),
			CCasualPacketMemoReply::Result::Fail,
			pCasualPacket->GetReceiver(),
			static_cast<SHORT>( ::strlen( pCasualPacket->GetReceiver() ) )
			);
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->MemoReply ), m_pCasualPackets->MemoReply.GetLength() );
	}

	// ����ڸ� ã�� ���
	else
	{
		m_pCasualPackets->MemoReply.SetProperty(
			pCasualPacket->GetFindType(),
			CCasualPacketMemoReply::Result::Success,
			pCasualPacket->GetReceiver(),
			static_cast<SHORT>( ::strlen( pCasualPacket->GetReceiver() ) )
			);
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->MemoReply ), m_pCasualPackets->MemoReply.GetLength() );

		SendData( pFindUser, reinterpret_cast<const CHAR*>( pCasualPacket ), pCasualPacket->GetLength() );
	}
}

// �ʴ��ϱ�
inline VOID CNJeanGate::PacketProcessInvite( CUser* pUser, CCasualPacketInvite* pCasualPacket )
{
	//
	// Step 1 : ���� �˻�
	//
	CChannel*	pChannel;
	CRoom*		pRoom;
	BOOL		bWaitRoom;

	GetUserPlace( pUser, pChannel, pRoom, bWaitRoom );

	if( pRoom == NULL || bWaitRoom )
	{
		m_pCasualPackets->InviteReply.SetProperty(
			pCasualPacket->GetFindType(),
			CCasualPacketInviteReply::Result::Fail,
			pCasualPacket->GetReceiver(),
			static_cast<SHORT>( ::strlen( pCasualPacket->GetReceiver() ) )
			);
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->InviteReply ), m_pCasualPackets->InviteReply.GetLength() );
	}

	CUser* pFindUser = NULL;
	switch( pCasualPacket->GetFindType() )
	{
	case CCasualPacketInvite::FindType::ID :
		pFindUser = m_UserMap.FindFromID( pCasualPacket->GetReceiver() );
		break;

	case CCasualPacketInvite::FindType::Nick :
		pFindUser = m_UserMap.FindFromNick( pCasualPacket->GetReceiver() );
		break;
	}

	// ����ڸ� ã�� ���� ���
	if( pFindUser == NULL )
	{
		m_pCasualPackets->InviteReply.SetProperty(
			pCasualPacket->GetFindType(),
			CCasualPacketInviteReply::Result::Fail,
			pCasualPacket->GetReceiver(),
			static_cast<SHORT>( ::strlen( pCasualPacket->GetReceiver() ) )
			);
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->InviteReply ), m_pCasualPackets->InviteReply.GetLength() );
	}

	// ����ڸ� ã�� ���
	else
	{
		m_pCasualPackets->InviteReply.SetProperty(
			pCasualPacket->GetFindType(),
			CCasualPacketInviteReply::Result::Success,
			pCasualPacket->GetReceiver(),
			static_cast<SHORT>( ::strlen( pCasualPacket->GetReceiver() ) )
			);
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->InviteReply ), m_pCasualPackets->InviteReply.GetLength() );

		pCasualPacket->SetIndex(
			pUser->GetChannelIndex(),
			pUser->GetRoomIndex()
			);
		if( pRoom->IsPassword() )
		{
			pCasualPacket->SetPass(
				pRoom->GetPassword().c_str(),
				static_cast<INT>( pRoom->GetPassword().length() ) );
		}

		SendData( pFindUser, reinterpret_cast<const CHAR*>( pCasualPacket ), pCasualPacket->GetLength() );
	}
}

inline VOID
CNJeanGate::PacketProcessCompulseExitRequest( CUser* pUser, CCasualPacketCompulsionExitRequest* pCasualPacket )
{
	CUser*		pExitUser;	// ����� �����
	CChannel*	pChannel;	// ����� ����ڰ� ���� ä��
	CRoom*		pRoom;		// ����� ����ڰ� ���� ��
	BOOL		bWaitRoom;	// pRoom�� �������� ����

	//
	// ���� �˻�
	//
	pExitUser = m_UserMap.FindFromID( pCasualPacket->GetUserID() );
	if( pExitUser == NULL )
	{
		m_pCasualPackets->CompulseExitReply.SetProperty(
			CCasualPacketCompulsionExitReply::Result::FailNotExist,
			pCasualPacket->GetUserID(),
			static_cast<INT>( strlen( pCasualPacket->GetUserID() ) )
			);
		SendData(
			pUser,
			reinterpret_cast<const CHAR*>( &m_pCasualPackets->CompulseExitReply ),
			m_pCasualPackets->CompulseExitReply.GetLength()
			);
		return;
	}

	GetUserPlace( pExitUser, pChannel, pRoom, bWaitRoom );

	if( !pRoom || bWaitRoom )
	{
		m_pCasualPackets->CompulseExitReply.SetProperty(
			CCasualPacketCompulsionExitReply::Result::FailNotExist,
			pExitUser->GetID().c_str(),
			static_cast<INT>( pExitUser->GetID().length() )
			);
		SendData(
			pUser,
			reinterpret_cast<const CHAR*>( &m_pCasualPackets->CompulseExitReply ),
			m_pCasualPackets->CompulseExitReply.GetLength()
			);
		return;
	}

	SHORT nResult = CCasualPacketCompulsionExitReply::Result::FailETC;
	if( !IsCorrectCondition( nResult, pUser, PacketCondition::CompulseExit, reinterpret_cast<ULONG_PTR>( pExitUser ), reinterpret_cast<ULONG_PTR>( pRoom ) ) )
	{
		m_pCasualPackets->CompulseExitReply.SetProperty(
//			CCasualPacketCompulsionExitReply::Result::FailETC,
			nResult,
			pExitUser->GetID().c_str(),
			static_cast<INT>( pExitUser->GetID().length() )
			);
		SendData(
			pUser,
			reinterpret_cast<const CHAR*>( &m_pCasualPackets->CompulseExitReply ),
			m_pCasualPackets->CompulseExitReply.GetLength()
			);
		return;
	}

	m_pCasualPackets->CompulseExitReply.SetProperty(
		CCasualPacketCompulsionExitReply::Result::Success,
		pExitUser->GetID().c_str(),
		static_cast<INT>( pExitUser->GetID().length() )
		);
	SendData(
		pUser,
		reinterpret_cast<const CHAR*>( &m_pCasualPackets->CompulseExitReply ),
		m_pCasualPackets->CompulseExitReply.GetLength()
		);

	AddSignal(
		CCasualCommon::CasualSignalMessage::CompulseExitSignal,
		reinterpret_cast<ULONG_PTR>( pExitUser ),
		static_cast<ULONG_PTR>( CCasualPacketCompulsionExited::Type::Normal )	// ���� Ÿ��
		);
}

VOID CNJeanGate::OnLoginSuccess( CUser* pUser )
{
}

VOID
CNJeanGate::OnOpenRoomReply( CUser* pUser, CRoom* pRoom )
{
	// Open ������ �����Ѵ�.
	m_pCasualPackets->OpenRoomReply.SetProperty(
		CCasualPacketOpenRoomReply::Result::Success,
		pRoom->GetRoomIndex() );
	SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->OpenRoomReply ), m_pCasualPackets->OpenRoomReply.GetLength() );
}

VOID CNJeanGate::OnJoinReply( CUser* pUser, CRoom* pRoom )
{
	// ������ ����ڿ��� ���� ������ ����
	m_pCasualPackets->JoinRoomReply.SetProperty(
		CCasualPacketJoinRoomReply::Result::Success,
		pUser->GetRoomIndex() );
	SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinRoomReply ), m_pCasualPackets->JoinRoomReply.GetLength() );
}

VOID CNJeanGate::OnAutoJoinReply( CUser* pUser, CRoom* pRoom )
{
	// ������ ����ڿ��� ���� ������ ����
	m_pCasualPackets->AutoJoinRoomReply.SetProperty(
		CCasualPacketAutoJoinRoomReply::Result::Success,
		pUser->GetRoomIndex() );
	SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->AutoJoinRoomReply ), m_pCasualPackets->AutoJoinRoomReply.GetLength() );
}