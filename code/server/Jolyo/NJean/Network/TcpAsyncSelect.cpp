#include "StdAfx.h"
#include ".\TcpAsyncSelect.h"
#include <Mswsock.h>

using namespace Network;
using namespace Network::TcpAsyncSelect;
using namespace Common::DataStructure;

/**************************************************************************************
	������ / �Ҹ���
***************************************************************************************/
CTcpAsyncSelect::CTcpAsyncSelect( HWND hWindow, UINT uiMessage )
:	m_pNetworkHandler		( NULL		)
,	m_aListener				( NULL		)
,	m_nNumOfConnectSockets	( 0			)
,	m_nNumOfListenSockets	( 0			)
,	m_nRecvBuffSize			( 0			)
,	m_nSendBuffSize			( 0			)
,	m_hWindow				( hWindow	)
,	m_uiMessage				( uiMessage	)
{
}

CTcpAsyncSelect::~CTcpAsyncSelect(void)
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

BOOL CTcpAsyncSelect::Create( ITcpNetworkHandler* pNetworkHandler, INT nNumOfListener, INT nNumOfConnectSockets, INT nRecvBuffSize, INT nSendBuffSize/*, UINT nAcceptPoolSize, BOOL bAcceptRecvEvent = FALSE*/ )
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
		m_aListener[i].nServicerIndex	= -1;
	}
	if( m_aListener == NULL )
	{
		Destroy();
		return FALSE;
	}

	// Connect Socket Pool ����
	if( !m_ConnectSocketPool.Create( nNumOfConnectSockets ) )
	{
		Destroy();
		return FALSE;
	}
	//
	// Overlapped Socket Pool�� ��� ��ü���� ���۸� ����
	//
	CCommSocket** paCommSocket = new CCommSocket*[nNumOfConnectSockets];
	for( INT i = 0; i < nNumOfConnectSockets; i++ )
	{
		paCommSocket[i] = m_ConnectSocketPool.Alloc();
		if( !paCommSocket[i]->Create( &m_ConnectSocketPool, nRecvBuffSize, nSendBuffSize ) )
		{
			Destroy();
			return FALSE;
		}
	}
	for( INT i = 0; i < nNumOfConnectSockets; i++ )
	{
		m_ConnectSocketPool.Free( paCommSocket[i] );
	}
	delete[] paCommSocket;


	return TRUE;
}

// ���ҽ����� �����Ѵ�.
void CTcpAsyncSelect::Destroy()
{
	m_ConnectSocketPool.Destroy();

	if( m_aListener )
		delete[] m_aListener;
	m_aListener = NULL;



	m_nNumOfConnectSockets	= 0;
	m_nNumOfConnectSockets	= 0;
	m_nCreatedLister		= 0;
}

void CTcpAsyncSelect::FirePacket( DWORD wParam, DWORD lParam )
{
	SOCKET	sSocket		= wParam;
	const WORD wEvent	= WSAGETSELECTEVENT( lParam );
	const WORD wError	= WSAGETSELECTERROR( lParam );

	if( wError != 0 )
	{
		ProcessClose( sSocket );
		return;
	}

	switch( wEvent )
	{
	case FD_ACCEPT:
		ProcessAccept( sSocket );
		break;

	case FD_CONNECT:
		ProcessConnect( sSocket );
		break;

	case FD_CLOSE:
		ProcessClose( sSocket );
		break;

	case FD_READ:
		ProcessRecv( sSocket );
		break;

	case FD_WRITE:
		ProcessSend( sSocket );
		break;

	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////
BOOL CTcpAsyncSelect::CreateListener( INT nServiceIndex, INT nNumOfCommSockets, INT nAcceptPoolSize, BOOL bAcceptRecvEvent/* = FALSE*/ )
{
	// bAcceptRecvEvent�� �������� �ʴ´�.
	if( bAcceptRecvEvent )
	{
		return FALSE;
	}
	else
	{
		m_aListener[m_nCreatedLister].sListenSocket = ::socket( AF_INET, SOCK_STREAM, 0 );
		if( m_aListener[m_nCreatedLister].sListenSocket == INVALID_SOCKET )
			return FALSE;

		if( !m_aListener[m_nCreatedLister].CommSocketPool.Create( nNumOfCommSockets ) )
		{
			return FALSE;
		}
	}

	m_aListener[m_nCreatedLister].nServicerIndex = nServiceIndex;

	m_nCreatedLister++;

	return TRUE;
}

// ������ Listen ���Ͽ� Bind�� �Ѵ�.
BOOL CTcpAsyncSelect::Bind( INT nServiceIndex, const sockaddr* addr, INT namelen, INT nListenQueue/* = 5*/ )
{
	// Listener �˻�
	int nIndex = -1;
	for( int i = 0; i < m_nNumOfListenSockets; i++ )
	{
		if( nServiceIndex == m_aListener[i].nServicerIndex )
		{
			nIndex = i;
			break;
		}
	}
	if( nIndex < 0 )
		return FALSE;

	// binding
	if( !::bind( m_aListener[nIndex].sListenSocket, (LPSOCKADDR)&addr, namelen ) )
	{
		return FALSE;
	}

	// listening
	if( ::listen( m_aListener[nIndex].sListenSocket, nListenQueue ) == SOCKET_ERROR )
		return FALSE;

	return TRUE;
}

// ������ Listen ���Ͽ� Bind�� �Ѵ�.
BOOL CTcpAsyncSelect::Bind( INT nServiceIndex, INT nPort, u_long hostAddr/* = INADDR_ANY*/, short family/* = AF_INET*/, INT nListenQueue/* = 5*/ )
{
	// Listener �˻�
	int nIndex = -1;
	for( int i = 0; i < m_nNumOfListenSockets; i++ )
	{
		if( nServiceIndex == m_aListener[i].nServicerIndex )
		{
			nIndex = i;
			break;
		}
	}
	if( nIndex < 0 )
		return FALSE;


	// binding
	SOCKADDR_IN	serverSockAddr;

    serverSockAddr.sin_addr.s_addr	= htonl( hostAddr );
	serverSockAddr.sin_family		= family;
	serverSockAddr.sin_port			= htons( nPort );

	if( !::bind( m_aListener[nIndex].sListenSocket, (LPSOCKADDR)&serverSockAddr, sizeof(serverSockAddr) ) )
	{
		return FALSE;
	}

	// listening
	if( ::listen( m_aListener[nIndex].sListenSocket, nListenQueue ) == SOCKET_ERROR )
		return FALSE;

	return TRUE;
}

BOOL CTcpAsyncSelect::StartListen( INT nServiceIndex )
{
	// Listener �˻�
	int nIndex = -1;
	for( int i = 0; i < m_nNumOfListenSockets; i++ )
	{
		if( nServiceIndex == m_aListener[i].nServicerIndex )
		{
			nIndex = i;
			break;
		}
	}
	if( nIndex < 0 )
		return FALSE;


	// Start Listen
	int nRetVal = ::WSAAsyncSelect(
		m_aListener[nIndex].sListenSocket,
		m_hWindow,
		m_uiMessage,
		FD_ACCEPT | FD_CLOSE );

	if( nRetVal == INVALID_SOCKET )
		return FALSE;

	return TRUE;
}
//////////////////////////////////////////////////////////////////////////////


// �ٸ� ������ ������ �Ѵ�.
BOOL CTcpAsyncSelect::Connect( ULONG_PTR lpOwnerKey, const sockaddr* addr, INT namelen )
{
	CCommSocket* pCommSocket = m_ConnectSocketPool.Alloc();
	if( pCommSocket == NULL )
		return FALSE;

	if( !pCommSocket->CreateSocket() )
	{
		m_ConnectSocketPool.Free( pCommSocket );
		return FALSE;
	}

	pCommSocket->SetOwnerKey( lpOwnerKey );

	int nRetVal = ::WSAAsyncSelect(
		pCommSocket->GetSocket(),
		m_hWindow,
		m_uiMessage,
		FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE );

	if( nRetVal == INVALID_SOCKET )
	{
		pCommSocket->CloseSocket();
		m_ConnectSocketPool.Free( pCommSocket );
		return FALSE;
	}

	m_pNetworkHandler->OnAllocNetworkObject( (ULONG_PTR)pCommSocket );

	int nRet = ::WSAConnect( pCommSocket->GetSocket(), addr, namelen, NULL, NULL, NULL, NULL );
	if( nRet == SOCKET_ERROR )
	{
		if( ::WSAGetLastError() == WSAEWOULDBLOCK )
		{
			// Connect�� ������ �ʿ� ���
			m_mapSocket.mapSocketKey.insert( SocketKey_Pair( pCommSocket->GetSocket(), pCommSocket ) );
			return TRUE;
		}
		else
		{
			pCommSocket->CloseSocket();
			m_ConnectSocketPool.Free( pCommSocket );
			m_pNetworkHandler->OnFreeNetworkObject( (ULONG_PTR)pCommSocket );
			return FALSE;
		}
	}

	// Connect�� ������ �ʿ� ���
	m_mapSocket.mapSocketKey.insert( SocketKey_Pair( pCommSocket->GetSocket(), pCommSocket ) );


	ProcessConnect( pCommSocket->GetSocket() );

	printf( "����!!\n" );

	return TRUE;
}


// sendBuff�� ���� �����Ѵ�.
BOOL CTcpAsyncSelect::Send( ULONG_PTR lpNetworkKey, const char* sendBuff, INT nLen, INT& nOutError )
{
	CCommSocket* pCommSocket = (CCommSocket*)lpNetworkKey;

	return pCommSocket->Send( sendBuff, nLen );
}


// �ش� Ű�� ���� ��ü�� �ݴ´�.
void CTcpAsyncSelect::CloseSocket( ULONG_PTR lpNetworkKey )
{
	CCommSocket*	pCommSocket = (CCommSocket*)lpNetworkKey;

	// �ش� ������ �ݴ´�.
	pCommSocket->CloseSocket();
}

// �ش�Ű�� ���� ��ü�� Shutdown�Ѵ�.
void CTcpAsyncSelect::ShutDown( ULONG_PTR lpNetworkKey )
{
	CCommSocket*	pCommSocket = (CCommSocket*)lpNetworkKey;

	pCommSocket->ShutDown();
}

void CTcpAsyncSelect::SetConnectRecvEvent( ULONG_PTR lpNetworkKey )
{
	CCommSocket*	pCommSocket = (CCommSocket*)lpNetworkKey;
	pCommSocket->SetEvent();
}

void CTcpAsyncSelect::ResetConnectRecvEvent( ULONG_PTR lpNetworkKey )
{
	CCommSocket*	pCommSocket = (CCommSocket*)lpNetworkKey;
	pCommSocket->ResetEvent();
}

void CTcpAsyncSelect::WaitConnectRecvEvent( ULONG_PTR lpNetworkKey )
{
	CCommSocket*	pCommSocket = (CCommSocket*)lpNetworkKey;
	pCommSocket->WaitEvent();
}

void CTcpAsyncSelect::SetRecvDisconnectEvent( ULONG_PTR lpNetworkKey )
{
	CCommSocket*	pCommSocket = (CCommSocket*)lpNetworkKey;
	pCommSocket->SetEvent2();
}

void CTcpAsyncSelect::ResetRecvDisconnectEvent( ULONG_PTR lpNetworkKey )
{
	CCommSocket*	pCommSocket = (CCommSocket*)lpNetworkKey;
	pCommSocket->ResetEvent2();
}

void CTcpAsyncSelect::WaitRecvDisconnectEvent( ULONG_PTR lpNetworkKey )
{
	CCommSocket*	pCommSocket = (CCommSocket*)lpNetworkKey;
	pCommSocket->WaitEvent2();
}


void CTcpAsyncSelect::ResetRttTime( ULONG_PTR lpNetworkKey )
{
	CCommSocket*	pCommSocket = (CCommSocket*)lpNetworkKey;
	pCommSocket->ResetRttTime();
}

BOOL CTcpAsyncSelect::CanStartRTT( ULONG_PTR lpNetworkKey )
{
	CCommSocket*	pCommSocket = (CCommSocket*)lpNetworkKey;
	return pCommSocket->CanStartRTT();
}

void CTcpAsyncSelect::SetRttStartTime( ULONG_PTR lpNetworkKey )
{
	CCommSocket*	pCommSocket = (CCommSocket*)lpNetworkKey;
	pCommSocket->SetRttStartTime();
}

void CTcpAsyncSelect::SetRttEndTime( ULONG_PTR lpNetworkKey )
{
	CCommSocket*	pCommSocket = (CCommSocket*)lpNetworkKey;
	pCommSocket->SetRttEndTime();
}

void CTcpAsyncSelect::GetRttTime( ULONG_PTR lpNetworkKey, DWORD& out_dwRttTime )
{
	CCommSocket*	pCommSocket = (CCommSocket*)lpNetworkKey;
	out_dwRttTime = pCommSocket->GetRttTime();
}



/**************************************************************************************
	Private Operations
***************************************************************************************/

inline void CTcpAsyncSelect::ProcessAccept( SOCKET sSocket )
{
}

inline void CTcpAsyncSelect::ProcessConnect( SOCKET sSocket )
{
	m_mapSocket.IterSocketKey = m_mapSocket.mapSocketKey.find( sSocket );
	if( m_mapSocket.IterSocketKey == m_mapSocket.mapSocketKey.end() )
	{
		printf( "ProcessConnect????????????????????????\n" );
		return;
	}

	CCommSocket* pCommSocket = m_mapSocket.IterSocketKey->second;

	m_pNetworkHandler->OnConnect( (ULONG_PTR)pCommSocket );

	printf( "���� : %d\n", (int)sSocket );
}

inline void CTcpAsyncSelect::ProcessClose( SOCKET sSocket )
{
	m_mapSocket.IterSocketKey = m_mapSocket.mapSocketKey.find( sSocket );
	if( m_mapSocket.IterSocketKey == m_mapSocket.mapSocketKey.end() )
	{
		printf( "ProcessConnect????????????????????????\n" );
		return;
	}

	CCommSocket*					pCommSocket		= m_mapSocket.IterSocketKey->second;
	CMemoryPoolSafty<CCommSocket>*	pCommSocketPool	= pCommSocket->GetCommSocketPool();
	pCommSocket->CloseSocket();

	m_pNetworkHandler	->OnClose				( (ULONG_PTR)pCommSocket );
	pCommSocketPool		->Free					( pCommSocket );
	m_pNetworkHandler	->OnFreeNetworkObject	( (ULONG_PTR) pCommSocket );
}

inline void CTcpAsyncSelect::ProcessRecv( SOCKET sSocket )
{
	m_mapSocket.IterSocketKey = m_mapSocket.mapSocketKey.find( sSocket );
	if( m_mapSocket.IterSocketKey == m_mapSocket.mapSocketKey.end() )
	{
		printf( "ProcessRecv????????????????????????\n" );
		return;
	}

	CCommSocket*	pCommSocket = m_mapSocket.IterSocketKey->second;
	int				nReceive	= pCommSocket->Receive();

	if( nReceive <= 0 )
	{
//		ProcessClose( pCommSocket->GetSocket() );
		return;
	}

	// Recv �̺�Ʈ�� �����Ѵ�.
	m_pNetworkHandler->OnReceive(
		(ULONG_PTR)pCommSocket,
		pCommSocket->GetRecvBuffer(),
		nReceive );
}

inline void CTcpAsyncSelect::ProcessSend( SOCKET sSocket )
{
	m_mapSocket.IterSocketKey = m_mapSocket.mapSocketKey.find( sSocket );
	if( m_mapSocket.IterSocketKey == m_mapSocket.mapSocketKey.end() )
	{
		printf( "ProcessRecv????????????????????????\n" );
		return;
	}

	CCommSocket*	pCommSocket = m_mapSocket.IterSocketKey->second;

	if( pCommSocket->GetSentBytes() == 0 )
		return;

	// Send �̺�Ʈ�� �����Ѵ�.
	m_pNetworkHandler->OnSend(
		(ULONG_PTR)pCommSocket,
		pCommSocket->GetSendBuffer(),
		pCommSocket->GetSentBytes()
		);
}

/*
// ��ϵ� IO�ڵ� ó���� Accept
inline void CTcpAsyncSelect::ProcessAcceptCompletion( DWORD dwNumberOfBytesTransferred, ULONG_PTR lpNetworkKey, LPOVERLAPPED lpOverlapped )
{
	COverlappedListenSocket*					pListenSocket;
	CMemoryPoolSafty<COverlappedCommSocket>*	pCommSocketPool;
	COverlappedCommSocket*						pCommSocket;
	
	pListenSocket	= (COverlappedListenSocket*)lpNetworkKey;
	pCommSocketPool	= pListenSocket->GetCommSocketPool();

	// Recv/Send�� overlapped ��ü �Ҵ�
	pCommSocket = pCommSocketPool->Alloc();

	if( pCommSocket == NULL )
	{
		::closesocket( pListenSocket->GetSocketOfOverlappedObject( lpOverlapped ) );

		// ���� AcceptEx�� ����
		if( !pListenSocket->AcceptEx( lpOverlapped ) )
		{
			//
			// ???????????????????????????????????
			//
		}
		return;
	}

	pCommSocket->Init();

	// Parent ���� ����
	pCommSocket->m_pParentOvSocket	= pListenSocket;

	// Overlapped Object�� �Ҵ�Ǿ����� �뺸�Ѵ�.
	m_pNetworkHandler->OnAllocOverlappedObject( (ULONG_PTR)pCommSocket );

	pCommSocket->SetSocket(	pListenSocket->GetSocketOfOverlappedObject( lpOverlapped ) );

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
		pCommSocketPool->Free( pCommSocket );

		// Overlapped Object�� �����Ǿ����� �뺸�Ѵ�.
		m_pNetworkHandler->OnFreeOverlappedObject( (ULONG_PTR)pCommSocket );

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
			pCommSocket->CloseSocket();
			m_pNetworkHandler->OnClose( (ULONG_PTR)pCommSocket );

			pCommSocketPool->Free( pCommSocket );

			// Overlapped Object�� �����Ǿ����� �뺸�Ѵ�.
			m_pNetworkHandler->OnFreeOverlappedObject( (ULONG_PTR)pCommSocket );

			// ���� AcceptEx�� ����
			if( !pListenSocket->AcceptEx( lpOverlapped ) )
			{
				// ????????????????????????????????????????
			}
			return;
		}
	}

	// ���� AcceptEx�� ����
	if( !pListenSocket->AcceptEx( lpOverlapped ) )
	{
		// ????????????????????????????????????
	}
}

// ��ϵ� IO�ڵ� ó���� Accept and Recv
inline void CTcpAsyncSelect::ProcessAcceptRecvCompletion( DWORD dwNumberOfBytesTransferred, ULONG_PTR lpNetworkKey, LPOVERLAPPED lpOverlapped )
{
}

// ��ϵ� IO�ڵ� ó���� Recv
inline void CTcpAsyncSelect::ProcessRecvCompletion( DWORD dwNumberOfBytesTransferred, ULONG_PTR lpNetworkKey, LPOVERLAPPED lpOverlapped )
{
	COverlappedCommSocket*						pCommSocket;
	CMemoryPoolSafty<COverlappedCommSocket>*	pCommSocketPool;

	pCommSocket		= (COverlappedCommSocket*)lpNetworkKey;
	pCommSocketPool	= pCommSocket->GetCommSocketPool();


	// Ŭ���̾�Ʈ�� ������ ����
	if( dwNumberOfBytesTransferred == 0 )
	{
		// ������ ����, �ش� �ڵ��� ��ȯó�� �Ѵ�.
		// �ش� �ڵ��� ��ȯ �� close �뺸�� Recv������ �����Ѵ�.
		INT nError = ::WSAGetLastError();
		pCommSocket->CloseSocket();

		m_pNetworkHandler->OnClose( (ULONG_PTR)pCommSocket );

		pCommSocketPool->Free( pCommSocket );

		// Overlapped Object�� �����Ǿ����� �뺸�Ѵ�.
		m_pNetworkHandler->OnFreeOverlappedObject( lpNetworkKey );
		return;
	}

	// Recv �̺�Ʈ�� �����Ѵ�.
	m_pNetworkHandler->OnReceive(
		lpNetworkKey,
		pCommSocket->GetRecvBuffer(),
		dwNumberOfBytesTransferred );

	// ���� Recv �̺�Ʈ�� ����Ѵ�.
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

			// �˼� ���� ���� ����..
		default:
			pCommSocket->CloseSocket();

			m_pNetworkHandler->OnClose( (ULONG_PTR)pCommSocket );

			pCommSocketPool->Free( pCommSocket );

			// Overlapped Object�� �����Ǿ����� �뺸�Ѵ�.
			m_pNetworkHandler->OnFreeOverlappedObject( lpNetworkKey );
			return;
		}
	}
}

// ��ϵ� IO�ڵ� ó���� Send
inline void CTcpAsyncSelect::ProcessSendCompletion( DWORD dwNumberOfBytesTransferred, ULONG_PTR lpNetworkKey, LPOVERLAPPED lpOverlapped )
{
	COverlappedCommSocket* pCommSocket;
	pCommSocket = (COverlappedCommSocket*)lpNetworkKey;


	// Send �̺�Ʈ�� �����Ѵ�.
	m_pNetworkHandler->OnSend( lpNetworkKey );
}


/**************************************************************************************
	Protected Implementations
*************************************************************************************** /

// IIocp Implementation : ��ϵ� IO �ڵ��� �Ϸ� �̺�Ʈ�� �ޱ� ���� �������̽�
void CTcpAsyncSelect::OnGetQueuedCompletionStatus( BOOL bRet, DWORD dwNumberOfBytesTransferred, ULONG_PTR lpNetworkKey, LPOVERLAPPED lpOverlapped )
{
	COverlappedSocket* pOverlappedSocket = (COverlappedSocket*)lpNetworkKey;

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
		// ???????????????????????????????????????
		break;
	}
}
*/