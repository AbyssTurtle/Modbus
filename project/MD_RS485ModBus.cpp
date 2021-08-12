#include "MD_RS485ModBus.h"
#include "MI_Impl.h"
#include <QDebug>
#include <QMutexLocker>
using namespace Modbus;

MD_RS485ModBus::MD_RS485ModBus(QObject* parent)
	:QObject(parent),
	m_impl(new MI_Impl())
{
	
	QObject::connect(m_impl.get(),&MI_Impl::connectStateChanged,this,&MD_RS485ModBus::connectStateChanged);
	QObject::connect(m_impl.get(),&MI_Impl::errorOccurred,this,&MD_RS485ModBus::errorOccurred);
}


void MD_RS485ModBus::openDevice(const MD_ModbusSerialParameters& _param)
{
	m_impl->openDevice(_param);
}

Modbus::MD_RS485ModBus::~MD_RS485ModBus()
{
	closeDevice();
}

void Modbus::MD_RS485ModBus::closeDevice()
{
	return m_impl->closeDevice();
}

QModbusDevice::State Modbus::MD_RS485ModBus::deviceConnectState()
{
	return m_impl->deviceConnectState();
}

MC_FutureWatch<bool>*  Modbus::MD_RS485ModBus::enableDrivenMachine(int _index, bool _enabled)
{
	return m_impl->enableDrivenMachine(_index,_enabled);
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::stopDrivenMachine(int _index, bool _isValid)
{
	return m_impl->stopDrivenMachine(_index, _isValid);
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::goHome(int _index, bool _isValid)
{
	return m_impl->goHome(_index, _isValid);
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::clearAlarm(int _index, bool _isValid)
{
	return m_impl->clearAlarm(_index, _isValid);
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::setMoveParam(int _index, const std::map<MP_MoveParam::MP_ParamType, int>& _param)
{
	return m_impl->setMoveParam(_index, _param);
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::startRunPositionModel(int _index, bool _isAbsolutePosition)
{
	return m_impl->startRunPositionModel(_index,_isAbsolutePosition);
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::startRunPosition(int _index, bool _isValid)
{
	return m_impl->startRunPosition(_index, _isValid);
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::runDrivenPositiveRotary(int _index, bool _isValid)
{
	return m_impl->runDrivenPositiveRotary(_index, _isValid);
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::runDrivenNegativeRotary(int _index, bool _isValid)
{
	return m_impl->runDrivenNegativeRotary(_index, _isValid);
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::setDrivenRotaryParam(int _index, const std::map<MP_DrivenRotaryParam::MP_DrivenRotaryParamType, int>& _param)
{
	return m_impl->setDrivenRotaryParam(_index, _param);
}

MC_FutureWatch<int>* Modbus::MD_RS485ModBus::readCurrentPosition(int _index)
{
	return m_impl->readCurrentPositon(_index);
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::readIsInPlace(int _index)
{
	return m_impl->readIsInPlace(_index);
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::readIsGoHomeFinish(int _index)
{
	return m_impl->readIsGoHomeFinish(_index);
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::readIsDrivenMachineEnabled(int _index)
{
	return m_impl->readIsDrivenMachineEnabled(_index);
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::readIsAlarm(int _index)
{
	return m_impl->readIsAlarm(_index);
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::readIsDrivenMachineRunnging(int _index)
{
	return m_impl->readIsDrivenMachineRunnging(_index);
}

MC_FutureWatch<int>* Modbus::MD_RS485ModBus::readMoveParam(int _index, MP_MoveParam::MP_ParamType _type)
{
	return m_impl->readMoveParam(_index,_type);
}

MC_FutureWatch<std::pair<QModbusPdu::FunctionCode, QByteArray>>* Modbus::MD_RS485ModBus::sendMessage(int _index, QModbusPdu::FunctionCode _functionCode, const QByteArray &_payload)
{
	return m_impl->sendMessage(_index, _functionCode, _payload);
}

QString Modbus::MD_RS485ModBus::getConnectStateString(QModbusDevice::State _state)
{
	QString connectStateString;
	switch(_state)
	{
		case QModbusDevice::UnconnectedState:
			connectStateString = u8"未连接";
			break;
		case QModbusDevice::ConnectingState:
			connectStateString = u8"正在连接";
			break;
		case QModbusDevice::ConnectedState:
			connectStateString = u8"已连接";
			break;
		case QModbusDevice::ClosingState:
			connectStateString = u8"正在关闭";
			break;
		default:
			break;
	}
	return connectStateString;
}

void Modbus::MD_RS485ModBus::setProtocolType(int _index, MP_ProtocolType _type)
{
	m_impl->setProtocolType(_index, _type);
}

Modbus::MP_ProtocolType Modbus::MD_RS485ModBus::getProtocolType(int _index)
{
	return m_impl->getProtocolType(_index);
}

MC_FutureWatch<int>* Modbus::MD_RS485ModBus::readErrorCode(int _index)
{
	return m_impl->readErrorCode(_index);
}

MC_FutureWatch<std::vector<int>>* Modbus::MD_RS485ModBus::readErrorCodeRecord(int _index)
{
	return m_impl->readErrorCodeRecord(_index);
}
