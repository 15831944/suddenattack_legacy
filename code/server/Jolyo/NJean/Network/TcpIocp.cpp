#include "StdAfx.h"
#include ".\TcpIocp.h"
#include <Mswsock.h>

using namespace Network;
using namespace Network::TcpIOCP;
using namespace Common::DataStructure;

/**************************************************************************************
	������ / �Ҹ���
***************************************************************************************/
CTcpIocp::CTcpIocp()
:	m_pNetworkHandler		( NULL	)
,	m_aListener				( NULL	)
,	m_nNumOfConnectSockets	( 0		)
,	m_nNumOfListenSockets	( 0		)
,	m_nRecvBuffSize			( 0		)
,	m_nSendBuffSize			( 0		)
{
	m_a=0;
}

CTcpIocp::~CTcpIocp(void)
{
}


/**************************************************************************************
	Public Operations
***************************************************************************************/


// ���ҽ����� �����Ѵ�.
//		nNumOfCommSockets : Iocp Server�� ������ų �� �ִ� ������ ����
//		nRecvBuffSize : Iocp Server�� Ŭ���̾�Ʈ�� ����� �ϴ� �� ��ü�� Recv Buffer�� ũ��
//		nSendBuffSize : Iocp Server�� Ŭ���̾�Ʈ�� ����� �ϴ� �� ��ü�� Send Buffer�� ũ��
//		nAcceptPoolSize : Accept�� ���� ������ �ϱ� ���ؼ� �̸� ����� Accept�� Pool�� ũ��
//		bAcceptRecvEvent : Accept �̺�Ʈ�� ������ ����, True�̸� Accept�̺�Ʈ�� ���� �ʰ�, ������ Recv �߻��� AcceptRecv �̺�Ʈ�� �޴´�.

BOOL CTcpIocp::Create( ITcpNetworkHandler* pNetworkHandler, INT nNumOfListener, INT nNumOfConnectSockets, INT nRecvBuffSize, INT nSendBuffSize/*, UINT nAcceptPoolSize, BOOL bAcceptRecvEvent = FALSE*/ )
{
	// ��� ���� ����
	m_pNetworkHandler		= pNetworkHandler;
	m_nNumOfConnectSockets	= nNumOfConnectSockets;
	m_nNumOfListenSockets	= nNumOfListener;

	m_nRecvBuffSize			= nRecvBuffSize;
	m_nSendBuffSize			= nSendBuffSize;

	m_nCreatedLister		= 0;


	// Listener ����
	m_aListener	= new TagListener[ nNumOfListener ];
	for( int i = 0; i < nNumOfListener; i++ )
	{
		m_aListener[i].OverlappedListenSocket.SetServiceIndex( -1 );
	}
	if( m_aListener == NULL )
	{
		Destroy();
		return FALSE;
	}

	// Connect Socket Pool ����
	if( !m_OverlappedConnectSocketPool.Create( nNumOfConnectSockets ) )
	{
		Destroy();
		return FALSE;
	}

	//
	// Overlapped Socket Pool�� ��� ��ü���� ���۸� ����
	//
	COverlappedCommSocket** pOverlappedSocket = new COverlappedCommSocket*[nNumOfConnectSockets];
	for( INT i = 0; i < nNumOfConnectSockets; i++ )
	{
		pOverlappedSocket[i] = m_OverlappedConnectSocketPool.Alloc();
		if( !pOverlappedSocket[i]->Create( &m_OverlappedConnectSocketPool, nRecvBuffSize, nSendBuffSize ) )
		{
			Destroy();
			return FALSE;
		}
	}
	for( INT i = 0; i < nNumOfConnectSockets; i++ )
	{
		m_OverlappedConnectSocketPool.Free( pOverlappedSocket[i] );
	}
	delete[] pOverlappedSocket;

	m_AsyncIocp.Create( this );

	return TRUE;
}

// ���ҽ����� �����Ѵ�.
void CTcpIocp::Destroy()
{
	COverlappedCommSocket** pOverlappedSocket = new COverlappedCommSocket*[m_nNumOfConnectSockets];
	for( INT i = 0; i < m_nNumOfConnectSockets; i++ )
	{
		pOverlappedSocket[i] = m_OverlappedConnectSocketPool.Alloc();
		pOverlappedSocket[i]->Destroy();
	}
	for( INT i = 0; i < m_nNumOfConnectSockets; i++ )
	{
		m_OverlappedConnectSocketPool.Free( pOverlappedSocket[i] );
	}
	delete[] pOverlappedSocket;

	m_OverlappedConnectSocketPool.Destroy();

	if( m_aListener )
		delete[] m_aListener;
	m_aListener = NULL;


	m_nNumOfConnectSockets	= 0;
	m_nNumOfConnectSockets	= 0;
	m_nCreatedLister		= 0;
}

//////////////////////////////////////////////////////////////////////////////
BOOL CTcpIocp::CreateListener( INT nServiceIndex, INT nNumOfCommSockets, INT nAcceptPoolSize, BOOL bAcceptRecvEvent/* = FALSE*/ )
{
	m_aListener[m_nCreatedLister].OverlappedListenSocket.SetServiceIndex( nServiceIndex );

	if( bAcceptRecvEvent )
	{
		if( !m_aListener[m_nCreatedLister].OverlappedListenSocket.Create(
			&m_aListener[m_nCreatedLister].OverlappedCommSocketPool,
			m_nRecvBuffSize,
			nAcceptPoolSize ) )
		{
			return FALSE;
		}

		if( !m_aListener[m_nCreatedLister].OverlappedCommSocketPool.Create( nNumOfCommSockets ) )
		{
			m_aListener[m_nCreatedLister].OverlappedListenSocket.Destroy();
			return FALSE;
		}
		//
		// Overlapped Socket Pool�� ��� ��ü���� ���۸� ����
		//
		COverlappedCommSocket** pOverlappedSocket = new COverlappedCommSocket*[nNumOfCommSockets];
		for( INT i = 0; i < nNumOfCommSockets; i++ )
		{
			pOverlappedSocket[i] = m_aListener[m_nCreatedLister].OverlappedCommSocketPool.Alloc();
			if( !pOverlappedSocket[i]->Create(
				&m_aListener[m_nCreatedLister].OverlappedCommSocketPool,
				m_nRecvBuffSize,
				m_nSendBuffSize ) )
			{
				Destroy();
				return FALSE;
			}
		}
		for( INT i = 0; i < nNumOfCommSockets; i++ )
		{
			m_aListener[m_nCreatedLister].OverlappedCommSocketPool.Free( pOverlappedSocket[i] );
		}
		delete[] pOverlappedSocket;
	
	}
	else
	{
		if( !m_aListener[m_nCreatedLister].OverlappedListenSocket.Create(
			&m_aListener[m_nCreatedLister].OverlappedCommSocketPool,
			0,
			nAcceptPoolSize ) )
		{
			return FALSE;
		}

		if( !m_aListener[m_nCreatedLister].OverlappedCommSocketPool.Create( nNumOfCommSockets ) )
		{
			m_aListener[m_nCreatedLister].OverlappedListenSocket.Destroy();
			return FALSE;
		}
		//
		// Overlapped Socket Pool�� ��� ��ü���� ���۸� ����
		//
		COverlappedCommSocket** pOverlappedSocket = new COverlappedCommSocket*[nNumOfCommSockets ];
		for( INT i = 0; i < nNumOfCommSockets; i++ )
		{
			pOverlappedSocket[i] = m_aListener[m_nCreatedLister].OverlappedCommSocketPool.Alloc();
			if( !pOverlappedSocket[i]->Create(
				&m_aListener[m_nCreatedLister].OverlappedCommSocketPool,
				m_nRecvBuffSize,
				m_nSendBuffSize ) )
			{
				Destroy();
				return FALSE;
			}
		}
		for( INT i = 0; i < nNumOfCommSockets; i++ )
		{
			m_aListener[m_nCreatedLister].OverlappedCommSocketPool.Free( pOverlappedSocket[i] );
		}
		delete[] pOverlappedSocket;
	}

	m_nCreatedLister++;
	return TRUE;
}

// ������ Listen ���Ͽ� Bind�� �Ѵ�.
BOOL CTcpIocp::Bind( INT nServiceIndex, const sockaddr* addr, INT namelen, INT nListenQueue/* = 5*/ )
{
	int nIndex = -1;
	for( int i = 0; i < m_nNumOfListenSockets; i++ )
	{
		if( nServiceIndex == m_aListener[i].OverlappedListenSocket.GetServiceIndex() )
		{
			nIndex = i;
			break;
		}
	}
	if( nIndex < 0 )
		return FALSE;

	if( !m_aListener[nIndex].OverlappedListenSocket.Bind( addr, namelen ) )
		return FALSE;

	if( !m_aListener[nIndex].OverlappedListenSocket.Listen( nListenQueue ) )
		return FALSE;

	return TRUE;
}

// ������ Listen ���Ͽ� Bind�� �Ѵ�.
BOOL CTcpIocp::Bind( INT nServiceIndex, INT nPort, u_long hostAddr/* = INADDR_ANY*/, short family/* = AF_INET*/, INT nListenQueue/* = 5*/ )
{
	int nIndex = -1;
	for( int i = 0; i < m_nNumOfListenSockets; i++ )
	{
		if( nServiceIndex == m_aListener[i].OverlappedListenSocket.GetServiceIndex() )
		{
			nIndex = i;
			break;
		}
	}
	if( nIndex < 0 )
		return FALSE;

	if( !m_aListener[nIndex].OverlappedListenSocket.Bind( nPort, hostAddr, family ) )
		return FALSE;

	if( !m_aListener[nIndex].OverlappedListenSocket.Listen( nListenQueue ) )
		return FALSE;

	return TRUE;
}

BOOL CTcpIocp::StartListen( INT nServiceIndex )
{
	int nIndex = -1;
	for( int i = 0; i < m_nNumOfListenSockets; i++ )
	{
		if( nServiceIndex == m_aListener[i].OverlappedListenSocket.GetServiceIndex() )
		{
			nIndex = i;
			break;
		}
	}
	if( nIndex < 0 )
		return FALSE;

	COverlappedListenSocket* pOvListenSocket = &m_aListener[nIndex].OverlappedListenSocket;
	
	if( !m_AsyncIocp.AssignIoCompletionPort( pOvListenSocket->GetIOHandle(), (ULONG_PTR)pOvListenSocket ) )
	{
		return FALSE;
	}

	if( !pOvListenSocket->RunAcceptPool() )
	{
		return FALSE;
	}

	return TRUE;
}
//////////////////////////////////////////////////////////////////////////////


// �ٸ� ������ ������ �Ѵ�.
BOOL CTcpIocp::Connect( ULONG_PTR lpOwnerKey, const sockaddr* addr, INT namelen )
{
	// Connect�� ���� ó���� ���� ������ ���̴�.

	/*

	// Recv/Send�� overlapped ��ü �Ҵ�
	COverlappedCommSocket* pCommSocket;
	pCommSocket = m_OverlappedConnectSocketPool.Alloc();

	pCommSocket->Init();

	pCommSocket->SetOwnerKey( lpOwnerKey );


	pCommSocket->m_sSocket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);

	if( pCommSocket == NULL )
		return FALSE;

	if( !pCommSocket->Connect( addr, namelen ) )
	{
		pCommSocket->CloseSocket();
		m_OverlappedConnectSocketPool.Free( pCommSocket );
		return FALSE;
	}

	m_pNetworkHandler->OnAllocNetworkObject( (ULONG_PTR)pCommSocket );

	// IOCP�� Comm Socket �ڵ� ���
	if( !m_AsyncIocp.AssignIoCompletionPort( pCommSocket->GetIOHandle(), (ULONG_PTR)pCommSocket ) )
	{
		// ������ ����, �ش� �ڵ��� ��ȯó�� �Ѵ�.
		pCommSocket->CloseSocket();

		// Overlapped Object�� �����Ǿ����� �뺸�Ѵ�.
		m_pNetworkHandler->OnFreeNetworkObject( (ULONG_PTR)pCommSocket );

		m_OverlappedConnectSocketPool.Free( pCommSocket );
		return FALSE;
	}

	// Connect �̺�Ʈ�� �뺸�Ѵ�.
	m_pNetworkHandler->OnConnect( (ULONG_PTR)pCommSocket );

	// IOCP�� Receive �̺�Ʈ ���
	BOOL bRet;
	bRet = pCommSocket->RecvPost();
	if( bRet )
	{
		// ������ ����, �ش� �ڵ��� ��ȯó�� �Ѵ�.
		INT ErrCode = ::WSAGetLastError();
		switch( ErrCode )
		{
		case WSA_IO_PENDING:	// Overlapped operations will complete later
			break;

		default:
			pCommSocket->CloseSocket();
			m_pNetworkHandler->OnClose( (ULONG_PTR)pCommSocket );

			// Overlapped Object�� �����Ǿ����� �뺸�Ѵ�.
			m_pNetworkHandler->OnFreeNetworkObject( (ULONG_PTR)pCommSocket );

			m_OverlappedConnectSocketPool.Free( pCommSocket );

			return FALSE;
		}
	}
	else
	{
		INT ErrCode = ::WSAGetLastError();
	}


	*/

	return TRUE;
}


// sendBuff�� ���� �����Ѵ�.
BOOL CTcpIocp::Send( ULONG_PTR lpNetworkKey, const char* sendBuff, INT nLen, INT& nOutError )
{
	ASSERT( nLen > 0 );

	COverlappedCommSocket* pCommSocket = (COverlappedCommSocket*)lpNetworkKey;

	::CopyMemory( pCommSocket->GetSendBuffer(), sendBuff, nLen );

	pCommSocket->SetSendLength( nLen );

	BOOL bRet = pCommSocket->SendPost( nOutError );

	return bRet;
}


// �ش� Ű�� ���� ��ü�� �ݴ´�.
void CTcpIocp::CloseSocket( ULONG_PTR lpNetworkKey )
{
	COverlappedCommSocket* pCommSocket = (COverlappedCommSocket*)lpNetworkKey;

	//char strLog[1024];
	//sprintf( strLog, "Close %d\r\n", (int)lpNetworkKey );
	//OutputDebugString( strLog );

	// �ش� ������ �ݴ´�.
	pCommSocket->CloseSocket();
}

// �ش�Ű�� ���� ��ü�� Shutdown�Ѵ�.
void CTcpIocp::ShutDown( ULONG_PTR lpNetworkKey )
{
	COverlappedCommSocket* pCommSocket = (COverlappedCommSocket*)lpNetworkKey;

	pCommSocket->ShutDown();
}

void CTcpIocp::SetConnectRecvEvent( ULONG_PTR lpNetworkKey )
{
	COverlappedCommSocket* pCommSocket = (COverlappedCommSocket*)lpNetworkKey;
	pCommSocket->SetEvent();
}

void CTcpIocp::ResetConnectRecvEvent( ULONG_PTR lpNetworkKey )
{
	COverlappedCommSocket* pCommSocket = (COverlappedCommSocket*)lpNetworkKey;
	pCommSocket->ResetEvent();
}

void CTcpIocp::WaitConnectRecvEvent( ULONG_PTR lpNetworkKey )
{
	COverlappedCommSocket* pCommSocket = (COverlappedCommSocket*)lpNetworkKey;
	pCommSocket->WaitEvent();
}

void CTcpIocp::SetRecvDisconnectEvent( ULONG_PTR lpNetworkKey )
{
	COverlappedCommSocket* pCommSocket = (COverlappedCommSocket*)lpNetworkKey;
	pCommSocket->SetEvent2();
}

void CTcpIocp::ResetRecvDisconnectEvent( ULONG_PTR lpNetworkKey )
{
	COverlappedCommSocket* pCommSocket = (COverlappedCommSocket*)lpNetworkKey;
	pCommSocket->ResetEvent2();
}

void CTcpIocp::WaitRecvDisconnectEvent( ULONG_PTR lpNetworkKey )
{
	COverlappedCommSocket* pCommSocket = (COverlappedCommSocket*)lpNetworkKey;
	pCommSocket->WaitEvent2();
}


void CTcpIocp::ResetRttTime( ULONG_PTR lpNetworkKey )
{
	COverlappedCommSocket* pCommSocket = (COverlappedCommSocket*)lpNetworkKey;
	pCommSocket->ResetRttTime();
}


BOOL CTcpIocp::CanStartRTT( ULONG_PTR lpNetworkKey )
{
	COverlappedCommSocket*	pCommSocket = (COverlappedCommSocket*)lpNetworkKey;
	return pCommSocket->CanStartRTT();
}

void CTcpIocp::SetRttStartTime( ULONG_PTR lpNetworkKey )
{
	COverlappedCommSocket* pCommSocket = (COverlappedCommSocket*)lpNetworkKey;
	pCommSocket->SetRttStartTime();
}

void CTcpIocp::SetRttEndTime( ULONG_PTR lpNetworkKey )
{
	COverlappedCommSocket* pCommSocket = (COverlappedCommSocket*)lpNetworkKey;
	pCommSocket->SetRttEndTime();
}

void CTcpIocp::GetRttTime( ULONG_PTR lpNetworkKey, DWORD& out_dwRttTime )
{
	COverlappedCommSocket* pCommSocket = (COverlappedCommSocket*)lpNetworkKey;

	out_dwRttTime = pCommSocket->GetRttTime();
}


/**************************************************************************************
	Private Operations
***************************************************************************************/

// ��ϵ� IO�ڵ� ó���� Accept
inline void CTcpIocp::ProcessAcceptCompletion( DWORD dwNumberOfBytesTransferred, ULONG_PTR lpNetworkKey, LPOVERLAPPED lpOverlapped )
{
	COverlappedListenSocket*					pListenSocket;
	CMemoryPoolSafty<COverlappedCommSocket>*	pCommSocketPool;
	COverlappedCommSocket*						pCommSocket;
	
	pListenSocket	= (COverlappedListenSocket*)lpNetworkKey;
	pCommSocketPool	= pListenSocket->GetCommSocketPool();

	// Recv/Send�� overlapped ��ü �Ҵ�
	pCommSocket = pCommSocketPool->Alloc();
	
	if( pCommSocket->GetRecvBuffer() == NULL )
	{
		BOOL bCreate = pCommSocket->Create( &m_OverlappedConnectSocketPool, m_nRecvBuffSize, m_nSendBuffSize );
		ASSERT( bCreate );
	}

	pCommSocket->Init();
	pCommSocket->SetConnect();

	// Parent ���� ����
	pCommSocket->m_pParentOvSocket	= pListenSocket;

	// Overlapped Object�� �Ҵ�Ǿ����� �뺸�Ѵ�.
	m_pNetworkHandler->OnAllocNetworkObject( (ULONG_PTR)pCommSocket );

	pCommSocket->SetSocket(	pListenSocket->GetSocketOfOverlappedObject( lpOverlapped ) );
	pCommSocket->SetPeerAddr( (sockaddr_in *) pListenSocket->GetPeerAddr( lpOverlapped ) );

	// AcceptEx ���� ��ü�� ����
	INT err;
	err = ::setsockopt( pCommSocket->GetSocket(), 
	    SOL_SOCKET, 
	    SO_UPDATE_ACCEPT_CONTEXT, 
	    (char *)&pCommSocket->m_pParentOvSocket->m_sSocket, 
	    sizeof(pCommSocket->m_pParentOvSocket->m_sSocket) );

	// IOCP�� Comm Socket �ڵ� ���
	if( !m_AsyncIocp.AssignIoCompletionPort( pCommSocket->GetIOHandle(), (ULONG_PTR)pCommSocket ) )
	{
		// ������ ����, �ش� �ڵ��� ��ȯó�� �Ѵ�.
		pCommSocket->CloseSocket();

		// Overlapped Object�� �����Ǿ����� �뺸�Ѵ�.
//		m_pNetworkHandler->OnFreeNetworkObject( (ULONG_PTR)pCommSocket );

//		pCommSocketPool->Free( pCommSocket );

		// ���� AcceptEx�� ����
		if( !pListenSocket->AcceptEx( lpOverlapped ) )
		{
			// ??????????????????????????????????????????????
		}
		return;
	}


	// Accept �̺�Ʈ�� �뺸�Ѵ�.
	m_pNetworkHandler->OnAccept( (ULONG_PTR)pCommSocket );



	// IOCP�� Receive �̺�Ʈ ���
	BOOL bRet;
	bRet = pCommSocket->RecvPost();
	if( bRet )
	{
		// ������ ����, �ش� �ڵ��� ��ȯó�� �Ѵ�.
		INT ErrCode = ::WSAGetLastError();
		switch( ErrCode )
		{
		case WSA_IO_PENDING:	// Overlapped operations will complete later
			break;

		default:
//			pCommSocket->CloseSocket();
			m_pNetworkHandler->OnClose( (ULONG_PTR)pCommSocket );

			// Overlapped Object�� �����Ǿ����� �뺸�Ѵ�.
//			m_pNetworkHandler->OnFreeNetworkObject( (ULONG_PTR)pCommSocket );

//			pCommSocketPool->Free( pCommSocket );

			// ���� AcceptEx�� ����
//			if( !pListenSocket->AcceptEx( lpOverlapped ) )
//			{
				// ????????????????????????????????????????
//			}
//			return;
			break;
		}
	}

	// ���� AcceptEx�� ����
	if( !pListenSocket->AcceptEx( lpOverlapped ) )
	{
		// ????????????????????????????????????
	}
}

// ��ϵ� IO�ڵ� ó���� Accept and Recv
inline void CTcpIocp::ProcessAcceptRecvCompletion( DWORD dwNumberOfBytesTransferred, ULONG_PTR lpNetworkKey, LPOVERLAPPED lpOverlapped )
{
}

// ��ϵ� IO�ڵ� ó���� Recv
inline void CTcpIocp::ProcessRecvCompletion( DWORD dwNumberOfBytesTransferred, ULONG_PTR lpNetworkKey, LPOVERLAPPED lpOverlapped )
{
	COverlappedCommSocket*						pCommSocket;
	CMemoryPoolSafty<COverlappedCommSocket>*	pCommSocketPool;

	pCommSocket		= (COverlappedCommSocket*)lpNetworkKey;
	pCommSocketPool	= pCommSocket->GetCommSocketPool();


	// Ŭ���̾�Ʈ�� ������ ����
	if( dwNumberOfBytesTransferred == 0 )
	{
		INT nError = ::WSAGetLastError();

		m_pNetworkHandler->OnClose( (ULONG_PTR)pCommSocket );
		return;
	}

	m_pNetworkHandler->OnReceive(
		lpNetworkKey,
		pCommSocket->GetRecvBuffer(),
		dwNumberOfBytesTransferred );

	BOOL bRet;
	bRet = pCommSocket->RecvPost();
	if( bRet )
	{
		INT ErrCode = ::WSAGetLastError();
		switch( ErrCode )
		{
		case WSA_IO_PENDING:	// Overlapped operations will complete later
			break;

		default:
			m_pNetworkHandler->OnClose( (ULONG_PTR)pCommSocket );
			return;
		}
	}
}

// ��ϵ� IO�ڵ� ó���� Send
inline void CTcpIocp::ProcessSendCompletion( DWORD dwNumberOfBytesTransferred, ULONG_PTR lpNetworkKey, LPOVERLAPPED lpOverlapped )
{
	COverlappedCommSocket* pCommSocket;
	pCommSocket = (COverlappedCommSocket*)lpNetworkKey;

	// Send �̺�Ʈ�� �����Ѵ�.
	m_pNetworkHandler->OnSend( lpNetworkKey, pCommSocket->GetSendBuffer(), dwNumberOfBytesTransferred );


	int a = 3;
}


/**************************************************************************************
	Protected Implementations
***************************************************************************************/

// IIocp Implementation 
//����lpNetworkKey��GetQueuedCompletionStatus��lpCompletKey,ΪCOverlappedListenSocket
void CTcpIocp::OnGetQueuedCompletionStatus( BOOL bRet, DWORD dwNumberOfBytesTransferred, ULONG_PTR lpNetworkKey, LPOVERLAPPED lpOverlapped )
{
	COverlappedSocket* pOverlappedSocket = (COverlappedSocket*)lpNetworkKey;

//	if( bRet == FALSE && pOverlappedSocket->GetIOType( lpOverlapped ) == COverlappedSocket::SendType )
//	{
//	}

	switch( pOverlappedSocket->GetIOType( lpOverlapped ) )
	{
	case COverlappedSocket::AcceptType :
		ProcessAcceptCompletion			( dwNumberOfBytesTransferred, lpNetworkKey, lpOverlapped );
		break;

	case COverlappedSocket::AcceptRecvType :
		ProcessAcceptRecvCompletion		( dwNumberOfBytesTransferred, lpNetworkKey, lpOverlapped );
		break;

	case COverlappedSocket::RecvType :
		ProcessRecvCompletion			( dwNumberOfBytesTransferred, lpNetworkKey, lpOverlapped );
		break;

	case COverlappedSocket::SendType :
		ProcessSendCompletion			( dwNumberOfBytesTransferred, lpNetworkKey, lpOverlapped );
		break;

	default :
		break;
	}

	int a = 3;
}