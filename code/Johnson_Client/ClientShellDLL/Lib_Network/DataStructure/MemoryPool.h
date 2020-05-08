/*****************************************************************************************************************
	CMemoryPool

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

namespace GamehiSolution
{
	namespace Common
	{
		namespace DataStructure
		{
			template<class Type>
			class CMemoryPool
			{
			public:
				CMemoryPool(void)
				:	m_pElementsSetList	( NULL	)
				{
				}
				virtual ~CMemoryPool(void){}
			private:
				// ��ü�� �ֹ��� Linked List
				struct TagElement
				{
					Type		Element;
					TagElement*	pNext;
					TagElement*	pBefore;
					BOOL		bAlloc;
				};

				struct TagElementsSet
				{
					TagElement*		aElementsSet;
					TagElementsSet*	pNext;
				};

				// ��ü�� ��� ��ü �迭
//				TagElement*	m_aElementsSet;
				TagElementsSet*
							m_pElementsSetList;

				int			m_nNumOfAlloc;

				// ������� �ʴ� overlapped list
				TagElement*	m_pUnusedElementsList;

				int			m_nNumOfElements;
				int			m_nNumOfElementsSet;

			public:
				// ���ҽ����� �����Ѵ�.
				virtual BOOL	Create( UINT nNumOfElements )
				{
					//
					// TagElement ��ü �迭�� �����Ѵ�.
					//
					m_nNumOfElements	= nNumOfElements;
					m_nNumOfElementsSet	= 1;

					m_pElementsSetList					= new TagElementsSet;
					m_pElementsSetList->pNext			= NULL;
					m_pElementsSetList->aElementsSet	= new TagElement[m_nNumOfElements];

					//
					// ��� ��ü�� �����Ѵ�.
					//
					for( int i = 0; i < m_nNumOfElements; i++ )
					{
						if( i > 0 )
						{
							m_pElementsSetList->aElementsSet[i].pBefore	= &m_pElementsSetList->aElementsSet[i-1];
							m_pElementsSetList->aElementsSet[i-1].pNext	= &m_pElementsSetList->aElementsSet[i];
						}
						m_pElementsSetList->aElementsSet[i].bAlloc		= FALSE;
					}
					m_pElementsSetList->aElementsSet[0].pBefore					= NULL;
					m_pElementsSetList->aElementsSet[m_nNumOfElements-1].pNext	= NULL;

					m_pUnusedElementsList	= m_pElementsSetList->aElementsSet;
					m_nNumOfAlloc			= 0;
					return TRUE;

					/*

					m_aElementsSet = new TagElement[nNumOfElements];
					if( m_aElementsSet == NULL )
						return FALSE;

					//
					// ��� ��ü�� �����Ѵ�.
					//
					for( int i = 0; i < m_nNumOfElements; i++ )
					{
						if( i > 0 )
						{
							m_aElementsSet[i].pBefore	= &m_aElementsSet[i-1];
							m_aElementsSet[i-1].pNext	= &m_aElementsSet[i];
						}
						m_aElementsSet[i].bAlloc		= FALSE;
					}
					m_aElementsSet[0].pBefore = NULL;
					m_aElementsSet[m_nNumOfElements-1].pNext = NULL;

					m_pUnusedElementsList	= m_aElementsSet;
					m_nNumOfAlloc			= 0;
					return TRUE;
					*/
				}
				// ���ҽ����� �����Ѵ�.
				virtual void	Destroy()
				{
					while( m_pElementsSetList )
					{
						if( m_pElementsSetList->aElementsSet )
						{
							delete[] m_pElementsSetList->aElementsSet;
							m_pElementsSetList->aElementsSet = NULL;
						}
						TagElementsSet* pOld = m_pElementsSetList;
						m_pElementsSetList = m_pElementsSetList->pNext;

						delete pOld;
					}
					/*
					if( m_aElementsSet )
					{
						delete[] m_aElementsSet;
						m_aElementsSet = NULL;
					}
					*/
				}

				// Memory Pool���� ��ü�� �Ҵ� �޴´�.
				virtual Type*	Alloc()
				{
					if( m_pUnusedElementsList == NULL )
					{
						TagElementsSet* pSet	= new TagElementsSet;
						pSet->pNext				= NULL;
						pSet->aElementsSet		= new TagElement[m_nNumOfElements];

						//
						// ��� ��ü�� �����Ѵ�.
						//
						for( int i = 0; i < m_nNumOfElements; i++ )
						{
							if( i > 0 )
							{
								pSet->aElementsSet[i].pBefore	= &pSet->aElementsSet[i-1];
								pSet->aElementsSet[i-1].pNext	= &pSet->aElementsSet[i];
							}
							pSet->aElementsSet[i].bAlloc		= FALSE;
						}
						pSet->aElementsSet[0].pBefore					= NULL;
						pSet->aElementsSet[m_nNumOfElements-1].pNext	= NULL;

						m_pElementsSetList = pSet;

						m_pUnusedElementsList	= pSet->aElementsSet;

						m_nNumOfElementsSet++;
					}

					TagElement* pTagElement;

					pTagElement = m_pUnusedElementsList;
					m_pUnusedElementsList = m_pUnusedElementsList->pNext;
					if( m_pUnusedElementsList )
						m_pUnusedElementsList->pBefore = NULL;

//					ASSERT( !pTagElement->bAlloc );
					pTagElement->bAlloc = TRUE;

					m_nNumOfAlloc++;

					return (Type*)&(pTagElement->Element);
				}

				// Memory Pool�� ��ü�� ��ȯ�Ѵ�.
				virtual VOID	Free( Type* pElement )
				{
					TagElement* pTagElement = (TagElement*)pElement;

					if( !pTagElement->bAlloc )
					{
						int a = 3;
					}
//					ASSERT( pTagElement->bAlloc );

					pTagElement->pNext					= m_pUnusedElementsList;
					pTagElement->pBefore				= NULL;
					if( m_pUnusedElementsList )
						m_pUnusedElementsList->pBefore	= pTagElement;
					m_pUnusedElementsList				= pTagElement;

					m_nNumOfAlloc--;

					pTagElement->bAlloc = FALSE;
				}

				int		GetAllocatedSize()
				{
					return m_nNumOfAlloc;
				}

				// Pool Size�� ��ȯ�Ѵ�.
				int		GetPoolSize()
				{
					return m_nNumOfElements * m_nNumOfElementsSet;
				}

			};// class CMemoryPool
		}// namespace DataStructure

	}// namespace Common

}// namespace GamehiSolution