#pragma once
#include <Windows.h>

#include "..\..\DataStructure\InnerManager.h"

namespace GamehiSolution
{
	namespace Session
	{
		namespace Gate
		{
			template<class Type>
			class CAliveManager
			{
			public:
				CAliveManager		(void)
					:	m_paAliveList	( NULL	)
					,	m_pCloseWaitList( NULL	)
				{}
				~CAliveManager(void){}

			private:
				int		m_nTimeoutTickCount;

				Common::DataStructure::CInnerManager<Type>*
					m_pCloseWaitList;
				Common::DataStructure::CInnerManager<Type>**
					m_paAliveList;

			public:
				BOOL	Create( int nTimeoutTickCount )
				{
					m_paAliveList		= new Common::DataStructure::CInnerManager<Type>* [nTimeoutTickCount];
					if( m_paAliveList == NULL )
						return FALSE;

					for( int i = 0; i < nTimeoutTickCount; i++ )
					{
						m_paAliveList[i] = new Common::DataStructure::CInnerManager<Type>;
						m_paAliveList[i]->Init();
					}

					m_pCloseWaitList = new Common::DataStructure::CInnerManager<Type>;

					m_nTimeoutTickCount	= nTimeoutTickCount;
					return TRUE;
				}

				void	Destroy()
				{
					if( m_aAliveList )
					{
						for( int i = 0; i < m_nTimeoutTickCount; i++ )
						{
							if( m_paAliveList[i] )
							{
								m_paAliveList[i]->Destroy();
								delete m_paAliveList[i];
							}
							m_paAliveList[i] = NULL;
						}
						delete[] m_paAliveList;
					}
					m_paAliveList = NULL:

					if( m_pCloseWaitList )
						delete m_pCloseWaitList;
					m_pCloseWaitList = NULL;
				}

				// Ÿ�� �ƿ��� ����� �� �ְ� ����Ʈ�� �߰��Ѵ�.
				BOOL	Add		( Common::DataStructure::CInnerObject<Type>* pInnerObject )
				{
					m_paAliveList[0]->Add( pInnerObject );
					return TRUE;
				}

				// Ÿ�� �ƿ� ����Ʈ���� �����Ѵ�.
				BOOL	Remove	( Common::DataStructure::CInnerObject<Type>* pInnerObject )
				{
					BOOL bRemove = FALSE;
					for( int i = 0; i < m_nTimeoutTickCount; i++ )
					{
						bRemove = m_paAliveList[i]->Remove( pInnerObject );
						if( bRemove == TRUE )
							break;
					}

					return bRemove;
				}

				void	RepositeOnAliveList( Common::DataStructure::CInnerObject<Type>* pInnerObject )
				{
					BOOL bRemove = FALSE;
					for( int i = 0; i < m_nTimeoutTickCount; i++ )
					{
						bRemove = m_paAliveList[i]->Remove( pInnerObject );
						if( bRemove == TRUE )
							break;
					}
					if( bRemove )
						m_paAliveList[0]->Add( pInnerObject );
				}

				// Ÿ�Ӿƿ� �߻��� Close ó���� ����Ʈ�� �����Ѵ�.
				void	DoTimeoutProcess()
				{
					Common::DataStructure::CInnerManager<Type>*
						pManager;
					pManager = m_pCloseWaitList;
					for( int i = m_nTimeoutTickCount - 1; i >= 0; i-- )
					{
						if( i == m_nTimeoutTickCount - 1 )
							m_pCloseWaitList = m_paAliveList[i];
						else
							m_paAliveList[i+1] = m_paAliveList[i];
					}
					m_paAliveList[0] = pManager;
				}

				BOOL	PopTimeoutObject( Type* &pOutElement )
				{
					return m_pCloseWaitList->Pop( pOutElement );
				}
			};// class CAliveManager
		}// namespace Gate
	}// namespace Session
}// namespace GamehiSolution