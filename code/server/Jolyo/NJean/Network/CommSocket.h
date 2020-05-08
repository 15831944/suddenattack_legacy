/*****************************************************************************************************************
	CCommSocket : CSocket

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

#include "Socket.h"
#include "..\..\DataStructure\MemoryPoolSafty.h"

namespace Network
{
	namespace TcpAsyncSelect
	{
		class CCommSocket :
			public CSocket
		{
			// ����/�Ҹ���
		public:
			CCommSocket(void);
			~CCommSocket(void);

			// Attributes
		private:
			// Recv/Send�� ���ؼ� iocp�� ����� overlapped object
			struct TagBuffer
			{
				char*		pBuffer;
				int			nBuffSize;
			};

			WSABUF					m_SendBuffer;
			WSABUF					m_RecvBuffer;
			DWORD					m_dwSentBytes;

			BOOL					m_bCreateBuffer;	// ������ ���� ����

			BOOL					m_bFirstRecv;		// ù��° Recv���������� ����
			// accept/connect�� Recv ������ �ʱ� ������ ���� CS �ذ��� ���� ���

			HANDLE					m_hInitEvent;		// accept/connect�� Recv ������ �ʱ� ������ ���� CS �ذ��� ���� ���
			HANDLE					m_hCloseEvent;		// Recvd�� Close ������ �ʱ� ������ ���� CS �ذ��� ���� ���

			CRITICAL_SECTION		m_csSend;			// Send ����ȭ�� ���� ��ü

			Common::DataStructure::CMemoryPoolSafty<CCommSocket>*
				m_pCommSocketPool;						// �ڽ��� ���� Pool�� ������

			DWORD					m_dwRTT;			// RTT Time
			DWORD					m_dwRttStart;
			BOOL					m_bRTT;				// RTT üũ�� �� �ִ��� ����

		public:
			CSocket*				m_pParentSocket;

		public:

			// ���۸� �����Ѵ�.
			BOOL	Create( Common::DataStructure::CMemoryPoolSafty<CCommSocket>* pCommSocketPool, int nRecvBuffSize, int nSendBuffSize );

			// ���۸� �����Ѵ�.
			void	Destroy();

			// �̸� �Ҵ�� ���۸� assign�Ѵ�.
//				void	AssignBuffer( char* pRecvBuff, int nRecvBuffSize, char* pSendBuff, int nSendBuffSize );

			// Connect�� �����Ѵ�.
			BOOL	Connect( const sockaddr* addr, int namelen );

			BOOL	CreateSocket();

			// IOCP�� Receive �̺�Ʈ�� ��Ͻ�Ų��.
			INT		Receive();

			// IOCP�� Send �̺�Ʈ�� ��Ͻ�Ų��.
			BOOL	Send( const char* pData, int nDataLen );

			DWORD	GetSentBytes()
			{
				return m_dwSentBytes;
			}


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
				::ZeroMemory( &addr, nAddrLen );
				::getsockname( m_sSocket, (sockaddr*) &addr, &nAddrLen );

				return ::inet_ntoa( addr.sin_addr );
			}

			LPCTSTR	GetPeerAddr()
			{
				sockaddr_in	addr;
				int			nAddrLen = 16;
				::ZeroMemory( &addr, nAddrLen );
				::getpeername( m_sSocket, (sockaddr*) &addr, &nAddrLen );

				return ::inet_ntoa( addr.sin_addr );
			}

			/*
			// Recv�� Overlapped Object������ ��ȯ�Ѵ�.
			BOOL	IsRecvOverlappedObject( LPOVERLAPPED lpOv )
			{
				return lpOv == &m_ovRecvChannel;
			}
			*/

			// Recv Buffer�� ��ȯ�Ѵ�.
			char*	GetRecvBuffer()
			{
				return m_RecvBuffer.buf;
			}

			// Send Buffer�� ��ȯ�Ѵ�.
			char*	GetSendBuffer()
			{
				return m_SendBuffer.buf;
			}
			
			Common::DataStructure::CMemoryPoolSafty<CCommSocket>*
					GetCommSocketPool()
			{
				return m_pCommSocketPool;
			}

			// Socket�� Close�Ѵ�.
			void	CloseSocket()
			{
				if( ::closesocket( m_sSocket ) == SOCKET_ERROR )
				{
//						printf( "Close Error : %d\n", WSAGetLastError() );
				}
			}

			// Socket�� ShutDown�Ѵ�.
			void	ShutDown()
			{
				if( ::shutdown( m_sSocket, SD_BOTH ) == SOCKET_ERROR )
				{
//						printf( "Shutdown Error : %d\n", WSAGetLastError() );
				}
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
				m_dwRTT = -1;
				m_bRTT	= TRUE;
			}

			BOOL	CanStartRTT()
			{
				return m_bRTT;
			}

			void	SetRttStartTime()
			{
				m_bRTT			= FALSE;
				m_dwRttStart	= ::GetTickCount();
			}

			void	SetRttEndTime()
			{
				DWORD dwRttEnd = ::GetTickCount();
				if( dwRttEnd >= m_dwRttStart &&
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
//				virtual IOType GetIOType( LPOVERLAPPED lpOv );
		};// class CCommSocket

	}// namespace TcpIOCP
}// namespace Network
