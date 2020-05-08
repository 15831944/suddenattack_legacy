/*****************************************************************************************************************
	CSocket

	�ۼ��� : ������
	�ۼ��� : 2004�� 2�� 25~27��

	��  �� :
		IOCP�� ��ϵǴ� Overlapped Socket Class
		Accept, Recv, Send ���� �̺�Ʈ�� �ޱ� ���� ���Ǵ� Class�� base Class
	��  �� :
		1. IOCP�κ��� �Ϸ� �̺�Ʈ�� ������, CSocket���� Ref�� �޴´�.
		2. CSocket�� vitual function�� GetIOType�� ���ؼ� ���� ��ü�� Ÿ���� �޴´�.
		   ���� �� Ŭ������ ��ӹ��� Ŭ������ �ݵ�� GetIOType�� �����ؾ� �Ѵ�.
*****************************************************************************************************************/


#pragma once

#include <Winsock2.h>
#include <Windows.h>

namespace GamehiSolution
{
	namespace Network
	{
		namespace TcpAsyncSelect
		{
			//////////////////////////////////////////////////////////////////////////////////////////////////////////
			// OverlappedSocket�� BaseClass
			class CSocket
			{
			public:
				CSocket(void){}
				~CSocket(void){}

			public:
				enum ErrorCode
				{
					BufferCreateFail		= 1,	// ���� ���� ����
					SocketCreateFail		= 2,	// ���� ���� ����
					SetSockOptFail			= 3,	// ���� �ɼ� ���� ����
					AcceptExFail			= 4,	// AcceptEx ��� ����
					AcceptPoolCreateFail	= 5		// Accept Pool ���� ����
				};

				/*
				enum IOType
				{
					NoneType		= 0,
					AcceptType		= 1,
					AcceptRecvType	= 2,
					RecvType		= 3,
					SendType		= 4
				};
				*/

				// Attributtes
			public:
				SOCKET			m_sSocket;
				CSocket*		m_pParentSocket;

			protected:
				DWORD			m_dwErrCode;			// ������ ���� �ڵ�

				unsigned long		m_lpdwOwnerKey;			// �� ������ ����ϴ� ��ü�� Ű
				unsigned long		m_lpAssemblerKey;		// �� �������κ��� Data Assemble�� �ϴ� ��ü�� Ű

				int				m_nServiceIndex;		// ������ ���� �ε���

				DWORD			m_dwAssemblerHandle;


				// Operations
			public:
				/*
				// IOCP�� ����� IO �ڵ��� ��ȯ�Ѵ�.
				HANDLE	GetIOHandle()
				{
					return (HANDLE) m_sSocket;
				}

				// ������ ������ ��ȯ�Ѵ�.
				DWORD	GetLastError(void)
				{
					return m_dwErrCode;
				}
				*/

				void	SetOwnerKey( unsigned long dwOwnerKey )
				{
					m_lpdwOwnerKey = dwOwnerKey;
				}
				unsigned long	GetOwnerKey()
				{
					return m_lpdwOwnerKey;
				}

				void	SetAssemblerKey( unsigned long lpAssemblerKey, DWORD dwAssemblerHandle )
				{
					m_lpAssemblerKey	= lpAssemblerKey;
					m_dwAssemblerHandle	= dwAssemblerHandle;
				}
				void	GetAssemblerKey( unsigned long& lpAssemblerKey, DWORD& dwAssemblerHandle )
				{
					lpAssemblerKey		= m_lpAssemblerKey;
					dwAssemblerHandle	= m_dwAssemblerHandle;
				}

				int		GetServiceIndex()
				{
					return m_nServiceIndex;
				}
				void	SetServiceIndex( int nServiceIndex )
				{
					m_nServiceIndex = nServiceIndex;
				}

				// Overroadable
			public:
				/*
				// IOType�� ��ȯ�Ѵ�.
				virtual IOType GetIOType( LPOVERLAPPED lpOv )
				{
					return IOType::NoneType;
				}

				virtual void	Init()
				{
					m_pParentOvSocket = NULL;
				}
				*/
			};// class CSocket

		}// namespace TcpIOCP
	}// namespace Network
}// namespace GamehiSolution