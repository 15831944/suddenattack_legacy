#include "StdAfx.h"

#include "ClientGate.h"

#include "NJean/Application/Packet.h"
using namespace Application::Packets;

using namespace GamehiSolution::ClientModule;
using namespace GamehiSolution::Application;
using namespace GamehiSolution::Session::Message;
using namespace GamehiSolution::Session::Manage;
using namespace GamehiSolution::Session::Gate;
using namespace GamehiSolution::Common::DataStructure;


CClientGate::CClientGate()
{
}

CClientGate::~CClientGate()
{
}

BOOL CClientGate::Create( /*IClientGateHandler* pClientGateHandler,*/ HWND hWindow, UINT uiMessage, INT nMaxPacketSize )
{
	CSessionManager::NetworkType
			networkType				= CSessionManager::NetworkType::TCP_ASYNCSELECT;
	CSessionManager::QueryType
			queryType				= CSessionManager::QueryType::HTTP;
	int		nRecvQueueSize			= 1000;
	int		nSendQueueSize			= 1000;
	int		nQueryQueueSize			= 1000;
	int		nSignalQueueSize		= 1000;
	int		nNumOfListener			= 1;
	int		nNumOfConnectSockets	= 10;
	int		nPacketSize				= 256;
	int		nResponseBuffer			= 65536;	// 64KB

//	m_pClientGateHandler			= pClientGateHandler;

	BOOL bRet = CSessionGate::Create(
					networkType,
					queryType,
					nRecvQueueSize,
					nSendQueueSize,
					nQueryQueueSize,
					nSignalQueueSize,
					nNumOfListener,
					nNumOfConnectSockets,
					nPacketSize,
					nResponseBuffer,
					hWindow,
					uiMessage );
	if( !bRet ) return FALSE;

	m_peer.Create( nMaxPacketSize );

	//
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	//


	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// �ۼ��� : ������
	// �ۼ��� : 2004�� 5�� 21��
	// ��  �� : Client Listener ����
	//		
	//
	
	int		nNumOfCommSockets;		// �ش� ���񼭸� ���� ����� �� �ִ� Ŭ���̾�Ʈ�� ����

	// Listener ����
//	nServicerIndex		= ServicerIndex::Client;
	nNumOfCommSockets	= 10;
	bRet = CreateListener( ServicerType::Peers, nNumOfCommSockets, 0 );
	if( !bRet ) return FALSE;

	// �ش� Listener�� Client���� �޸� Ǯ ����
	bRet = m_PeerPool.Create( nNumOfCommSockets );
	if( !bRet ) return FALSE;

	m_nNumOfClients		= nNumOfCommSockets;
//	m_nAliveManaged		= nNumOfCommSockets;

	// Listener ������ ��
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////



	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	bRet = CreateEnd();
	if( !bRet ) return FALSE;

	m_bConnected = FALSE;

	return TRUE;
}

INT CClientGate::Start( INT nPort )
{
	int		nServicerIndex;			// ����Ǵ� ���� �ε���
//	int		nPort;					// ������ ��Ʈ ��ȣ

	nServicerIndex		= ServicerType::Peers;
//	nPort				= 4444;
	while( TRUE )
	{
		if( StartListen( nServicerIndex, nPort ) )
			break;
		nPort++;
	}

	return nPort;

	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.

	/*

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// �ۼ��� : ������
	// �ۼ��� : 2004�� 5�� 21��
	// ��  �� : Client Listener Accept ����
	//
	int		nServicerIndex;			// ����Ǵ� ���� �ε���
	int		nPort;					// ������ ��Ʈ ��ȣ

	nServicerIndex		= ServicerIndex::Client;
	nPort				= 5555;
	StartListen( nServicerIndex, nPort );
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	*/

	int		nTickTime;				// Tick �߻� Ÿ�� (ms ����)
	int		nMaxTick;				// ����� Ÿ�̸��� �ִ� Tick (�ݵ�� Max���� ���� �ʾƵ� �ȴ�. �ٸ� �� ���� �Ѵ� ������ŭ�� ������尡 �ִ�.
	int		nTimerSignalQueueSize;	// Ÿ�̸Ӱ� �ѹ� �߻���Ű�� �ñ׳��� ���� : Create���� ������ �ִ� �ñ׳��� ������ �Ѿ�� �ȵȴ�.
	int		nAliveTickError;		// Alive ó���� ���� Timeout Tick���� ���� ��� ����
	int		nAliveTimeoutCount;		// nAliveTimeoutCount * nAliveTickError = Timeout Tick

	nTickTime				= 1000;
	nMaxTick				= 10;
	nTimerSignalQueueSize	= 500;
	nAliveTickError			= 3;
	nAliveTimeoutCount		= 10;		// 30�� ������ Ping üũ
	// ���߿� ��� 0���� Set
	
	// Start�� blocking �Լ��̴�.
	// Start�� ��� ������ �޽����� ó���ϴ� �κ��� ��� �ִ�.
	// ���� return FALSE�� ����ٸ�, Start �ʱ�ȭ ������ ����̴�.
	return CSessionGate::Start( nTickTime, nMaxTick, nTimerSignalQueueSize, nAliveTickError, nAliveTimeoutCount, m_nAliveManaged );
}

void CClientGate::Destroy()
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	m_peer.Destroy();
	m_PeerPool.Destroy();

	CSessionGate::Destroy();
}


void CClientGate::OnSiganl( DWORD wParam, DWORD lParam )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
}

void CClientGate::OnResponse( CPeer* pPeer, DWORD dwError, INT nResult, char* pBuffer )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
}

void CClientGate::OnAccept( CPeer* pPeer, INT nServicerIndex )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	m_LastValue.nServicerIndex	= nServicerIndex;
	m_LastValue.pPeer			= pPeer;
	m_LastValue.packetType		= PacketType::AcceptType;
	m_LastValue.pData			= NULL;
	m_LastValue.nDataLength		= 0;
}

void CClientGate::OnConnect( CPeer* pPeer, INT nServicerIndex )
{
	if( !pPeer->IsBeginConnect() )
	{
		return;
	}

	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	m_LastValue.nServicerIndex	= nServicerIndex;
	m_LastValue.pPeer			= pPeer;
	m_LastValue.packetType		= PacketType::ConnectType;
	m_LastValue.pData			= NULL;
	m_LastValue.nDataLength		= 0;

	if( m_LastValue.nServicerIndex == ServicerType::Client )
		m_bConnected			= TRUE;
}

void CClientGate::OnBeginConnect( CPeer* pPeer, INT nServicerIndex )
{
	pPeer->SetBeginConnect( TRUE );

	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	m_LastValue.nServicerIndex	= nServicerIndex;
	m_LastValue.pPeer			= pPeer;
	m_LastValue.packetType		= PacketType::BeginConnectType;
	m_LastValue.pData			= NULL;
	m_LastValue.nDataLength		= 0;
}

void CClientGate::OnClose( CPeer* pPeer, INT nServicerIndex )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	m_LastValue.nServicerIndex	= nServicerIndex;
	m_LastValue.pPeer			= pPeer;
	m_LastValue.packetType		= PacketType::CloseType;
	m_LastValue.pData			= NULL;
	m_LastValue.nDataLength		= 0;

	if( m_LastValue.nServicerIndex == ServicerType::Client )
		m_bConnected			= FALSE;
}

void CClientGate::OnData( CPeer* pPeer, INT nServicerIndex, char* pPacket, INT nPacketLength )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	m_LastValue.nServicerIndex	= nServicerIndex;
	m_LastValue.pPeer			= pPeer;
	m_LastValue.packetType		= PacketType::ReceiveType;
	m_LastValue.pData			= pPacket;
	m_LastValue.nDataLength		= nPacketLength;

}

void CClientGate::OnTimeout( CPeer* pPeer )
{
	// p2p�� ping ó���� ����.

	// TimeoutDisconnectRequest( pPeer );
}

CPeer* CClientGate::AllocPeer( INT nServicerIndex )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	CPeer* pPeer;
	switch( nServicerIndex )
	{
	case ServicerType::Peers :
//	case ServicerType::P2PServer :
		pPeer = m_PeerPool.Alloc();
		break;

	default :
		break;
	}

	return pPeer;
}

void CClientGate::FreePeer( CPeer* pPeer, INT nServicerIndex )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	switch( nServicerIndex )
	{
	case ServicerType::Peers :
	case ServicerType::P2PServer :
		m_PeerPool.Free( pPeer );
		break;

	default :
		break;
	}
}

void CClientGate::InitListenerPacketType(
		INT									nServicerIndex,
		CPacketCheckSum::CheckSumType&		out_CheckSumType,
		CPacketAssembler::PacketUnitType&	out_PacketUnitType )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	/*
	m_pClientGateHandler->InitListenerPacketType(
		out_CheckSumType,
		out_PacketUnitType
		);
	*/
	out_CheckSumType	= CPacketCheckSum::CheckSumType::TwoByteZoroOne;
	out_PacketUnitType	= CPacketAssembler::PacketUnitType::Length;
}

void CClientGate::InitLengthFieldOfPacket(
		INT									nServicerIndex,
		INT&								out_nLengthFieldPos,
		CPacketAssembler::LengthSizeType&	out_lengthSizeType,
		CPacketAssembler::LengthType&		out_lengthType,
		BOOL&								out_bLittleEndian )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	/*
	m_pClientGateHandler->InitLengthFieldOfPacket(
		out_nLengthFieldPos,
		out_lengthSizeType,
		out_lengthType,
		out_bLittleEndian
		);
	*/

	out_nLengthFieldPos		= CPacket::GetLengthPos();
	out_lengthSizeType		= CPacketAssembler::LengthSizeType::TwoByte;
	out_lengthType			= CPacketAssembler::LengthType::Total;
	out_bLittleEndian		= TRUE;
}

BOOL CClientGate::IsAliveManageObject( INT nServicerIndex )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.

	return FALSE;
}

BOOL CClientGate::Connect( LPCTSTR lpszIP, INT nPort, ServicerType type )
{
	if( type == ServicerType::Client )
	{
		m_bConnected	= FALSE;
		return CSessionGate::Connect( &m_peer, ServicerType::Client, lpszIP, nPort );
	}
	else
	{
		CPeer* pPeer;
		pPeer = m_PeerPool.Alloc();
		return CSessionGate::Connect( pPeer, ServicerType::P2PServer, lpszIP, nPort );
	}
}

void CClientGate::Send( char* pData, INT nDataSize )
{
	SendData( &m_peer, pData, nDataSize );
}

void CClientGate::Close()
{
	CloseRequest( &m_peer );
}

void CClientGate::Send( CPeer* pPeer, char* pData, INT nDataSize )
{
	SendData( pPeer, pData, nDataSize );
}

void CClientGate::Close( CPeer* pPeer )
{
	pPeer->SetBeginConnect( FALSE );
	CloseRequest( pPeer );
}

void CClientGate::Pinging( DWORD dwTick )
{
	CPacketPing pingPacket;

	pingPacket.SetTick( dwTick );

	SendData( &m_peer, (char*)&pingPacket, pingPacket.GetLength() );
}