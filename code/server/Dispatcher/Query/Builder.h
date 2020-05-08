#pragma once

#include "../IQuery.h"

namespace Dispatcher {		namespace Query {

class Builder //���ݿ����
:	public					IQuery::IBuilder
{
public:
							Builder();
	virtual					~Builder();
	//ע��build�ĵ���˳�򣬵�һ��Build���½���һ��Query���Ժ�Ϊ�ڴ˻��������.
	//��������IRequester�е�Regist��Dispatcher::IHandle(��������CSessionGate��������)����
	virtual void			Build( IQuery * i_pQuery, CSTRING & i_strSP );
	virtual void			Build( IQuery * i_pQuery, CSTRING & i_strParam, CHAR i_cData, CBOOL i_bInput );
	virtual void			BuildTiny( IQuery * i_pQuery, CSTRING & i_strParam, BYTE i_byData, CBOOL i_bInput );
	virtual void			Build( IQuery * i_pQuery, CSTRING & i_strParam, SHORT i_nData, CBOOL i_bInput );
	virtual void			Build( IQuery * i_pQuery, CSTRING & i_strParam, INT i_nData, CBOOL i_bInput );
	virtual void			Build( IQuery * i_pQuery, CSTRING & i_strParam, __int64 i_nData, CBOOL i_bInput );
	virtual void			Build( IQuery * i_pQuery, CSTRING & i_strParam, CSTRING & i_strData, CBOOL i_bInput, INT i_nDataLen );
	virtual void			BuildLowStr( IQuery * i_pQuery, CSTRING & i_strParam, const CHAR * i_strData, CBOOL i_bInput, INT i_nDataLen );
};

} /* Query */				} /* Dispatcher */