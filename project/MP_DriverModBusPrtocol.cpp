#include "MP_DriverModBusPrtocol.h"
#include <QDebug>
using namespace ModbusProtocol;
QByteArray MP_DriverModBusProtocol::intToByteArray(int _n, int _base, int _size)
{
	auto data{QByteArray::number(_n,_base)};
	data.trimmed();
	if(data.size() > _size)
	{
		data = data.right(_size);
	}
	else if(data.size() < _size)
	{
		//data.insert(0, _size - data.size(), char('0'));//left pad zero
		data = data.rightJustified(_size, char('0'));
	}
	return data;
}


