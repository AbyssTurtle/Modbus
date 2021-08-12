#pragma once
#include <QSerialPort>
#include<QString>
#include <QSettings>
#include "MD_RS485ModBus_Global.h"
namespace Modbus
{
	struct MD_RS485MODBUS_EXPORT MD_ModbusSerialParameters
	{
		QString m_portName = u8"COM1"; //端口号
		QSerialPort::Parity m_parity = QSerialPort::NoParity; //奇偶校验位
		QSerialPort::BaudRate m_baud = QSerialPort::Baud115200;//波特率
		QSerialPort::DataBits m_dataBits = QSerialPort::Data8; //数据位
		QSerialPort::StopBits m_stopBits = QSerialPort::OneStop;//停止位
		int m_responseTime = 1000; //响应时间
		int m_numberOfRetries = 3; //重试次数

		//保存参数
		void save(QSettings& _settings) const;
		//载入参数
		void load(QSettings& _settings);
	};
}



