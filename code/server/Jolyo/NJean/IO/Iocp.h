/*****************************************************************************************************************
	CIocp

	�ۼ��� : ������
	�ۼ��� : 2004�� 3�� 8��

	��  �� :
		IOCP�� Ŭ�����ν� ������ �Ѵ�.
	��  �� :
		1. Method Create�� ����Ͽ� IOCP �ڵ��� �����Ѵ�.
		2. AssignIoCompletionPort�� ����Ͽ� IO �ڵ��� Key���� ������ IOCP �ڵ鿡 ��Ͻ�Ų��.
		3. IO �ڵ��� ����� ���� Key���� NULL�� �Ǿ�� �ȵȴ�.
		4. PostQueuedCompletionStatus�� ���ؼ� IOCP ť�� Post ��Ų��.
		5. GetQueuedCompletionStatus�� ���ؼ� IOCP ť�� ������ Get ��Ų��.
		test
*****************************************************************************************************************/

#pragma once

#include <Windows.h>

namespace IO
{
	namespace IOCP
	{
		class CIocp
		{
			// ���� / �Ҹ���
		public:
			CIocp(void);
			~CIocp(void);

			// Attributes
		private:
			DWORD		m_dwWaitMilliSeconds;	// GetQueuedCompletionStatus������ ��� �ð�

		public:
			HANDLE		m_hIOCP;				// IOCP Handle

			// Operations
		public:
			// IOCP�� �����Ѵ�.
			//		dwWaitMilliSeconds : wait �ð�
			virtual BOOL	Create( DWORD dwWaitMilliSeconds = INFINITE );

			// IOCP�� �����Ѵ�.
			virtual void	Destroy();

			// IOCP�� �ڵ��� ����Ѵ�.
			BOOL	AssignIoCompletionPort( HANDLE FileHandle, ULONG_PTR CompletionKey );

			// IOCP ť�� Post�Ѵ�.
			BOOL	PostQueuedCompletionStatus( ULONG_PTR lpCompletionKey, LPOVERLAPPED lpOverlapped );

			// IOCP ť���� Get�Ѵ�.
			BOOL	GetQueuedCompletionStatus( LPDWORD lpNumberOfBytesTransferred, PULONG_PTR lpCompletionKey, LPOVERLAPPED* lpOverlapped );
		};
	}// namespace IOCP

}// namespace IO
