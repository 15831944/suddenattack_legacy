/*****************************************************************************************************************
	CMemoryPoolSafty

	�ۼ��� : ������
	�ۼ��� : 2004�� 3�� 5��

	��  �� :
		1. ����� ��ü�� �Ҵ� �� ��ȯ�� �߻��ϴ� ���, ȿ���� ���̱� ���ؼ� �޸� Ǯ�ν� �����Ѵ�.
		2. ���� �ִ� ��ü���� �̸� �Ҵ��Ͽ�, �ʿ�ÿ� �ش� �޸��� �����͸� ��ȯ���ش�.
		3. ��� Ŭ������ ������ �ǵ��� ���ø����� ���۵Ǿ���.
	��  �� :
		1. �޸� Ǯ�� ����� Ŭ������ ���ø����� �����Ѵ�.
		2. Create�� ���ؼ� �̸� ������ ��ü�� ���� �����Ѵ�.
		3. Alloc�� ���ؼ� �޸� Ǯ���� ��ü�� �Ҵ�޴´�.
		4. Free�� ���ؼ� �޸� Ǯ�� ��ü�� ��ȯ�Ѵ�.
		5. �޸� Ǯ�� ��� ����� ������ Destroy�� ���ؼ� ���ҽ��� �����Ѵ�.
*****************************************************************************************************************/

#pragma once

#include <Windows.h>
#include "MemoryPool.h"

namespace Common
{
	namespace DataStructure
	{
		template<class Type>
		class CMemoryPoolSafty : public CMemoryPool<Type>
		{
		private:
			CRITICAL_SECTION		m_cs;
		public:
			// ���ҽ����� �����Ѵ�.
			virtual BOOL	Create( UINT nNumOfElements )
			{
				::InitializeCriticalSection( &m_cs );

				return CMemoryPool<Type>::Create( nNumOfElements );
			}
			// ���ҽ����� �����Ѵ�.
			virtual void	Destroy()
			{
				CMemoryPool<Type>::Destroy();

				::DeleteCriticalSection( &m_cs );
			}

			// Memory Pool���� ��ü�� �Ҵ� �޴´�.
			virtual Type*	Alloc()
			{
				::EnterCriticalSection( &m_cs );

				Type* pType;
				pType = CMemoryPool<Type>::Alloc();

				::LeaveCriticalSection( &m_cs );

				return pType;
			}

			// Memory Pool�� ��ü�� ��ȯ�Ѵ�.
			virtual VOID	Free( Type* pElement )
			{
				::EnterCriticalSection( &m_cs );

				CMemoryPool<Type>::Free( pElement );

				::LeaveCriticalSection( &m_cs );
			}

		};// class CMemoryPoolSafty
	}// namespace DataStructure

}// namespace Common
