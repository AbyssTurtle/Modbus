#include "MI_Impl.h"

#include <QModbusRtuSerialMaster>
#include <QTimer>
#include <QDebug>
#include <queue>

#include "MP_YaKoStepModbusProtocol.h"
#include "MP_FuJiServerDriverModbusProtocol.h"
#include "MC_FutureWatch.h"
#include "MC_FutureWatchResultProvider.h"
using namespace Modbus;
using namespace ModbusProtocol;

class MD_MultiSendMonitorResult : public QObject
{
public:
	MD_MultiSendMonitorResult(QObject* _parent = nullptr):QObject(_parent) {}

	std::function<void(void)> m_checkRunResultFunctor; 
	std::function<void(void)> m_endFunctor;
	std::vector <
		std::tuple<QModbusPdu::FunctionCode,
		QByteArray,
		QPointer<MC_FutureWatch<bool>>>> m_data;
};


MD_RS485ModBus::MI_Impl::MI_Impl(QObject* parent /*= Q_NULLPTR*/) :
	QObject(parent),
	m_modbusDevice(new QModbusRtuSerialMaster())
{
	m_driverProtocolMap[YaKo_StepMotorDriver] = std::make_shared<MP_YaKoStepModbusProtocol>();
	m_driverProtocolMap[FuJi_ServerDriver] = std::make_shared<MP_FuJiServerDriverModbusProtocol>();
	makeConnections();
}

void MD_RS485ModBus::MI_Impl::openDevice(const MD_ModbusSerialParameters& _param)
{
	if (_param.m_portName.isEmpty())
	{
		qDebug() << "com name is empty";
		return;
	}
	if (!m_modbusDevice)
	{
		qDebug() << "modbus is not create!";
		return;
	}

	auto curState(m_modbusDevice->state());
	if (curState != QModbusDevice::UnconnectedState)
	{
		return;
		qDebug() << "modbus connect state is " << curState;
	}

	m_modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter, _param.m_portName);
	m_modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter, _param.m_parity);
	m_modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, _param.m_baud);
	m_modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, _param.m_dataBits);
	m_modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, _param.m_stopBits);

	m_modbusDevice->setTimeout(_param.m_responseTime);
	m_modbusDevice->setNumberOfRetries(_param.m_numberOfRetries);

	if (m_modbusDevice->connectDevice())
	{
		qDebug() << "modbus connect success!";
	}
	else
	{
		qDebug() << "modbus connect fail!";
	}
}

void MD_RS485ModBus::MI_Impl::closeDevice()
{
	m_modbusDevice->disconnectDevice();
}

MC_FutureWatch<bool>*
MD_RS485ModBus::MI_Impl::enableDrivenMachine(int _index, bool _enabled)
{
	auto adu(getProtocol(_index)->getWriteEnableDrivenMachineAdu(_enabled));
	return sendWriteRegister(_index, adu.first, adu.second);
}

MC_FutureWatch<bool>*
MD_RS485ModBus::MI_Impl::stopDrivenMachine(int _index, bool _isValid)
{
	auto adu(getProtocol(_index)->getWriteStopDrivenMachineAdu(_isValid));
	return sendWriteRegister(_index, adu.first, adu.second);
}

void MD_RS485ModBus::MI_Impl::makeConnections()
{
	QObject::connect(m_modbusDevice.get(), &QModbusDevice::stateChanged, [this](QModbusDevice::State state)
	{
		emit connectStateChanged(state);
	});

	QObject::connect(m_modbusDevice.get(), &QModbusDevice::errorOccurred, [this]()
	{
		emit errorOccurred(m_modbusDevice->errorString());
	});
}



void Modbus::MD_RS485ModBus::MI_Impl::setProtocolType(int _index, MP_ProtocolType _type)
{
	m_protocolTypeMap[_index] = _type;
}

Modbus::MP_ProtocolType Modbus::MD_RS485ModBus::MI_Impl::getProtocolType(int _index)
{
	auto iter(m_protocolTypeMap.find(_index));
	if (iter != m_protocolTypeMap.end())
	{
		return iter->second;
	}
	return YaKo_StepMotorDriver;
}

std::shared_ptr<MP_DriverModBusProtocol> Modbus::MD_RS485ModBus::MI_Impl::getProtocol(int _index)
{
	auto iter(m_driverProtocolMap.find(getProtocolType(_index)));
	if (iter != m_driverProtocolMap.end())
	{
		return iter->second;
	}
	qDebug() << "modbus cannot find appropriate protocol";
	Q_ASSERT(iter != m_driverProtocolMap.end());
	return std::shared_ptr<MP_DriverModBusProtocol>();
}



MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::MI_Impl::sendWriteRegister(int _index, QModbusPdu::FunctionCode _functionCode, const QByteArray &_payload)
{
	return sendRegisterMessage<bool>(_index, _functionCode, _payload, [=](QModbusPdu::FunctionCode _retFunctionCode, const QByteArray& _retPayload)
	{
		return getProtocol(_index)->checkIsReturnMessageIndicateSuccess(_functionCode, _payload, _retFunctionCode, _retPayload);
	}, [=](QModbusPdu::FunctionCode, const QByteArray&)
	{
		return true;
	});
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::MI_Impl::goHome(int _index, bool _isValid)
{
	auto adu(getProtocol(_index)->getWriteGoHomeAdu(_isValid));
	return sendWriteRegister(_index, adu.first, adu.second);
}


MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::MI_Impl::clearAlarm(int _index, bool _isValid)
{
	auto adu(getProtocol(_index)->getWriteClearAlarmAdu(_isValid));
	return sendWriteRegister(_index, adu.first, adu.second);
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::MI_Impl::setMoveParam(int _index, const std::map<MP_MoveParam::MP_ParamType, int>& _param)
{
	auto adu(getProtocol(_index)->getWriteMoveParamAdu(_param));
	return writeMultiTimes(_index, adu);
}

MC_FutureWatch<bool>*
Modbus::MD_RS485ModBus::MI_Impl::
writeMultiTimes(int _index, const std::vector<std::pair<QModbusPdu::FunctionCode, QByteArray>>& _data)
{
	if (_data.empty())
	{
		auto ret(new MC_FutureWatch<bool>());
		auto resultProvider = MC_FutureWatchResultProvider();
		resultProvider.setIsSuccess(*ret, false);
		resultProvider.setResult(*ret, false);
		resultProvider.setErrorInfo(*ret, u8"None parameters. ");
		resultProvider.setFutureWatchFinished(*ret);
		return ret;
	}

	auto result = QPointer<MD_MultiSendMonitorResult>(new MD_MultiSendMonitorResult());

	for (const auto& var : _data)
	{
		result->m_data.push_back(std::make_tuple(var.first, var.second, nullptr));
	}

	auto retWatch(QPointer<MC_FutureWatch<bool>>(new MC_FutureWatch<bool>()));

	auto deleterFunctor = [result]()
	{
		if (result.isNull()) 
		{
			return;
		}
		for (auto& var : result->m_data)
		{
			if (!std::get<2>(var).isNull())
			{
				std::get<2>(var)->deleteLater();
			}
		}
		result->deleteLater();
	};

	//end factor
	result->m_endFunctor = [result, retWatch, deleterFunctor]()
	{
		if (retWatch.isNull())
		{
			deleterFunctor();
			return;
		}

		if (result.isNull()) 
		{
			return;
		}

		QString info;
		bool isSuccess = true;
		for (const auto& var : result->m_data)
		{
			auto watch = std::get<2>(var);
			if (watch)
			{
				if (!watch->getIsFinished()) 
				{
					isSuccess = false;
					info.append(u8"Has not finish！");
					continue;
				}
				if (!watch->getIsSuccess())
				{
					isSuccess = false;
					info.append(watch->getErrorString());
				}
				else 
				{
					MC_FutureWatchResultProvider().setResult(*watch, true);
				}
			}
			else
			{
				isSuccess = false;
				info.append("Not write:")
					.append(u8"Function code :")
					.append(QString::number(static_cast<int>(std::get<0>(var)), 16))
					.append(", data body:").append(QString::fromUtf8(std::get<1>(var))).append(". ");
			}
		}

		auto resultProvider = MC_FutureWatchResultProvider();
		resultProvider.setIsSuccess(*retWatch, isSuccess);
		resultProvider.setResult(*retWatch, isSuccess);
		resultProvider.setErrorInfo(*retWatch, info);
		resultProvider.setFutureWatchFinished(*retWatch);

		deleterFunctor();
	};

	//run factor
	result->m_checkRunResultFunctor = [result, _index, retWatch]()
	{
		if (retWatch.isNull())
		{
			if (!result.isNull()) 
			{
				result->m_endFunctor();
			}
			return;
		}

		if(result.isNull())
		{
			return;
		}

		//检测是否有失败，失败可以直接返回结果
		bool isHasFail = false;
		for (const auto& var : result->m_data)
		{
			auto watch = std::get<2>(var);
			if (!watch.isNull())
			{
				if (watch->getIsFinished() && !watch->getIsSuccess())
				{
					isHasFail = true;
					break;
				}
			}
		}
		if (isHasFail)	 //fail
		{
			result->m_endFunctor();
			return;
		}

		//检测是否完成
		bool isAllFinish = true;
		for (auto& var : result->m_data)
		{
			auto curWatch{ std::get<2>(var)};
			if (!curWatch.isNull()) 
			{
				if (!curWatch->getIsFinished()) 
				{
					isAllFinish = false;
					break;
				}
			}
		}

		if (isAllFinish)
		{
			//end
			result->m_endFunctor();
		}
	};

	//发送指令
	for (auto& var : result->m_data)
	{
		if (!std::get<2>(var))
		{
			auto curWatch = QPointer<MC_FutureWatch<bool>>(sendWriteRegister(_index, std::get<0>(var), std::get<1>(var)));
			std::get<2>(var) = curWatch;

			QObject::connect(curWatch, &MC_FutureWatch<bool>::finished, curWatch, [result]()
			{
				if (result.isNull())
				{
					return;
				}
				result->m_checkRunResultFunctor();
			},Qt::QueuedConnection);
		}
	}
	QMetaObject::invokeMethod(result, [result]() 
	{
		result->m_checkRunResultFunctor();
	});
	
	return retWatch;
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::MI_Impl::startRunPositionModel(int _index, bool _isAbsolutePosition)
{

	auto adu(getProtocol(_index)->getWriteRunPositionModeAdu(_isAbsolutePosition));
	if (!getProtocol(_index)->isNeedToSendMessage(adu.first))
	{
		auto watch(new MC_FutureWatch<bool>());
		auto resultProvider = MC_FutureWatchResultProvider();
		resultProvider.setResult(*watch, true);
		resultProvider.setIsSuccess(*watch, true);
		resultProvider.setFutureWatchFinished(*watch);
		return watch;
	}
	return sendWriteRegister(_index, adu.first, adu.second);
}

MC_FutureWatch<int>* Modbus::MD_RS485ModBus::MI_Impl::readCurrentPositon(int _index)
{
	auto adu(getProtocol(_index)->getReadCurrentPositionAdu());
	return sendRegisterMessage<int>(_index, adu.first, adu.second, [=](QModbusPdu::FunctionCode _retFunctionCode, const QByteArray& _retPayload)
	{
		return getProtocol(_index)->checkIsReturnMessageIndicateSuccess(adu.first, adu.second, _retFunctionCode, _retPayload);
	}, [=](QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnedPayload)
	{
		return getProtocol(_index)->getPositonFromReturnMessage(_returnedFunctionCode, _returnedPayload);
	});
}

MC_FutureWatch<int>* Modbus::MD_RS485ModBus::MI_Impl::readMoveParam(int _index, MP_MoveParam::MP_ParamType _type)
{
	auto adu(getProtocol(_index)->getReadMoveParamAdu(_type));
	return sendRegisterMessage<int>(_index, adu.first, adu.second, [=](QModbusPdu::FunctionCode _retFunctionCode, const QByteArray& _retPayload)
	{
		return getProtocol(_index)->checkIsReturnMessageIndicateSuccess(adu.first, adu.second, _retFunctionCode, _retPayload);
	}, [=](QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnedPayload)
	{
		return getProtocol(_index)->getReadMoveParamFromReturnMessage(_type, _returnedFunctionCode, _returnedPayload);
	});
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::MI_Impl::readIsInPlace(int _index)
{
	auto adu(getProtocol(_index)->getReadIsInPlaceAdu());
	return sendRegisterMessage<bool>(_index, adu.first, adu.second, [=](QModbusPdu::FunctionCode _retFunctionCode, const QByteArray& _retPayload)
	{
		return getProtocol(_index)->checkIsReturnMessageIndicateSuccess(adu.first, adu.second, _retFunctionCode, _retPayload);
	}, [=](QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnedPayload)
	{
		return getProtocol(_index)->checkIsReturnMessageIndicateIsInPlace(_returnedFunctionCode, _returnedPayload);
	});
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::MI_Impl::readIsGoHomeFinish(int _index)
{
	auto adu(getProtocol(_index)->getReadIsGoHomeFinishAdu());
	return sendRegisterMessage<bool>(_index, adu.first, adu.second, [=](QModbusPdu::FunctionCode _retFunctionCode, const QByteArray& _retPayload)
	{
		return getProtocol(_index)->checkIsReturnMessageIndicateSuccess(adu.first, adu.second, _retFunctionCode, _retPayload);
	}, [=](QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnedPayload)
	{
		return getProtocol(_index)->checkIsReturnMessageIndicateIsGoHomeFinish(_returnedFunctionCode, _returnedPayload);;
	});
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::MI_Impl::readIsDrivenMachineEnabled(int _index)
{
	auto adu(getProtocol(_index)->getReadIsDrivenMachineEnabledAdu());
	return sendRegisterMessage<bool>(_index, adu.first, adu.second, [=](QModbusPdu::FunctionCode _retFunctionCode, const QByteArray& _retPayload)
	{
		return getProtocol(_index)->checkIsReturnMessageIndicateSuccess(adu.first, adu.second, _retFunctionCode, _retPayload);
	}, [=](QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnedPayload)
	{
		return getProtocol(_index)->checkIsReturnMessageIndicateIsDrivenMachineEnabled(_returnedFunctionCode, _returnedPayload);
	});
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::MI_Impl::readIsAlarm(int _index)
{
	auto adu(getProtocol(_index)->getReadIsAlarmAdu());
	return sendRegisterMessage<bool>(_index, adu.first, adu.second, [=](QModbusPdu::FunctionCode _retFunctionCode, const QByteArray& _retPayload)
	{
		return getProtocol(_index)->checkIsReturnMessageIndicateSuccess(adu.first, adu.second, _retFunctionCode, _retPayload);
	}, [=](QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnedPayload)
	{
		return getProtocol(_index)->checkIsReturnMessageIndicateIsAlarm(_returnedFunctionCode, _returnedPayload);
	});
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::MI_Impl::readIsDrivenMachineRunnging(int _index)
{
	auto adu(getProtocol(_index)->getReadIsDrivenMachineRunngingAdu());
	return sendRegisterMessage<bool>(_index, adu.first, adu.second, [=](QModbusPdu::FunctionCode _retFunctionCode, const QByteArray& _retPayload)
	{
		return getProtocol(_index)->checkIsReturnMessageIndicateSuccess(adu.first, adu.second, _retFunctionCode, _retPayload);
	}, [=](QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnedPayload)
	{
		return getProtocol(_index)->checkIsReturnMessageIndicateIsDrivenMachineRunning(_returnedFunctionCode, _returnedPayload);
	});
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::MI_Impl::runDrivenPositiveRotary(int _index, bool _isValid)
{
	auto adu(getProtocol(_index)->getWriteRunDrivenPositiveRotaryAdu(_isValid));
	return sendWriteRegister(_index, adu.first, adu.second);
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::MI_Impl::runDrivenNegativeRotary(int _index, bool _isValid)
{
	auto adu(getProtocol(_index)->getWriteRunDrivenNegativeRotaryAdu(_isValid));
	return sendWriteRegister(_index, adu.first, adu.second);
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::MI_Impl::setDrivenRotaryParam(int _index, const std::map<MP_DrivenRotaryParam::MP_DrivenRotaryParamType, int>& _param)
{
	auto adu(getProtocol(_index)->getWriteDrivenRotaryParamAdu(_param));
	return writeMultiTimes(_index, adu);
}

MC_FutureWatch<bool>* Modbus::MD_RS485ModBus::MI_Impl::startRunPosition(int _index, bool _isValid)
{
	auto adu(getProtocol(_index)->getWriteRunPoisitonAdu(_isValid));
	return sendWriteRegister(_index, adu.first, adu.second);
}

MC_FutureWatch<std::pair<QModbusPdu::FunctionCode, QByteArray>>*
Modbus::MD_RS485ModBus::MI_Impl::
sendMessage(int _index, QModbusPdu::FunctionCode _functionCode, const QByteArray &_payload)
{
	return sendRegisterMessage<std::pair<QModbusPdu::FunctionCode, QByteArray>>(_index, _functionCode, _payload, [=](QModbusPdu::FunctionCode _retFunctionCode, const QByteArray& _retPayload)
	{
		return true;
	}, [=](QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnedPayload)
	{
		return std::make_pair(_returnedFunctionCode, _returnedPayload);
	});
}

MC_FutureWatch<int>* Modbus::MD_RS485ModBus::MI_Impl::readErrorCode(int _index)
{
	auto adu(getProtocol(_index)->getReadErrorCodeAdu());
	return sendRegisterMessage<int>(_index, adu.first, adu.second, [=](QModbusPdu::FunctionCode _retFunctionCode, const QByteArray& _retPayload)
	{
		return getProtocol(_index)->checkIsReturnMessageIndicateSuccess(adu.first, adu.second, _retFunctionCode, _retPayload);
	}, [=](QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnedPayload)
	{
		return getProtocol(_index)->getReadErrorCodeFromReturnMessage(_returnedFunctionCode, _returnedPayload);
	});
}

MC_FutureWatch<std::vector<int>>* Modbus::MD_RS485ModBus::MI_Impl::readErrorCodeRecord(int _index)
{
	auto adu(getProtocol(_index)->getReadErrorCodeRecordAdu());
	if (!getProtocol(_index)->isNeedToSendMessage(adu.first))
	{
		auto watch(new MC_FutureWatch<std::vector<int>>());
		auto resultProvider = MC_FutureWatchResultProvider();
		resultProvider.setResult(*watch, std::vector<int>());
		resultProvider.setIsSuccess(*watch, true);
		resultProvider.setFutureWatchFinished(*watch);
		return watch;
	}
	return sendRegisterMessage<std::vector<int>>(_index, adu.first, adu.second, [=](QModbusPdu::FunctionCode _retFunctionCode, const QByteArray& _retPayload)
	{
		return getProtocol(_index)->checkIsReturnMessageIndicateSuccess(adu.first, adu.second, _retFunctionCode, _retPayload);
	}, [=](QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnedPayload)
	{
		return getProtocol(_index)->getReadErrorCodeRecordFromReturnMessage(_returnedFunctionCode, _returnedPayload);
	});
}

QModbusDevice::State Modbus::MD_RS485ModBus::MI_Impl::deviceConnectState()
{
	return m_modbusDevice->state();
}

