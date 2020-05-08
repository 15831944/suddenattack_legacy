/*****************************************************************************************************************
*****************************************************************************************************************/

#pragma once

#include <Windows.h>

namespace GamehiSolution
{
	namespace Session
	{
		namespace Gate
		{
			class CHttpResponser
			{
				// ���� / �Ҹ���
			public:
				CHttpResponser();
				~CHttpResponser(void);

				// Attributes
			private:
				char*	m_aBuffer;
				int		m_nMaxSize;		// ������ ũ��
				int		m_nCurrSize;	// ���۸��� ũ��

				int		m_nType;		// Responser�� Ÿ��

				int		m_nResult;
				DWORD	m_dwError;

				// Operations
			public:
				virtual BOOL	Create( int nMaxSize );
				virtual void	Destroy();

				virtual void	Reset()
				{
					m_nCurrSize	= 0;
					m_dwError	= 0;
				}

				virtual void	AddData( const char* pData, int nSize );

				virtual void	SetResult( int nResult )
				{
					m_nResult = nResult;
				}
				virtual int		GetResult()
				{
					return m_nResult;
				}

				virtual void	SetError( DWORD dwError )
				{
					m_dwError = dwError;
				}
				virtual DWORD	GetError()
				{
					return m_dwError;
				}

				char*	GetData()
				{
					return m_aBuffer;
				}
				int		GetDataSize()
				{
					return m_nCurrSize;
				}

				int		GetType()
				{
					return m_nType;
				}
				void	SetType( int nType )
				{
					m_nType = nType;
				}
			};// CHttpResponser
		}// namespace Gate
	}// namespace Session
}// namespace GamehiSolution