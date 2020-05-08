#pragma once

#include "../Dispatcher/Handler/Provider.h"

#include "../Share/Packet/Builder.h"

#include "./Channel/Factory.h"
#include "./Room/Factory.h"
#include "./Player/Builder.h"
#include "./Player/Container.h"
#include "./NoneResponser/Provider.h"
#include "./NoneSignal/Provider.h"
#include "./Caster/Provider.h"
#include "./Formula/Container.h"

#include "./Table/Factory.h"

#include "./Manager/Builder.h"
#include "./Manager/Process/Container.h"

#include "./Server/Provider.h"

#include "./Log/Factory.h"

namespace GameServer {

class Construct //������ȫ�ֱ����Ĺ���
{
public:
	Construct(); //����֮��ÿ�����Ψһ����ȷ��
	~Construct();

private:
	Server::Provider *					m_pServer;	//��������IP��port��������ѯchannel������������Ϣ

	::Dispatcher::Handler::Provider *	m_pDispatcherHandler;//�Ự����

	::Packet::Builder *					m_pPacketBuilder;

	Channel::Factory *					m_pChannelFactory;
	Room::Factory *						m_pRoomFactory;
	Player::Builder *					m_pPlayerBuilder;
	Player::Container *					m_pPlayerContainer;
	NoneResponser::Provider *			m_pNoneResponser;
	NoneSignal::Provider *				m_pNoneSignal;
	Caster::Provider *					m_pCaster;

	Manager::Builder *					m_pManagerBuilder;
	Manager::Process::Container *		m_pManagerProcessContainer;

	Formula::Container *				m_pFormulaContainer;
	Table::Factory *					m_pTableFactory;


	Log::Factory *						m_pLogFactory;

};

} /* GameServer */