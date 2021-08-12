#pragma once
#include "MD_RS485ModBus.h"
#include "MD_ModbusSerialParameters.h"
#include "MC_FutureWatch.h"
#include "MP_MoveParam.h"
#include <QModbusDevice>
#include <QModbusPdu>
#include <QObject>
#include <QPointer>
#include <functional>

class QModbusRtuSerialMaster;
class MP_DriverModBusPrtocol;
namespace ModbusProtocol
{
	class MP_DriverModBusProtocol;
	class MP_YaKoStepModbusProtocol;
}

class Modbus::MD_RS485ModBus::MI_Impl : public QObject
{
	Q_OBJECT
public:
	MI_Impl(QObject* parent = Q_NULLPTR);

	public slots:
	void openDevice(const MD_ModbusSerialParameters& _param);
	void closeDevice();
	QModbusDevice::State deviceConnectState();
	MC_FutureWatch<bool>* enableDrivenMachine(int _index, bool _enabled);
	MC_FutureWatch<bool>* stopDrivenMachine(int _index, bool _isEmergency);
	MC_FutureWatch<bool>* goHome(int _index, bool _isValid);
	MC_FutureWatch<bool>* clearAlarm(int _index, bool _isValid);
	MC_FutureWatch<bool>* setMoveParam(int _index, const std::map<MP_MoveParam::MP_ParamType, int>& _param);
	MC_FutureWatch<bool>* runDrivenPositiveRotary(int _index, bool _isValid);
	MC_FutureWatch<bool>* runDrivenNegativeRotary(int _index, bool _isValid);
	MC_FutureWatch<bool>* setDrivenRotaryParam(int _index, const std::map<MP_DrivenRotaryParam::MP_DrivenRotaryParamType, int>& _param);
	MC_FutureWatch<bool>* startRunPositionModel(int _index, bool _isAbsolutePosition);
	MC_FutureWatch<bool>* startRunPosition(int _index, bool _isValid);
	MC_FutureWatch<int>* readCurrentPositon(int _index);
	MC_FutureWatch<bool>* readIsInPlace(int _index);

	MC_FutureWatch<bool>* readIsGoHomeFinish(int _index);
	MC_FutureWatch<bool>* readIsDrivenMachineEnabled(int _index);
	MC_FutureWatch<bool>* readIsAlarm(int _index);
	MC_FutureWatch<bool>* readIsDrivenMachineRunnging(int _index);

	MC_FutureWatch<int>* readMoveParam(int _index, MP_MoveParam::MP_ParamType _type);

	MC_FutureWatch <std::pair<QModbusPdu::FunctionCode, QByteArray >>* sendMessage(int _index, QModbusPdu::FunctionCode _functionCode, const QByteArray &_payload);

	void setProtocolType(int _index, MP_ProtocolType _type);
	MP_ProtocolType getProtocolType(int _index);
	std::shared_ptr<ModbusProtocol::MP_DriverModBusProtocol> getProtocol(int _index);
	MC_FutureWatch<int>* readErrorCode(int _index);
	MC_FutureWatch<std::vector<int>>* readErrorCodeRecord(int _index);

signals:
	/** modbus连接信号变化信号 */
	void connectStateChanged(QModbusDevice::State _state);

	/** modbus 出错信号 */
	void errorOccurred(const QString& _error);


	private slots:
	void makeConnections();

private:
	template <typename T> MC_FutureWatch<T>* sendRegisterMessage(int _index, QModbusPdu::FunctionCode _functionCode, const QByteArray &_payload,
		std::function<bool(QModbusPdu::FunctionCode, const QByteArray&)> _functorDecideIsReturnMessageIndicateSuccess,
		std::function<T(QModbusPdu::FunctionCode, const QByteArray&)> _functorOnSuccess);

	MC_FutureWatch<bool>* sendWriteRegister(int _index, QModbusPdu::FunctionCode _functionCode, const QByteArray &_payload);
	MC_FutureWatch<bool>* writeMultiTimes(int _index, const std::vector<std::pair<QModbusPdu::FunctionCode, QByteArray>>& _data);

	std::shared_ptr<QModbusRtuSerialMaster> m_modbusDevice;

	std::map<int, MP_ProtocolType> m_protocolTypeMap;
	std::map<MP_ProtocolType, std::shared_ptr<ModbusProtocol::MP_DriverModBusProtocol>> m_driverProtocolMap;
};


template <typename T>  MC_FutureWatch<T>* Modbus::MD_RS485ModBus::MI_Impl::sendRegisterMessage(
	int _index,
	QModbusPdu::FunctionCode _functionCode,
	const QByteArray &_payload,
	std::function<bool(QModbusPdu::FunctionCode, const QByteArray&)> _functorDecideIsReturnMessageIndicateSuccess,
	std::function<T(QModbusPdu::FunctionCode, const QByteArray&)> _functorOnSuccess)
{
	//qDebug() << u8"before send ->function code : " << _functionCode << u8" " << QByteArray::fromHex(_payload);
	auto reply = QPointer<QModbusReply>(m_modbusDevice->sendRawRequest(QModbusRequest(_functionCode, QByteArray::fromHex(_payload)), _index));

	auto watch(QPointer<MC_FutureWatch<T>>(new MC_FutureWatch<T>([reply]()
	{
		reply->deleteLater();
	})));

	if (reply.isNull())
	{
		MC_FutureWatchResultProvider resultProvider;
		resultProvider.setIsSuccess(*watch, false);
		resultProvider.setErrorInfo(*watch, QString(u8"Error:send(functionCode: %1,payload: %2 ).  %3")
			.arg(_functionCode)
			.arg(QString::fromUtf8(_payload))
			.arg(u8"Create reply fail! Please check modbus state."));

		if (watch.isNull())
		{
			MC_FutureWatchResultProvider().setFutureWatchFinished(*watch);
		}
		return watch;
	}

	auto functor = [=]()
	{
		if (reply.isNull() || watch.isNull())
		{
			return;
		}
		QString errorString;
		if (reply->error() == QModbusDevice::NoError)
		{
			if (_functorDecideIsReturnMessageIndicateSuccess(reply->rawResult().functionCode(), reply->rawResult().data().toHex()))
			{
				MC_FutureWatchResultProvider().setIsSuccess(*watch, true);
				MC_FutureWatchResultProvider().setResult(*watch, _functorOnSuccess(reply->rawResult().functionCode(), reply->rawResult().data().toHex()));
			}
			else
			{
				errorString = QString(u8"Error: reply(functionCode: %1,payload: %2 ),send(functionCode: %3,payload: %4 ).  %5")
					.arg(reply->rawResult().functionCode())
					.arg(QString::fromUtf8(reply->rawResult().data()))
					.arg(_functionCode)
					.arg(QString::fromUtf8(_payload))
					.arg(getProtocol(_index)->dealWithError(_functionCode, _payload, reply->rawResult().functionCode(), reply->rawResult().data()));
			}
		}
		else
		{
			errorString = QString(u8"ModbusDevice error: %1").arg(reply->errorString());
		}

		watch->setErrorString(errorString);
		MC_FutureWatchResultProvider().setFutureWatchFinished(*watch);
	};

	QObject::connect(reply, &QModbusReply::finished, reply, functor);

	if (reply->isFinished())
	{
		functor();
	}
	return watch;
}
