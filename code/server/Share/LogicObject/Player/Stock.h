#pragma once

#include "./Type.h"

namespace LogicObject {		namespace Player {

struct Stock //��ɫ�ĳƺ��ͼ���
{
	typedef					::LogicObject::Player::Type
							TYPE;

							Stock();

	void					SetNick( const char * i_strNick );
	const char *			GetNick() const;

	unsigned char			m_byGrade;

private:
	char					m_strNick[ TYPE::s_nUserNick ];
};

} /* Player */				} /* LogicObject */