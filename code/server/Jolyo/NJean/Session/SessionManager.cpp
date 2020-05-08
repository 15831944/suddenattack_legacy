#include "StdAfx.h"
#include ".\SessionManager.h"
#include "..\..\DataStructure\List.h"

using namespace Common::DataStructure;

using namespace Network;
using namespace Session::Manage;
using namespace Session::Message;
using namespace Common::DataStructure;
#ifndef CLIENT_MODULE
	using namespace Query::HTTP;
	using namespace Network::TcpIOCP;
#else
	using namespace Network::TcpAsyncSelect;
#endif

#include "..\Application\Packet.h"

#include "../../../Dispatcher/IConsole.h"

CSessionManager::CSessionManager()
{
	m_bFirstRecvMsg = TRUE;
}

CSessionManager::~CSessionManager(void)
{
}

// ���� ���ҽ��� �����Ѵ�.
BOOL CSessionManager::Create( NetworkType networkType, QueryType queryType, INT nRecvQueueSize, INT nSendQueueSize, INT nQueryQueueSize, INT nEventQueueSize, INT nNumOfListener, INT nNumOfConnectSockets, INT nPacketSize, HWND hWindow, UINT uiMessage )
{
	m_nPacketSize = nPacketSize;
	m_bFirstRecvMsg = TRUE;

	switch( networkType )
	{
#ifndef CLIENT_MODULE
	case NetworkType::TCP_IOCP :
		m_pNetwork = new CTcpIocp();
		break;
#else
	case NetworkType::TCP_ASYNCSELECT :
		m_pNetwork = new CTcpAsyncSelect( hWindow, uiMessage );
		break;
#endif

	default :
		return FALSE;
	}

	switch( queryType )
	{
	case QueryType::HTTP :
		break;

	case QueryType::ADO :
		break;

	default :
		return FALSE;
	}

	// �Ʒ� ��ü�� Network ��üó�� �߻�ȭ ó���� �ؾ� �Ѵ�.
	m_HttpMessage.Create( nPacketSize );
#ifndef CLIENT_MODULE
	m_InetHttp.Create( nQueryQueueSize, this );
	m_SyncInet1.Create();
	m_SyncInet2.Create();
#endif

	if( !m_SendMessage.Create( nPacketSize ) )			return FALSE;
	if( !m_SendManagePacket.Create( nPacketSize ) )		return FALSE;

	if( !m_RecvMessageQueue.Create( nRecvQueueSize, nPacketSize, nEventQueueSize ) )
	{
		m_SendMessage.Destroy();
		return FALSE;
	}

//	nSendQueueSize = 15;
	if( !m_SendMessageQueue.Create( nSendQueueSize, nPacketSize ) )
	{
		m_SendMessage.Destroy();
		m_RecvMessageQueue.Destroy();
		return FALSE;
	}

	if( !m_QueryMessageQueue.Create( nNumOfConnectSockets, nPacketSize ) )		return FALSE;
	if( !m_RecvPacketBufferPool.Create( nRecvQueueSize ) )						return FALSE;
	if( !m_SendPacketBufferPool.Create( nSendQueueSize ) )						return FALSE;


	/////////////////////////
	// ������ �� ����
	if( !m_SendPacketPool.Create( nSendQueueSize * 2 ) )
	{
		return FALSE;
	}
	CSendPacket** paSendPacket = new CSendPacket*[nSendQueueSize * 2];

	for( int i = 0; i < nSendQueueSize * 2; i++ )
	{
		paSendPacket[i] = m_SendPacketPool.Alloc();
		paSendPacket[i]->Create( nPacketSize*2 );
	}
	for( int j = 0; j < nSendQueueSize * 2; j++ )
	{
		m_SendPacketPool.Free( paSendPacket[j] );
	}
	delete[] paSendPacket;

	m_lpRecentNetworkKey	= NULL;
	m_PacketBufferListPool.Create( nSendQueueSize * 2 );

	//
	///////////////////////////////

	if( !m_pNetwork->Create( this, nNumOfListener, nNumOfConnectSockets, nPacketSize, nPacketSize ) )
	{
		return FALSE;
	}

	m_nNumOfPacketAssembler = nNumOfConnectSockets;

	m_pPackets	= new TagPackets;


	// None DataType ����
	m_OldMessage.SetType( CMessage::MessageType::AcceptType );

	::InitializeCriticalSection( &m_csSend );
	::InitializeCriticalSection( &m_csHttp );


//	m_nDebug=0;

	//
	// Send Thread ����
	//
#ifndef CLIENT_MODULE
	DWORD dwThreadId = 0;
	::CloseHandle( ::CreateThread( NULL, 0, SendThreadStartingPoint, this, 0, &dwThreadId ) );
	::CloseHandle( ::CreateThread( NULL, 0, SendThreadStartingPoint, this, 0, &dwThreadId ) );
//	::CloseHandle( ::CreateThread( NULL, 0, SendThreadStartingPoint, this, 0, &dwThreadId ) );
//	::CloseHandle( ::CreateThread( NULL, 0, SendThreadStartingPoint, this, 0, &dwThreadId ) );

	//::CloseHandle( ::CreateThread( NULL, 0, QueryThreadStartingPoint1, this, 0, &dwThreadId ) );
	//::CloseHandle( ::CreateThread( NULL, 0, QueryThreadStartingPoint2, this, 0, &dwThreadId ) );
#endif
	return TRUE;
}

BOOL CSessionManager::CreateListener( INT nListenIndex, INT nNumOfCommSockets, INT nAcceptPoolSize, BOOL bAcceptRecvEvent/* = FALSE*/ )
{
	if( !m_pNetwork->CreateListener( nListenIndex, nNumOfCommSockets, nAcceptPoolSize, bAcceptRecvEvent ) )
		return FALSE;

	m_nNumOfPacketAssembler += nNumOfCommSockets;
	return TRUE;
}

BOOL CSessionManager::CreateEnd()
{
	if( !m_PacketAssemblerPool.Create( m_nNumOfPacketAssembler ) )
		return FALSE;

//	m_nDebugAssem = 0;
	
	CPacketAssembler** pPacketAssembler = new CPacketAssembler*[m_nNumOfPacketAssembler];
	for( INT i = 0; i < m_nNumOfPacketAssembler; i++ )
	{
		pPacketAssembler[i] = m_PacketAssemblerPool.Alloc();
		if( !pPacketAssembler[i]->Create( m_nPacketSize * 2, m_nNumOfPacketAssembler ) )
		{
			return FALSE;
		}
	}
	for( INT j = 0; j < m_nNumOfPacketAssembler; j++ )
	{
		m_PacketAssemblerPool.Free( pPacketAssembler[j] );
	}
	delete[] pPacketAssembler;

	return TRUE;
}

// ���� ���ҽ��� �����Ѵ�.
void CSessionManager::Destroy()
{
	m_pNetwork->Destroy();

	m_SendMessageQueue.Destroy();
	m_RecvMessageQueue.Destroy();
	m_QueryMessageQueue.Destroy();
	m_SendPacketPool.Destroy();

#ifndef CLIENT_MODULE
	m_InetHttp.Destroy();
	m_SyncInet1.Destroy();
	m_SyncInet2.Destroy();
#endif
	m_HttpMessage.Destroy();

	::DeleteCriticalSection( &m_csSend );
	::DeleteCriticalSection( &m_csHttp );
}

INT CSessionManager::GetServiceIndex( ULONG_PTR lpCompletionKey )
{
	return m_pNetwork->GetServicerIndex( lpCompletionKey );
}



// ��ʼ����Ϣ���Ķ�������.
void CSessionManager::InitPacketType(
		ULONG_PTR							lpCompletionKey,
		CPacketCheckSum::CheckSumType		checkSumType,
		CPacketAssembler::PacketUnitType	packetUnitType )
{
	//
	// lpCompletionKey�� �ش��ϴ� PacketAssembler�� �ʱ�ȭ�Ѵ�.
	//
	
	ULONG_PTR			pAssemblerKey;
	CPacketAssembler*	pPacketAssembler;
	DWORD				dwAssemblerHandle;
	m_pNetwork->GetAssemblerKey( lpCompletionKey, pAssemblerKey, dwAssemblerHandle );
	pPacketAssembler = (CPacketAssembler*)pAssemblerKey;
	pPacketAssembler->Init( checkSumType, packetUnitType );
}

void CSessionManager::InitLengthFieldOfPacket(
	ULONG_PTR lpCompletionKey,
	int nLengthFieldPos,
	CPacketAssembler::LengthSizeType lengthSizeType,
	CPacketAssembler::LengthType lengthType/* = Network::CPacketAssembler::LengthType::Total*/,
	BOOL bLittleEndian/* = TRUE*/ )
{
	//
	// lpCompletionKey�� �ش��ϴ� PacketAssembler�� �ʱ�ȭ�Ѵ�.
	//
	ULONG_PTR			pAssemblerKey;
	CPacketAssembler*	pPacketAssembler;
	DWORD				dwAssemblerHandle;
	m_pNetwork->GetAssemblerKey( lpCompletionKey, pAssemblerKey, dwAssemblerHandle );
	pPacketAssembler = (CPacketAssembler*)pAssemblerKey;
	pPacketAssembler->InitLengthField( nLengthFieldPos, lengthSizeType, lengthType, bLittleEndian );
}

//���͵�ǰ�����е���Ϣ����Ϣ����
VOID CSessionManager::SendMessageFlush()
{
	if( m_lpRecentNetworkKey == NULL )
		return;

	CMessage	message( CMessage::MessageType::DataType, m_lpRecentNetworkKey, m_pPacketBufferList );
	m_SendMessageQueue.EnqueuePacket( message );
	m_lpRecentNetworkKey	= NULL;
}
//��pData���͵���ǰ�������Ϣ����
VOID CSessionManager::SendMessage( ULONG_PTR pNetworkKey, const CHAR* pData, INT nDataSize, DWORD dwHandle )
{
	DWORD				dwAssemblerHandle;
	ULONG_PTR			pAssemblerKey;
	CPacketBuffer*		pSendPacketBuffer;
	CPacketAssembler*	pAssembler;

	m_pNetwork->GetAssemblerKey( pNetworkKey, pAssemblerKey, dwAssemblerHandle );

	// �������� �ʴ� Peer�� Send�� �� ��� �߻��Ѵ�. ���ø����̼� ���������� ����
	if( pAssemblerKey == NULL )
		return;
	ASSERT( pAssemblerKey != NULL );


	pAssembler = reinterpret_cast<CPacketAssembler*>( pAssemblerKey );

	pSendPacketBuffer	= m_SendPacketBufferPool.Alloc();
	pSendPacketBuffer->CopyData( pData, nDataSize, dwHandle, pAssembler->AllocSendIndex() );
	//���ϸ��ݲ���pData,���һ����Ϣ��

	if( nDataSize == 0 )
		int a = 3;

	if( m_lpRecentNetworkKey == NULL )
	{
		//��ǰû�з��͵Ķ���ʱ,���ڴ����ȡ��һ���ڴ�
		m_lpRecentNetworkKey	= pNetworkKey;
		m_pPacketBufferList		= m_PacketBufferListPool.Alloc();
	}

	if( m_lpRecentNetworkKey == pNetworkKey )
	{
		m_pPacketBufferList->Add( pSendPacketBuffer );	//������Ҫ���͵Ķ����뵱ǰ���Ͷ���һ��,���������
	}
	else
	{
		//����,����Ϣ�����ϵ�m_SendMessageQueue��
		CMessage	message( CMessage::MessageType::DataType, m_lpRecentNetworkKey, m_pPacketBufferList );
		m_SendMessageQueue.EnqueuePacket( message );
		m_pPacketBufferList		= m_PacketBufferListPool.Alloc();
		m_pPacketBufferList->Add( pSendPacketBuffer );
		m_lpRecentNetworkKey	= pNetworkKey;
	}
	

	/*

#ifndef CLIENT_MODULE
		// ť�� �ֱ� ���� �ڵ� ���� ��ŷ�Ѵ�.
		CMessage	message( CMessage::MessageType::DataType, pNetworkKey, pSendPacketBuffer );
		m_SendMessageQueue.EnqueuePacket( message );

#else
		// ť�� �ֱ� ���� �ڵ� ���� ��ŷ�Ѵ�.
		CMessage	message( CMessage::MessageType::DataType, pNetworkKey, pSendPacketBuffer );
		SendPacket( message );

#endif
	*/
}

// ���ݾ������Ϣ��msg���͵�������Ϣ���л�Query��Ϣ����
void CSessionManager::SendMessage( CMessage& message )
{
	// ������ ��Ŷ�̸� ��ٷ� Send�� �ϰ�, �� ���� ��Ŷ ���ʹ� OnSend���� ó���� �Ѵ�.
	int nError = 0;

	DWORD				dwAssemblerHandle;
	ULONG_PTR			pAssemblerKey;
	CPacketAssembler*	pAssembler;

	Common::DataStructure::CListEx<CPacketBuffer*>*
						pPacketBufferList;
	CPacketBuffer*		pSendPacketBuffer;

	switch( message.GetType() )
	{
	case CMessage::MessageType::HttpRequest :
		m_QueryMessageQueue.EnqueuePacket( message );
		break;

	case CMessage::MessageType::DbReqeust :
		m_DbReqeustMessageQueue.EnqueuePacket( message );
		break;

	case CMessage::MessageType::CloseType :
		{
			m_pNetwork->GetAssemblerKey( message.GetNetworkKey(), pAssemblerKey, dwAssemblerHandle );
			pAssembler = reinterpret_cast<CPacketAssembler*>( pAssemblerKey );
			pPacketBufferList		= m_PacketBufferListPool.Alloc();//Ϊʲô���ܴ�message.GetPacketBufferList()
			pSendPacketBuffer		= m_SendPacketBufferPool.Alloc();
			// Close���� ����
			pSendPacketBuffer->CopyData( NULL, 0, 0, pAssembler->AllocSendIndex() );
			pPacketBufferList->Add( pSendPacketBuffer );
//			message.SetSendingIndex( pAssembler->AllocSendIndex() );
			CMessage	closeMessage( CMessage::MessageType::DataType, message.GetNetworkKey(), pPacketBufferList );
			m_SendMessageQueue.EnqueuePacket( closeMessage );
		}
		break;

	default:
		m_SendMessageQueue.EnqueuePacket( message );
		break;

	}
}

//ֱ�ӷ�����Ϣ����recv��Ϣ�յ�Ping��Ϣʱֱ�ӻظ���send�̴߳�m_SendMessageQueue�еõ�һ��������
void CSessionManager::SendPacket( CMessage& message )
{
	//
	// step 1 : Assembler�� �����´�.
	//

	ULONG_PTR			pAssemblerKey;
	CPacketAssembler*	pPacketAssembler;
	DWORD				dwAssemblerHandle;
	m_pNetwork->GetAssemblerKey( message.GetNetworkKey(), pAssemblerKey, dwAssemblerHandle );
	pPacketAssembler = (CPacketAssembler*)pAssemblerKey; //��message��Ӧ����Ϣ��

	
	if( pPacketAssembler == NULL )//û�н�������soket����Ϣ����һ���Ǵ���Ĳ�����Ϣ
	{
		Common::DataStructure::CListEx<CPacketBuffer*>*
			pPacketBufferList	= message.GetPacketBufferList();

		while( !pPacketBufferList->IsEmpty() )
		{
			m_SendPacketBufferPool.Free( pPacketBufferList->Dequeue() );//��sendpacketbuffer�н���socket��Ӧ��������Ϣռ�õ��ڴ��ͷ�
		}
		m_PacketBufferListPool.Free( pPacketBufferList );//��packetbufferlist��Ҳ�ͷ�
		return;
	}

	pPacketAssembler->Lock();


	// Send�� List�� �����´�.
	int		nError;
	BOOL	bRet;
	CListEx<CPacketBuffer*>* plstPacketBuffer = pPacketAssembler->GetPacketBufferList();

	Common::DataStructure::CListEx<CPacketBuffer*>*
		pPacketBufferList	= message.GetPacketBufferList();
	while( !pPacketBufferList->IsEmpty() )
	{
		CPacketBuffer*	pPacketBuffer	= pPacketBufferList->Dequeue();
		plstPacketBuffer->SortAdd( pPacketBuffer, pPacketBuffer->GetSendIndex() );//����message�е���Ϣ��˳������buffer��
	}
	m_PacketBufferListPool.Free( pPacketBufferList ); //����packet buff����message��������Ϣ������Ҫ�������ͷ�

	// List�� ù��° ��Ŷ�� �����Ѵ�.
	CPacketBuffer* pPacketBuffer = plstPacketBuffer->GetRearElement(); //�������һ����Ϣ�������ж�

	/*/
	if( plstPacketBuffer->GetElementsCount() > 100 )
	{
		::printf( "����������������������������������������������������\n" );
		::printf( "Send Over %d\n", plstPacketBuffer->GetElementsCount() );
		::printf( "����������������������������������������������������\n" );
//		char a[100];
//		scanf( "%s", a );
//		int b;
//		b = 3;
	}
	/*/

	if( pPacketBuffer == NULL )
	{
		pPacketAssembler->Unlock();
		return;
	}

	if( !pPacketAssembler->IsSending( pPacketBuffer->GetSendIndex() ) ||
		pPacketAssembler->IsOverlapped() )
	{
		//��ǰ��Ϣ��ID���ѷ��Ͱ�idС�����ڷ����У��˳�
		pPacketAssembler->Unlock();
		return;
	}
	if( pPacketBuffer->GetSendIndex() > 0 )
		pPacketAssembler->AddSendIndex();

	// CloseType
	if( pPacketBuffer->GetDataSize() == 0 )
	{
		//��û����Ϣ���ݣ��ͷŶ�Ӧ�ڴ�
		m_SendPacketBufferPool.Free( plstPacketBuffer->Dequeue() );
		pPacketAssembler->Unlock();

		OnFreeNetworkObject( message.GetNetworkKey() );
		m_pNetwork->FreeNetworkKey( message.GetNetworkKey() );
		return;
	}

	if( pPacketBuffer->GetHandle() != dwAssemblerHandle ) //socket һ������������ͬһ����ɫ��һ���Ǵ������Ϣ��
	{
		while( !plstPacketBuffer->IsEmpty() )//�ͷ��ڴ�
		{
			pPacketBuffer	= plstPacketBuffer->GetRearElement();
			if( !pPacketAssembler->IsSending( pPacketBuffer->GetSendIndex() ) )
			{
				break;
			}

			if( pPacketBuffer->GetSendIndex() > 0 )
			{
				pPacketAssembler->AddSendIndex();
			}

			if( pPacketBuffer->GetDataSize() == 0 )//close������
			{
				m_SendPacketBufferPool.Free( plstPacketBuffer->Dequeue() );
				pPacketAssembler->Unlock();

				OnFreeNetworkObject( message.GetNetworkKey() );
				m_pNetwork->FreeNetworkKey( message.GetNetworkKey() );
				return;
			}
			m_SendPacketBufferPool.Free( plstPacketBuffer->Dequeue() );
		}
		pPacketAssembler->Unlock();
		return;
	}

	// ��ȷ����Ϣ��
	CPacket* pPacket = reinterpret_cast<CPacket*>( pPacketBuffer->GetData() );
	if( pPacket->GetCommand() == CPacket::PredefineCommand::Ping ||
		pPacket->GetCommand() == CPacket::PredefineCommand::PingAck )
	{
		/*/
		if( !m_pNetwork->CanStartRTT( message.GetNetworkKey() ) )
		{
			printf( "++++++++++++++++++++++++++++++++++++++++++++++++\n" );
			printf( "Do not Recv Ack Ping!!\n" );
			printf( "++++++++++++++++++++++++++++++++++++++++++++++++\n" );
//			pPacketAssembler->Unlock();
//			return;
		}
		/*/

		m_pNetwork->SetRttStartTime( message.GetNetworkKey() );//ping��Ϣ���ö�Ӧsocket��ʱ�������mygate������ȷ��host
	}

	//������Ϣ��
	INT nChangeLength;
	pPacketAssembler->SetCheckSum( pPacketBuffer->GetData(), pPacketBuffer->GetDataSize(), nChangeLength );
	pPacketBuffer->SetDataSize( nChangeLength );

	pPacketAssembler->SetOverlapped( TRUE );
	bRet = m_pNetwork->Send(
		(ULONG_PTR)message.GetNetworkKey(),
		pPacketBuffer->GetData(),
		pPacketBuffer->GetDataSize(),
		nError );

	if( !bRet )
	{
		m_pNetwork->CloseSocket( (ULONG_PTR)message.GetNetworkKey() );
		pPacketAssembler->SetOverlapped( FALSE );
	}
	pPacketAssembler->Unlock();
}

void CSessionManager::SendPacket( ULONG_PTR lpNetworkKey, CPacketBuffer* pPacketBuffer )
{
	ASSERT( FALSE );
}


// Ÿ�̸Ӹ� ���۽�Ų��.
void CSessionManager::StartTimer( int nTickTime, int nMaxTickCount, int nSize )
{
	m_Timer.Create( nTickTime, nMaxTickCount, nSize, (ITimerHandler*)this );
}

// Ÿ�̸Ӹ� �߰��Ѵ�.
ULONG_PTR CSessionManager::AddTimer( int nTickCount, const CSignal& signal )
{
	return m_Timer.SetTimer( nTickCount, signal );
}

// ITimerHandler
void CSessionManager::OnTimer( const CSignal& signal )
{
	m_RecvMessageQueue.AddSignalPacket( signal );
}

// �̺�Ʈ�� �߰��Ѵ�.
void CSessionManager::AddSignal( const CSignal& signal )
{
	m_RecvMessageQueue.AddSignalPacket( signal );
}

void CSessionManager::SetReceiveEvent( ULONG_PTR lpNetworkKey )
{
	m_pNetwork->SetConnectRecvEvent( lpNetworkKey );
}


//accept��Ӧ����ʼ��m_RecvMessageQueue
void CSessionManager::OnAccept( ULONG_PTR lpNetworkKey )
{
	// Receive / AcceptConnect ����ȭ�� ���� �ʱ�ȭ�� �Ѵ�.
	//char strLog[1024];
	//sprintf( strLog, "Accept %d\r\n", (int)lpNetworkKey );
	//OutputDebugString( strLog );


	m_pNetwork->ResetConnectRecvEvent( lpNetworkKey );

	m_pNetwork->ResetRttTime( lpNetworkKey );

	ULONG_PTR			pAssemblerKey;
//	CPacketAssembler*	pPacketAssembler;
	DWORD				dwAssemblerHandle;
	m_pNetwork->GetAssemblerKey( lpNetworkKey, pAssemblerKey, dwAssemblerHandle );

	//char a[100];
	//sprintf( a, "accept %d\r\n", (int) lpNetworkKey );
	//::OutputDebugString( a );

	m_RecvMessageQueue.EnqueuePacket( CMessage( lpNetworkKey, CMessage::MessageType::AcceptType, dwAssemblerHandle ) );
}

void CSessionManager::OnConnect( ULONG_PTR lpNetworkKey )
{
	// Receive / AcceptConnect ����ȭ�� ���� �ʱ�ȭ�� �Ѵ�.
	m_pNetwork->ResetConnectRecvEvent( lpNetworkKey );

	m_pNetwork->ResetRttTime( lpNetworkKey );

	m_RecvMessageQueue.EnqueuePacket( CMessage( lpNetworkKey, CMessage::MessageType::ConnectType ) );
}

void CSessionManager::OnReceive( ULONG_PTR lpNetworkKey, char* pRecvBuff, int nReceivedLength )
{
	// ù��° Receive�̸�, Connect/Accept�� ���� �̺�Ʈ�� ��ٸ���.
	m_pNetwork->ResetRecvDisconnectEvent( lpNetworkKey );
	m_pNetwork->WaitConnectRecvEvent( lpNetworkKey );

	ULONG_PTR			pAssemblerKey;
	CPacketAssembler*	pPacketAssembler;
	DWORD				dwAssemblerHandle;
	m_pNetwork->GetAssemblerKey( lpNetworkKey, pAssemblerKey, dwAssemblerHandle );	//��ȡ��ǰNetworkKey�е�ԭ����Ϣ
	pPacketAssembler = (CPacketAssembler*)pAssemblerKey;

	if( !pPacketAssembler->Assemble( pRecvBuff, nReceivedLength ) ) //�����յ���Ϣ����Queue��
	{
		m_pNetwork->SetRecvDisconnectEvent( lpNetworkKey );
		return;
	}

	char* pPacket;
	int nPacketSize = 0;
	int nResult = 0;
//	try
	{
		while( pPacket = (char*)pPacketAssembler->GetCompletePacket( nPacketSize, nResult ) )
		{
			//����ÿһ����
			CPacket* pDataPacket = (CPacket*)pPacket;

			CPacketBuffer*	pRecvPacketBuffer;
//			::OutputDebugString( "Recv\r\n" );
			pRecvPacketBuffer	= m_RecvPacketBufferPool.Alloc();
			pRecvPacketBuffer->CopyData( pPacket, nPacketSize );

			switch( pDataPacket->GetCommand() )
			{

			//
			// Step 1 : �켱 ó���� ��Ŷ�� Message Q�� Enqueue���� �ʰ� ��ٷ� ó���Ѵ�.
			//
				// Client -> Server�� ������ ��Ŷ
			case CPacket::PredefineCommand::Ping :
				{
					CPacketPing*	pPingPacket;
					pPingPacket		= (CPacketPing*)pDataPacket;
					if( pPingPacket->GetRealLength() != nPacketSize )//��Ϣ��������֤
					{
//#pragma message( "===============================\r\n" )
//#pragma message( "������ ���Ḧ ���� �ӽ÷α�\r\n" )
//#pragma message( "===============================\r\n" )
//						::Dispatcher::ICONSOLE()->WriteUnsafe( "DoClose\tLowLevel\tPing\r\n" );
						SendMessage( CMessage( lpNetworkKey, CMessage::MessageType::DoShutdownType ) );//���������ر�����
						m_RecvPacketBufferPool.Free( pRecvPacketBuffer );
						break;
					}

					// �Ƚ������ڴ���Ϊ�ͷ�
//					m_RecvMessageQueue.EnqueuePacket( CMessage( lpNetworkKey, pPacket, nPacketSize ) );
					m_RecvMessageQueue.EnqueuePacket(
						CMessage( CMessage::MessageType::DataType, lpNetworkKey, pRecvPacketBuffer ) );


					// ���⼭���� �� ���� : ackPacket�� ��� ��Ŷ Ŭ������ ��Ʈ���� �����.. TagPackets.. ó���Ұ�..
					// �Ʒ��� SendPacket�� Send Q �� ������ �ʰ� �켱������ ������ ���� ó��

//					break;
					ULONG_PTR	pAssemblerKey;
					DWORD		dwAssemblerHandle;
					m_pNetwork->GetAssemblerKey( lpNetworkKey, pAssemblerKey, dwAssemblerHandle );

					CPacketBuffer* pSendPacketBuffer = m_SendPacketBufferPool.Alloc();
					pSendPacketBuffer->CopyData(
						reinterpret_cast<CHAR*>( &m_pPackets->PingAck ),
						m_pPackets->PingAck.GetLength(),
						dwAssemblerHandle,
						0
						);

					Common::DataStructure::CListEx<CPacketBuffer*>* pPacketBufferList;
					pPacketBufferList = m_PacketBufferListPool.Alloc();
					pPacketBufferList->Add( pSendPacketBuffer );
					CMessage	message( CMessage::MessageType::DataType, lpNetworkKey, pPacketBufferList );
					SendPacket( message );	//������Ϣ��

//					CMessage	message( CMessage::MessageType::DataType, lpNetworkKey, pSendPacketBuffer );
//					SendPacket( message );
//					SendPacket( lpNetworkKey, pSendPacketBuffer, TRUE );

					/*
					m_SendManagePacket.SetSendPacket(
						lpNetworkKey,
						reinterpret_cast<VOID*>( &m_pPackets->PingAck ), m_pPackets->PingAck.GetLength() );

					DWORD		dwAssemblerHandle;
					ULONG_PTR	pAssemblerKey;
					m_pNetwork->GetAssemblerKey( m_SendManagePacket.GetNetworkKey(), pAssemblerKey, dwAssemblerHandle );
					m_SendManagePacket.SetSendHandle( dwAssemblerHandle );

					SendPacket( m_SendManagePacket, TRUE );
					*/

//					m_RecvPacketBufferPool.Free( pRecvPacketBuffer );
				}
				break;

				// Server -> Client�� ������ ��Ŷ
//			case CPacket::PredefineCommand::PingAck :
//				{
//					CPacketPingAck*	pPingAckPacket;
//					pPingAckPacket	= (CPacketPingAck*)pDataPacket;
//					if( pPingAckPacket->GetRealLength() != nPacketSize )
//					{
////#pragma message( "===============================\r\n" )
////#pragma message( "������ ���Ḧ ���� �ӽ÷α�\r\n" )
////#pragma message( "===============================\r\n" )
////						::Dispatcher::ICONSOLE()->WriteUnsafe( "DoClose\tLowLevel\tPingAck\r\n" );
//						SendMessage( CMessage( lpNetworkKey, CMessage::MessageType::DoShutdownType ) );
//						break;
//					}
//
//					// Rtt Ÿ���� üũ�Ѵ�.
//					m_pNetwork->SetRttEndTime( lpNetworkKey );
//
//					ULONG_PTR	pAssemblerKey;
//					DWORD		dwAssemblerHandle;
//					m_pNetwork->GetAssemblerKey( lpNetworkKey, pAssemblerKey, dwAssemblerHandle );
//
//					CPacketBuffer* pSendPacketBuffer = m_SendPacketBufferPool.Alloc();
//					pSendPacketBuffer->CopyData(
//						reinterpret_cast<CHAR*>( &m_pPackets->Ack ),
//						m_pPackets->Ack.GetLength(),
//						dwAssemblerHandle,
//						0
//						);
//					Common::DataStructure::CListEx<CPacketBuffer*>* pPacketBufferList;
//					pPacketBufferList = m_PacketBufferListPool.Alloc();
//					pPacketBufferList->Add( pSendPacketBuffer );
//					CMessage	message( CMessage::MessageType::DataType, lpNetworkKey, pPacketBufferList );
//					SendPacket( message );
////					CMessage	message( CMessage::MessageType::DataType, lpNetworkKey, pSendPacketBuffer );
////					SendPacket( message );
//
//					/*
//					CPacketAck	ackPacket;
//					m_SendManagePacket.SetSendPacket( lpNetworkKey, (void*)&ackPacket, ackPacket.GetLength() );
//
//					DWORD		dwAssemblerHandle;
//					ULONG_PTR	pAssemblerKey;
//					m_pNetwork->GetAssemblerKey( m_SendManagePacket.GetNetworkKey(), pAssemblerKey, dwAssemblerHandle );
//					m_SendManagePacket.SetSendHandle( dwAssemblerHandle );
//
//					SendPacket( m_SendManagePacket, TRUE );
//					*/
//				}
//				break;
//
				// Client -> Server�� ������ ��Ŷ
			case CPacket::PredefineCommand::Ack :
				{
					CPacketAck*		pAckPacket;
					pAckPacket		= (CPacketAck*)pDataPacket;
					if( pAckPacket->GetRealLength() != nPacketSize )
					{
//#pragma message( "===============================\r\n" )
//#pragma message( "������ ���Ḧ ���� �ӽ÷α�\r\n" )
//#pragma message( "===============================\r\n" )
//						::Dispatcher::ICONSOLE()->WriteUnsafe( "DoClose\tLowLevel\tAck\r\n" );
						SendMessage( CMessage( lpNetworkKey, CMessage::MessageType::DoShutdownType ) );
						m_RecvPacketBufferPool.Free( pRecvPacketBuffer );
						break;
					}

					// Rtt Ÿ���� üũ�Ѵ�.
					m_pNetwork->SetRttEndTime( lpNetworkKey ); //�õ�PING����ʱ

					DWORD dwRttTime;
					m_pNetwork->GetRttTime( lpNetworkKey, dwRttTime );

					m_RecvPacketBufferPool.Free( pRecvPacketBuffer );
				}
				break;

			//
			// Step 2 : ��Ŷ�� Message Q�� Enqueue�Ѵ�.
			//
			default :
//				m_RecvMessageQueue.EnqueuePacket( CMessage( lpNetworkKey, pPacket, nPacketSize ) );
				m_RecvMessageQueue.EnqueuePacket(
					CMessage( CMessage::MessageType::DataType, lpNetworkKey, pRecvPacketBuffer ) );//��Ϣ������
				break;
			}

		}
	}
//	catch( int nErrorCode )
	if( nResult > 0 ) //1�����岻����2������ռ����õ���Ϣ���Ͳ��ԣ�3������ռ�൫��Ϣ���Ͳ���
	{
//		int a = nErrorCode;
		m_pNetwork->CloseSocket( lpNetworkKey );
		m_pNetwork->SetRecvDisconnectEvent( lpNetworkKey );
		
		//FILE		*fp = NULL;
		//SYSTEMTIME	st;
		//GetLocalTime( &st );
		//fp = fopen( "ERROR_LOG.txt", "a" );
		//if( fp==NULL ) return;
		//fprintf( fp,
		//		 "[%d/%d/%d %d:%d:%d] Packet Error    ERROR CODE: %d From %s\n",
		//		 st.wYear,
		//		 st.wMonth,
		//		 st.wDay,
		//		 st.wHour, 
		//		 st.wMinute,
		//		 st.wSecond,
		//		 nErrorCode,
		//		 m_pNetwork->GetPeerAddr( lpNetworkKey ) );
		//fclose( fp );

		//printf( "+++++++++++++++++++++++++++++++++++++++++++++++++\n" );
		//printf( "Packet Error    ERROR CODE: %d From %s\n", nErrorCode, m_pNetwork->GetPeerAddr( lpNetworkKey ) );
		//printf( "+++++++++++++++++++++++++++++++++++++++++++++++++\n" );

		return;
	}
	m_pNetwork->SetRecvDisconnectEvent( lpNetworkKey );
}

void CSessionManager::OnSend( ULONG_PTR lpNetworkKey, char* pRecvBuff, int nSendedLength )
{
	ULONG_PTR				pAssemblerKey;
	CPacketAssembler*		pPacketAssembler;
	CListEx<CPacketBuffer*>*	plstPacketBuffer;		// SendPacket List
	CPacketBuffer*			pSendPacketBuffer;		// Send�� ��Ŷ ����
	DWORD				dwAssemblerHandle;

	m_pNetwork->GetAssemblerKey( lpNetworkKey, pAssemblerKey, dwAssemblerHandle );
	pPacketAssembler	= reinterpret_cast<CPacketAssembler*>( pAssemblerKey );
	plstPacketBuffer	= pPacketAssembler->GetPacketBufferList();

	pPacketAssembler->Lock();

	if( !pPacketAssembler->IsOverlapped() )
	{
		pPacketAssembler->Unlock();
		return;
	}
	
	pPacketAssembler->SetOverlapped( FALSE );
	

	int		nError;
	BOOL	bRet;

	/*
	if( pPacketAssembler->IsFree() )
	{
		OnFreeNetworkObject( lpNetworkKey );
		m_pNetwork->FreeNetworkKey( lpNetworkKey );
		pPacketAssembler->Unlock();
		return;
	}
	*/

	// Send�� ����Ÿ���� ���� ���۷��� ��
	pSendPacketBuffer	= plstPacketBuffer->GetRearElement();

	if( pSendPacketBuffer == NULL )
	{
		//������
		CPacket* pPacket = reinterpret_cast<CPacket*>( pRecvBuff );
		int a = pPacket->GetCommand();
		char a1[100];
		sprintf( a1, "Assert!! %d\r\n", a );
		::OutputDebugString( a1 );
		pPacketAssembler->Unlock();
		return;
	}


	/*
	if( pSendPacketBuffer->GetHandle() != dwAssemblerHandle )
	{
		while( !plstPacketBuffer->IsEmpty() )
		{
			if( !pPacketAssembler->IsSending( pSendPacketBuffer->GetSendIndex() ) )
			{
				break;
			}

			if( pSendPacketBuffer->GetSendIndex() > 0 )
				pPacketAssembler->AddSendIndex();

			pSendPacketBuffer	= plstPacketBuffer->Dequeue();
			if( pSendPacketBuffer->GetDataSize() == 0 )
			{
				OnFreeNetworkObject( lpNetworkKey );
				m_pNetwork->FreeNetworkKey( lpNetworkKey );
				pPacketAssembler->Unlock();

				m_SendPacketBufferPool.Free( pSendPacketBuffer );
				return;
			}
			m_SendPacketBufferPool.Free( pSendPacketBuffer );
		}
		pPacketAssembler->Unlock();
		return;
	}
	*/


	// �ٸ��� �޺κ��� ������
	if( pSendPacketBuffer->GetDataSize() > nSendedLength )
	{
		if( pSendPacketBuffer->GetHandle() != dwAssemblerHandle )
		{
			m_SendPacketBufferPool.Free( plstPacketBuffer->Dequeue() );

			while( !plstPacketBuffer->IsEmpty() )
			{
				pSendPacketBuffer	= plstPacketBuffer->GetRearElement();
				if( !pPacketAssembler->IsSending( pSendPacketBuffer->GetSendIndex() ) )
				{
					break;
				}

				if( pSendPacketBuffer->GetSendIndex() > 0 )
				{
					pPacketAssembler->AddSendIndex();
				}

				if( pSendPacketBuffer->GetDataSize() == 0 )
				{
					m_SendPacketBufferPool.Free( plstPacketBuffer->Dequeue() );
					pPacketAssembler->Unlock();

					OnFreeNetworkObject( lpNetworkKey );
					m_pNetwork->FreeNetworkKey( lpNetworkKey );
					return;
				}
				m_SendPacketBufferPool.Free( plstPacketBuffer->Dequeue() );
			}
			pPacketAssembler->Unlock();
			return;
		}

		pSendPacketBuffer->ReplaceDataPos( nSendedLength );//ָ�����

		pPacketAssembler->SetOverlapped( TRUE );

		// �������� �̹� äũ���� �پ� �ִ�.
		bRet = m_pNetwork->Send(
			lpNetworkKey,
			pSendPacketBuffer->GetData(),
			pSendPacketBuffer->GetDataSize(),
			nError );

		if( !bRet )
		{
			m_pNetwork->CloseSocket( lpNetworkKey );
			pPacketAssembler->SetOverlapped( FALSE );
		}
	}
	else
	{
		// ���� ��Ŷ�� ����
		pSendPacketBuffer = plstPacketBuffer->Dequeue();
		m_SendPacketBufferPool.Free( pSendPacketBuffer );

		pSendPacketBuffer = plstPacketBuffer->GetRearElement();
		if( pSendPacketBuffer == NULL )
		{
			pPacketAssembler->Unlock();
			return;
		}

		if( pPacketAssembler->IsFree() )
		{
			while( !plstPacketBuffer->IsEmpty() )
			{
				pSendPacketBuffer	= plstPacketBuffer->GetRearElement();
				if( !pPacketAssembler->IsSending( pSendPacketBuffer->GetSendIndex() ) )
				{
					break;
				}

				if( pSendPacketBuffer->GetSendIndex() > 0 )
				{
					pPacketAssembler->AddSendIndex();
				}

				if( pSendPacketBuffer->GetDataSize() == 0 )
				{
					m_SendPacketBufferPool.Free( plstPacketBuffer->Dequeue() );
					pPacketAssembler->Unlock();

					OnFreeNetworkObject( lpNetworkKey );
					m_pNetwork->FreeNetworkKey( lpNetworkKey );
					return;
				}
				m_SendPacketBufferPool.Free( plstPacketBuffer->Dequeue() );
			}
			pPacketAssembler->Unlock();
			return;
		}


		if( !pPacketAssembler->IsSending( pSendPacketBuffer->GetSendIndex() ) )
		{
			pPacketAssembler->Unlock();
			return;
		}
		if( pSendPacketBuffer->GetSendIndex() > 0 )
			pPacketAssembler->AddSendIndex();

		if( pSendPacketBuffer->GetDataSize() == 0 )
		{
			m_SendPacketBufferPool.Free( plstPacketBuffer->Dequeue() );
			pPacketAssembler->Unlock();

			OnFreeNetworkObject( lpNetworkKey );
			m_pNetwork->FreeNetworkKey( lpNetworkKey );
			return;
		}

		// Send�� ��Ŷ�� Ping ��Ŷ�̸� �ð��� ����Ѵ�.
		CPacket* pPacket = reinterpret_cast<CPacket*>( pSendPacketBuffer->GetData() );
		if( pPacket->GetCommand() == CPacket::PredefineCommand::Ping ||
			pPacket->GetCommand() == CPacket::PredefineCommand::PingAck )
		{
			m_pNetwork->SetRttStartTime( lpNetworkKey );
		}

		// Check Sum�� �����Ѵ�.
		INT nChangeLength;
		pPacketAssembler->SetCheckSum( pSendPacketBuffer->GetData(), pSendPacketBuffer->GetDataSize(), nChangeLength );
		pSendPacketBuffer->SetDataSize( nChangeLength );

		pPacketAssembler->SetOverlapped( TRUE );
		bRet = m_pNetwork->Send(
			lpNetworkKey,
			pSendPacketBuffer->GetData(),
			pSendPacketBuffer->GetDataSize(),
			nError );

		if( !bRet )
		{
			m_pNetwork->CloseSocket( lpNetworkKey );
			pPacketAssembler->SetOverlapped( FALSE );
		}
	}
	pPacketAssembler->Unlock();
}

void CSessionManager::OnClose( ULONG_PTR lpNetworkKey )
{
	m_pNetwork->WaitRecvDisconnectEvent( lpNetworkKey );

	//char a[100];
	//sprintf( a, "Close %d\r\n", (int) lpNetworkKey );
	//::OutputDebugString( a );

	ULONG_PTR				pAssemblerKey;
	CPacketAssembler*		pPacketAssembler;
	DWORD					dwAssemblerHandle;
	
	m_pNetwork->GetAssemblerKey( lpNetworkKey, pAssemblerKey, dwAssemblerHandle );
	pPacketAssembler = (CPacketAssembler*)pAssemblerKey;
	pPacketAssembler->Lock();

	m_pNetwork->CloseSocket( lpNetworkKey );

	pPacketAssembler->ResetHandle();
	m_pNetwork->SetAssemblerKey( lpNetworkKey, pAssemblerKey, pPacketAssembler->GetHandle() );
	m_pNetwork->GetAssemblerKey( lpNetworkKey, pAssemblerKey, dwAssemblerHandle );

//				char a1[100];
//				sprintf( a1, "OnClose!! %d %d\r\n", (ULONG_PTR)lpNetworkKey, dwAssemblerHandle );
//				::OutputDebugString( a1 );

	pPacketAssembler->Unlock();

	m_RecvMessageQueue.EnqueuePacket( CMessage( lpNetworkKey, CMessage::MessageType::CloseType ) );
}

void CSessionManager::OnAllocNetworkObject( ULONG_PTR lpNetworkKey )
{
	// ������Ӧ�Ļ�����
	CPacketAssembler* pPacketAssembler = m_PacketAssemblerPool.Alloc();
	if( !pPacketAssembler->IsCreated() )
	{
		if( !pPacketAssembler->Create( m_nPacketSize * 2, m_nNumOfPacketAssembler ) )
		{
			int a = 3;
			return;
		}
	}
//	m_nDebugAssem++;

	pPacketAssembler->Lock();
	if( !pPacketAssembler->GetPacketBufferList()->IsEmpty() )
	{
		int a = 3;
	}

	if( pPacketAssembler->IsFree() )
		int a = 3;

	if( pPacketAssembler->IsOverlapped() )
		int a = 3;

	if( !pPacketAssembler->IsCreated() )
	{
		pPacketAssembler->Create( m_nPacketSize * 2, m_nNumOfPacketAssembler );
	}
	pPacketAssembler->ResetCloseWait();


	// ��Ŷ ������� ���
	m_pNetwork->SetAssemblerKey( lpNetworkKey, (ULONG_PTR)pPacketAssembler, pPacketAssembler->GetHandle() );


	// �ӽ÷� SessionGate::Start���� ����� �ű��.
	InitPacketType( lpNetworkKey, CPacketCheckSum::CheckSumType::TwoByteZoroOne, CPacketAssembler::PacketUnitType::Length );
	InitLengthFieldOfPacket( lpNetworkKey, CPacket::GetLengthPos(), CPacketAssembler::LengthSizeType::TwoByte, CPacketAssembler::LengthType::Total );

	if( pPacketAssembler->IsOverlapped() )
		int a = 3;

	pPacketAssembler->Unlock();
}

void CSessionManager::OnFreeNetworkObject( ULONG_PTR lpNetworkKey )
{
	ULONG_PTR				pAssemblerKey;
	CPacketAssembler*		pPacketAssembler;
	DWORD					dwAssemblerHandle;
	CListEx<CPacketBuffer*>*	plstPacketBuffer;		// SendPacket List
	CPacketBuffer*			pPacketBuffer;
	
	m_pNetwork->GetAssemblerKey( lpNetworkKey, pAssemblerKey, dwAssemblerHandle );
	pPacketAssembler = (CPacketAssembler*)pAssemblerKey;

	if( pPacketAssembler->IsOverlapped() )
		int a = 3;
	pPacketAssembler->Reset();

	plstPacketBuffer	= pPacketAssembler->GetPacketBufferList();
	while( !plstPacketBuffer->IsEmpty() )
	{
		pPacketBuffer	= plstPacketBuffer->Dequeue();
		m_SendPacketBufferPool.Free( pPacketBuffer );
//		m_nDebug--;
	}
//					char a[100];
//					sprintf( a, "OnFree %d %d\r\n", (int)lpNetworkKey, (int)pAssemblerKey );
//					::OutputDebugString( a );

	m_PacketAssemblerPool.Free( pPacketAssembler );
	m_pNetwork->SetAssemblerKey( lpNetworkKey, NULL, 0 );
	//m_nDebugAssem--;
}


// ��Ŷ�� �����Ѵ�.
void CSessionManager::RecvMessage( CMessage& outMessage )
{
	if( m_OldMessage.GetType() == CMessage::MessageType::DataType ||
		m_OldMessage.GetType() == CMessage::MessageType::HttpResponse )
	{
		if( m_bFirstRecvMsg )
		{
			m_bFirstRecvMsg = FALSE;
		}
		else
		{
//			if( m_OldMessage.GetType() == CMessage::MessageType::DataType )
//				::OutputDebugString( "Free Data\r\n" );
//			else
//				::OutputDebugString( "Free Query\r\n" );
			// Message�� Data �϶��� �Ҵ�� ���� �����Ѵ�.
			CPacketBuffer* pPacketBuffer = m_OldMessage.GetPacketBuffer();
			m_RecvPacketBufferPool.Free( pPacketBuffer );
		}
	}
	m_RecvMessageQueue.DequeuePacket( outMessage );
	m_OldMessage = outMessage;
}




void CSessionManager::OnRecvInet( ULONG_PTR lpInetKey, ULONG_PTR lpContext )
{
	// ������ ���� Response�� Response Q�� ������ ���Ͽ���.


#ifndef CLIENT_MODULE
	::EnterCriticalSection( &m_csHttp );

	CSyncInet* pInet = reinterpret_cast<CSyncInet*>( lpInetKey );

	int nResult = pInet->HttpResult( lpInetKey );

	BOOL	bRet;
	DWORD	dwResponseSize;
	while( TRUE )
	{
		CPacketBuffer* pPacketBuffer = m_RecvPacketBufferPool.Alloc();
//		::OutputDebugString( "Query\r\n" );
//		nRet = m_InetHttp.HttpResponse( lpInetKey, pPacketBuffer->GetData(), pPacketBuffer->GetBufferSize() );
		bRet = pInet->Response( lpInetKey, pPacketBuffer->GetData(), pPacketBuffer->GetBufferSize(), dwResponseSize );
		if( !bRet || dwResponseSize == 0 )
		{
//			::OutputDebugString( "Free Query\r\n" );
			m_RecvPacketBufferPool.Free( pPacketBuffer );
			break;
		}

		pPacketBuffer->SetDataSize( dwResponseSize );
//		m_HttpMessage.SetHttpData( nRet, lpContext );
		m_RecvMessageQueue.EnqueuePacket(
			CMessage( CMessage::MessageType::HttpResponse, (CRequestObject*) lpContext, pPacketBuffer )
			);
	}

	if( bRet )
		m_RecvMessageQueue.EnqueuePacket( CMessage( lpContext, CMessage::MessageType::HttpResponseEnd, nResult ) );
	else
		m_RecvMessageQueue.EnqueuePacket( CMessage( lpContext, CMessage::MessageType::HttpError, ERROR_NOT_SUPPORTED ) );

	/*
	if( nRet == 0 )
	{
		m_InetHttp.Close( lpInetKey );
		m_RecvMessageQueue.EnqueuePacket( CMessage( lpContext, CMessage::MessageType::HttpResponseEnd, nResult ) );
	}
	else
	{
//		m_InetHttp.Close( lpInetKey );
//		m_RecvMessageQueue.EnqueuePacket( CMessage( lpContext, CMessage::MessageType::HttpError, ERROR_NOT_SUPPORTED ) );
	}
	*/
	::LeaveCriticalSection( &m_csHttp );
#endif
}

void CSessionManager::OnErrorInet( ULONG_PTR lpContext, DWORD nError )
{
	m_RecvMessageQueue.EnqueuePacket( CMessage( lpContext, CMessage::MessageType::HttpError, nError ) );
//	m_ResponseMessageQueue.EnqueuePacket( CMessage( lpContext, CMessage::MessageType::HttpError, nError ) );
}

DWORD __stdcall CSessionManager::QueryThreadStartingPoint1(PVOID pvParam)
{
#ifndef CLIENT_MODULE
	CSessionManager*	pThis			= (CSessionManager*) pvParam;
	CMessage			queryMessage;

	Query::IQueryHandler*	queryObjerver	= pThis;

	BOOL	bRet;
	DWORD	dwError;
	while( TRUE )
	{
		pThis->m_QueryMessageQueue.DequeuePacket( queryMessage );
		bRet = pThis->m_SyncInet1.Request( queryMessage.GetQueryBuffer()->GetData(), dwError );

		if( !bRet )
		{
			pThis->OnErrorInet( reinterpret_cast<ULONG_PTR>( queryMessage.GetRequestObject() ), dwError );
		}
		else
		{
			pThis->OnRecvInet( reinterpret_cast<ULONG_PTR>( &pThis->m_SyncInet1 ), reinterpret_cast<ULONG_PTR>( queryMessage.GetRequestObject() ) );
		}
	}

	/*
	// ���� ��ü ����
	m_hOpen = ::InternetOpen( NULL, INTERNET_OPEN_TYPE_DIRECT , NULL, NULL, NULL );

	while( TRUE )
	{
		pThis->m_SendMessageQueue.DequeuePacket( queryMessage );
		{
			pQueryBuffer = queryMessage.GetQueryBuffer();


		}
	}
	*/
#endif
	return 0;
}

DWORD __stdcall CSessionManager::QueryThreadStartingPoint2(PVOID pvParam)
{
#ifndef CLIENT_MODULE
	CSessionManager*	pThis			= (CSessionManager*) pvParam;
	CMessage			queryMessage;

	Query::IQueryHandler*	queryObjerver	= pThis;

	BOOL	bRet;
	DWORD	dwError;
	while( TRUE )
	{
		pThis->m_QueryMessageQueue.DequeuePacket( queryMessage );
		bRet = pThis->m_SyncInet2.Request( queryMessage.GetQueryBuffer()->GetData(), dwError );

		if( !bRet )
		{
			pThis->OnErrorInet( reinterpret_cast<ULONG_PTR>( queryMessage.GetRequestObject() ), dwError );
		}
		else
		{
			pThis->OnRecvInet( reinterpret_cast<ULONG_PTR>( &pThis->m_SyncInet2 ), reinterpret_cast<ULONG_PTR>( queryMessage.GetRequestObject() ) );
		}
	}

	/*
	// ���� ��ü ����
	m_hOpen = ::InternetOpen( NULL, INTERNET_OPEN_TYPE_DIRECT , NULL, NULL, NULL );

	while( TRUE )
	{
		pThis->m_SendMessageQueue.DequeuePacket( queryMessage );
		{
			pQueryBuffer = queryMessage.GetQueryBuffer();


		}
	}
	*/
#endif
	return 0;
}

DWORD __stdcall CSessionManager::SendThreadStartingPoint(PVOID pvParam)
{
	CSessionManager*	pThis		= (CSessionManager*) pvParam;

	CMessage sendMessage;

	while( TRUE )
	{
		pThis->m_SendMessageQueue.DequeuePacket( sendMessage );

		switch( sendMessage.GetType() )
		{
		case CMessage::MessageType::DoCloseType :
		case CMessage::MessageType::DoShutdownType :
			pThis->m_pNetwork->CloseSocket( (ULONG_PTR)sendMessage.GetNetworkKey() );
			break;

		//case CMessage::MessageType::DoCloseType :
		//	pThis->m_pNetwork->CloseSocket( (ULONG_PTR)sendMessage.GetNetworkKey() );
		//	break;

		//case CMessage::MessageType::DoShutdownType :
		//	pThis->m_pNetwork->ShutDown( (ULONG_PTR)sendMessage.GetNetworkKey() );
		//	break;

		case CMessage::MessageType::CloseType:
		case CMessage::MessageType::DataType :
			pThis->SendPacket( sendMessage );
			break;

		default:
			int a;
			a = 3;
			break;

		}
	}
	return 0;
}

//��m_DbReqeustMessageQueue��ȡ����Ϣ�� -��m_RecvMessageQueue
//ִ�д洢����
DWORD __stdcall CSessionManager::DbThreadStartingPoint(PVOID pvParam)
{
#ifndef CLIENT_MODULE
	if(FAILED(::CoInitialize(NULL)))
		assert( FALSE );

	CSessionManager*	pThis			= (CSessionManager*) pvParam;
	CMessage			queryMessage;
	CMessage			dbConnMessage;

	Query::IQueryHandler*	queryObjerver	= pThis;

	while( TRUE )
	{
		pThis->m_DbReqeustMessageQueue.DequeuePacket( queryMessage );
		CDbObject* pDB = queryMessage.GetDbObject();

		_ConnectionPtr	pDbConn;
		pThis->m_DbConnMessageQueue.DequeuePacket( dbConnMessage );
		pDbConn = dbConnMessage.GetDbConnObject()->GetDbConn();

		BOOL bSuccess = TRUE;
		long state = -1;
		try
		{
			state = pDbConn->GetState();
			switch( state )
			{
			case adStateOpen:
				break;
			case adStateClosed:
				pDbConn->Open(
					pThis->m_dbConnInfo.clsServer,
					pThis->m_dbConnInfo.clsID,
					pThis->m_dbConnInfo.clsPass,
					adConnectUnspecified
					);
				break;
			case adStateConnecting:
				break;
			case adStateExecuting:
				break;
			case adStateFetching:
				break;
			}
			pDB->GetCommand()->ActiveConnection = pDbConn;
	
			//Test
			ParametersPtr param = pDB->GetCommand()->GetParameters();
			long lCn = pDB->GetCommand()->Parameters->Count;
			_bstr_t strname = pDB->GetCommand()->CommandText;
			TCHAR str[2048];
			sprintf(str,"%d:%s\n", pDB->GetMessage(),static_cast<TCHAR*>(strname));
			OutputDebugString(str);
			_variant_t vIndex;

			for( int i = 0; i < lCn; i++)
			{
				vIndex = (short)i;
				_ParameterPtr  p = pDB->GetCommand()->Parameters->GetItem(&vIndex);
				sprintf(str,"Parameter name = '%s'",(LPCSTR)p->Name);
				OutputDebugString(str);
				_bstr_t strvalue = (_bstr_t)p->Value;
				sprintf(str,"value = '%s'\n",static_cast<TCHAR*>(strvalue) );
				OutputDebugString(str);
			}
			//
			pDB->GetCommand()->Execute( NULL, NULL, adExecuteNoRecords | adCmdStoredProc );
			state = pDbConn->GetState();
		}
		catch (_com_error &e)
		{
			::_bstr_t clsSource( e.Source() );
			::_bstr_t clsDetail( e.Description() );

			char strSource[ 1024 ];
			char strDetail[ 1024 ];
			sprintf( strSource, "Dev\t[LOW_DB] Source: %s\r\n", static_cast<CHAR *>( clsSource ) );
			sprintf( strDetail, "Dev\t[LOW_DB] Detail: %x, %s\r\n", e.Error(), static_cast<CHAR *>( clsDetail ) );

			::Dispatcher::ICONSOLE()->WriteUnsafe( strSource );
			::Dispatcher::ICONSOLE()->WriteUnsafe( strDetail );

			if( e.Error() == 0x80004005 )	// �Ϲ� ��Ʈ��ũ ����
			{
				bSuccess = FALSE;
			}

			pDbConn->Close();
			DEBUG_BREAK;

		}
		catch( ... )
		{
			bSuccess = FALSE;
		}
		pThis->m_DbConnMessageQueue.EnqueuePacket( dbConnMessage );

		pDB->SetSuccess( bSuccess );

		pThis->m_RecvMessageQueue.EnqueuePacket( CMessage( CMessage::DbResponse, pDB ) );
	}



#endif
	return 0;
}

INT CSessionManager::CreateDatabase( INT nDbConnectionCount, LPCTSTR lpszConnection )
{

	if(FAILED(::CoInitialize(NULL)))
		assert( FALSE );

	char					strConn[ 1024 ];
	char *					strConnection[ 100 ];
	int nCount = 0;
	
	strcpy( strConn, lpszConnection );
	char * pToken			= strtok( strConn, "\\" );
	while( pToken != NULL )
	{
		pToken				= strtok( NULL, "\\" );
	}

	m_paDbConn = new CDbConnObject[nDbConnectionCount];
	m_DbConnMessageQueue.Create( nDbConnectionCount, 1 );
	m_DbObjectPool.Create( m_nNumOfPacketAssembler );
	m_DbReqeustMessageQueue.Create( m_nNumOfPacketAssembler, 1 );

	OutputDebugString( lpszConnection );
	OutputDebugString( "\r\n" );

	m_dbConnInfo.clsServer	= lpszConnection;
	m_dbConnInfo.clsID		= "";
	m_dbConnInfo.clsPass	= "";


	INT nConnected = 0;
	for( INT i = 0; i < nDbConnectionCount; i++ )
	{
		m_paDbConn[i].GetDbConn().CreateInstance(__uuidof(Connection));

		HRESULT hr = E_FAIL;
		try
		{
			hr = m_paDbConn[i].GetDbConn()->Open(
				m_dbConnInfo.clsServer,
				m_dbConnInfo.clsID,
				m_dbConnInfo.clsPass,
				adConnectUnspecified
				);
		}
		catch (_com_error &e)
		{
			::_bstr_t clsSource( e.Source() );
			::_bstr_t clsDetail( e.Description() );

			char strSource[ 1024 ];
			char strDetail[ 1024 ];
			sprintf( strSource, "Dev\t[ERROR] Source: %s\r\n", static_cast<CHAR *>( clsSource ) );
			sprintf( strDetail, "Dev\t[ERROR] Detail: %s\r\n", static_cast<CHAR *>( clsDetail ) );

			printf( "%s\r\n%s\r\n", strSource, strDetail );
			OutputDebugString( strSource );
			OutputDebugString( strDetail );
		}
		catch( ... )
		{
		}

		if( SUCCEEDED( hr ) == TRUE )
		{
			nConnected++;
			m_DbConnMessageQueue.EnqueuePacket( CMessage( &m_paDbConn[i] ) );
		}
	}
	OutputDebugString( "DB Connect Success\r\n" );
	for( INT i = 0; i < nConnected; i++ )
	{
		DWORD dwThreadId = 0;
		::CloseHandle( ::CreateThread( NULL, 0, DbThreadStartingPoint, this, 0, &dwThreadId ) );
	}



	return nConnected;
}

CDbObject* CSessionManager::AllocDbObject()
{
	CDbObject* pDB = m_DbObjectPool.Alloc();

	pDB->GetCommand().CreateInstance( __uuidof(Command) );
	ASSERT(pDB->GetCommand());

	pDB->GetCommand()->CommandType = adCmdStoredProc;
	return pDB;
}

void CSessionManager::FreeDbObject( CDbObject* pDb )
{
	pDb->GetCommand().Release();
	pDb->GetCommand().Detach();
	m_DbObjectPool.Free( pDb );
}