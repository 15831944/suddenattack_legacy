#pragma once

#include "User.h"
#include "..\DataStructure\InnerManager.h"

namespace CasualGame
{
	class CRoom
	{
	public:
		CRoom();
		virtual ~CRoom();

	public:
		enum	InnerIndexType		// �� �ȿ����� Index��ȣ Ÿ��
		{
			User,		// ����ڰ� �����
			Empty,		// �������
			Close,		// ����� �� ����
		};

	private:
		enum
		{
			MaxUser	= 16,
		};

		Common::DataStructure::CInnerObject<CRoom>
			m_InnerObjectForGame;
		Common::DataStructure::CInnerObject<CRoom>
			m_InnerObjectForChannel;

		CPocket*	m_pPocket;				// ������� ���Ӻ� Ưȭ�� Ŭ����

		Common::DataStructure::CInnerManager<CUser>
			m_InnerManagerUser;

		INT			m_nMaxUser;				// �濡 �ִ�� �� �� �ִ� ����� �� : �� �����ڰ� �����Ѵ�.

		INT			m_nRoomIndex;			// ���� ���� �ε���
		INT			m_nChannelIndex;		// ���� ���� ä�� �ε���

		CUser*		m_pCaptine;				// ����

		string		m_strRoomName;
		
		string		m_strPassword;			// �� ��й�ȣ
		BOOL		m_bPassword;			// ��й�ȣ ����

		DWORD		m_dwRTT;

		char		m_caOption[CCasualCommon::CasualStringLength::RoomOption];
											// 8����Ʈ �ɼ�
		
		InnerIndexType	m_aUserIndex[ MaxUser ];
//			INT				m_nUsingIndex;

	public:
		VOID		SetRTT( DWORD dwRTT )
		{
			m_dwRTT = dwRTT;
		}
		DWORD		GetRTT()
		{
			return m_dwRTT;
		}
		void		Init	( INT nChannelIndex );

		void		Open	( INT nMaxUser, LPCTSTR strRoomName );
		void		Open	( INT nMaxUser, LPCTSTR strRoomName, LPCTSTR strPassword );

		// �濡 ������ �Ѵ�.
		BOOL		Join	( CUser* pUser );

		// �濡�� ������. : nIndex(��ȿ����� �ڽ��� ��ȣ)
		BOOL		Leave	( CUser* pUser );


		//
		// Get/Set Methods
		//

		// Inner Index�� �Ҵ�޴´�.
		CHAR		AllocEmptyInnerIndex();

		// Inner Index�� �����Ѵ�.
		VOID		FreeEmptyInnerIndex( INT nIndex );

		// Inner Index�� �����ϰ� �Ѵ�.
		BOOL		SetToCloseInnerIndex( INT nIndex );

		// Inner Index�� ����ϰ� �Ѵ�.
		VOID		ResetToCloseInnerIndex( INT nIndex );

		CPocket*	GetPocket()
		{
			return m_pPocket;
		}
		void		SetPocket( CPocket* pPocket )
		{
			m_pPocket = pPocket;
		}

		INT			GetChannelIndex()
		{
			return m_nChannelIndex;
		}

		// ������ �����´�.
		CUser*		GetCaptine()
		{
			return m_pCaptine;
		}
		VOID		SetCaptine( CUser* pCaptine )
		{
			m_pCaptine = pCaptine;
		}

		INT			GetRoomIndex()
		{
			return m_nRoomIndex;
		}
		void		SetRoomIndex( INT nRoomIndex )
		{
			m_nRoomIndex = nRoomIndex;
		}

		INT			GetMaxUser()
		{
			return m_nMaxUser;
		}
		void		SetMaxUser( INT nMaxUser )
		{
			m_nMaxUser = nMaxUser;
		}

		INT			GetNumOfUsers()
		{
			return m_InnerManagerUser.GetNumOfInnerObject();
		}
		
		string&		GetRoomName()
		{
			return m_strRoomName;
		}
		void		SetRoomName( LPCTSTR strRoomName )
		{
			m_strRoomName = strRoomName;
		}

		BOOL		IsPassword()
		{
			return m_bPassword;
		}
		void		SetPassword( BOOL bPassword, LPCTSTR strPassword )
		{
			m_bPassword = bPassword;
			if( bPassword )
				m_strPassword = strPassword;
		}
		string&		GetPassword()
		{
			return m_strPassword;
		}

		BOOL		IsCorrectPassword( LPCTSTR strPassword )
		{
			return m_strPassword.compare( strPassword ) == 0;
		}

		char		GetOption	( INT nIndex )
		{
			if( nIndex < 0 || nIndex >= CCasualCommon::CasualStringLength::RoomOption )
				return -1;

			return m_caOption[nIndex];
		}
		void		SetOption	( INT nIndex, char cOption )
		{
			if( nIndex < 0 || nIndex >= CCasualCommon::CasualStringLength::RoomOption )
				return;

			m_caOption[nIndex] = cOption;
		}

		const char*	GetOption()
		{
			return m_caOption;
		}
		void		SetOption( const char* caOption )
		{
			::memcpy( m_caOption, caOption, CCasualCommon::CasualStringLength::RoomOption );
		}

		// ���� ����ִ����� �˻��Ѵ�.
		BOOL		IsEmpty()
		{
			return m_InnerManagerUser.IsEmpty();
		}

		Common::DataStructure::CInnerObject<CRoom>*
			GetInnerObjectForGame()
		{
			return &m_InnerObjectForGame;
		}
		Common::DataStructure::CInnerObject<CRoom>*
			GetInnerObjectForChannel()
		{
			return &m_InnerObjectForChannel;
		}

		// ���� ù��° Ŭ���̾�Ʈ ���񼭸� �����´�.
		CUser*	GetFirstUser();

		// ���� ���� Ŭ���̾�Ʈ ���񼭸� �����´�.
		CUser*	GetNextUser();
	};// CRoom

}// namespace CasualGame
