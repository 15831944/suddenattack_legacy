/*****************************************************************************************************************
*****************************************************************************************************************/


#include "..\..\Common\Log.h"
using namespace GamehiSolution::Common::Debug;

#include "MemoryPool.h"

#pragma once

#include <Windows.h>

namespace GamehiSolution
{
	namespace Common
	{
		namespace DataStructure
		{
			template<class Type>
			class CList
			{
			public:
				CList(VOID)
					:	m_nNumOfElements	( 0		)
					,	m_pHead				( NULL	)
					,	m_pRear				( NULL	)
				{
				}
				virtual ~CList(VOID){}
			private:
				// ��ü�� �ֹ��� Linked List
				struct TagElement
				{
					Type		Element;
					TagElement*	pNext;
					TagElement*	pBefore;
				};

				TagElement*		m_pHead;
				TagElement*		m_pRear;

				INT				m_nNumOfElements;
				CMemoryPool<TagElement>
								m_ElementPool;

			public:
				BOOL		Create	( INT nListSize )
				{
					m_nNumOfElements	= 0;
					m_pHead				= NULL;
					m_pRear				= NULL;
					return m_ElementPool.Create( nListSize );
				}
				VOID		Destroy	()
				{
					m_ElementPool.Destroy();
				}

				VOID		Add		( Type element )
				{
					TagElement* pElement = m_ElementPool.Alloc();
					pElement->pNext = pElement->pBefore = NULL;
					pElement->Element = element;

					if( m_nNumOfElements == 0 )
					{
						m_pRear = m_pHead = pElement;
					}
					else
					{
						pElement->pNext		= m_pHead;
						m_pHead->pBefore	= pElement;

						m_pHead				= pElement;
					}

					m_nNumOfElements++;
				}

				// ���� ���� Dequeue�ǵ��� Add�Ѵ�.
				VOID		AddFirst	( Type element )
				{
					TagElement* pElement = m_ElementPool.Alloc();
					pElement->pNext = pElement->pBefore = NULL;
					pElement->Element = element;

					if( m_nNumOfElements == 0 )
					{
						m_pRear = m_pHead = pElement;
					}
					else
					{
						m_pRear->pNext		= pElement;
						pElement->pBefore	= m_pRear;
						m_pRear				= pElement;
					}

					m_nNumOfElements++;
				}

				Type		Dequeue	()
				{
					Type	type;
					TagElement*	pElement;
					pElement = m_pRear;

					if( pElement == NULL )
						return NULL;

					type	= pElement->Element;
					m_pRear	= pElement->pBefore;
					if( m_pRear	== NULL )
						m_pHead	= NULL;
					m_ElementPool.Free( pElement );

					m_nNumOfElements--;

					return type;
				}

				Type		Pop		()
				{
					Type	type;
					TagElement*	pElement;
					pElement = m_pHead;

					if( m_pHead == NULL )
						return NULL;

					type	= pElement->Element;
					m_pHead	= pElement->pNext;
					if( m_pHead	== NULL )
						m_pRear	= NULL;

					m_ElementPool.Free( pElement );

					m_nNumOfElements--;

					return type;
				}

				// Element�� �������� �ʰ�, ������ ��ȯ�Ѵ�.
				Type		GetHeadElement()
				{
					if( m_pHead == NULL )
						return NULL;

					TagElement*	pElement;
					pElement = m_pHead;
					return pElement->Element;
				}
				// Element�� �������� �ʰ�, ������ ��ȯ�Ѵ�.
				Type		GetRearElement()
				{
					if( m_pRear == NULL )
						return NULL;

					TagElement*	pElement;
					pElement = m_pRear;
					return pElement->Element;
				}

				BOOL		IsEmpty()
				{
					return m_nNumOfElements == 0;
				}

				INT			GetElementsCount()
				{
					return m_nNumOfElements;
				}
			};// class CList
		}// namespace DataStructure

	}// namespace Common

}// namespace GamehiSolution