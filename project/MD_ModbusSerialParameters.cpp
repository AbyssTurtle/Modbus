#include "MD_ModbusSerialParameters.h"
#include <QString>
using namespace Modbus;
static const QString s_groupName = u8"modbusSerialParameters";
static const QString s_portNameKeyName = u8"portName";
static const QString s_parityKeyName = u8"parity";
static const QString s_baudKeyName = u8"baud";
static const QString s_dataBitsKeyName = u8"dataBits";
static const QString s_stopBitsKeyName = u8"stopBits";
static const QString s_responseTimeKeyName = u8"responseTime";
static const QString s_numberOfRetries = u8"numberOfRetries";


void MD_ModbusSerialParameters::save(QSettings& _settings) const 
{
	_settings.beginGroup(s_groupName);
	_settings.setValue(s_portNameKeyName, m_portName);
	_settings.setValue(s_parityKeyName, m_parity);
	_settings.setValue(s_baudKeyName,m_baud);
	_settings.setValue(s_dataBitsKeyName,m_dataBits);
	_settings.setValue(s_stopBitsKeyName,m_stopBits);
	_settings.setValue(s_responseTimeKeyName,m_responseTime);
	_settings.setValue(s_numberOfRetries,m_numberOfRetries);
	_settings.endGroup();
	_settings.sync();
}

void MD_ModbusSerialParameters::load(QSettings& _settings)
{
	_settings.beginGroup(s_groupName);
	m_portName = _settings.value(s_portNameKeyName, u8"COM1").toString();
	m_parity = _settings.value(s_parityKeyName, QSerialPort::NoParity).value<QSerialPort::Parity>();
	m_baud = _settings.value(s_baudKeyName, QSerialPort::Baud115200).value<QSerialPort::BaudRate>() ;
	m_dataBits = _settings.value(s_dataBitsKeyName, QSerialPort::Data8).value<QSerialPort::DataBits>();
	m_stopBits = _settings.value(s_stopBitsKeyName, QSerialPort::OneStop).value<QSerialPort::StopBits>();
	m_responseTime = _settings.value(s_responseTimeKeyName, 1000).toInt();
	m_numberOfRetries = _settings.value(s_numberOfRetries, 3).toInt();
	_settings.endGroup();
}
