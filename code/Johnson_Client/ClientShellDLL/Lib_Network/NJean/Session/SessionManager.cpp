#include "StdAfx.h"
#include "./SessionManager.h"
#include "../../DataStructure/List.h"
using namespace GamehiSolution::Common::DataStructure;

using namespace GamehiSolution::Network;
using namespace GamehiSolution::Session::Manage;
using namespace GamehiSolution::Session::Message;
using namespace GamehiSolution::Common::DataStructure;
using namespace GamehiSolution::Network::TcpAsyncSelect;

#include "../Application/Packet.h"
using namespace Application::Packets;

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
	case NetworkType::TCP_ASYNCSELECT :
		m_pNetwork = new CTcpAsyncSelect( hWindow, uiMessage );
		break;

	default :
		return FALSE;
	}

	switch( queryType )
	{
	case QueryType::HTTP :
		break;

	default :
		return FALSE;
	}

	// �Ʒ� ��ü�� Network ��üó�� �߻�ȭ ó���� �ؾ� �Ѵ�.
	m_HttpMessage.Create( nPacketSize );

	if( !m_SendMessage.Create( nPacketSize ) )
	{
		return FALSE;
	}

	if( !m_SendManagePacket.Create( nPacketSize ) )
	{
		return FALSE;
	}

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

	if( !m_RecvPacketBufferPool.Create( nRecvQueueSize ) )
	{
		return FALSE;
	}
	if( !m_SendPacketBufferPool.Create( nSendQueueSize ) )
	{
		return FALSE;
	}


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


	//
	// Send Thread ����
	//
#ifndef CLIENT_MODULE
	DWORD dwThreadId = 0;
//	::CloseHandle( ::CreateThread( NULL, 0, SendThreadStartingPoint, this, 0, &dwThreadId ) );
//	::CloseHandle( ::CreateThread( NULL, 0, SendThreadStartingPoint, this, 0, &dwThreadId ) );
//	::CloseHandle( ::CreateThread( NULL, 0, SendThreadStartingPoint, this, 0, &dwThreadId ) );
//	::CloseHandle( ::CreateThread( NULL, 0, SendThreadStartingPoint, this, 0, &dwThreadId ) );
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
	
	CPacketAssembler** pPacketAssembler = new CPacketAssembler*[m_nNumOfPacketAssembler];
	for( INT i = 0; i < m_nNumOfPacketAssembler; i++ )
	{
		pPacketAssembler[i] = m_PacketAssemblerPool.Alloc();
		if( !pPacketAssembler[i]->Create( m_nPacketSize * 2 ) )
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
	/*
	CSendPacket** paSendPacket = new CSendPacket*[m_SendPacketPool.GetPoolSize()];
	for( int i = 0; i < m_SendPacketPool.GetPoolSize(); i++ )
	{
		paSendPacket[i] = m_SendPacketPool.Alloc();
		paSendPacket[i]->Destroy();
	}
	for( int j = 0; j < m_SendPacketPool.GetPoolSize(); j++ )
	{
		m_SendPacketPool.Free( paSendPacket[j] );
	}
	delete[] paSendPacket;
	*/


	m_pNetwork->Destroy();

	m_SendMessageQueue.Destroy();
	m_RecvMessageQueue.Destroy();
	m_SendPacketPool.Destroy();

	m_SendMessage.Destroy();
	m_SendManagePacket.Destroy();

	m_RecvPacketBufferPool.Destroy();
	m_SendPacketBufferPool.Destroy();

	m_HttpMessage.Destroy();

	delete m_pPackets;


	/*
	CPacketAssembler** pPacketAssembler = new CPacketAssembler*[m_PacketAssemblerPool.GetPoolSize()];
	for( INT k = 0; k < m_PacketAssemblerPool.GetPoolSize(); k++ )
	{
		pPacketAssembler[k] = m_PacketAssemblerPool.Alloc();
		pPacketAssembler[k]->Destroy();
	}
	for( INT l = 0; l < m_nNumOfPacketAssembler; l++ )
	{
		m_PacketAssemblerPool.Free( pPacketAssembler[l] );
	}
	delete[] pPacketAssembler;
	*/
	m_PacketAssemblerPool.Destroy();

	delete m_pNetwork;

	::DeleteCriticalSection( &m_csSend );
	::DeleteCriticalSection( &m_csHttp );
}

INT CSessionManager::GetServiceIndex( unsigned long lpCompletionKey )
{
	return m_pNetwork->GetServicerIndex( lpCompletionKey );
}



// Packet�� ���õ� �ʱ�ȭ
void CSessionManager::InitPacketType(
		unsigned long							lpCompletionKey,
		CPacketCheckSum::CheckSumType		checkSumType,
		CPacketAssembler::PacketUnitType	packetUnitType )
{
	//
	// lpCompletionKey�� �ش��ϴ� PacketAssembler�� �ʱ�ȭ�Ѵ�.
	//
	
	unsigned long			pAssemblerKey;
	CPacketAssembler*	pPacketAssembler;
	m_pNetwork->GetAssemblerKey( lpCompletionKey, pAssemblerKey );
	pPacketAssembler = (CPacketAssembler*)pAssemblerKey;
	pPacketAssembler->Init( checkSumType, packetUnitType );
}

void CSessionManager::InitLengthFieldOfPacket(
	unsigned long lpCompletionKey,
	int nLengthFieldPos,
	CPacketAssembler::LengthSizeType lengthSizeType,
	CPacketAssembler::LengthType lengthType/* = Network::CPacketAssembler::LengthType::Total*/,
	BOOL bLittleEndian/* = TRUE*/ )
{
	//
	// lpCompletionKey�� �ش��ϴ� PacketAssembler�� �ʱ�ȭ�Ѵ�.
	//
	unsigned long			pAssemblerKey;
	CPacketAssembler*	pPacketAssembler;
	m_pNetwork->GetAssemblerKey( lpCompletionKey, pAssemblerKey );
	pPacketAssembler = (CPacketAssembler*)pAssemblerKey;
	pPacketAssembler->InitLengthField( nLengthFieldPos, lengthSizeType, lengthType, bLittleEndian );
}

VOID CSessionManager::SendMessage( unsigned long pNetworkKey, const CHAR* pData, INT nDataSize )
{
	unsigned long		pAssemblerKey;
	CPacketBuffer*	pSendPacketBuffer;

	m_pNetwork->GetAssemblerKey( pNetworkKey, pAssemblerKey );
	pSendPacketBuffer	= m_SendPacketBufferPool.Alloc();
	pSendPacketBuffer->CopyData( pData, nDataSize );



	// ť�� �ֱ� ���� �ڵ� ���� ��ŷ�Ѵ�.
	CMessage	message( CMessage::MessageType::DataType, pNetworkKey, pSendPacketBuffer );
	SendPacket( message );

}

// ��Ŷ�� �����Ѵ�.
void CSessionManager::SendMessage( CMessage& message )
{
	// ������ ��Ŷ�̸� ��ٷ� Send�� �ϰ�, �� ���� ��Ŷ ���ʹ� OnSend���� ó���� �Ѵ�.
	int nError = 0;


	switch( message.GetType() )
	{
	case CMessage::MessageType::DoCloseType :
		m_pNetwork->CloseSocket( (unsigned long)message.GetNetworkKey() );
		break;

	case CMessage::MessageType::DoShutdownType :
		m_pNetwork->ShutDown( (unsigned long)message.GetNetworkKey() );
		break;

	default :
		SendPacket( message );
		break;

	}
}

void CSessionManager::SendPacket( CMessage& message, BOOL bFirst )
{
	//
	// step 1 : Assembler�� �����´�.
	//

	unsigned long			pAssemblerKey;
	CPacketAssembler*	pPacketAssembler;
	m_pNetwork->GetAssemblerKey( (unsigned long)message.GetNetworkKey(), pAssemblerKey );
	pPacketAssembler = (CPacketAssembler*)pAssemblerKey;

	// pPacektAssembler�� NULL�̸�,
	if( pPacketAssembler == NULL )
	{
		if( message.GetType() != CMessage::MessageType::CloseType )
			m_SendPacketBufferPool.Free( message.GetPacketBuffer() );
		return;
	}

	pPacketAssembler->Lock();


	// Send�� List�� �����´�.
	int		nError;
	BOOL	bRet;
	CList<CPacketBuffer*>* plstPacketBuffer = pPacketAssembler->GetPacketBufferList();
	if( plstPacketBuffer->IsEmpty() )
	{
		if( message.GetType() == CMessage::MessageType::CloseType )
		{
			OnFreeNetworkObject( message.GetNetworkKey() );
			m_pNetwork->FreeNetworkKey( message.GetNetworkKey() );
		}
		// DataType
		else
		{
			CPacketBuffer*	pPacketBuffer	= message.GetPacketBuffer();

			// �켱 �Է��� �Ϸ��� ���� �ֱ� ��Ŷ �ٷ� ������ ������ �Ѵ�.
			// ���� �ֱ� ��Ŷ�� ������ ���۵� �� �ֱ� �����̴�.
			if( bFirst )
			{
				CPacketBuffer* pTmp		= plstPacketBuffer->Dequeue();
				plstPacketBuffer->AddFirst( pPacketBuffer );
				plstPacketBuffer->AddFirst( pTmp );
			}
			else
			{
				plstPacketBuffer->Add( pPacketBuffer );
			}

			// Send�� ��Ŷ�� Ping ��Ŷ�̸� �ð��� ����Ѵ�.
			CPacket* pPacket = reinterpret_cast<CPacket*>( pPacketBuffer->GetData() );
			if( pPacket->GetCommand() == CPacket::PredefineCommand::Ping ||
				pPacket->GetCommand() == CPacket::PredefineCommand::PingAck )
			{
				/*/
				if( !m_pNetwork->CanStartRTT( message.GetNetworkKey() ) )
				{
					m_SendPacketBufferPool.Free( plstPacketBuffer->Dequeue() );
					return;
				}
				/*/

				m_pNetwork->SetRttStartTime( message.GetNetworkKey() );
			}

			// Check Sum�� �����Ѵ�.
			INT nChangeLength;
			pPacketAssembler->SetCheckSum( pPacketBuffer->GetData(), pPacketBuffer->GetDataSize(), nChangeLength );
			pPacketBuffer->SetDataSize( nChangeLength );

			bRet = m_pNetwork->Send(
				(unsigned long)message.GetNetworkKey(),
				pPacketBuffer->GetData(),
				pPacketBuffer->GetDataSize(),
				nError );

			if( bRet )
			{
				m_SendPacketBufferPool.Free( plstPacketBuffer->Dequeue() );
			}
		}
	}
	else
	{
		if( message.GetType() == CMessage::MessageType::CloseType )
		{
			pPacketAssembler->SetFree();
		}

		// DataType
		else
		{
			CPacketBuffer*	pPacketBuffer	= message.GetPacketBuffer();

			if( bFirst )
			{
				CPacketBuffer* pTmp		= plstPacketBuffer->Dequeue();
				plstPacketBuffer->AddFirst( pPacketBuffer );
				plstPacketBuffer->AddFirst( pTmp );
			}
			else
			{
				plstPacketBuffer->Add( pPacketBuffer );
			}

			// Check Sum�� �����Ѵ�.
			INT nChangeLength;
			pPacketAssembler->SetCheckSum( pPacketBuffer->GetData(), pPacketBuffer->GetDataSize(), nChangeLength );
			pPacketBuffer->SetDataSize( nChangeLength );
		}
	}
	pPacketAssembler->Unlock();
}

void CSessionManager::SendPacket( unsigned long lpNetworkKey, CPacketBuffer* pPacketBuffer, BOOL bFirst )
{
	//ASSERT( FALSE );
	/*
	//
	// step 2 : send�� ��ü�� �ٸ� ��ü�̸� send packet�� drop ��Ų��.
	//
	unsigned long			pAssemblerKey;
	CPacketAssembler*	pPacketAssembler;
	m_pNetwork->GetAssemblerKey( lpNetworkKey, pAssemblerKey );
	pPacketAssembler = (CPacketAssembler*)pAssemblerKey;
	if( pPacketBuffer->GetHandle() != dwAssemblerHandle )
	{
//		m_SendPacketPool.Free( pSendPacket );
		m_SendPacketBufferPool.Free( pPacketBuffer );
		return;
	}



	// Check Sum�� �����Ѵ�.
	INT nChangeLength;
	pPacketAssembler->SetCheckSum( pPacketBuffer->GetData(), pPacketBuffer->GetDataSize(), nChangeLength );
	pPacketBuffer->SetDataSize( nChangeLength );

	pPacketAssembler->Lock();


	// Send�� List�� �����´�.
	int		nError;
	BOOL	bRet;
	CList<CPacketBuffer*>* plstPacketBuffer = pPacketAssembler->GetPacketBufferList();
	if( plstPacketBuffer->IsEmpty() )
	{
		// �켱 �Է��� �Ϸ��� ���� �ֱ� ��Ŷ �ٷ� ������ ������ �Ѵ�.
		// ���� �ֱ� ��Ŷ�� ������ ���۵� �� �ֱ� �����̴�.
		if( bFirst )
		{
			CPacketBuffer* pTmp		= plstPacketBuffer->Dequeue();
			plstPacketBuffer->AddFirst( pPacketBuffer );
			plstPacketBuffer->AddFirst( pTmp );
		}
		else
		{
			plstPacketBuffer->Add( pPacketBuffer );
		}

		pPacketAssembler->Unlock();

		// Send�� ��Ŷ�� Ping ��Ŷ�̸� �ð��� ����Ѵ�.
		CPacket* pPacket = reinterpret_cast<CPacket*>( pPacketBuffer->GetData() );
		if( pPacket->GetCommand() == CPacket::PredefineCommand::Ping ||
			pPacket->GetCommand() == CPacket::PredefineCommand::PingAck )
		{
			if( !m_pNetwork->CanStartRTT( lpNetworkKey ) )
			{
				return;
			}

			m_pNetwork->SetRttStartTime( lpNetworkKey );
		}

		bRet = m_pNetwork->Send(
			lpNetworkKey,
			pPacketBuffer->GetData(),
			pPacketBuffer->GetDataSize(),
			nError );

		if( !bRet )
		{
			pPacketAssembler->Lock();

			// OnFree�� ���ÿ� ����� �� �����Ƿ� ���ǰ˻縦 ���� �Ѵ�.
			if( !plstPacketBuffer->IsEmpty() )
			{
				plstPacketBuffer->Pop();
				m_SendPacketBufferPool.Free( pPacketBuffer );
			}

			pPacketAssembler->Unlock();
		}
	}
	else
	{
		if( bFirst )
		{
			CPacketBuffer* pTmp		= plstPacketBuffer->Dequeue();
			plstPacketBuffer->AddFirst( pPacketBuffer );
			plstPacketBuffer->AddFirst( pTmp );
		}
		else
		{
			plstPacketBuffer->Add( pPacketBuffer );
		}

		pPacketAssembler->Unlock();
	}

	/*
	int		nError;
	BOOL	bRet;
	
	if( pPacketAssembler->IsEmptySendList() )
	{
		pPacketAssembler->AddToSendList( pSendPacket, bFirst );

		// Send�� ��Ŷ�� Ping ��Ŷ�̸� �ð��� ����Ѵ�.
		CPacket* pPacket = (CPacket*)pPacketAssembler->GetSendBuffer();
		if( pPacket->GetCommand() == CPacket::PredefineCommand::Ping ||
			pPacket->GetCommand() == CPacket::PredefineCommand::PingAck )
		{
			if( !m_pNetwork->CanStartRTT( message.GetNetworkKey() ) )
			{
				pPacketAssembler->Unlock();
				return;
			}

			m_pNetwork->SetRttStartTime( message.GetNetworkKey() );
		}

		ASSERT( pPacketAssembler->GetSendSize() );

		char* pData = pPacketAssembler->GetSendBuffer();
		int nSize = pPacketAssembler->GetSendSize();

		pPacketAssembler->Unlock();

		bRet = m_pNetwork->Send(
			(unsigned long)message.GetNetworkKey(),
			pData,
			nSize,
			nError );

		if( !bRet )
		{
			pPacketAssembler->Lock();

			pSendPacket = pPacketAssembler->RemoveFromSendList( nSize );
			if( pSendPacket != NULL )
				
			m_SendPacketPool.Free( pSendPacket );

			pPacketAssembler->Unlock();
		}
	}
	else
	{
		pPacketAssembler->AddToSendList( pSendPacket, bFirst );
		pPacketAssembler->Unlock();
	}
	*/
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
			// Message�� Data �϶��� �Ҵ�� ���� �����Ѵ�.
			CPacketBuffer* pPacketBuffer = m_OldMessage.GetPacketBuffer();
			m_RecvPacketBufferPool.Free( pPacketBuffer );
		}
	}
	m_RecvMessageQueue.DequeuePacket( outMessage );
	
	m_OldMessage = outMessage;
}


// Ÿ�̸Ӹ� ���۽�Ų��.
void CSessionManager::StartTimer( int nTickTime, int nMaxTickCount, int nSize )
{
	m_Timer.Create( nTickTime, nMaxTickCount, nSize, (ITimerHandler*)this );
}

// Ÿ�̸Ӹ� �߰��Ѵ�.
void CSessionManager::AddTimer( int nTickCount, const CSignal& signal )
{
	m_Timer.SetTimer( nTickCount, signal );
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

void CSessionManager::SetReceiveEvent( unsigned long lpNetworkKey )
{
	m_pNetwork->SetConnectRecvEvent( lpNetworkKey );
}


void CSessionManager::OnAccept( unsigned long lpNetworkKey )
{
	// Receive / AcceptConnect ����ȭ�� ���� �ʱ�ȭ�� �Ѵ�.
	m_pNetwork->ResetConnectRecvEvent( lpNetworkKey );

	m_pNetwork->ResetRttTime( lpNetworkKey );

	m_RecvMessageQueue.EnqueuePacket( CMessage( lpNetworkKey, CMessage::MessageType::AcceptType ) );
}

void CSessionManager::OnConnect( unsigned long lpNetworkKey )
{
		// Receive / AcceptConnect ����ȭ�� ���� �ʱ�ȭ�� �Ѵ�.
	m_pNetwork->ResetConnectRecvEvent( lpNetworkKey );

	m_pNetwork->ResetRttTime( lpNetworkKey );

	m_RecvMessageQueue.EnqueuePacket( CMessage( lpNetworkKey, CMessage::MessageType::ConnectType ) );
}

void CSessionManager::OnBeginConnect( unsigned long lpNetworkKey )
{
	m_RecvMessageQueue.EnqueuePacket( CMessage( lpNetworkKey, CMessage::MessageType::BeginConnectType ) );
}

void CSessionManager::OnReceive( unsigned long lpNetworkKey, char* pRecvBuff, int nReceivedLength )
{
	// ù��° Receive�̸�, Connect/Accept�� ���� �̺�Ʈ�� ��ٸ���.
	m_pNetwork->ResetRecvDisconnectEvent( lpNetworkKey );
	m_pNetwork->WaitConnectRecvEvent( lpNetworkKey );

	unsigned long			pAssemblerKey;
	CPacketAssembler*	pPacketAssembler;
	m_pNetwork->GetAssemblerKey( lpNetworkKey, pAssemblerKey );
	pPacketAssembler = (CPacketAssembler*)pAssemblerKey;

	if( !pPacketAssembler->Assemble( pRecvBuff, nReceivedLength ) )
	{
		m_pNetwork->SetRecvDisconnectEvent( lpNetworkKey );
		return;
	}

	char* pPacket;
	int nPacketSize = 0;
	try
	{
		while( pPacket = (char*)pPacketAssembler->GetCompletePacket( nPacketSize ) )
		{
			CPacket* pDataPacket = (CPacket*)pPacket;

			CPacketBuffer*	pRecvPacketBuffer;
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
					if( pPingPacket->GetRealLength() != nPacketSize )
					{
						SendMessage( CMessage( lpNetworkKey, CMessage::MessageType::DoShutdownType ) );
					}

					// Alive�� ���ؼ� Ping Packet�� App�� ������.
					m_RecvMessageQueue.EnqueuePacket(
						CMessage( CMessage::MessageType::DataType, lpNetworkKey, pRecvPacketBuffer ) );


					// ���⼭���� �� ���� : ackPacket�� ��� ��Ŷ Ŭ������ ��Ʈ���� �����.. TagPackets.. ó���Ұ�..
					// �Ʒ��� SendPacket�� Send Q �� ������ �ʰ� �켱������ ������ ���� ó��

					unsigned long	pAssemblerKey;
					m_pNetwork->GetAssemblerKey( lpNetworkKey, pAssemblerKey );

					CPacketBuffer* pSendPacketBuffer = m_SendPacketBufferPool.Alloc();
					pSendPacketBuffer->CopyData(
						reinterpret_cast<CHAR*>( &m_pPackets->PingAck ),
						m_pPackets->PingAck.GetLength()
						);

					CMessage	message( CMessage::MessageType::DataType, lpNetworkKey, pSendPacketBuffer );
					SendPacket( message );
				}
				break;

				// Server -> Client�� ������ ��Ŷ
			case CPacket::PredefineCommand::PingAck :
				{
					CPacketPingAck*	pPingAckPacket;
					pPingAckPacket	= (CPacketPingAck*)pDataPacket;
					if( pPingAckPacket->GetRealLength() != nPacketSize )
					{
						SendMessage( CMessage( lpNetworkKey, CMessage::MessageType::DoShutdownType ) );
						break;
					}

					// Rtt Ÿ���� üũ�Ѵ�.
					m_pNetwork->SetRttEndTime( lpNetworkKey );

					unsigned long	pAssemblerKey;
					m_pNetwork->GetAssemblerKey( lpNetworkKey, pAssemblerKey );

					CPacketBuffer* pSendPacketBuffer = m_SendPacketBufferPool.Alloc();
					pSendPacketBuffer->CopyData(
						reinterpret_cast<CHAR*>( &m_pPackets->Ack ),
						m_pPackets->Ack.GetLength()
						);
					CMessage	message( CMessage::MessageType::DataType, lpNetworkKey, pSendPacketBuffer );
					SendPacket( message );
				}
				break;

				// Client -> Server�� ������ ��Ŷ
			case CPacket::PredefineCommand::Ack :
				{
					CPacketAck*		pAckPacket;
					pAckPacket		= (CPacketAck*)pDataPacket;
					if( pAckPacket->GetRealLength() != nPacketSize )
					{
						SendMessage( CMessage( lpNetworkKey, CMessage::MessageType::DoShutdownType ) );
						break;
					}

					// Rtt Ÿ���� üũ�Ѵ�.
					m_pNetwork->SetRttEndTime( lpNetworkKey );

					DWORD dwRttTime;
					m_pNetwork->GetRttTime( lpNetworkKey, dwRttTime );
				}
				break;

			//
			// Step 2 : ��Ŷ�� Message Q�� Enqueue�Ѵ�.
			//
			default :
				m_RecvMessageQueue.EnqueuePacket(
					CMessage( CMessage::MessageType::DataType, lpNetworkKey, pRecvPacketBuffer ) );
				break;
			}

		}
	}
	catch( int nErrorCode )
	{
		m_pNetwork->ShutDown( lpNetworkKey );
		m_pNetwork->SetRecvDisconnectEvent( lpNetworkKey );
		nErrorCode = 0;
		return;
	}
	m_pNetwork->SetRecvDisconnectEvent( lpNetworkKey );
}

void CSessionManager::OnSend( unsigned long lpNetworkKey, char* pRecvBuff, int nSendedLength )
{
	unsigned long				pAssemblerKey;
	CPacketAssembler*		pPacketAssembler;
	CList<CPacketBuffer*>*	plstPacketBuffer;		// SendPacket List
	CPacketBuffer*			pSendPacketBuffer;		// Send�� ��Ŷ ����

	m_pNetwork->GetAssemblerKey( lpNetworkKey, pAssemblerKey );
	pPacketAssembler	= reinterpret_cast<CPacketAssembler*>( pAssemblerKey );
	plstPacketBuffer	= pPacketAssembler->GetPacketBufferList();

	pPacketAssembler->Lock();
	
	int		nError;
	BOOL	bRet;

	if( pPacketAssembler->IsFree() )
	{
		OnFreeNetworkObject( lpNetworkKey );
		m_pNetwork->FreeNetworkKey( lpNetworkKey );
		pPacketAssembler->Unlock();
		return;
	}
	
	if( plstPacketBuffer->IsEmpty() )
	{
		pPacketAssembler->Unlock();
		return;
	}
	

	// ť�׵� ��� ��Ŷ ����
	pSendPacketBuffer	= plstPacketBuffer->GetRearElement();
	pSendPacketBuffer->ReplaceDataPos( nSendedLength );

	if( pSendPacketBuffer->GetDataSize() == 0 )
	{
		m_SendPacketBufferPool.Free( plstPacketBuffer->Dequeue() );
		pSendPacketBuffer = plstPacketBuffer->GetRearElement();
	}

	while( pSendPacketBuffer )
	{
		bRet = m_pNetwork->Send(
			lpNetworkKey,
			pSendPacketBuffer ->GetData(),
			pSendPacketBuffer ->GetDataSize(),
			nError );

		if( !bRet )
			break;

		m_SendPacketBufferPool.Free( plstPacketBuffer->Dequeue() );
		pSendPacketBuffer = plstPacketBuffer->GetRearElement();
	}
	pPacketAssembler->Unlock();
}

void CSessionManager::OnClose( unsigned long lpNetworkKey )
{
	m_pNetwork->WaitRecvDisconnectEvent( lpNetworkKey );

	m_RecvMessageQueue.EnqueuePacket( CMessage( lpNetworkKey, CMessage::MessageType::CloseType ) );
}

void CSessionManager::OnAllocNetworkObject( unsigned long lpNetworkKey )
{
	// ��Ŷ ����� �Ҵ� �� �ʱ�ȭ
	CPacketAssembler* pPacketAssembler = m_PacketAssemblerPool.Alloc();

	if( !pPacketAssembler->GetPacketBufferList()->IsEmpty() )
	{
		int a = 3;
	}


	if( !pPacketAssembler->IsCreated() )
	{
		pPacketAssembler->Create( m_nPacketSize * 2 );
	}
	pPacketAssembler->ResetCloseWait();


	// ��Ŷ ������� ���
	m_pNetwork->SetAssemblerKey( lpNetworkKey, (unsigned long)pPacketAssembler );


	// �ӽ÷� SessionGate::Start���� ����� �ű��.
	InitPacketType( lpNetworkKey, CPacketCheckSum::CheckSumType::TwoByteZoroOne, CPacketAssembler::PacketUnitType::Length );
	InitLengthFieldOfPacket( lpNetworkKey, CPacket::GetLengthPos(), CPacketAssembler::LengthSizeType::TwoByte, CPacketAssembler::LengthType::Total );
}

void CSessionManager::OnFreeNetworkObject( unsigned long lpNetworkKey )
{
	unsigned long				pAssemblerKey;
	CPacketAssembler*		pPacketAssembler;
//	DWORD					dwAssemblerHandle;
	CList<CPacketBuffer*>*	plstPacketBuffer;		// SendPacket List
	CPacketBuffer*			pPacketBuffer;
	
	m_pNetwork->GetAssemblerKey( lpNetworkKey, pAssemblerKey );
	pPacketAssembler = (CPacketAssembler*)pAssemblerKey;

	pPacketAssembler->Reset();

	plstPacketBuffer	= pPacketAssembler->GetPacketBufferList();
	while( !plstPacketBuffer->IsEmpty() )
	{
		pPacketBuffer	= plstPacketBuffer->Dequeue();
		m_SendPacketBufferPool.Free( pPacketBuffer );
	}

	m_PacketAssemblerPool.Free( pPacketAssembler );
	m_pNetwork->SetAssemblerKey( lpNetworkKey, NULL );
}




void CSessionManager::OnRecvInet( unsigned long lpInetKey, unsigned long lpContext )
{
	/*
#ifndef CLIENT_MODULE
	::EnterCriticalSection( &m_csHttp );
	int nResult = m_InetHttp.HttpResult( lpInetKey );

	int nRet;
	while( TRUE )
	{
		CPacketBuffer* pPacketBuffer = m_RecvPacketBufferPool.Alloc();
		nRet = m_InetHttp.HttpResponse( lpInetKey, pPacketBuffer->GetData(), pPacketBuffer->GetBufferSize() );
		if( nRet <= 0 )
			break;

		pPacketBuffer->SetDataSize( nRet );
//		m_HttpMessage.SetHttpData( nRet, lpContext );
		m_RecvMessageQueue.EnqueuePacket(
			CMessage( CMessage::MessageType::HttpResponse, (CRequestObject*) lpContext, pPacketBuffer )
			);
	}
    
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
	::LeaveCriticalSection( &m_csHttp );
#endif
	*/
}

void CSessionManager::OnErrorInet( unsigned long lpContext, DWORD nError )
{
	m_RecvMessageQueue.EnqueuePacket( CMessage( lpContext, CMessage::MessageType::HttpError, nError ) );
}

DWORD __stdcall CSessionManager::SendThreadStartingPoint(PVOID pvParam)
{
	CSessionManager*	pThis		= (CSessionManager*) pvParam;
	CMessage&			sendMessage2	= pThis->m_SendMessage;
	CMessage sendMessage;
//	sendMessage.Create( 256 );

	while( TRUE )
	{
		pThis->m_SendMessageQueue.DequeuePacket( sendMessage );
		switch( sendMessage.GetType() )
		{

		case CMessage::MessageType::DoCloseType :
			pThis->m_pNetwork->CloseSocket( (unsigned long)sendMessage.GetNetworkKey() );
			break;

		case CMessage::MessageType::DoShutdownType :
			pThis->m_pNetwork->ShutDown( (unsigned long)sendMessage.GetNetworkKey() );
			break;

		case CMessage::MessageType::DataType :
			pThis->SendPacket( sendMessage );
			break;

		default:
			break;

		}
	}
	return 0;
}