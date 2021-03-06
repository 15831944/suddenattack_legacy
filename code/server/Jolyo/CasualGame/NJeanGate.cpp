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
	INT		nTickTime;				// Tick 발생 타임 (ms 단위)
	INT		nMaxTick;				// 등록할 타이머의 최대 Tick (반드시 Max값을 넣지 않아도 된다. 다만 이 값을 넘는 비율만큼의 오버헤드가 있다.
	INT		nTimerSignalQueueSize;	// 타이머가 한번 발생시키는 시그널의 개수 : Create에서 선언한 최대 시그널의 개수를 넘어서는 안된다.
	INT		nAliveTickError;		// Alive 처리를 위한 Timeout Tick값의 오차 허용 범위
	INT		nAliveTimeoutCount;		// nAliveTimeoutCount * nAliveTickError = Timeout Tick

	nTickTime				= 1000;
	nMaxTick				= 10;
	nTimerSignalQueueSize	= 500;
	nAliveTickError			= 3;
	nAliveTimeoutCount		= 10;		// 30초 단위로 Ping 체크
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
	// 여기에 소스 코드를 추가한다. : n개의 Listener를 Create 시켜준 뒤에 CreateEnd를 호출한다.
	//


	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 작성자 : 강진협
	// 작성일 : 2004년 5월 21일
	// 내  용 : Client Listener 생성
	//		1. NJean와 연결되는 네트워크는 1개의 Listen이며 ServicerIndex::Client의 인덱스를 가진다.
	//

	INT		nServicerIndex;			// 연결되는 서비서 인덱스
	INT		nNumOfCommSockets;		// 해당 서비서를 통해 연결될 수 있는 클라이언트의 개수
	INT		nAcceptPoolSize;		// 해당 서비서의 Accept Pool 크기

	// Listener 생성
	nServicerIndex		= ServicerIndex::Client;
	nNumOfCommSockets	= nMaxClients;
	nAcceptPoolSize		= 4;
	bRet = CreateListener( nServicerIndex, nNumOfCommSockets, nAcceptPoolSize );
	if( !bRet ) return FALSE;

	// 해당 Listener의 Client들의 메모리 풀 생성
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

	// Listener 생성의 끝
	bRet = CreateEnd();
	if( !bRet ) return FALSE;
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////




	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 작성자 : 강진협
	// 작성일 : 2004년 5월 22일
	// 내  용 : Room에 관련된 객체 초기화
	//
	m_nMaxRoom			= m_nNumOfClients;
	m_nMaxRoomUser		= nMaxRoomUser;

	m_baTmpRoomInnerIndex	= new BOOL[m_nMaxRoomUser];

	m_InnerManagerUnusedRoom	.Init();
	m_InnerManagerUsedRoom		.Init();


	// 전체 Room을 한번에 찾기 위한 InnerIndex 등록
	m_aRoom = new CRoom[m_nMaxRoom];
	for( INT i = 0; i < m_nMaxRoom; i++ )
	{
		m_aRoom[i].SetRoomIndex	( i );
		m_aRoom[i].SetPocket	( NewPocket( PocketType::Room ) );
	}


	//
	// 채널 생성 : 임시로 채널 2개, 채널당 Room 20개로 한다. 사용자 200으로 한다.
	//
	INT		nRooms;	// 채널당 방의 개수
	INT		nUsers; // 채널당 사용자 개수
	string	strChannelName;

	m_nMaxChannel	= nMaxChannels + 1;		// 0 痰鱗청唐쏵흙channel돨실�サ캣騈�

	m_aChannel = new CChannel[m_nMaxChannel];
	for( INT i = 1; i < m_nMaxChannel; i++ )
	{
		// 하위 클래스에서 nUsers, nRooms, strChannelName을 가져온다.
		InitChannel( i, nUsers, nRooms, strChannelName );

		m_aChannel[i].Init( nUsers, nRooms, strChannelName, i );

		m_aChannel[i].SetPocket	( NewPocket( PocketType::Channel ) );

		m_aRoom[i-1].Init( i );
		m_aRoom[i-1].Open( nUsers, "대기실" );
		m_aChannel[i].SetWaitRoom( &m_aRoom[i-1] );
	}
	m_aChannel[0].Init( m_nNumOfClients, 0, "청쏵channel", 0 );

//	for( INT i = m_nMaxChannel - 1; i < m_nMaxRoom; i++ )
	for( INT i = m_nMaxRoom - 1; i >= m_nMaxChannel - 1; i-- )//？？？ 槨痂척꼇角m_nMaxRoom
	{
		m_InnerManagerUnusedRoom.Add( m_aRoom[i].GetInnerObjectForGame() );
	}


	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////


	// 패킷들 생성
	m_pCasualPackets	= new TLibPackets;
	
	

	return TRUE;
}

BOOL CNJeanGate::Start()
{
	// 여기에 소스 코드를 추가한다.

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 작성자 : 강진협
	// 작성일 : 2004년 5월 21일
	// 내  용 : Client Listener Accept 시작
	//
	INT		nServicerIndex;			// 연결되는 서비서 인덱스

	nServicerIndex		= ServicerIndex::Client;
	StartListen( nServicerIndex, m_nPort );
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	INT		nAliveTickError;		// Alive 처리를 위한 Timeout Tick값의 오차 허용 범위

	nAliveTickError			= 3;
	
	// Start는 blocking 함수이다.
	// Start는 모든 서버의 메시지를 처리하는 부분을 담고 있다.
	// 만일 return FALSE가 생긴다면, Start 초기화 실패인 경우이다.
	return CSessionGate::Start( nAliveTickError );
}

VOID CNJeanGate::Destroy()
{
	// 여기에 소스 코드를 추가한다.

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 작성자 : 강진협
	// 작성일 : 2004년 5월 21일
	// 내  용 : User Pool을 제거한다.
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
	// 여기에 소스 코드를 추가한다.

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
	// 여기에 소스 코드를 추가한다.

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
	// 여기에 소스 코드를 추가한다.

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
	// 여기에 소스 코드를 추가한다.

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
	// 여기에 소스 코드를 추가한다.

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
	// 여기에 소스 코드를 추가한다.
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
	// 여기에 소스 코드를 추가한다.
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
	// 여기에 소스 코드를 추가한다.
	switch( nServicerIndex )
	{
	case ServicerIndex::Client :
		break;

	default:
		return;
	}

	CUser* pUser = (CUser*) pPeer;

	pUser->SetLogined		( FALSE	);		// 로그인 중이 아님
	pUser->SetService		( FALSE	);		// 서비스 중이 아님
	pUser->SetMapped		( FALSE );		// 맵에 등록되지 않음
	pUser->SetRoomIndex		( -1	);		// 방에 참여중이 아님
	pUser->SetChannelIndex	( -1	);		// 채널에 참여중이 아님

	OnCasualAccept( pUser );
}

VOID CNJeanGate::OnConnect( CPeer* pPeer, INT nServicerIndex )
{
	// 여기에 소스 코드를 추가한다.
	OnCasualConnect( (CUser*)pPeer );
}

VOID CNJeanGate::OnClose( CPeer* pPeer, INT nServicerIndex )
{
	// 여기에 소스 코드를 추가한다.
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
	// 사용자가 서비스 중이 아니면 방에 입장하지도, 로그인 세션이 등록되지도 않는다.
	//
	if( !pUser->IsService() || pUser->GetChannelIndex() < 0 )
		return;

	//
	// 사용자가 대기채널에 있으면
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
	// 채널에 입장중인 경우
	//

	// 퇴장하는 사용자가 속한 방과 채널의 Ref를 가져온다.
	CRoom*		pRoom		= &m_aRoom[pUser->GetRoomIndex()];
	CChannel*	pChannel	= &m_aChannel[pUser->GetChannelIndex()];

	// 채널에서 나간다.
	pChannel->Leave( pUser );

	// 일반 방에 Join 중인 경우
	if( !pChannel->IsWaitRoom( pRoom ) )
	{
		// 퇴장하는 방에 남아 있는 사용자들에게 퇴장 정보를 전달
		m_pCasualPackets->RemoveRoomUser.SetProperty(
			pUser->GetID().c_str(),
			static_cast<INT>( pUser->GetID().length() )
			);

		SendToRoom( pRoom, pUser, &m_pCasualPackets->RemoveRoomUser, m_pCasualPackets->RemoveRoomUser.GetLength() );

		// 방에서 나간다. 이 때, 방이 비어 있으면 Unused 객체에 반환한다.
		if( !pRoom->Leave( pUser ) )
		{
			int a = 3;
		}
		pRoom->FreeEmptyInnerIndex( pUser->GetRoomInnerIndex() );

		if( pRoom->IsEmpty() )
		{
			m_InnerManagerUsedRoom		.Remove	( pRoom->GetInnerObjectForGame() );
//			m_InnerManagerUnusedRoom	.Push	( pRoom->GetInnerObjectForGame() );

			// 대기실의 모든 사용자에게 방이 닫혔음을 전달한다.
			m_pCasualPackets->RemoveRoom.SetProperty(
				pRoom->GetRoomIndex()
				);

			SendToRoom( pChannel->GetWaitRoom(), NULL, &m_pCasualPackets->RemoveRoom, m_pCasualPackets->RemoveRoom.GetLength() );

			// 채널에서 해당 방을 제거한다.
			pChannel->RemoveRoom( pRoom );
		}
		else
		{
			// 대기실의 모든 사용자에게 방정보 변경을 전달한다.
			m_pCasualPackets->RoomInfoChange.SetProperty(
				pRoom->GetRoomIndex(),
				pRoom->GetMaxUser(),
				pRoom->GetNumOfUsers(),
				pRoom->IsPassword(),
				pRoom->GetOption()
				);

			SendToRoom( pChannel->GetWaitRoom(), NULL, &m_pCasualPackets->RoomInfoChange, m_pCasualPackets->RoomInfoChange.GetLength() );

			// 자신이 방장이었으면 방장을 새로 뽑기위한 시그널을 보낸다.
			if( pUser->IsCaptine() && !pRoom->IsEmpty() )
			{
				// 방장을 새로 위임할지 결정 
				SHORT	nResult = 0;
				if( IsCorrectCondition( nResult, pUser, PacketCondition::ChangeCaptine, reinterpret_cast<ULONG_PTR>( pRoom ) ) )
				{
					// 방장을 새로 뽑기위한 시그널을 보낸다.
					AddSignal( CCasualCommon::CasualSignalMessage::ChangeCaptineSignal, pUser->GetRoomIndex(), 0 );
				}
				else
				{
					// 현재 방의 모든 사용자를 강퇴시킨다.
					CUser* pRoomUser = pRoom->GetFirstUser();
					while( pRoomUser )
					{
						// 강퇴 시그널을 보낸다.
						AddSignal(
							CCasualCommon::CasualSignalMessage::CompulseExitSignal,							// 강퇴메시지
							reinterpret_cast<ULONG_PTR>( pRoomUser ),										// 강퇴되는 사용자
							static_cast<ULONG_PTR>( CCasualPacketCompulsionExited::Type::CloseRoom )	// 강퇴 타입
							);
						pRoomUser = pRoom->GetNextUser();
					}
				}
			}

			if( pUser->IsCaptine() )
				pUser->SetCaptine( FALSE );
		}

	}
	// 대기실에 Join 중인 경우
	else
	{
		// 대기실에서 나간다.
		pRoom->Leave( pUser );

		// 대기실에 남아 있는 사용자들에게 퇴장 정보를 전달
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
	// 여기에 소스 코드를 추가한다.
	CPacket* pCasualPacket = (CPacket*) pPacket;
	CUser* pUser;
	pUser = static_cast<CUser*>( pPeer );

	switch( nServicerIndex )
	{
	case ServicerIndex::Client :
		switch( pCasualPacket->GetCommand() )
		{
		case CCasualCommon::CasualPacketCommand::LoginRequest :
			// 실제 패킷으로 캐스팅
			CCasualPacketLoginRequest* pLoginRequest;
			pLoginRequest = (CCasualPacketLoginRequest*) pCasualPacket;

			// 실제 패킷 길이와 맞는지 검사
			if( nPacketLength != pLoginRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 패킷 처리
			PacketProcessLoginRequest( pUser, pLoginRequest );
			break;

		case CCasualCommon::CasualPacketCommand::SecLoginRequest :
			// 실제 패킷으로 캐스팅
			CCasualPacketSecLoginRequest* pSecLoginRequest;
			pSecLoginRequest = (CCasualPacketSecLoginRequest*) pCasualPacket;

			// 실제 패킷 길이와 맞는지 검사
			if( nPacketLength != pSecLoginRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 패킷 처리
			PacketProcessSecLoginRequest( pUser, pSecLoginRequest );
			break;

		case CCasualCommon::CasualPacketCommand::JoinChannelRequest :
			// 실제 패킷으로 캐스팅
			CCasualPacketJoinChannelRequest* pJoinChannelRequest;
			pJoinChannelRequest = (CCasualPacketJoinChannelRequest*) pCasualPacket;

			// 실제 패킷 길이와 맞는지 검사
			if( nPacketLength != pJoinChannelRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 서비스 중인지 검사
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 패킷 처리
			PacketProcessJoinChannelRequest( pUser, pJoinChannelRequest );
			break;

		case CCasualCommon::CasualPacketCommand::LeaveChannelRequest :
			// 실제 패킷으로 캐스팅
			CCasualPacketLeaveChannelRequest* pLeaveChannelRequest;
			pLeaveChannelRequest = (CCasualPacketLeaveChannelRequest*) pCasualPacket;

			// 실제 패킷 길이와 맞는지 검사
			if( nPacketLength != pLeaveChannelRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 서비스 중인지 검사
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 패킷 처리
			PacketProcessLeaveChannelRequest( pUser, pLeaveChannelRequest );
			break;

		case CCasualCommon::CasualPacketCommand::OpenRoomRequest :
			// 실제 패킷으로 캐스팅
			CCasualPacketOpenRoomRequest* pOpenRoomRequest;
			pOpenRoomRequest = (CCasualPacketOpenRoomRequest*) pCasualPacket;

			// 실제 패킷 길이와 맞는지 검사
			if( nPacketLength != pOpenRoomRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 서비스 중인지 검사
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 패킷 처리
			PacketProcessOpenRoomRequest( pUser, pOpenRoomRequest );
			break;

		case CCasualCommon::CasualPacketCommand::JoinRoomRequest :
			// 실제 패킷으로 캐스팅
			CCasualPacketJoinRoomRequest* pJoinRoomRequest;
			pJoinRoomRequest = (CCasualPacketJoinRoomRequest*) pCasualPacket;

			// 실제 패킷 길이와 맞는지 검사
			if( nPacketLength != pJoinRoomRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 서비스 중인지 검사
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 패킷 처리
			PacketProcessJoinRoomRequest( pUser, pJoinRoomRequest );
			break;

		case CCasualCommon::CasualPacketCommand::LeaveRoomRequest :
			// 실제 패킷으로 캐스팅
			CCasualPacketLeaveRoomRequest* pLeaveRoomRequest;
			pLeaveRoomRequest = (CCasualPacketLeaveRoomRequest*) pCasualPacket;

			// 실제 패킷 길이와 맞는지 검사
			if( nPacketLength != pLeaveRoomRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 서비스 중인지 검사
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 패킷 처리
			PacketProcessLeaveRoomRequest( pUser, pLeaveRoomRequest );
			break;

		case CCasualCommon::CasualPacketCommand::AutoJoinRoomRequest :
			// 실제 패킷으로 캐스팅
			CCasualPacketAutoJoinRoomRequest* pAutoJoinRoomRequest;
			pAutoJoinRoomRequest = (CCasualPacketAutoJoinRoomRequest*) pCasualPacket;

			// 실제 패킷 길이와 맞는지 검사
			if( nPacketLength != pAutoJoinRoomRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 서비스 중인지 검사
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 패킷 처리
			PacketProcessAutoJoinRoomRequest( pUser, pAutoJoinRoomRequest );
			break;

		case CCasualCommon::CasualPacketCommand::FindRequest :

			::OutputDebugString( "FindRequest\n" );

			// 실제 패킷으로 캐스팅
			CCasualPacketFindRequest* pFindRequest;
			pFindRequest = (CCasualPacketFindRequest*) pCasualPacket;

			// 실제 패킷 길이와 맞는지 검사
			if( nPacketLength != pFindRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 서비스 중인지 검사
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 패킷 처리
			PacketProcessFindRequest( pUser, pFindRequest );
			break;

		case CCasualCommon::CasualPacketCommand::Chat :
			// 실제 패킷으로 캐스팅
			CCasualPacketChat* pChat;
			pChat = (CCasualPacketChat*) pCasualPacket;

			// 실제 패킷 길이와 맞는지 검사
			if( nPacketLength != pChat->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 서비스 중인지 검사
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 패킷 처리
			PacketProcessChat( pUser, pChat );
			break;

		case CCasualCommon::CasualPacketCommand::Memo :
			// 실제 패킷으로 캐스팅
			CCasualPacketMemo* pMemo;
			pMemo = (CCasualPacketMemo*) pCasualPacket;

			// 실제 패킷 길이와 맞는지 검사
			if( nPacketLength != pMemo->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 서비스 중인지 검사
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 패킷 처리
			PacketProcessMemo( pUser, pMemo );
			break;

		case CCasualCommon::CasualPacketCommand::Invite :
			// 실제 패킷으로 캐스팅
			CCasualPacketInvite* pInvite;
			pInvite = (CCasualPacketInvite*) pCasualPacket;

			// 실제 패킷 길이와 맞는지 검사
			if( nPacketLength != pInvite->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 서비스 중인지 검사
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 패킷 처리
			PacketProcessInvite( pUser, pInvite );
			break;

		case CCasualCommon::CasualPacketCommand::CompulseExitRequest :
			// 실제 패킷으로 캐스팅
			CCasualPacketCompulsionExitRequest* pCompulseExitRequest;
			pCompulseExitRequest = (CCasualPacketCompulsionExitRequest*) pCasualPacket;

			// 실제 패킷 길이와 맞는지 검사
			if( nPacketLength != pCompulseExitRequest->GetRealLength() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 서비스 중인지 검사
			if( !pUser->IsService() )
			{
				CloseRequest( pPeer );
				break;
			}

			// 패킷 처리
			PacketProcessCompulseExitRequest( pUser, pCompulseExitRequest );
			break;

			/*
		case CCasualCommon::CasualPacketCommand::TestRequest :
			// 실제 패킷으로 캐스팅
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
		// 서비스 중인지 검사
		if( !pUser->IsService() )
		{
			//청唐되쩍냥묘
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

// 방의 Ref를 반환한다.
CRoom* CNJeanGate::GetRoomRef( CUser* pUser)
{
	if( pUser->GetChannelIndex() <= 0 || pUser->GetChannelIndex() >= m_nMaxChannel )
	{
		return NULL;
	}

	return &m_aRoom[ pUser->GetRoomIndex() ];
}

// 방의 Ref를 반환한다.
CRoom* CNJeanGate::GetRoomRef( INT nRoomIndex )
{
	if( nRoomIndex < 0 || nRoomIndex >= m_nMaxRoom )
		return NULL;

	return &m_aRoom[ nRoomIndex ];
}

// 채널의 Ref를 반환한다.
CChannel* CNJeanGate::GetChannelRef( CUser* pUser )
{
	if( pUser->GetChannelIndex() <= 0 || pUser->GetChannelIndex() >= m_nMaxChannel )
	{
		return NULL;
	}

	return &m_aChannel	[ pUser->GetChannelIndex() ];
}

// 채널의 Ref를 반환한다.
CChannel* CNJeanGate::GetChannelRef( INT nChannelIndex )
{
	if( nChannelIndex <= 0 || nChannelIndex >= m_nMaxChannel )
	{
		return NULL;
	}

	return &m_aChannel	[ nChannelIndex	];
}

// 해당 방이 대기실인지를 반환한다.
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

// 해당 방이 대기실인지를 반환한다.
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

// 해당 채널이 대기채널인지를 반환한다.
BOOL CNJeanGate::IsWaitChannel ( CChannel* pChannel )
{
	return pChannel->GetChannelIndex() == 0;
}

BOOL CNJeanGate::IsWaitChannel ( INT nChannel )
{
	return nChannel == 0;
}

// 해당 방이 속한 채널의 Ref를 반환한다.
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


// 해당 방에 메시지를 중계한다.
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

// P2P에서 방장이 될 수 있는지를 설정한다.
inline VOID CNJeanGate::InspectP2PCaptine( CUser* pUser, LPCTSTR strIP )
{
	if( ::strcmp( GetPeerAddr( pUser ), strIP ) == 0 )
		pUser->SetP2PCaptine( TRUE );
	else
		pUser->SetP2PCaptine( FALSE );
}

// 서버 내의 모든 사용자에게 메시지를 전달한다.
VOID CNJeanGate::SendToAllUser( const CPacket* pPacket, INT nPacketSize )
{
	CUser* pUser;

	// 0번 대기 채널의 사용자에게는 메시지를 보내지 않는다.
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

// 채널 내의 대기실에 메시지를 전달한다.
VOID CNJeanGate::SendToWaitRoom( CChannel* pChannel, const CUser* pExeptUser, const CPacket* pPacket, INT nPacketSize )
{
	CRoom* pWaitRoom = pChannel->GetWaitRoom();
	SendToRoom( pWaitRoom, pExeptUser, pPacket, nPacketSize );
}

// 채널 내의 모든 사용자에게 메시지를 전달한다.
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

// 서버 내의 모든 열린 방에 메시지를 전달한다.
VOID CNJeanGate::SendToAllRoomUser( const CPacket* pPacket, INT nPacketSize )
{
	CRoom*	pRoom;
	CUser*	pUser;

	// 0번 대기 채널의 사용자에게는 메시지를 보내지 않는다.
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

// 채널 내의 모든 열린 방에 메시지를 전달한다.
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

// 사용자의 위치를 반환한다.
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
// Response 처리
inline VOID CNJeanGate::LoginReply( CUser* pUser, BOOL bLogined )
{
	if( !bLogined )
	{
		// 클라이언트에 로그인 실패를 통보후 연결 해제
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
		
	// 연결된 세션이 없는 경우
	if( !pFindUser )
	{
		// 맵에 등록
		m_UserMap.Insert( pUser->GetUserNo(), pUser->GetID(), pUser->GetNick(), pUser );

		// 로그인 여부 등록
		pUser->SetLogined( TRUE );

		// 서비스 가능 등록
		pUser->SetService( TRUE );

		// 맵에 등록됨
		pUser->SetMapped( TRUE );

		// 기본 채널에 등록
		m_aChannel[0].Join( pUser );
		pUser->SetChannelIndex( 0 );

		// 클라이언트에 로그인 성공을 통보
		m_pCasualPackets->LoginReply.SetProperty( CCasualPacketLoginReply::LoginResult::Success );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->LoginReply ), m_pCasualPackets->LoginReply.GetLength() );

		OnLoginSuccess( pUser );
	}
	else
	{
		// 로그인 여부 등록
		pUser->SetLogined( TRUE );

		// 클라이언트에 다른곳에서 연결중을 통보
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
// 시그널 처리

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
	// 현재 방의 사람들에게 방장 변경 패킷을 전송한다.
	m_pCasualPackets->RoomCaptineChange.SetProperty(
		pCaptine->GetID().c_str(),
		static_cast<INT>( pCaptine->GetID().length() )
		);
	SendToRoom( pRoom, NULL, &m_pCasualPackets->RoomCaptineChange, m_pCasualPackets->RoomCaptineChange.GetLength() );
}

inline VOID CNJeanGate::SignalProcessCompulseExit( ULONG_PTR wParam, ULONG_PTR lParam )
{
	CUser*		pUser;		// 퇴장하는 사용자
	CRoom*		pRoom;		// 사용자가 속했던 방
	CChannel*	pChannel;	// 사용자가 속했던 채널
	
	CCasualPacketCompulsionExited::Type
				type;	// 강제 퇴장 타입 : 일반 퇴장도 같은 시그널로 처리 하므로 lParam으로 구별을 한다.

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


	// 강제 퇴장 당함을 통지한다.
	if( type != CCasualPacketCompulsionExited::Type::Mine )
	{
		m_pCasualPackets->CompulsionExited.SetProperty(
			type );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->CompulsionExited ), m_pCasualPackets->CompulsionExited.GetLength() );
	}


	// 방에서 나간다. 이 때, 방이 비어 있으면 Unused 객체에 반환한다.
	if( !pRoom->Leave( pUser ) )
	{
		int a = 3;
	}

	pRoom->FreeEmptyInnerIndex( pUser->GetRoomInnerIndex() );
	if( pRoom->IsEmpty() )
	{
		m_InnerManagerUsedRoom		.Remove	( pRoom->GetInnerObjectForGame() );
//		m_InnerManagerUnusedRoom	.Push	( pRoom->GetInnerObjectForGame() );

		// 채널에서 해당 방을 제거한다.
		pChannel->RemoveRoom( pRoom );
	}
	else
	{
		// 자신이 방장이었으면
		if( pUser->IsCaptine() )
		{
			// 방장을 새로 위임할지 결정 
			SHORT nResult = 0;
			if( IsCorrectCondition( nResult, pUser, PacketCondition::ChangeCaptine, reinterpret_cast<ULONG_PTR>( pRoom ) ) )
			{
				// 방장을 새로 뽑기위한 시그널을 보낸다.
				AddSignal( CCasualCommon::CasualSignalMessage::ChangeCaptineSignal, pUser->GetRoomIndex(), 0 );
			}
			else
			{
				// 현재 방의 모든 사용자를 강퇴시킨다.
				CUser* pRoomUser = pRoom->GetFirstUser();
				while( pRoomUser )
				{
					// 강퇴 시그널을 보낸다.
					AddSignal(
						CCasualCommon::CasualSignalMessage::CompulseExitSignal,							// 강퇴메시지
						reinterpret_cast<ULONG_PTR>( pRoomUser ),										// 강퇴되는 사용자
						static_cast<ULONG_PTR>( CCasualPacketCompulsionExited::Type::CloseRoom )	// 강퇴 타입
						);
					pRoomUser = pRoom->GetNextUser();
				}
			}
		}
	}
	pUser->SetCaptine( FALSE );

	// 대기실에 입장을 한다.
	CRoom* pWaitRoom = pChannel->GetWaitRoom();
	pWaitRoom->Join( pUser );
	pUser->SetRoomIndex( pWaitRoom->GetRoomIndex() );

	// 방에서 나간다.
	m_pCasualPackets->LeaveRoomReply.SetProperty( CCasualPacketLeaveRoomReply::Result::Success );
	SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->LeaveRoomReply ), m_pCasualPackets->LeaveRoomReply.GetLength() );

	// 채널 입장을 통보한다. : 대기실 입장과 동일하게 취급
	m_pCasualPackets->JoinChannelReply.SetProperty(
		CCasualPacketJoinChannelReply::Result::Success,
		pWaitRoom->GetNumOfUsers() - 1,
		pChannel->GetNumOfOpenRooms()
		);
	SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinChannelReply ), m_pCasualPackets->JoinChannelReply.GetLength() );



	//
	// Step 6 : 방 리스트를 전달 : Step 4,5와 변경
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
	// Step 4 : 방의 사용자들에게 사용자 퇴장을 전달
	//
	m_pCasualPackets->RemoveRoomUser.SetProperty(
		pUser->GetID().c_str(),
		static_cast<INT>( pUser->GetID().length() )
		);
	SendToRoom( pRoom, NULL, &m_pCasualPackets->RemoveRoomUser, m_pCasualPackets->RemoveRoomUser.GetLength() );


	//
	// Step 5 : 대기실의 사용자들과의 정보 교환
	//

	// 방이 비어있으면, 방이 제거되었음을 통보
	if( pRoom->IsEmpty() )
	{
		m_pCasualPackets->RemoveRoom.SetProperty(
			pRoom->GetRoomIndex()
			);

		// 기존 대기실 사용자에게는 방 정보 변경을 전달
		SendToRoom( pWaitRoom, pUser, &m_pCasualPackets->RemoveRoom, m_pCasualPackets->RemoveRoom.GetLength() );

		m_pCasualPackets->UserInfo.SetProperty(
			pUser->GetGrade(),	
			pUser->GetID().c_str(),
			static_cast<INT>( pUser->GetID().length() ),
			pUser->GetNick().c_str(),
			static_cast<INT>( pUser->GetNick().length() )
			);

		// 기존 대기실 사용자에게는 현재 사용자의 대기실 입장을 전달
		SendToRoom( pWaitRoom, pUser, &m_pCasualPackets->UserInfo, m_pCasualPackets->UserInfo.GetLength() );

		CUser* pWaitUser;
		pWaitUser = pWaitRoom->GetFirstUser();
		while( pWaitUser )
		{
			if( pWaitUser != pUser )
			{
				// 현재 사용자에게는 기존 대기실 사용자의 정보를 전달
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

	// 방이 열려있으면, 방정보가 변경되었음을 통보
	else
	{
		m_pCasualPackets->RoomInfoChange.SetProperty(
			pRoom->GetRoomIndex(),
			pRoom->GetMaxUser(),
			pRoom->GetNumOfUsers(),
			pRoom->IsPassword(),
			pRoom->GetOption()
			);

		// 기존 대기실 사용자에게는 방 정보 변경을 전달
		SendToRoom( pWaitRoom, pUser, &m_pCasualPackets->RoomInfoChange, m_pCasualPackets->RoomInfoChange.GetLength() );

		m_pCasualPackets->UserInfo.SetProperty(
			pUser->GetGrade(),
			pUser->GetID().c_str(),
			static_cast<INT>( pUser->GetID().length() ),
			pUser->GetNick().c_str(),
			static_cast<INT>( pUser->GetNick().length() )
			);

		// 기존 대기실 사용자에게는 현재 사용자의 대기실 입장을 전달
		SendToRoom( pWaitRoom, pUser, &m_pCasualPackets->UserInfo, m_pCasualPackets->UserInfo.GetLength() );

		CUser* pWaitUser;
		pWaitUser = pWaitRoom->GetFirstUser();
		while( pWaitUser )
		{
			if( pWaitUser != pUser )
			{
				// 현재 사용자에게는 기존 대기실 사용자의 정보를 전달
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
// 패킷 처리
//

VOID CNJeanGate::OnPacketLogin( CUser* pUser, LPCTSTR strUserNo, LPCTSTR strID, LPCTSTR strNick )
{
	pUser->SetUserInfo( strUserNo,
						strID,
						strNick	);

	pUser->SetCaptine( FALSE );
//	pUser->SetP2PCaptine( FALSE );
	pUser->SetGameCaptine( TRUE );

	if( pUser->GetIP().compare( GetPeerAddr( pUser ) ) == 0 ) //？？？켜돛뻘콘꼇寧湳찐？
		pUser->SetP2PCaptine( TRUE );
	else
		pUser->SetP2PCaptine( FALSE );

//	pUser->SetP2PCaptine( TRUE );
}

// 로그인 요청 처리
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

	// 암호화 풀기 처리 추가 예정

	// 암호화 : 황순구씨 소스 카피
	CUserSCDApp cDataEn;

	char	szResult[4096];
	char	szOutput[4096];
	char	szKey[100];

	// szKey 설정하기 : 현재 날짜 : ex) 20020101
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

// 채널 입장 요청 처리
inline VOID CNJeanGate::PacketProcessJoinChannelRequest( CUser* pUser, CCasualPacketJoinChannelRequest* pCasualPacket )
{
	CRoom*		pWaitRoom;	// 사용자가 속할 채널의 대기실
	CChannel*	pChannel;	// 사용자가 속할 채널

	//
	// Stpe 1 : 예외 검사
	//

	// 채널 범위가 넘는 경우
	if( pCasualPacket->GetChannelIndex() <= 0 ||
		pCasualPacket->GetChannelIndex() >= m_nMaxChannel )
	{
		m_pCasualPackets->JoinChannelReply.SetProperty(
			CCasualPacketJoinChannelReply::Result::FailOutOfChannel
			);
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinChannelReply ), m_pCasualPackets->JoinChannelReply.GetLength() );
		return;
	}
	
	// 이미 다른 채널에 속한 경우
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
		//뎠품실�ツ柄宙픈슉슛홤hannnel
		m_pCasualPackets->JoinChannelReply.SetProperty(
//			CCasualPacketJoinChannelReply::Result::FailETC
			nResult
			);
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinChannelReply ), m_pCasualPackets->JoinChannelReply.GetLength() );
		return;
	}


	//
	// Step 2 : 채널에 입장
	//

	// 닒m_aChannel[0]櫓藁놔，LoginReply냥묘珂，쉥실�ゼ談崙斡�
	m_aChannel[0].Leave( pUser );

	// 해당 채널에 입장
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
	// Step 3 : 대기실에 입장
	//

	pWaitRoom	= pChannel->GetWaitRoom();

	pWaitRoom->Join( pUser );
	pUser->SetRoomIndex( pWaitRoom->GetRoomIndex() );



	//
	// Step 4 : 대기실안의 사용자들의 정보 교환
	//

	// 입장한 사용자에게 입장 성공을 전달
	m_pCasualPackets->JoinChannelReply.SetProperty(
		CCasualPacketJoinChannelReply::Result::Success,
		pWaitRoom->GetNumOfUsers() - 1,		// 자기 자신은 전체 사용자의 수에 포함하지 않는다.
		pChannel->GetNumOfOpenRooms()
		);
	SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinChannelReply ), m_pCasualPackets->JoinChannelReply.GetLength() );


	//
	// Step 5 : 방의 정보를 전달한다.
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
	// Step 4 : 5와 변경되었음
	//

	// 대기실안의 사용자에게 입장한 사용자의 정보를 전달할 패킷
	m_pCasualPackets->UserInfo.SetProperty(
		pUser->GetGrade(),
		pUser->GetID().c_str(),
		static_cast<INT>( pUser->GetID().length() ),
		pUser->GetNick().c_str(),
		static_cast<INT>( pUser->GetNick().length() )
		);

	// Room안의 사용자에게 입장한 사용자의 정보를 전달
	SendToRoom( pWaitRoom, pUser, &m_pCasualPackets->UserInfo, m_pCasualPackets->UserInfo.GetLength() );

	CUser* pRoomUser = pWaitRoom->GetFirstUser();
	while( pRoomUser )
	{
		if( pRoomUser != pUser )
		{
			// 입장한 사용자에게 Room안의 사용자 정보를 전달
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




	// 방장이 아님을 설정한다.
	pUser->SetCaptine( FALSE );
}

// 채널 퇴장 요청 처리
inline VOID CNJeanGate::PacketProcessLeaveChannelRequest( CUser* pUser, CCasualPacketLeaveChannelRequest* pCausalPacket )
{
	CRoom*		pRoom;		// 사용자가 현재 속한 방
	CRoom*		pWaitRoom;	// 사용자가 속할 채널의 대기실
	CChannel*	pChannel;	// 사용자가 속할 채널

	//
	// Stpe 1 : 예외 검사
	//

	// 채널에 없는 경우
	if( pUser->GetChannelIndex() <= 0 ||
		pUser->GetChannelIndex() >= m_nMaxChannel )
	{
		return;
	}

	pChannel	= &m_aChannel[pUser->GetChannelIndex()];
	pWaitRoom	= pChannel->GetWaitRoom();
	pRoom		= &m_aRoom[pUser->GetRoomIndex()];

	// 대기실에 있지 않은 경우
	if( !pChannel->IsWaitRoom( pRoom ) )
		return;


	//
	// Step 2 : 채널에서 나간다.
	//
	pChannel->Leave( pUser );
	CCasualPacketLeaveChannelReply	leaveChannelPacket;
	SendData( pUser, reinterpret_cast<const CHAR*>( &leaveChannelPacket ), leaveChannelPacket.GetLength() );

	// 대기 채널에 입장한다.
	m_aChannel[0].Join( pUser );
	pUser->SetChannelIndex( 0 );


	// 대기실에서 나간다.
	pWaitRoom->Leave( pUser );

	// 대기실에 남아 있는 사용자들에게 퇴장 정보를 전달
	m_pCasualPackets->RemoveUser.SetProperty(
		pUser->GetID().c_str(),
		static_cast<INT>( pUser->GetID().length() )
		);

	SendToRoom( pWaitRoom, NULL, &m_pCasualPackets->RemoveUser, m_pCasualPackets->RemoveUser.GetLength() );
}

// 방 개설 요청 처리
inline VOID CNJeanGate::PacketProcessOpenRoomRequest( CUser* pUser, CCasualPacketOpenRoomRequest* pCasualPacket )
{
	CChannel*	pChannel;	// 사용자가 속한 채널
	CRoom*		pWaitRoom;	// 사용자가 속한 대기실

	//
	// Step 1 : 예외 검사
	//

	// 방의 최대 참여자가, 실제 방의 사이즈보다 큰 경우
	if( pCasualPacket->GetMaxUsers() > m_nMaxRoomUser )
	{
		m_pCasualPackets->OpenRoomReply.SetProperty(
			CCasualPacketOpenRoomReply::Result::FailOverUsers,
			-1 );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->OpenRoomReply ), m_pCasualPackets->OpenRoomReply.GetLength() );
		return;
	}

	// 채널에 속하지 않은 경우
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

	// 대기실에 없는 경우
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
	// Step 2 : 새 Room 할당
	//

	BOOL		bRet;
	CRoom*		pNewRoom;

	pChannel	= &m_aChannel[ pUser->GetChannelIndex() ];

	bRet = m_InnerManagerUnusedRoom.Pop( pNewRoom );

	// 할당에 실패를 하면
	if( !bRet )
	{
		m_pCasualPackets->OpenRoomReply.SetProperty(
			CCasualPacketOpenRoomReply::Result::FailFull,
			-1 );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->OpenRoomReply ), m_pCasualPackets->OpenRoomReply.GetLength() );
		return;
	}
	pNewRoom->Init( pChannel->GetChannelIndex() );

	// 채널에 개설에 개설된 방을 추가한다.
	bRet = pChannel->AddRoom( pNewRoom );

	// 채널에 방추가가 실패를 하면...
	if( !bRet )
	{
		m_InnerManagerUnusedRoom.Add( pNewRoom->GetInnerObjectForGame() );

		m_pCasualPackets->OpenRoomReply.SetProperty(
			CCasualPacketOpenRoomReply::Result::FailFull,
			-1 );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->OpenRoomReply ), m_pCasualPackets->OpenRoomReply.GetLength() );
		return;
	}

	// 할당에 성공한 Room을 m_InnerManagerUsedRoom에 삽입하고, Room정보를 설정한다.
	m_InnerManagerUsedRoom.Add( pNewRoom->GetInnerObjectForGame() );
	pNewRoom->Open(
		pCasualPacket->GetMaxUsers(),
		pCasualPacket->GetRoomName()
		);



	//
	// Step 3 : 대기실 사용자들에게 현재 사용자가 대기실에서 나갔음을 통보
	//

	// 대기실에서 나간다.
	pWaitRoom->Leave( pUser );

	// 대기실에 남아 있는 사용자들에게 퇴장 정보를 전달
	m_pCasualPackets->RemoveUser.SetProperty(
		pUser->GetID().c_str(),
		static_cast<INT>( pUser->GetID().length() )
		);

	SendToRoom( pWaitRoom, pUser, &m_pCasualPackets->RemoveUser, m_pCasualPackets->RemoveUser.GetLength() );



	//
	// Step 4 : 대기실 사용자들에게 새로 방 개설을 통보
	//

	// Open한 Room에 입장을 한다.
	if( !pNewRoom->Join( pUser ) )
	{
		// 절대로 이쪽에 올 수는 없다.
		return;
	}
	pUser->SetRoomIndex( pNewRoom->GetRoomIndex() );

	// 방을 개설한 자는 0번째 인덱스를 가질 것이다.
	pUser->SetRoomInnerIndex( pNewRoom->AllocEmptyInnerIndex() );

	// 방장으로 설정한다.
	pUser->SetCaptine( TRUE );
	pNewRoom->SetCaptine( pUser );

	// 방에 패스워드 설정 유무를 설정한다.
	pNewRoom->SetPassword( pCasualPacket->IsPassword(), pCasualPacket->GetPassword() );
	
	// 옵션을 설정한다.
	for( INT i = 0; i < CCasualCommon::CasualStringLength::RoomOption; i++ )
		pNewRoom->SetOption( i, pCasualPacket->GetOption( i ) );

	pNewRoom->SetRTT( GetRttTime( pUser ) );

	OnOpenRoomReply( pUser, pNewRoom );
	/*
	// Open 성공을 통지한다.
	m_pCasualPackets->OpenRoomReply.SetProperty(
		CCasualPacketOpenRoomReply::Result::Success,
		pNewRoom->GetRoomIndex() );
	SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->OpenRoomReply ), m_pCasualPackets->OpenRoomReply.GetLength() );
	*/


	// 대기실의 모든 사용자에게 생성된 Room정보를 전달한다.
	
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

	// 방장 변경 시그널을 통해서 방장을 변경한다.
//	AddSignal( CCasualCommon::CasualSignalMessage::ChangeCaptineSignal, (DWORD) pUser->GetRoomIndex(), 0 );
}

// 방 참여 요청 처리
inline VOID CNJeanGate::PacketProcessJoinRoomRequest( CUser* pUser, CCasualPacketJoinRoomRequest* pCasualPacket )
{
	CChannel*	pChannel;	// 사용자가 속한 채널
	CRoom*		pWaitRoom;	// 사용자가 속한 대기실
	CRoom*		pRoom;		// 사용자가 입장하고자 하는 방

	//
	// Step 1 : 예외 검사
	//

	// 채널에 속하지 않은 경우
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

	// 대기실에 없는 경우
	if( !pChannel->IsWaitRoom( pWaitRoom ) )
	{
		m_pCasualPackets->JoinRoomReply.SetProperty(
			CCasualPacketJoinRoomReply::Result::FailExist,
			pUser->GetRoomIndex() );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinRoomReply ), m_pCasualPackets->JoinRoomReply.GetLength() );
		return;
	}

	// Room Index가 영역 밖인 경우
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

	// Room이 다른 채널에 존재하는 경우
	if( pRoom->GetChannelIndex() != pUser->GetChannelIndex() )
	{
		m_pCasualPackets->JoinRoomReply.SetProperty(
			CCasualPacketJoinRoomReply::Result::FailOutOfRoom,
			pUser->GetRoomIndex() );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinRoomReply ), m_pCasualPackets->JoinRoomReply.GetLength() );
		return;
	}

	// 사용자가 입장하려는 순간에 방이 닫힌 경우
	if( pRoom->IsEmpty() )
	{
		m_pCasualPackets->JoinRoomReply.SetProperty(
			CCasualPacketJoinRoomReply::Result::FailClose,
			pUser->GetRoomIndex() );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinRoomReply ), m_pCasualPackets->JoinRoomReply.GetLength() );
		return;
	}


	//
	// Step 2 : 방에 입장
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

	// 대기실에서 나간다.
	pWaitRoom->Leave( pUser );

	// 방에 입장한다.
	BOOL bRet;
	bRet = pRoom->Join( pUser );

	// 방이 꽉찬 경우
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
	// 방 안에서의 자신의 index를 설정한다.
	//
	pUser->SetRoomInnerIndex( pRoom->AllocEmptyInnerIndex() );


	//
	// Step 3 : 대기실 사용자들에게 현재 사용자가 대기실에서 나갔음을 통보
	//
	m_pCasualPackets->RemoveUser.SetProperty(
		pUser->GetID().c_str(),
		static_cast<INT>( pUser->GetID().length() )
		);

	SendToRoom( pWaitRoom, pUser, &m_pCasualPackets->RemoveUser, m_pCasualPackets->RemoveUser.GetLength() );


	//
	// Step 4 : 방안의 사용자들의 정보 교환
	//
	OnJoinReply( pUser, pRoom );

	//
	// Step 5 : 대기실 사용자에게 방의 정보가 바뀌었음을 전달한다.
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

// 방에서 나가기 요청 처리
inline VOID CNJeanGate::PacketProcessLeaveRoomRequest( CUser* pUser, CCasualPacketLeaveRoomRequest* pCasualPacket )
{
	// 나갈 수 있는지 조건검사..
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

	// 방에서 나가는 것은.. 자신을 강제 퇴장 시키는 것과 동일하다.
	AddSignal(
		CCasualCommon::CasualSignalMessage::CompulseExitSignal,
		reinterpret_cast<ULONG_PTR>( pUser ),
		static_cast<ULONG_PTR>( CCasualPacketCompulsionExited::Type::Mine )	// 강퇴 타입
		);
}

// 방에 자동 입장 요청 처리
inline VOID CNJeanGate::PacketProcessAutoJoinRoomRequest( CUser* pUser, CCasualPacketAutoJoinRoomRequest* pCasualPacket )
{
	CChannel*	pChannel;	// 사용자가 속한 채널
	CRoom*		pWaitRoom;	// 사용자가 속한 대기실

	//
	// Step 1 : 예외 검사
	//

	// 채널에 속하지 않은 경우
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

	// 대기실에 없는 경우
	if( !pChannel->IsWaitRoom( pWaitRoom ) )
	{
		m_pCasualPackets->AutoJoinRoomReply.SetProperty(
			CCasualPacketAutoJoinRoomReply::Result::FailExist,
			pUser->GetRoomIndex() );
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->AutoJoinRoomReply ), m_pCasualPackets->AutoJoinRoomReply.GetLength() );
		return;
	}

	// 사용자가 자동입장을 할 수 없는 경우 : 하위 클래스에서 처리
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


	CRoom*		pOpenRoom;	// 자동으로 입장할 방

	//
	// Step 2 : 자동 입장
	//
	CRoom*		pTmpRoom;	// 특정방에 들어가지 않기 위해 사용하는 변수
	pTmpRoom	= pChannel->GetFirstRoom();
	pOpenRoom	= NULL;

	while( pTmpRoom )
	{
		// 방에 더 들어갈 수 있는지 검사
		if( pTmpRoom->GetNumOfUsers() < pTmpRoom->GetMaxUser()	&&
			pTmpRoom->GetCaptine() != NULL							&& 	// 방장이 없으면 아직 개설중인 방
			!pTmpRoom->IsPassword()									// 비번이 없는 방
			)
		{
			if( !IsCorrectCondition( nResult, pUser, PacketCondition::JoinRoom, reinterpret_cast<ULONG_PTR>( pTmpRoom ) ) )
			{
				pTmpRoom = pChannel->GetNextRoom();
				continue;
			}

			if( pTmpRoom->GetRoomIndex() == pCasualPacket->GetLastRoomIndex() )
			{
				pOpenRoom = pTmpRoom;		// case A??? 角뤠壇맡瞳늪break？殮쌈쏵흙�求琯훣oom
			}
			else
			{
				break;
			}
		}

		pTmpRoom = pChannel->GetNextRoom();
	}

	// 찾으면 찾은 방을 설정 아니면 위의 case A에 의해 설정된 방으로 입장, 없으면 NULL
	if( pTmpRoom )
		pOpenRoom = pTmpRoom;

	//
	// Step 3 : 입장 실패시 실패 메시지 전달
	//
	if( !pOpenRoom )
	{
		// 입장 실패 전달
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
	// Step 5 : 대기실 사용자에게 사용자 퇴장 전달
	//
	pWaitRoom->Leave( pUser );

	m_pCasualPackets->RemoveUser.SetProperty(
		pUser->GetID().c_str(),
		static_cast<INT>( pUser->GetID().length() )
		);
	SendToRoom( pWaitRoom, pUser, &m_pCasualPackets->RemoveUser, m_pCasualPackets->RemoveUser.GetLength() );

	BOOL bRet = pOpenRoom->Join( pUser );

	// 방이 꽉찬 경우
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
	// 방 안에서의 자신의 index를 설정한다.
	//
	pUser->SetRoomInnerIndex( pOpenRoom->AllocEmptyInnerIndex() );


	//
	// Step 4 : 입장 성공 전달
	//
	OnAutoJoinReply( pUser, pOpenRoom );

	//
	// Step 7 : 대기실 사용자에게 방의 정보가 바뀌었음을 전달한다.
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

// 사용자 찾기 요청 처리
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


	// 사용자를 찾지 못한 경우
	if( pFindUser == NULL )
	{
		m_pCasualPackets->FindReply.SetProperty(
			CCasualPacketFindReply::Result::Fail
			);
		SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->FindReply ), m_pCasualPackets->FindReply.GetLength() );
	}

	// 사용자를 찾은 경우
	else
	{
		// 사용자가 대기 채널에 있는지를 검사
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

// 채팅 메시지
inline VOID CNJeanGate::PacketProcessChat( CUser* pUser, CCasualPacketChat* pCasualPacket )
{
	//
	// Step 1 : 예외 검사
	//

	INT nRoomIndex = pUser->GetRoomIndex();

	if( nRoomIndex < 0 || pUser->GetChannelIndex() <= 0 )
		return;

	//
	// Step 2 :  현재 사용자가 속한 방의 모든 사용자에게 전달한다.
	//
	SendToRoom( &m_aRoom[nRoomIndex], pUser, pCasualPacket, pCasualPacket->GetLength() );
}


// 쪽지
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

	// 사용자를 찾지 못한 경우
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

	// 사용자를 찾은 경우
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

// 초대하기
inline VOID CNJeanGate::PacketProcessInvite( CUser* pUser, CCasualPacketInvite* pCasualPacket )
{
	//
	// Step 1 : 예외 검사
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

	// 사용자를 찾지 못한 경우
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

	// 사용자를 찾은 경우
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
	CUser*		pExitUser;	// 강퇴될 사용자
	CChannel*	pChannel;	// 강퇴될 사용자가 속한 채널
	CRoom*		pRoom;		// 강퇴될 사용자가 속한 방
	BOOL		bWaitRoom;	// pRoom이 대기실인지 여부

	//
	// 예외 검사
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
		static_cast<ULONG_PTR>( CCasualPacketCompulsionExited::Type::Normal )	// 강퇴 타입
		);
}

VOID CNJeanGate::OnLoginSuccess( CUser* pUser )
{
}

VOID
CNJeanGate::OnOpenRoomReply( CUser* pUser, CRoom* pRoom )
{
	// Open 성공을 통지한다.
	m_pCasualPackets->OpenRoomReply.SetProperty(
		CCasualPacketOpenRoomReply::Result::Success,
		pRoom->GetRoomIndex() );
	SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->OpenRoomReply ), m_pCasualPackets->OpenRoomReply.GetLength() );
}

VOID CNJeanGate::OnJoinReply( CUser* pUser, CRoom* pRoom )
{
	// 입장한 사용자에게 입장 성공을 전달
	m_pCasualPackets->JoinRoomReply.SetProperty(
		CCasualPacketJoinRoomReply::Result::Success,
		pUser->GetRoomIndex() );
	SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->JoinRoomReply ), m_pCasualPackets->JoinRoomReply.GetLength() );
}

VOID CNJeanGate::OnAutoJoinReply( CUser* pUser, CRoom* pRoom )
{
	// 입장한 사용자에게 입장 성공을 전달
	m_pCasualPackets->AutoJoinRoomReply.SetProperty(
		CCasualPacketAutoJoinRoomReply::Result::Success,
		pUser->GetRoomIndex() );
	SendData( pUser, reinterpret_cast<const CHAR*>( &m_pCasualPackets->AutoJoinRoomReply ), m_pCasualPackets->AutoJoinRoomReply.GetLength() );
}