#include "StdAfx.h"
#include ".\ListenSocket.h"

#include <Mswsock.h>

using namespace Network::TcpAsyncSelect;
using namespace	Common::DataStructure;

////////////////////////////////////////////////////////////////////////////////////////
//
//	CListenSocket
//

// ����/�Ҹ���
CListenSocket::CListenSocket(void)
{
//	m_aOverlappedSocket	= NULL;
	m_Buff				= NULL;
//	m_aOverlappedSocket	= NULL;
	m_nAcceptPoolSize	= 0;
//	m_pParentOvSocket	= NULL;
	m_sSocket			= INVALID_SOCKET;
}

CListenSocket::~CListenSocket(void)
{
}



/**************************************************************************************
	Public Operations
***************************************************************************************/

// ���ҽ����� �����Ѵ�.
BOOL CListenSocket::Create( CMemoryPoolSafty<CCommSocket>* pCommSocketPool, UINT nRecvBuffSize/* = 0*/, UINT nAcceptPoolSize/* = 100*/ )
{
	m_pCommSocketPool = pCommSocketPool;

	//
	// ���۸� �����Ѵ� : �⺻������ 16byte�� buffer�� �Ҵ��� �ؾ� �Ѵ�. �ڼ��� ������ MSDN�� AcceptEx�� ����
	//					 nAcceptPoolSize�� ������ŭ �̸� �ѹ��� �Ҵ��� �Ѵ�.
	//
	int nAcceptExBufferSize = 2 * ( sizeof(SOCKADDR_IN) + 16 );
	m_Buff = new char[ ( nRecvBuffSize + nAcceptExBufferSize ) * nAcceptPoolSize ];
	if( NULL == m_Buff )
	{
		m_dwErrCode = ErrorCode::BufferCreateFail;
		return FALSE;
	}
	m_nBuffSize = nRecvBuffSize+16;

	//
	// ������ �����Ѵ�.
	//
	m_sSocket = INVALID_SOCKET;
	m_sSocket	= ::WSASocket( AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED );
	if( INVALID_SOCKET == m_sSocket )
	{
		delete[] m_Buff;
		m_Buff = NULL;

		m_dwErrCode = ErrorCode::SocketCreateFail;
		return FALSE;
	}

	//
	// Accept Pool�� ������ �����Ѵ�.
	//
	/*
	m_nAcceptPoolSize = nAcceptPoolSize;
	if( m_aOverlappedSocket )
	{
		delete[] m_aOverlappedSocket;
		m_aOverlappedSocket = NULL;
	}
	m_aOverlappedSocket = new TagOverlappedSocket[m_nAcceptPoolSize];
	if( NULL == m_aOverlappedSocket )
	{
		delete[] m_Buff;
		m_Buff = NULL;
		::closesocket( m_sSocket );

		m_dwErrCode = ErrorCode::AcceptPoolCreateFail;
		return FALSE;
	}
	for( UINT i = 0; i < m_nAcceptPoolSize; i++ )
	{
		m_aOverlappedSocket[i].Buff = &m_Buff[ ( nRecvBuffSize + nAcceptExBufferSize ) * i ];
	}
	*/


	return TRUE;
}

// ���ҽ����� �����Ѵ�.
void CListenSocket::Destroy()
{
	::closesocket( m_sSocket );

	if( m_Buff )
		delete[] m_Buff;

	/*
	if( m_aOverlappedSocket )
		delete[] m_aOverlappedSocket;

	m_Buff				= NULL;
	m_aOverlappedSocket	= NULL;
	*/
}

// ���ε带 �Ѵ�.
BOOL CListenSocket::Bind( const sockaddr* addr, int namelen )
{
	if( ::bind( m_sSocket, addr, namelen ) )
		return FALSE;

	return TRUE;
}

// ���ε带 �Ѵ�.
BOOL CListenSocket::Bind( int nPort, u_long hostAddr/* = INADDR_ANY*/, short family/* = AF_INET*/ )
{
	SOCKADDR_IN	serverSockAddr;

    serverSockAddr.sin_addr.s_addr	= htonl( hostAddr );
	serverSockAddr.sin_family		= family;
	serverSockAddr.sin_port			= htons( nPort );

	if( ::bind( m_sSocket, (LPSOCKADDR)&serverSockAddr, sizeof(serverSockAddr) ) )
	{
		int a = ::WSAGetLastError();
		return FALSE;
	}

	return TRUE;
}

// ������ �Ѵ�.
BOOL CListenSocket::Listen( int nBacklog/* = 5*/ )
{
	if( ::listen( m_sSocket, nBacklog ) == SOCKET_ERROR )
		return FALSE;
	else
		return TRUE;
}

// Accept Pool�� ����Ѵ�.
BOOL CListenSocket::RunAcceptPool()
{
	// Recv �̺�Ʈ�� ���� ���
	if( m_nBuffSize > 16 )
	{
	}

	// Accept �̺�Ʈ�� ���� ���
	else
	{
		/*
		for( UINT i = 0; i < m_nAcceptPoolSize; i++ )
		{
			//
			// accept�� ���� ����
			//
			m_aOverlappedSocket[i].sAcceptSocket = CreateSocket( m_dwErrCode );
			if( INVALID_SOCKET == m_aOverlappedSocket[i].sAcceptSocket )
				return FALSE;

			//
			// Overlapped �ʱ�ȭ
			//
			::ZeroMemory( &m_aOverlappedSocket[i], sizeof(WSAOVERLAPPED) );
		}

		//
		// AcceptEx
		//
		for( UINT i = 0; i < m_nAcceptPoolSize; i++ )
		{
			DWORD       dwRecvNumBytes = 0;
			BOOL bRet = ::AcceptEx(
				m_sSocket,
				m_aOverlappedSocket[i].sAcceptSocket,
				m_aOverlappedSocket[i].Buff,
				0,
				sizeof(SOCKADDR_IN) + 16,
				sizeof(SOCKADDR_IN) + 16,
				&dwRecvNumBytes,
				&m_aOverlappedSocket[i] );

			if( !bRet ) 
			{
				int ErrCode = ::WSAGetLastError();
				if( ErrCode != WSA_IO_PENDING )
				{
					m_dwErrCode = ErrorCode::AcceptExFail;
					return FALSE;
				}
			}
		}
		*/
	}

	return TRUE;
}

// overlappedSocket ��ü�� �ٽ� AccextEx�Ѵ�.
BOOL CListenSocket::AcceptEx( LPOVERLAPPED ovlappedSocket )
{
	/*
	DWORD dwError;
	TagOverlappedSocket* pOvSocket = (TagOverlappedSocket*)ovlappedSocket;
	pOvSocket->sAcceptSocket = CreateSocket( dwError );
	if( INVALID_SOCKET == pOvSocket->sAcceptSocket )
	{
		m_dwErrCode = dwError;
		return FALSE;
	}

	DWORD       dwRecvNumBytes = 0;
	BOOL bRet = ::AcceptEx(
		m_sSocket,
		pOvSocket->sAcceptSocket,
		pOvSocket->Buff,
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&dwRecvNumBytes,
		pOvSocket );

	if( !bRet ) 
	{
		int ErrCode = ::WSAGetLastError();
		if( ErrCode != WSA_IO_PENDING )
		{
			m_dwErrCode = ErrorCode::AcceptExFail;
			::closesocket( pOvSocket->sAcceptSocket );

			::printf( "Fail AcceptEx : Can't Reg IOCP\n" );
			return FALSE;
		}
	}

	*/
	return TRUE;
}


/**************************************************************************************
	Private Operations
***************************************************************************************/

// iocp�� ������ �����Ѵ�.
inline SOCKET CListenSocket::CreateSocket( DWORD& nOutErrorcode )
{
	SOCKET socket;

	//
	// accept�� ���� ����
	//
	int         nRet;
	int         nZero = 0;
	LINGER      lingerStruct;

	socket = INVALID_SOCKET;
	socket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED); 
	if( INVALID_SOCKET == socket )
	{
		nOutErrorcode = ErrorCode::SocketCreateFail;
		return INVALID_SOCKET;
	}

	// Disable send buffering on the socket.  Setting SO_SNDBUF
	// to 0 causes winsock to stop bufferring sends and perform
	// sends directly from our buffers, thereby save one memory copy.
	nZero = 0;
	nRet = ::setsockopt( socket, SOL_SOCKET, SO_SNDBUF, (char*)&nZero, sizeof(nZero) );
	if( SOCKET_ERROR == nRet )
	{
		nOutErrorcode = ErrorCode::SetSockOptFail;
		return INVALID_SOCKET;
	}

	// Disable receive buffering on the socket.  Setting SO_RCVBUF 
	// to 0 causes winsock to stop bufferring receive and perform
	// receives directly from our buffers, thereby save one memory copy.
	nZero = 0;
	nRet = ::setsockopt( socket, SOL_SOCKET, SO_RCVBUF, (char*)&nZero, sizeof(nZero) );
	if( SOCKET_ERROR == nRet )
	{
		nOutErrorcode = ErrorCode::SetSockOptFail;
		return INVALID_SOCKET;
	}

	lingerStruct.l_onoff = 1;
	lingerStruct.l_linger = 0;
	nRet = ::setsockopt( socket, SOL_SOCKET, SO_LINGER, (char*)&lingerStruct, sizeof(lingerStruct) );
	if (SOCKET_ERROR == nRet) 
	{
		nOutErrorcode = ErrorCode::SetSockOptFail;
		return INVALID_SOCKET;
	}

	return socket;
}





/**************************************************************************************
	Public Implementations
***************************************************************************************/

// IO Type�� ��ȯ�Ѵ�.
/*
COverlappedSocket::IOType CListenSocket::GetIOType( LPOVERLAPPED lpOv )
{
	// Recv �̺�Ʈ�� ���� ���
	if( m_nBuffSize > 16 )
	{
		return COverlappedSocket::AcceptRecvType;
	}

	// Accept �̺�Ʈ�� ���� ���
	else
	{
		return COverlappedSocket::AcceptType;
	}
}

*/