#pragma once

#include "../IChannel.h"
#include "../../System/Memory/Segment.h"
#include "../../System/Collections/Tree.h"

namespace GameServer {		namespace Channel {

class Provider
:	public					IChannel
{
public:
							Provider();
	virtual					~Provider();

	void					Capture( CBYTE i_byIndex, CINT i_nMaxUser );
	void					Release();

	virtual CBYTE			GetIndex();
	virtual IRoom *			GetLobby();

	virtual CINT			GetMaxUser();
	virtual CINT			GetCurrentUser();

	virtual CINT			GetRoomCount();
	virtual IRoom *			Search( CINT i_nRoomIndex );

	virtual void			AddRoom( IRoom * i_pRoom );
	virtual void			RemoveRoom( IRoom * i_pRoom );
	
	virtual IRoom *			GetFirstRoom();
	virtual IRoom *			GetNextRoom();

private:
	typedef					Collections::Coupler<INT, IRoom *>
							COUPLER;
	typedef					Memory::Segment<128, COUPLER>
							POOL;
	typedef					Collections::Tree<COUPLER, Less<COUPLER>, POOL>
							TREE;
	typedef					TREE::Iterator
							ITERATOR;

private:
	IRoom *					m_pLobby;	//��������captureʱ��������ʵҲ��һ��room�����������ڴ����еĽ�ɫ
	INT						m_nMaxUser;	//�����������
	BYTE					m_byIndex;	
	POOL *					m_pPool;	//room ���ϣ�128��
	TREE *					m_pTree;	//room �����γɵĶ�����
	ITERATOR				m_stItr;

};

} /* Channel */				} /* GameServer */