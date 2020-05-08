/*****************************************************************************************************************
	CListenSocket : CSocket

	�ۼ��� : ������
	�ۼ��� : 2004�� 2�� 25~27��

	��  �� :
		Iocp�κ��� Accept �̺�Ʈ�� �ޱ� ���� Listen Socket Ŭ����
		Accept�� ȿ������ ���̱� ���� Accept Pool�� �����ȴ�.
	��  �� :
		1. Create�� ���ؼ� Recv ���ۿ�, Accept Pool Size�� �����Ѵ�.
		   Recv ���۰� ����� ��쿡�� Accept & Recv �̺�Ʈ�� ��ٸ���, �׷��� ������ Accept �̺�Ʈ�� ��ٸ���.
		2. Bind, Listen�� �Ͽ� ���� ���Ͽ� �Ӽ��� �ο��Ѵ�.
		3. RunAcceptPool�� ���ؼ� IOCP��ü�� Accept Pool�� ����Ѵ�.
		4. AcceptEx�� ����Ͽ� Accept �̺�Ʈ�� ���� ������, �Ǵٸ� Accept Pool�� ����Ѵ�.
		5. Accept �̺�Ʈ�� ������ GetSocketOfOverlappedObject�� ���ؼ� Ŭ���̾�Ʈ�� ����� ���� �ڵ��� ��´�.
		6. Destroy�� ���ؼ� ��� ���ҽ��� �����Ѵ�.
*****************************************************************************************************************/


#pragma once

#include <Winsock2.h>
#include <Windows.h>

#include "Socket.h"
#include "CommSocket.h"
#include "..\..\DataStructure\MemoryPoolSafty.h"

namespace Network
{
	namespace TcpAsyncSelect
	{
		class CListenSocket :
			public CSocket
		{
			// ����/�Ҹ���
		public:
			CListenSocket(void);
			~CListenSocket(void);

			// Attributes
		private:
			// accept pool�� ���� iocp�� ����� overlapped object
			/*
			struct TagOverlappedSocket : public WSAOVERLAPPED
			{
				SOCKET		sAcceptSocket;		// Accept Socket : ����� �ڿ� Comm Socket���� Assign�ȴ�.
				char*		Buff;				// AceeptEx�� ���� Buffer�� ũ��
			};
			*/

			char*			m_Buff;				// AcceptEx���� ����� ��� overlapped object���� ���۷ν� �ѹ��� �Ҵ��� �ڿ� �����ͷν� ������ ���� �Ҵ��Ѵ�.
			UINT			m_nBuffSize;		// ��ü ������ ũ��

			UINT			m_nAcceptPoolSize;	// accept pool�� ũ��

//				TagOverlappedSocket*
//					m_aOverlappedSocket;// overlapped socket�� �����迭

			Common::DataStructure::CMemoryPoolSafty<CCommSocket>*
				m_pCommSocketPool;

			// Operations
		public:
			// ���ҽ����� �����Ѵ�.
			BOOL	Create( Common::DataStructure::CMemoryPoolSafty<CCommSocket>* pCommSocketPool, UINT nRecvBuffSize = 0, UINT nAcceptPoolSize = 100 );

			// ���ҽ����� �����Ѵ�.
			void	Destroy();

			// ���ε带 �Ѵ�.
			BOOL	Bind( const sockaddr* addr, int namelen );
			BOOL	Bind( int nPort, u_long hostAddr = INADDR_ANY, short family = AF_INET );

			// ������ �Ѵ�.
			BOOL	Listen( int nBacklog = 5 );

			// Accept Pool�� ����Ѵ�.
			BOOL	RunAcceptPool();

			// overlappedSocket ��ü�� �ٽ� AccextEx�Ѵ�.
			BOOL	AcceptEx( LPOVERLAPPED ovlappedSocket );

			// Accept�̺�Ʈ�� ���� lpOv�� �ش��ϴ� SOCKET�� ��ȯ�Ѵ�.
			/*
			SOCKET	GetSocketOfOverlappedObject( LPOVERLAPPED lpOv )
			{
				TagOverlappedSocket* pOvSocket = (TagOverlappedSocket*)lpOv;
				return pOvSocket->sAcceptSocket;
			}
			*/

			Common::DataStructure::CMemoryPoolSafty<CCommSocket>*
					GetCommSocketPool()
			{
				return m_pCommSocketPool;
			}

		private:
			// iocp�� ������ �����Ѵ�.
			SOCKET	CreateSocket( DWORD& nOutErrorcode );

			// Implementations
		public:
			// IO Type�� ��ȯ�Ѵ�.
//				virtual IOType GetIOType( LPOVERLAPPED lpOv );
		};// class COverlappedServerSocket

	}// namespace TcpIOCP
}// namespace Network
