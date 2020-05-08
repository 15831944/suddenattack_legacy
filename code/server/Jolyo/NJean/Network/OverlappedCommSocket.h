/*****************************************************************************************************************
	COverlappedCommSocket : COverlappedSocket

	�ۼ��� : ������
	�ۼ��� : 2004�� 2�� 25~27��

	��  �� :
		Iocp�κ��� Recv/Send �̺�Ʈ�� �ޱ� ���� Communication Socket Ŭ����
	��  �� :
		1. CreateBuffer�� ���ؼ� ���� ���۸� �����ϰų�, AssignBuffer�� ���ؼ� �ܺ� ���۸� �����Ѵ�.
		2. CreateBuffer�� ����� ��쿡�� �ݵ�� ����ÿ� DestroyBuffer�� ���ؼ� ���ҽ��� �����Ѵ�.
		3. SetSocket�� ���� Iocp�� ����� Recv/Send�� ���� �ڵ��� Communication Socket�� �����Ѵ�.
		   �̴� Accept�� ���� ��ȯ�� Ŭ���̾�Ʈ�� ����Ǵ� ������ �����ؾ� �Ѵ�.
		   COverlappedListenSocket�� GetSocketOfOverlappedObject�� ���� ���� Ŭ���̾�Ʈ�� ����� ���� �ڵ��� �����ȴ�.
		4. RecvPost�� ���ؼ� Recv �̺�Ʈ�� IOCP�� ��Ͻ�Ų��.
		5. Send Buffer�� ä�� �ڿ� SendPost�� ���ؼ� Send�� IOCP�� ��Ͻ�Ų��.
		   GetSendBuffer�� ���ؼ� Send Buffer�� �����µڿ�, ���۸� ä���, �� ũ�⸦ SetSendLength�ν� �����Ѵ�.
		6. ���̻� ������� �ʴ� ������ CloseSocket�� ���ؼ� ������ �ݴ´�.
*****************************************************************************************************************/


#pragma once

#include <Winsock2.h>
#include <Windows.h>

#include "OverlappedSocket.h"
#include "..\..\DataStructure\MemoryPoolSafty.h"

namespace Network
{
	namespace TcpIOCP
	{
		class COverlappedCommSocket :
			public COverlappedSocket
		{
			// ����/�Ҹ���
		public:
			COverlappedCommSocket(void);
			~COverlappedCommSocket(void);

			// Attributes
		private:
			// Recv/Send�� ���ؼ� iocp�� ����� overlapped object
			struct TagOverlappedChannel : public WSAOVERLAPPED
			{
				WSABUF		wsaBuffer;
				int			nBuffSize;
			};

			TagOverlappedChannel	m_ovRecvChannel;	// Overlapped Recv
			TagOverlappedChannel	m_ovSendChannel;	// Overlapped Send


			BOOL					m_bCreateBuffer;	// ������ ���� ����

			BOOL					m_bFirstRecv;		// ù��° Recv���������� ����
			// accept/connect�� Recv ������ �ʱ� ������ ���� CS �ذ��� ���� ���

			HANDLE					m_hInitEvent;		// accept/connect�� Recv ������ �ʱ� ������ ���� CS �ذ��� ���� ���
			HANDLE					m_hCloseEvent;		// Recvd�� Close ������ �ʱ� ������ ���� CS �ذ��� ���� ���

			Common::DataStructure::CMemoryPoolSafty<COverlappedCommSocket>*
				m_pCommSocketPool;						// �ڽ��� ���� Pool�� ������


			DWORD					m_dwRTT;			// RTT Time
			DWORD					m_dwRttStart;
			BOOL					m_bRTT;				// RTT ���� ����

			CRITICAL_SECTION		m_csConnect;
			BOOL					m_bConnect;			// connect ����


			sockaddr_in				m_addrPeer;


		public:
			int m_nTest;
			int m_nErr;

			WSAOVERLAPPED			m_ovKeepAlive;

			// ���۸� �����Ѵ�.
			BOOL	Create( Common::DataStructure::CMemoryPoolSafty<COverlappedCommSocket>* pCommSocketPool, int nRecvBuffSize, int nSendBuffSize );

			// ���۸� �����Ѵ�.
			void	Destroy();

			VOID	SetConnect()
			{
				m_bConnect = TRUE;
			}
//				BOOL	IsConnect()
//				{
//					return m_bConnect;
//				}

			// �̸� �Ҵ�� ���۸� assign�Ѵ�.
			void	AssignBuffer( char* pRecvBuff, int nRecvBuffSize, char* pSendBuff, int nSendBuffSize );

			// Connect�� �����Ѵ�.
			BOOL	Connect( const sockaddr* addr, int namelen );

			// IOCP�� Receive �̺�Ʈ�� ��Ͻ�Ų��.
			BOOL	RecvPost();

			// IOCP�� Send �̺�Ʈ�� ��Ͻ�Ų��.
			BOOL	SendPost( INT& nOutError );


			// ������ �����Ѵ�.
			void	SetSocket( SOCKET socket )
			{
				m_sSocket = socket;
			}

			// Socket�� ��ȯ�Ѵ�.
			SOCKET	GetSocket()
			{
				return m_sSocket;
			}

			LPCTSTR	GetLocalAddr()
			{
				sockaddr_in	addr;
				int			nAddrLen = 16;
				ZeroMemory( &addr, nAddrLen );
				getsockname( m_sSocket, (sockaddr*) &addr, &nAddrLen );

				return inet_ntoa( addr.sin_addr );
			}

			LPCTSTR	GetPeerAddr()
			{
				/*/
				sockaddr_in	addr;
				int			nAddrLen = 16;
				::ZeroMemory( &addr, nAddrLen );
				::getsockname( m_sSocket, (sockaddr*) &addr, &nAddrLen );

				::ZeroMemory( &addr, nAddrLen );
				::getpeername( m_sSocket, (sockaddr*) &addr, &nAddrLen );

				DWORD dw = addr.sin_addr.s_addr;
				LPCTSTR strResult = ::inet_ntoa( addr.sin_addr );
				return strResult;
				/*/
				LPCTSTR strResult = inet_ntoa( m_addrPeer.sin_addr );
				return strResult;
			}
			void	SetPeerAddr( sockaddr_in * addr )
			{
				m_addrPeer = *addr;
			}

			// Recv�� Overlapped Object������ ��ȯ�Ѵ�.
			BOOL	IsRecvOverlappedObject( LPOVERLAPPED lpOv )
			{
				return lpOv == &m_ovRecvChannel;
			}

			// Recv Buffer�� ��ȯ�Ѵ�.
			char*	GetRecvBuffer()
			{
				return m_ovRecvChannel.wsaBuffer.buf;
			}

			// Send Buffer�� ��ȯ�Ѵ�.
			char*	GetSendBuffer()
			{
				return m_ovSendChannel.wsaBuffer.buf;
			}
			// Send WSABUF�� Length�� �����Ѵ�.
			void	SetSendLength( int nLen )
			{
				m_ovSendChannel.wsaBuffer.len = nLen;
			}

			Common::DataStructure::CMemoryPoolSafty<COverlappedCommSocket>*
					GetCommSocketPool()
			{
				return m_pCommSocketPool;
			}

			// Socket�� Close�Ѵ�.
			void	CloseSocket()
			{
				EnterCriticalSection( &m_csConnect );
				BOOL bConnected = m_bConnect;
				if( m_bConnect )
				{
					m_bConnect = FALSE;
				}
				LeaveCriticalSection( &m_csConnect );

				if( bConnected )
					closesocket( m_sSocket );

				
			}

			// Socket�� ShutDown�Ѵ�.
			void	ShutDown()
			{
				shutdown( m_sSocket, SD_BOTH );
			}

			// Event�� Reset�Ѵ�.
			void	ResetEvent()
			{
//					::ResetEvent( m_hInitEvent );
			}
			// Event�� Set�Ѵ�.
			void	SetEvent()
			{
//					::SetEvent( m_hInitEvent );
			}
			// Event�� Wait�Ѵ�.
			void	WaitEvent()
			{
//					::WaitForSingleObject( m_hInitEvent, INFINITE );
			}

			// Event�� Reset�Ѵ�.
			void	ResetEvent2()
			{
//					::ResetEvent( m_hCloseEvent );
			}
			// Event�� Set�Ѵ�.
			void	SetEvent2()
			{
//					::SetEvent( m_hCloseEvent );
			}
			// Event�� Wait�Ѵ�.
			void	WaitEvent2()
			{
//					::WaitForSingleObject( m_hCloseEvent, INFINITE );
			}

			void	ResetRttTime()
			{
				m_dwRTT = 0;
				m_bRTT	= TRUE;
			}

			BOOL	CanStartRTT()
			{
				return m_bRTT;
			}

			void	SetRttStartTime()
			{
				m_bRTT			= FALSE;
				m_dwRttStart	= GetTickCount();
			}

			void	SetRttEndTime()
			{
				DWORD dwRttEnd = GetTickCount();
				if( dwRttEnd > m_dwRttStart &&
					m_dwRttStart != 0 )
				{
					m_dwRTT	= dwRttEnd - m_dwRttStart;
				}
				m_dwRttStart	= 0;
				m_bRTT			= TRUE;
			}

			DWORD	GetRttTime()
			{
				return m_dwRTT;
			}

			// Implementations
		public:

			// IO Type�� ��ȯ�Ѵ�.
			virtual IOType GetIOType( LPOVERLAPPED lpOv );
		};// class COverlappedCommSocket

	}// namespace TcpIOCP
}// namespace Network
