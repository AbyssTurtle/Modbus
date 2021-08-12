#include "MP_YaKoStepModbusProtocol.h"
#include <QString>
#include <QDebug>
#include <QtEndian>
using namespace ModbusProtocol;
std::vector<MP_MoveParam::MP_ParamType> ModbusProtocol::MP_YaKoStepModbusProtocol::
m_orderedMoveParamTypes = MP_YaKoStepModbusProtocol::makeOrderdMoveParamTypes();

std::vector<MP_DrivenRotaryParam::MP_DrivenRotaryParamType> ModbusProtocol::MP_YaKoStepModbusProtocol::
m_orderedDrivenRotaryParamTypes = MP_YaKoStepModbusProtocol::makeOrderDrivenRotaryTypes();


std::vector<MP_MoveParam::MP_ParamType> ModbusProtocol::MP_YaKoStepModbusProtocol::makeOrderdMoveParamTypes()
{
	std::vector<MP_MoveParam::MP_ParamType> ret;
	ret.push_back(MP_MoveParam::StartSpeed);
	ret.push_back(MP_MoveParam::AccelerationTime);
	ret.push_back(MP_MoveParam::DecelerationTime);
	ret.push_back(MP_MoveParam::MaxSpeed);
	ret.push_back(MP_MoveParam::Position);
	return ret;
}

std::vector<MP_DrivenRotaryParam::MP_DrivenRotaryParamType> ModbusProtocol::MP_YaKoStepModbusProtocol::makeOrderDrivenRotaryTypes()
{
	std::vector<MP_DrivenRotaryParam::MP_DrivenRotaryParamType> ret;
	ret.push_back(MP_DrivenRotaryParam::AccelerationTime);
	ret.push_back(MP_DrivenRotaryParam::DecelerationTime);
	ret.push_back(MP_DrivenRotaryParam::Speed);
	return ret;
}


MP_YaKoStepModbusProtocol::MP_YaKoStepModbusProtocol()
{
}


MP_YaKoStepModbusProtocol::~MP_YaKoStepModbusProtocol()
{
}

/*******************************************************************/
/* 功    能：
/* 输入参数：
/* 输出参数：
/* 返 回 值：
/*******************************************************************/

AduData MP_YaKoStepModbusProtocol::getWriteStopDrivenMachineAdu(bool _isValid)
{
	return std::make_pair(QModbusPdu::FunctionCode(0x06), intToByteArray(0x28, 16) + intToByteArray((_isValid ? 0x1 : 0x2), 16));
}

AduData MP_YaKoStepModbusProtocol::getWriteGoHomeAdu(bool _isValid)
{
	return std::make_pair(QModbusPdu::FunctionCode(0x06), intToByteArray(0x30, 16) + intToByteArray((_isValid ? 0x1 : 0x0), 16));
}

AduData MP_YaKoStepModbusProtocol::getWriteClearAlarmAdu(bool _isValid)
{
	return std::make_pair(QModbusPdu::FunctionCode(0x06), intToByteArray(0x2A, 16) + intToByteArray((_isValid ? 0x1 : 0x0), 16));
}

std::vector<AduData>
MP_YaKoStepModbusProtocol::
getWriteMoveParamAdu(const std::map<MP_MoveParam::MP_ParamType, int> & _param)
{
	if (_param.empty())
	{
		return std::vector<AduData>();
	}

	//split
	auto slipParam(slipByOrder(reorderKeys(_param, m_orderedMoveParamTypes), [&](const MP_MoveParam::MP_ParamType& _a, const MP_MoveParam::MP_ParamType& _b)
	{
		return isMoveParamContinuous(_a, _b);
	}));

	std::vector<AduData> retData;
	for (const auto& slipVars : slipParam)
	{
		if (slipVars.empty())
		{
			continue;
		}

		AduData elemToAdd;
		QByteArray oneData;
		oneData += intToByteArray(getMoveParamStartAddress(slipVars.front().first), 16);

		if ((slipVars.size() != 1) || ((slipVars.size() == 1) && (calcRegisterCount(slipVars.front().first) != 1))) //multi register
		{
			elemToAdd.first = QModbusPdu::FunctionCode(0x10);

			int registerCount(0);
			for (const auto& elem : slipVars)
			{
				registerCount += calcRegisterCount(elem.first);
			}

			oneData += intToByteArray(registerCount, 16);
			oneData += intToByteArray(2 * registerCount, 16, 2);
		}
		else  //single register
		{
			elemToAdd.first = QModbusPdu::FunctionCode(0x06);
		}

		for (const auto& var : slipVars)
		{
			oneData += intToByteArray(var.second, 16, 4 * calcRegisterCount(var.first));
		}
		elemToAdd.second = oneData;
		retData.push_back(elemToAdd);
	}

	return retData;
}

AduData MP_YaKoStepModbusProtocol::getWriteRunPositionModeAdu(bool _isAbsolutePosition)
{
	m_isPositionAbsoluteModel = _isAbsolutePosition;
	return makeAlwaysSuccessStubAdu();
}

AduData MP_YaKoStepModbusProtocol::getReadCurrentPositionAdu()
{
	return std::make_pair(getFunctionCode(ReadRegister), intToByteArray(0x0A, 16) + intToByteArray(2, 16));
}

AduData MP_YaKoStepModbusProtocol::getReadDrivenMachineStateAdu()
{
	return std::make_pair(getFunctionCode(ReadRegister), intToByteArray(0x07, 16) + intToByteArray(1, 16));
}

std::pair <QModbusPdu::FunctionCode, QByteArray> MP_YaKoStepModbusProtocol::getReadMoveParamAdu(MP_MoveParam::MP_ParamType _moveParamType)
{
	return std::make_pair(getFunctionCode(ReadRegister),
		intToByteArray(getMoveParamStartAddress(_moveParamType), 16)
		+ intToByteArray(calcRegisterCount(_moveParamType)));
}

QString MP_YaKoStepModbusProtocol::dealWithError(QModbusPdu::FunctionCode _sendFunctionCode, const QByteArray& _sendDataBody,
	QModbusPdu::FunctionCode _returnFunctionCode, const QByteArray& _returnDataBody)
{
	if (_returnDataBody.isEmpty())
	{
		return QString(u8"Cannot resolve message: message size is 0.");
	}

	if (_returnFunctionCode == 0x83 && _returnDataBody.at(0) == 0x01)
	{
		return QString(u8"CRC checks error.");
	}

	if (_returnFunctionCode == 0x82 && _returnDataBody.at(0) == 0x02)
	{
		return QString(u8"Function code is not right.");
	}

	if (_returnFunctionCode == 0x83 && _returnDataBody.at(0) == 0x03)
	{
		return QString(u8"Data address is not valid. ");
	}

	if (_returnFunctionCode == 0x83 && _returnDataBody.at(0) == 0x05)
	{
		return QString(u8"Address read is overflow.");
	}

	if (_returnFunctionCode == 0x83 && _returnDataBody.at(0) == 0x06)
	{
		return QString(u8"Illegal read and write.");
	}

	if (_returnFunctionCode == 0x86 && _returnDataBody.at(0) == 0x07)
	{
		return QString(u8"Content writing error.");
	}

	return QString("Cannot resolve message.");
}



int MP_YaKoStepModbusProtocol::getMoveParamStartAddress(MP_MoveParam::MP_ParamType _paramType)
{
	int ret = 0;
	switch (_paramType)
	{
	case MP_MoveParam::MP_ParamType::StartSpeed:
		ret = 0x20;
		break;

	case MP_MoveParam::MP_ParamType::AccelerationTime:
		ret = 0x21;
		break;

	case MP_MoveParam::MP_ParamType::DecelerationTime:
		ret = 0x22;
		break;

	case MP_MoveParam::MP_ParamType::MaxSpeed:
		ret = 0x23;
		break;

	case MP_MoveParam::MP_ParamType::Position:
		ret = 0x24;
		break;

	default:
		break;
	}
	return ret;
}

int MP_YaKoStepModbusProtocol::calcRegisterCount(MP_MoveParam::MP_ParamType _paramType)
{
	if (_paramType == MP_MoveParam::MP_ParamType::Position)
	{
		return 2;
	}
	return 1;
}

std::pair <QModbusPdu::FunctionCode, QByteArray>
MP_YaKoStepModbusProtocol::
getWriteEnableDrivenMachineAdu(bool _enabled)
{
	return std::make_pair(getFunctionCode(WriteSingleRegister), intToByteArray(0x29, 16) + intToByteArray((_enabled ? 0x1 : 0x0), 16));
}

std::pair <QModbusPdu::FunctionCode, QByteArray>
ModbusProtocol::MP_YaKoStepModbusProtocol::
getWriteRunDrivenPositiveRotaryAdu(bool _isValid)
{
	return std::make_pair(getFunctionCode(WriteSingleRegister), intToByteArray(0x27, 16) + intToByteArray((_isValid ? 0x02 : 0x00), 16));
}

std::pair <QModbusPdu::FunctionCode, QByteArray>
ModbusProtocol::MP_YaKoStepModbusProtocol::
getWriteRunDrivenNegativeRotaryAdu(bool _isValid)
{
	return getWriteRunDrivenPositiveRotaryAdu(_isValid);
}

std::vector<std::pair <QModbusPdu::FunctionCode, QByteArray>>
ModbusProtocol::MP_YaKoStepModbusProtocol::
getWriteDrivenRotaryParamAdu(const std::map<MP_DrivenRotaryParam::MP_DrivenRotaryParamType, int>& _param)
{
	std::map<MP_MoveParam::MP_ParamType, int> param;
	for (const auto& var : _param)
	{
		if (var.first == MP_DrivenRotaryParam::Speed)
		{
			param[MP_MoveParam::MaxSpeed] = var.second;
		}
		else if (var.first == MP_DrivenRotaryParam::AccelerationTime)
		{
			param[MP_MoveParam::AccelerationTime] = var.second;
		}
		else if (var.first == MP_DrivenRotaryParam::DecelerationTime)
		{
			param[MP_MoveParam::DecelerationTime] = var.second;
		}
	}

	return getWriteMoveParamAdu(param);

	//if(_param.empty())
	//{
	//	return  ReturnType();
	//}

	////split
	//auto slipParam(slipByOrder(_param, [&](const MP_DrivenRotaryParam::MP_DrivenRotaryParamType& _a, const MP_DrivenRotaryParam::MP_DrivenRotaryParamType& _b)
	//{
	//	return isDrivenRotaryContinuous(_a, _b);
	//}));

	//std::vector<AduData> retData;
	//for(const auto& slipVars : slipParam)
	//{
	//	if(slipVars.empty())
	//	{
	//		continue;
	//	}

	//	AduData elemToAdd;
	//	QByteArray oneData;
	//	oneData.append(intToByteArray(getDrivenRotaryParamStartAddress(slipVars.front().first), 16));

	//	if(slipVars.size() != 1 || ((slipVars.size() == 1 && calcRegisterCount(slipVars.front().first)) != 1)) //multi register
	//	{
	//		elemToAdd.first = QModbusPdu::FunctionCode(0x10);

	//		int registerCount(0);
	//		for(const auto& elem : slipVars)
	//		{
	//			registerCount += calcRegisterCount(elem.first);
	//		}

	//		oneData.append(intToByteArray(registerCount), 16);
	//		oneData.append(intToByteArray(2 * registerCount, 16, 2));
	//	}
	//	else
	//	{
	//		elemToAdd.first = QModbusPdu::FunctionCode(0x06);
	//		oneData.append(intToByteArray(calcRegisterCount(slipVars.front().first), 16));
	//	}

	//	for(const auto& var : slipVars)
	//	{
	//		oneData.append(intToByteArray(var.second), 4 * calcRegisterCount(var.first));
	//	}
	//	elemToAdd.second = oneData;
	//	retData.push_back(elemToAdd);
	//}

	//return retData;
}

std::pair <QModbusPdu::FunctionCode, QByteArray> ModbusProtocol::MP_YaKoStepModbusProtocol::
getWriteRunPoisitonAdu(bool _isValid)
{
	return std::make_pair(getFunctionCode(WriteSingleRegister), intToByteArray(0x27, 16) + intToByteArray((m_isPositionAbsoluteModel ? 0x05 : 0x01), 16));
}

int
ModbusProtocol::MP_YaKoStepModbusProtocol::
getPositonFromReturnMessage(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)
{
	return static_cast<std::int32_t>(getIntFromResultMessage(_returnedFunctionCode, _returnDataBody));
}

AduData ModbusProtocol::MP_YaKoStepModbusProtocol::getReadIsInPlaceAdu()
{
	return getReadDrivenMachineStateAdu();
}

bool ModbusProtocol::MP_YaKoStepModbusProtocol::checkIsReturnMessageIndicateIsInPlace(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)
{
	if (_returnDataBody.size() < 6)
	{
		qDebug() << QString(u8"%1: %2").arg(__FUNCTION__).arg(u8"Data size is small.");
		return false;
	}
	return _returnDataBody.mid(2, _returnDataBody.left(2).toInt(nullptr, 16) * 2).toUShort(nullptr, 16) & 0x0001;
}

AduData ModbusProtocol::MP_YaKoStepModbusProtocol::getReadIsGoHomeFinishAdu()
{
	return getReadDrivenMachineStateAdu();
}

bool ModbusProtocol::MP_YaKoStepModbusProtocol::checkIsReturnMessageIndicateIsGoHomeFinish(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)
{
	if (_returnDataBody.size() < 6)
	{
		qDebug() << QString(u8"%1: %2").arg(__FUNCTION__).arg(u8"Data size is small.");
		return false;
	}
	return _returnDataBody.mid(2, _returnDataBody.left(2).toInt(nullptr, 16) * 2).toUShort(nullptr, 16) & 0x0002;
}

AduData ModbusProtocol::MP_YaKoStepModbusProtocol::getReadIsDrivenMachineEnabledAdu()
{
	return getReadDrivenMachineStateAdu();
}

bool ModbusProtocol::MP_YaKoStepModbusProtocol::checkIsReturnMessageIndicateIsDrivenMachineEnabled(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)
{
	if (_returnDataBody.size() < 6)
	{
		qDebug() << QString(u8"%1: %2").arg(__FUNCTION__).arg(u8"Data size is small.");
		return false;
	}
	return !(_returnDataBody.mid(2, _returnDataBody.left(2).toInt(nullptr, 16) * 2).toUShort(nullptr, 16) & 0x0010);
}

AduData ModbusProtocol::MP_YaKoStepModbusProtocol::getReadIsAlarmAdu()
{
	return getReadDrivenMachineStateAdu();
}

ModbusProtocol::AduData ModbusProtocol::MP_YaKoStepModbusProtocol::getReadErrorCodeAdu()
{
	return std::make_pair(getFunctionCode(ReadRegister), intToByteArray(0x06, 16) + intToByteArray(1, 16));
}

int ModbusProtocol::MP_YaKoStepModbusProtocol::getReadErrorCodeFromReturnMessage(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)
{
	return getIntFromResultMessage(_returnedFunctionCode, _returnDataBody);
}

int ModbusProtocol::MP_YaKoStepModbusProtocol::getIntFromResultMessage(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)
{
	if (_returnDataBody.size() < 2)
	{
		qDebug() << QString(u8"%1: %2").arg(__FUNCTION__).arg(u8"Data size is small.");
		return 0;
	}
	auto vaildDataSize(_returnDataBody.left(2).toInt(nullptr, 16) * 2);
	if (_returnDataBody.size() < (2 + vaildDataSize))
	{
		qDebug() << QString(u8"%1: %2").arg(__FUNCTION__).arg(u8"Data size is small.");
		return 0;
	}
	return _returnDataBody.mid(2, vaildDataSize).toULongLong(nullptr, 16);
}

ModbusProtocol::AduData ModbusProtocol::MP_YaKoStepModbusProtocol::getReadErrorCodeRecordAdu()
{
	return makeAlwaysSuccessStubAdu();
}

std::vector<int> ModbusProtocol::MP_YaKoStepModbusProtocol::getReadErrorCodeRecordFromReturnMessage(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)
{
	return std::vector<int>();
}

bool ModbusProtocol::MP_YaKoStepModbusProtocol::checkIsReturnMessageIndicateIsAlarm(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)
{
	if (_returnDataBody.size() < 6)
	{
		qDebug() << QString(u8"%1: %2").arg(__FUNCTION__).arg(u8"Data size is small.");
		return false;
	}
	return _returnDataBody.mid(2, _returnDataBody.left(2).toInt(nullptr, 16) * 2).toUShort(nullptr, 16) & 0x0008;
}

AduData ModbusProtocol::MP_YaKoStepModbusProtocol::getReadIsDrivenMachineRunngingAdu()
{
	return getReadDrivenMachineStateAdu();
}

bool ModbusProtocol::MP_YaKoStepModbusProtocol::checkIsReturnMessageIndicateIsDrivenMachineRunning(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)
{
	if (_returnDataBody.size() < 6)
	{
		qDebug() << QString(u8"%1: %2").arg(__FUNCTION__).arg(u8"Data size is small.");
		return false;
	}
	return _returnDataBody.mid(2, _returnDataBody.left(2).toInt(nullptr, 16) * 2).toUShort(nullptr, 16) & 0x0004;
}

bool ModbusProtocol::MP_YaKoStepModbusProtocol::checkIsReturnMessageIndicateSuccess(
	QModbusPdu::FunctionCode _sendFunctionCode, const QByteArray& _sendDataBody,
	QModbusPdu::FunctionCode _returnFunctionCode, const QByteArray& _returnDataBody)
{
	if (_returnDataBody.size() < 6)
	{
		qDebug() << QString(u8"%1: %2").arg(__FUNCTION__).arg(u8"Data size is small.");
		return false;
	}

	if (_sendFunctionCode == getFunctionCode(ReadRegister)) //读保存寄存器
	{
		return (_returnFunctionCode == _sendFunctionCode) &&
			(_returnDataBody.left(2).toInt(nullptr, 16) == _sendDataBody.mid(4, 4).toInt(nullptr, 16) * 2);
	}

	if (_sendFunctionCode == getFunctionCode(WriteSingleRegister))//写单个寄存器
	{
		return (_returnFunctionCode == _sendFunctionCode) && (_returnDataBody == _sendDataBody);
	}

	if (_sendFunctionCode == getFunctionCode(WriteMultiRegisters)) //写多个寄存器
	{
		if (_sendDataBody.size() < 8 || _returnDataBody.size() < 8)
		{
			qDebug() << QString(u8"%1: %2 (send:%3,return:%4)")
				.arg(__FUNCTION__)
				.arg(u8"Write multiple register data size is not right.")
				.arg(_sendDataBody.size())
				.arg(_returnDataBody.size());
			return false;
		}
		return (_returnFunctionCode == _sendFunctionCode) && (_returnDataBody.left(8) == _sendDataBody.left(8));
	}


	qDebug() << QString(u8"%1: %2").arg(__FUNCTION__).arg(u8"Unknow how to deal with return message.");
	return false;
}

int ModbusProtocol::MP_YaKoStepModbusProtocol::getReadMoveParamFromReturnMessage(MP_MoveParam::MP_ParamType _moveParamType, QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)
{
	if (_moveParamType == MP_MoveParam::Position)
	{
		return  static_cast<std::int32_t>(getIntFromResultMessage(_returnedFunctionCode, _returnDataBody));
	}

	return static_cast<std::int16_t>(getIntFromResultMessage(_returnedFunctionCode, _returnDataBody));
}

bool ModbusProtocol::MP_YaKoStepModbusProtocol::isMoveParamContinuous(MP_MoveParam::MP_ParamType _firstType, MP_MoveParam::MP_ParamType _secondType)
{
	if (m_orderedMoveParamTypes.empty())
	{
		qDebug() << "Move parameters types is empty!";
		return false;
	}

	auto iterFirst = std::find(m_orderedMoveParamTypes.begin(), m_orderedMoveParamTypes.end(), _firstType);
	auto iterSecond = std::find(m_orderedMoveParamTypes.begin(), m_orderedMoveParamTypes.end(), _secondType);
	if ((iterFirst == m_orderedMoveParamTypes.end()) || (iterSecond == m_orderedMoveParamTypes.end()))
	{
		qDebug() << " move parameters continuous :find move parameters fail.";
		return false;
	}

	if (std::abs(iterFirst - iterSecond) == 1)
	{
		return true;
	}
	return false;
}

bool ModbusProtocol::MP_YaKoStepModbusProtocol::isDrivenRotaryContinuous(MP_DrivenRotaryParam::MP_DrivenRotaryParamType _firstType, MP_DrivenRotaryParam::MP_DrivenRotaryParamType _secondType)
{
	if (m_orderedDrivenRotaryParamTypes.empty())
	{
		qDebug() << "Driven Rotary parameters types is empty!";
		return false;
	}

	auto iterFirst = std::find(m_orderedDrivenRotaryParamTypes.begin(), m_orderedDrivenRotaryParamTypes.end(), _firstType);
	auto iterSecond = std::find(m_orderedDrivenRotaryParamTypes.begin(), m_orderedDrivenRotaryParamTypes.end(), _secondType);
	if ((iterFirst == m_orderedDrivenRotaryParamTypes.end()) || (iterSecond == m_orderedDrivenRotaryParamTypes.end()))
	{
		qDebug() << " driven rotary parameters continuous :find move parameters fail.";
		return false;
	}

	if (std::abs(iterFirst - iterSecond) == 1)
	{
		return true;
	}
	return false;
}

bool ModbusProtocol::MP_YaKoStepModbusProtocol::isNeedToSendMessage(QModbusPdu::FunctionCode _functionCode)
{
	if (getFunctionCode(NoNeedToSendAlwaysSuccess) == _functionCode)
	{
		return false;
	}
	return true;
}

QModbusPdu::FunctionCode  ModbusProtocol::MP_YaKoStepModbusProtocol::getFunctionCode(FunctionCodeType _functionCodeType)
{
	switch (_functionCodeType)
	{
	case ReadRegister:
		return QModbusPdu::FunctionCode(0x03);
		break;
	case WriteSingleRegister:
		return QModbusPdu::FunctionCode(0x06);
		break;
	case WriteMultiRegisters:
		return QModbusPdu::FunctionCode(0x10);
	case NoNeedToSendAlwaysSuccess:
		return QModbusPdu::FunctionCode(0xFF);
		break;
	default:
		qDebug() << QString(u8"%1:Function code type(%2) is not support!").arg(__FUNCTION__).arg(QString::number(_functionCodeType, 16));
		return QModbusPdu::FunctionCode(0x00);
		break;
	}
}

int ModbusProtocol::MP_YaKoStepModbusProtocol::calcRegisterCount(MP_DrivenRotaryParam::MP_DrivenRotaryParamType _paramType)
{
	return 1;
}

ModbusProtocol::AduData ModbusProtocol::MP_YaKoStepModbusProtocol::makeAlwaysSuccessStubAdu()
{
	return std::make_pair(QModbusPdu::FunctionCode(0xFF), QByteArray());
}

int ModbusProtocol::MP_YaKoStepModbusProtocol::
getDrivenRotaryParamStartAddress(MP_DrivenRotaryParam::MP_DrivenRotaryParamType _paramType)
{
	int ret = 0;
	switch (_paramType)
	{
	case MP_DrivenRotaryParam::MP_DrivenRotaryParamType::AccelerationTime:
		ret = 0x21;
		break;

	case MP_DrivenRotaryParam::MP_DrivenRotaryParamType::DecelerationTime:
		ret = 0x22;
		break;

	case MP_DrivenRotaryParam::MP_DrivenRotaryParamType::Speed:
		ret = 0x23;
		break;

	default:
		break;
	}
	return ret;
}

