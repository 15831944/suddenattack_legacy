#include "StdAfx.h"

#include "./Closing.h"

namespace GameServer {		namespace Player {			namespace State {
namespace Login {

Closing::Closing()
{
	this->install();
}

Closing::~Closing()
{
	this->uninstall();
}

CBOOL
Closing::OnPacket( Memento * i_pMemento, PACKET * i_pPacket )
{
	// ����
	return TRUE;
}

void
Closing::OnClose( Memento * i_pMemento )
{
	// ����
}

void
Closing::OnResponse( Memento * i_pMemento, IQUERY * i_pQuery )
{
	// ����
}

} /* Login */
} /* State */				} /* Player */				} /* GameServer */