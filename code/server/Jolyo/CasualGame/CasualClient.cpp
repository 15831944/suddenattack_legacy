#include "StdAfx.h"

#include "CasualClient.h"

#include "..\NJean\Application\Packet.h"
using namespace GamehiSolution::Application::Packets;

using namespace GamehiSolution::CasualGame;
using namespace GamehiSolution::Application;
using namespace GamehiSolution::Session::Message;
using namespace GamehiSolution::Session::Manage;
using namespace GamehiSolution::Session::Gate;
using namespace GamehiSolution::Common::DataStructure;


CCasualClient::CCasualClient()
{
}

CCasualClient::~CCasualClient()
{
}

BOOL CCasualClient::Create( /*IClientModuleHandler* pClientModuleHandler,*/ HWND hWindow, UINT uiMessage, INT nMaxPacketSize )
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
	int		nNumOfConnectSockets	= 2;
	int		nPacketSize				= 256;
	int		nResponseBuffer			= 65536;	// 64KB

//	m_pClientModuleHandler			= pClientModuleHandler;

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
	//		1. NJean�� ����Ǵ� ��Ʈ��ũ�� 1���� Listen�̸� ServicerIndex::Client�� �ε����� ������.
	//
	/*
	int		nNumOfClients;

	int		nServicerIndex;			// ����Ǵ� ���� �ε���
	int		nNumOfCommSockets;		// �ش� ���񼭸� ���� ����� �� �ִ� Ŭ���̾�Ʈ�� ����
	int		nAcceptPoolSize;		// �ش� ������ Accept Pool ũ��

	// Listener ����
	nServicerIndex		= ServicerIndex::Client;
	nNumOfCommSockets	= 1000;
	nAcceptPoolSize		= 100;
	bRet = CreateListener( nServicerIndex, nNumOfCommSockets, nAcceptPoolSize );
	if( !bRet ) return FALSE;

	// �ش� Listener�� Client���� �޸� Ǯ ����
	bRet = m_UserPool.Create( nNumOfCommSockets );
	if( !bRet ) return FALSE;

	nNumOfClients		= nNumOfCommSockets;
	m_nAliveManaged		= nNumOfCommSockets;

	// Listener ������ ��
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	*/



	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	bRet = CreateEnd();
	if( !bRet ) return FALSE;

	return TRUE;
}

BOOL CCasualClient::Start()
{
	return FALSE;

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

void CCasualClient::Destroy()
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.

	CSessionGate::Destroy();
}


void CCasualClient::OnSiganl( DWORD wParam, DWORD lParam )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
}

void CCasualClient::OnResponse( CPeer* pPeer, DWORD dwError, INT nResult, char* pBuffer )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
}

void CCasualClient::OnAccept( CPeer* pPeer, INT nServicerIndex )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	m_LastValue.pPeer		= pPeer;
	m_LastValue.packetType	= PacketType::AcceptType;
	m_LastValue.pData		= NULL;
	m_LastValue.nDataLength	= 0;
}

void CCasualClient::OnConnect( CPeer* pPeer, INT nServicerIndex )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	m_LastValue.pPeer		= pPeer;
	m_LastValue.packetType	= PacketType::ConnectType;
	m_LastValue.pData		= NULL;
	m_LastValue.nDataLength	= 0;

	m_bConnected			= TRUE;
}

void CCasualClient::OnClose( CPeer* pPeer, INT nServicerIndex )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	m_LastValue.pPeer		= pPeer;
	m_LastValue.packetType	= PacketType::CloseType;
	m_LastValue.pData		= NULL;
	m_LastValue.nDataLength	= 0;

	m_bConnected			= FALSE;
}

void CCasualClient::OnData( CPeer* pPeer, INT nServicerIndex, char* pPacket, INT nPacketLength )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.

	m_LastValue.pPeer		= pPeer;
	m_LastValue.packetType	= PacketType::ReceiveType;
	m_LastValue.pData		= pPacket;
	m_LastValue.nDataLength	= nPacketLength;

}

void CCasualClient::OnTimeout( CPeer* pPeer )
{
	TimeoutDisconnectRequest( pPeer );
}

CPeer* CCasualClient::AllocPeer( INT nServicerIndex )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	CPeer* pPeer = NULL;

	return pPeer;
}

void CCasualClient::FreePeer( CPeer* pPeer, INT nServicerIndex )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
}

void CCasualClient::InitListenerPacketType(
		INT									nServicerIndex,
		CPacketCheckSum::CheckSumType&		out_CheckSumType,
		CPacketAssembler::PacketUnitType&	out_PacketUnitType )
{
	out_CheckSumType	= CPacketCheckSum::CheckSumType::TwoByteZoroOne;
	out_PacketUnitType	= CPacketAssembler::PacketUnitType::Length;

	/*
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	m_pClientModuleHandler->InitListenerPacketType(
		out_CheckSumType,
		out_PacketUnitType
		);
	*/
}

void CCasualClient::InitLengthFieldOfPacket(
		INT									nServicerIndex,
		INT&								out_nLengthFieldPos,
		CPacketAssembler::LengthSizeType&	out_lengthSizeType,
		CPacketAssembler::LengthType&		out_lengthType,
		BOOL&								out_bLittleEndian )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.
	out_nLengthFieldPos		= CPacket::GetLengthPos();
	out_lengthSizeType		= CPacketAssembler::LengthSizeType::TwoByte;
	out_lengthType			= CPacketAssembler::LengthType::Total;
	out_bLittleEndian		= TRUE;

	/*
	m_pClientModuleHandler->InitLengthFieldOfPacket(
		out_nLengthFieldPos,
		out_lengthSizeType,
		out_lengthType,
		out_bLittleEndian
		);
	*/
}

BOOL CCasualClient::IsAliveManageObject( INT nServicerIndex )
{
	// ���⿡ �ҽ� �ڵ带 �߰��Ѵ�.

	return FALSE;
}

BOOL CCasualClient::Connect( LPCTSTR lpszIP, INT nPort )
{
	m_bConnected	= FALSE;
	return CSessionGate::Connect( &m_peer, CCasualClient::ServicerIndex::GameServer, lpszIP, nPort );
}

void CCasualClient::Send( char* pData, INT nDataSize )
{
	SendData( &m_peer, pData, nDataSize );
}

void CCasualClient::Close()
{
	CloseRequest( &m_peer );
}

void CCasualClient::Pinging()
{
	CPacketPing pingPacket;

	SendData( &m_peer, (char*)&pingPacket, pingPacket.GetLength() );
}