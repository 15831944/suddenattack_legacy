#pragma once

#include "User.h"
#include "Room.h"
#include "..\DataStructure\InnerManager.h"

namespace CasualGame
{
	class CChannel
	{
	public:
		CChannel(void){}
		virtual ~CChannel(void){}

	private:
		CPocket*	m_pPocket;					// ������� ���Ӻ� Ưȭ�� Ŭ����

		Common::DataStructure::CInnerManager<CUser>
					m_InnerManagerUser;			// ä���� ��� ����� ����Ʈ

		Common::DataStructure::CInnerManager<CRoom>
					m_InnerManagerRoom;			// ä���� ��� �� ����Ʈ

	protected:
		int			m_nMaxUser;					// ä�ο� �ִ�� �� �� �ִ� ����� ��
		int			m_nMaxRoom;					// ä�ο� ������ �� �ִ� �ִ� ���� ����

		CRoom*		m_pWaitRoom;				// ����

		INT			m_nChannelIndex;			// ���� ���� �ε���

		string		m_strChannelName;

	public:
		void		Init	( INT nMaxUser, INT nMaxRoom, const string& strChannelName, INT nChannelIndex );

		CPocket*	GetPocket()
		{
			return m_pPocket;
		}
		void		SetPocket( CPocket* pPocket )
		{
			m_pPocket = pPocket;
		}

		// ä���� �ε����� �����´�.
		INT			GetChannelIndex()
		{
			return m_nChannelIndex;
		}

		const char*	GetChannelName()
		{
			return m_strChannelName.c_str();
		}

		void		SetWaitRoom	( CRoom* pWaitRoom )
		{
			m_pWaitRoom = pWaitRoom;
		}
		CRoom*		GetWaitRoom	()
		{
			return m_pWaitRoom;
		}
		BOOL		IsWaitRoom	( const CRoom* pRoom )
		{
			return m_pWaitRoom == pRoom;
		}

		BOOL		AddRoom		( CRoom* pRoom );
		BOOL		RemoveRoom	( CRoom* pRoom );

		INT			GetMaxRoom()
		{
			return m_nMaxRoom;
		}
		INT			GetNumOfOpenRooms()
		{
			return m_InnerManagerRoom.GetNumOfInnerObject();
		}
		// ä���� ù��° ���� �����´�.
		CRoom*	GetFirstRoom()
		{
			return m_InnerManagerRoom.GetFirstInnerObject();
		}

		// ä���� ���� ���� �����´�.
		CRoom*	GetNextRoom()
		{
			return m_InnerManagerRoom.GetNextInnerObject();
		}

		// ä�ο� ������ �Ѵ�.
		BOOL		Join	( CUser* pUser );

		// ä�ο��� ������.
		BOOL		Leave	( CUser* pUser );

		INT			GetMaxUser()
		{
			return m_nMaxUser;
		}
		INT			GetNumOfUsers()
		{
			return m_InnerManagerUser.GetNumOfInnerObject();
		}

		// ä���� ù��° Ŭ���̾�Ʈ ���񼭸� �����´�.
		CUser*	GetFirstUser()
		{
			return m_InnerManagerUser.GetFirstInnerObject();
		}

		// ä���� ���� Ŭ���̾�Ʈ ���񼭸� �����´�.
		CUser*	GetNextUser()
		{
			return m_InnerManagerUser.GetNextInnerObject();
		}
	};// CRoom

}// namespace CasualGame
