#include "MP_FuJiServerDriverModbusProtocol.h"
#include <QString>
#include <QDebug>

using namespace ModbusProtocol;

std::vector<MP_MoveParam::MP_ParamType>
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
m_orderedMoveParamTypes = MP_FuJiServerDriverModbusProtocol::makeOrderdMoveParamTypes();

std::vector<MP_DrivenRotaryParam::MP_DrivenRotaryParamType>
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
m_orderedDrivenRotaryParamTypes = MP_FuJiServerDriverModbusProtocol::makeOrderDrivenRotaryTypes();

MP_FuJiServerDriverModbusProtocol::MP_FuJiServerDriverModbusProtocol()
{
}


MP_FuJiServerDriverModbusProtocol::~MP_FuJiServerDriverModbusProtocol()
{
}

std::pair <QModbusPdu::FunctionCode, QByteArray>
MP_FuJiServerDriverModbusProtocol::
getWriteEnableDrivenMachineAdu(bool _enabled)
{
	return std::make_pair(getFunctionCode(WriteSingleBit),
		intToByteArray(0x020E/*CONT */, 16)				   // 0x0208/*CONT */
		+ intToByteArray((_enabled ? 0xFF00 : 0x0000), 16));
}

std::pair <QModbusPdu::FunctionCode, QByteArray>
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
getWriteStopDrivenMachineAdu(bool _isValid)
{
	return std::make_pair(getFunctionCode(WriteSingleBit),
		intToByteArray(0x0212/*CONT */, 16)
		+ intToByteArray((_isValid ? 0xFF00 : 0x0000), 16));
}

std::pair <QModbusPdu::FunctionCode, QByteArray>
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
getWriteGoHomeAdu(bool _isValid)
{
	return std::make_pair(getFunctionCode(WriteSingleBit),
		intToByteArray(0x020C/*CONT 11*/, 16)
		+ intToByteArray((_isValid ? 0xFF00 : 0x0000), 16));
}

std::pair <QModbusPdu::FunctionCode, QByteArray>
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
getWriteClearAlarmAdu(bool _isValid)
{
	return std::make_pair(getFunctionCode(WriteSingleBit),
		intToByteArray(0x0210/*CONT*/, 16)
		+ intToByteArray((_isValid ? 0xFF00 : 0x0000), 16));
}

std::vector<std::pair <QModbusPdu::FunctionCode, QByteArray>>
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
getWriteMoveParamAdu(const std::map<MP_MoveParam::MP_ParamType, int>& _param)
{
	if (_param.empty())
	{
		return std::vector<std::pair<QModbusPdu::FunctionCode, QByteArray>>();
	}

	//split
	auto slipParam(slipByOrder(reorderKeys(_param, m_orderedMoveParamTypes), [this](const MP_MoveParam::MP_ParamType& _a, const MP_MoveParam::MP_ParamType& _b)
	{
		return isMoveParamContinuous(_a, _b);
	}));

	std::vector<std::pair<QModbusPdu::FunctionCode, QByteArray>> retData;
	for (const auto& slipVars : slipParam)
	{
		if (slipVars.empty())
		{
			continue;
		}

		std::pair<QModbusPdu::FunctionCode, QByteArray> elemToAdd;
		QByteArray oneData;
		oneData += intToByteArray(getMoveParamStartAddress(slipVars.front().first), 16);

		elemToAdd.first = QModbusPdu::FunctionCode(0x10);

		int registerCount(0);
		for (const auto& elem : slipVars)
		{
			registerCount += calcRegisterCount(elem.first);
		}

		oneData += intToByteArray(registerCount, 16);
		oneData += intToByteArray(2 * registerCount, 16, 2);

		for (const auto& var : slipVars)
		{
			oneData += intToByteArray(zoomParam(var.first, var.second, true), 16, 8);
		}
		elemToAdd.second = oneData;
		retData.push_back(elemToAdd);
	}

	return retData;
}

std::pair <QModbusPdu::FunctionCode, QByteArray>
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
getWriteRunDrivenNegativeRotaryAdu(bool _isValid)
{
	return std::make_pair(QModbusPdu::FunctionCode(0x05), intToByteArray(0x020A/*CONT*/, 16)
		+ intToByteArray((_isValid ? 0xFF00 : 0x0000), 16));
}

int
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
getPositonFromReturnMessage(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)
{
	if (_returnDataBody.size() < 10)
	{
		qDebug() << QString(u8"%1 : Data size is not right!").arg(__FUNCTION__);
		return 0;
	}
	return  static_cast<std::int32_t>(_returnDataBody.mid(2, 8).toULongLong(nullptr, 16));
}

bool
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
checkIsReturnMessageIndicateIsInPlace(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)
{
	return getSingleBitData(_returnedFunctionCode, _returnDataBody);
}

bool ModbusProtocol::MP_FuJiServerDriverModbusProtocol::checkIsReturnMessageIndicateIsGoHomeFinish(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)
{
	return getSingleBitData(_returnedFunctionCode, _returnDataBody);
}

bool ModbusProtocol::MP_FuJiServerDriverModbusProtocol::checkIsReturnMessageIndicateIsDrivenMachineEnabled(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)
{
	return getSingleBitData(_returnedFunctionCode, _returnDataBody);
}

std::pair<QModbusPdu::FunctionCode, QByteArray> ModbusProtocol::MP_FuJiServerDriverModbusProtocol::getReadIsAlarmAdu()
{
	return std::make_pair(getFunctionCode(ReadBits),
		intToByteArray(0x0502/*CONT*/, 16)
		+ intToByteArray(0x1, 16));
}

ModbusProtocol::AduData ModbusProtocol::MP_FuJiServerDriverModbusProtocol::getReadErrorCodeAdu()
{
	return std::make_pair(getFunctionCode(ReadData),
		intToByteArray(0x2200, 16)
		+ intToByteArray(0x02, 16));
}

int ModbusProtocol::MP_FuJiServerDriverModbusProtocol::getReadErrorCodeFromReturnMessage(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)
{
	return getIntFromResultMessage(_returnedFunctionCode, _returnDataBody);
}

ModbusProtocol::AduData ModbusProtocol::MP_FuJiServerDriverModbusProtocol::getReadErrorCodeRecordAdu()
{
	return std::make_pair(getFunctionCode(ReadData),
		intToByteArray(0x2201, 16)
		+ intToByteArray(0x0028, 16));
}

std::vector<int> ModbusProtocol::MP_FuJiServerDriverModbusProtocol::getReadErrorCodeRecordFromReturnMessage(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)
{
	std::vector<int> ret;
	if (_returnDataBody.size() < 5)
	{
		qDebug() << QString(u8"%1: %2").arg(__FUNCTION__).arg(u8"Data size is small.");
		return ret;
	}
	auto vaildDataSize(_returnDataBody.left(2).toInt(nullptr, 16) * 2);
	if (_returnDataBody.size() < (1 + vaildDataSize))
	{
		qDebug() << QString(u8"%1: %2").arg(__FUNCTION__).arg(u8"Data size is small.");
		return ret;
	}
	for (int i = 0; i * 8 < vaildDataSize; ++i)
	{
		ret.emplace_back(_returnDataBody.mid(i * 8 + 2, 8).toInt(nullptr, 16));
	}
	return ret;
}

bool ModbusProtocol::MP_FuJiServerDriverModbusProtocol::checkIsReturnMessageIndicateIsAlarm(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)
{
	return getSingleBitData(_returnedFunctionCode, _returnDataBody);
}

bool
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
getSingleBitData(QModbusPdu::FunctionCode _functionCode, const QByteArray& _returnDataBody)
{
	if (_functionCode != 0x01)
	{
		qDebug() << QString(u8"%1 : Function code(receive：0x%2,should be: 0x%3) is not right!")
			.arg(__FUNCTION__)
			.arg(QString::number(_functionCode, 16));
		return false;
	}

	if (_returnDataBody.size() < 4)
	{
		qDebug() << QString(u8"%1 : Data size is not right!").arg(__FUNCTION__);
		return false;
	}
	if ((_returnDataBody.mid(2, 2).toInt(nullptr, 16) & 0x01) == 0x01)
	{
		return true;
	}
	return false;
}

bool ModbusProtocol::MP_FuJiServerDriverModbusProtocol::checkIsReturnMessageIndicateIsDrivenMachineRunning(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnedDataBody)
{
	return getSingleBitData(_returnedFunctionCode, _returnedDataBody);
}

std::pair <QModbusPdu::FunctionCode, QByteArray>
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
getReadMoveParamAdu(MP_MoveParam::MP_ParamType _moveParamType)
{
	return std::make_pair(getFunctionCode(ReadData),
		intToByteArray(getMoveParamStartAddress(_moveParamType), 16)
		+ intToByteArray(calcRegisterCount(_moveParamType)));
}

bool
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
checkIsReturnMessageIndicateSuccess(
	QModbusPdu::FunctionCode _sendFunctionCode,
	const QByteArray& _sendDataBody,
	QModbusPdu::FunctionCode _returnFunctionCode,
	const QByteArray& _returnDataBody)
{

	///#数据操作#
	if (_sendFunctionCode == getFunctionCode(ReadData))
	{
		if (_returnFunctionCode != _sendFunctionCode)
		{
			return false;
		}
		return true;
	}

	if (_sendFunctionCode == getFunctionCode(WriteData))
	{
		if (_returnFunctionCode != _sendFunctionCode)
		{
			return false;
		}
		return true;
	}

	if (_sendFunctionCode == getFunctionCode(ReadWriteData))
	{
		if (_returnFunctionCode != _sendFunctionCode)
		{
			return false;
		}
		return true;
	}

	///#位操作#
	if (_sendFunctionCode == getFunctionCode(ReadBits))
	{
		if (_returnFunctionCode != _sendFunctionCode)
		{
			return false;
		}

		return true;
	}

	if (_sendFunctionCode == getFunctionCode(WriteSingleBit))
	{
		if (_returnFunctionCode != _sendFunctionCode)
		{
			return false;
		}
		if (_returnDataBody.size() != 8)
		{
			return false;
		}
		if (_sendDataBody == _returnDataBody)
		{
			return true;
		}
		return false;
	}

	if (_sendFunctionCode == getFunctionCode(WriteMultiBits))
	{
		if (_returnFunctionCode != 0x05)
		{
			return false;
		}
		if (_returnDataBody.size() < 4)
		{
			return false;
		}
		if (_sendDataBody.left(2) == _returnDataBody.left(2))
		{
			return true;
		}
		return false;
	}

	///#维护#
	if (_sendFunctionCode == getFunctionCode(AfterEncho))
	{
		if (_returnFunctionCode != _sendFunctionCode)
		{
			return false;
		}
		if (_returnDataBody.size() != 8)
		{
			return false;
		}
		if (_sendDataBody == _returnDataBody)
		{
			return true;
		}
		return false;
	}
	return false;
}

std::pair<QModbusPdu::FunctionCode, QByteArray>
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
getReadIsDrivenMachineRunngingAdu()
{
	return std::make_pair(getFunctionCode(ReadBits),
		intToByteArray(0x020E/*CONT*/, 16)
		+ intToByteArray(0x1, 16));
}

std::pair<QModbusPdu::FunctionCode, QByteArray>
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
getReadIsDrivenMachineEnabledAdu()
{
	return std::make_pair(getFunctionCode(ReadBits),
		intToByteArray(0x0401/*CONT*/, 16)
		+ intToByteArray(0x1, 16));
}

std::pair<QModbusPdu::FunctionCode, QByteArray>
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
getReadIsGoHomeFinishAdu()
{
	return std::make_pair(getFunctionCode(ReadBits),
		intToByteArray(0x0308/*CONT*/, 16)	  // 0x0306/*CONT*/
		+ intToByteArray(0x1, 16));
}

std::pair<QModbusPdu::FunctionCode, QByteArray>
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
getReadIsInPlaceAdu()
{
	return std::make_pair(getFunctionCode(ReadBits),
		intToByteArray(0x0500/*OUT 指令当前位置*/, 16)
		+ intToByteArray(0x1, 16));
}

std::pair <QModbusPdu::FunctionCode, QByteArray>
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
getReadCurrentPositionAdu()
{
	return std::make_pair(getFunctionCode(ReadData),
		intToByteArray(0x1006/*OUT 指令当前位置*/, 16)
		+ intToByteArray(0x2, 16));
}

std::pair <QModbusPdu::FunctionCode, QByteArray>
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
getWriteRunPoisitonAdu(bool _isValid)
{
	return std::make_pair(getFunctionCode(WriteSingleBit), intToByteArray(0x020B/*CONT 11*/, 16)
		+ intToByteArray((_isValid ? 0xFF00 : 0x0000), 16));
}

std::pair <QModbusPdu::FunctionCode, QByteArray>
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
getWriteRunPositionModeAdu(bool _isAbsolutePosition)
{
	return std::make_pair(getFunctionCode(WriteData),
		intToByteArray(0x5100, 16)
		+ intToByteArray(0x2, 16)
		+ intToByteArray(0x4, 16, 2)
		+ intToByteArray((_isAbsolutePosition ? 0x0000 : 0x0100), 16, 4)
		+ intToByteArray(0x0000, 16, 4));
}

std::vector<std::pair <QModbusPdu::FunctionCode, QByteArray>>
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
getWriteDrivenRotaryParamAdu(const std::map<MP_DrivenRotaryParam::MP_DrivenRotaryParamType, int>& _param)
{
	if (_param.empty())
	{
		return std::vector<std::pair<QModbusPdu::FunctionCode, QByteArray>>();
	}

	//split
	auto slipParam(slipByOrder(reorderKeys(_param, m_orderedDrivenRotaryParamTypes), [&](const MP_DrivenRotaryParam::MP_DrivenRotaryParamType& _a, const MP_DrivenRotaryParam::MP_DrivenRotaryParamType& _b)
	{
		return isDrivenRotaryContinuous(_a, _b);
	}));

	std::vector<std::pair<QModbusPdu::FunctionCode, QByteArray>> retData;
	for (const auto& slipVars : slipParam)
	{
		if (slipVars.empty())
		{
			continue;
		}

		std::pair<QModbusPdu::FunctionCode, QByteArray> elemToAdd;
		QByteArray oneData;
		oneData += intToByteArray(getDrivenRotaryParamStartAddress(slipVars.front().first), 16);

		elemToAdd.first = QModbusPdu::FunctionCode(0x10);

		int registerCount(0);
		for (const auto& elem : slipVars)
		{
			registerCount += calcRegisterCount(elem.first);
		}

		oneData += intToByteArray(registerCount, 16);
		oneData += intToByteArray(2 * registerCount, 16, 2);

		for (const auto& var : slipVars)
		{
			oneData += intToByteArray(zoomParam(var.first, var.second, true), 16, 8);
		}
		elemToAdd.second = oneData;
		retData.push_back(elemToAdd);
	}

	return retData;
}

std::pair <QModbusPdu::FunctionCode, QByteArray>
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
getWriteRunDrivenPositiveRotaryAdu(bool _isValid)
{
	return std::make_pair(QModbusPdu::FunctionCode(0x05),
		intToByteArray(0x020F/*CONT */, 16)	   // 0x0209/*CONT */
		+ intToByteArray((_isValid ? 0xFF00 : 0x0000), 16));
}

QString
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
dealWithError(QModbusPdu::FunctionCode _sendFunctionCode, const QByteArray& _sendDataBody,
	QModbusPdu::FunctionCode _returnFunctionCode, const QByteArray& _returnDataBody)
{

	return QString().append(u8"send: function code : ").append(_sendFunctionCode).append(u8" data: ").append(_sendDataBody)
		.append(u8"receive: function code : ").append(_returnFunctionCode).append(u8" data: ").append(_returnDataBody);
}
int ModbusProtocol::MP_FuJiServerDriverModbusProtocol::getIntFromResultMessage(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)
{
	if (_returnDataBody.size() < 10)
	{
		qDebug() << QString(u8"%1: %2").arg(__FUNCTION__).arg(u8"Data size is small.");
		return 0;
	}
	auto vaildDataSize(_returnDataBody.left(2).toInt(nullptr, 16));
	if (_returnDataBody.size() < (1 + vaildDataSize))
	{
		qDebug() << QString(u8"%1: %2").arg(__FUNCTION__).arg(u8"Data size is small.");
		return 0;
	}
	return _returnDataBody.mid(2, vaildDataSize * 2).toULongLong(nullptr, 16);
}
int ModbusProtocol::MP_FuJiServerDriverModbusProtocol::getReadMoveParamFromReturnMessage(MP_MoveParam::MP_ParamType _moveParamType, QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)
{
	return zoomParam(_moveParamType, static_cast<std::int32_t>(getIntFromResultMessage(_returnedFunctionCode, _returnDataBody)), false);
}

bool ModbusProtocol::MP_FuJiServerDriverModbusProtocol::isMoveParamContinuous(MP_MoveParam::MP_ParamType _firstType, MP_MoveParam::MP_ParamType _secondType)
{
	if (m_orderedMoveParamTypes.empty())
	{
		qDebug() << "Move parameters types is empty!";
		return false;
	}

	if ((_firstType == MP_MoveParam::StartSpeed) || (_secondType == MP_MoveParam::StartSpeed))
	{
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

bool
ModbusProtocol::MP_FuJiServerDriverModbusProtocol::
isDrivenRotaryContinuous(
	MP_DrivenRotaryParam::MP_DrivenRotaryParamType _firstType,
	MP_DrivenRotaryParam::MP_DrivenRotaryParamType _secondType)
{
	if (m_orderedDrivenRotaryParamTypes.empty())
	{
		qDebug() << "Driven Rotary parameters types is empty!";
		return false;
	}

	/*auto iterFirst = std::find(m_orderedDrivenRotaryParamTypes.begin(), m_orderedDrivenRotaryParamTypes.end(), _firstType);
	auto iterSecond = std::find(m_orderedDrivenRotaryParamTypes.begin(), m_orderedDrivenRotaryParamTypes.end(), _secondType);
	if ((iterFirst == m_orderedDrivenRotaryParamTypes.end()) || (iterSecond == m_orderedDrivenRotaryParamTypes.end()))
	{
		qDebug() << " driven rotary parameters continuous :find move parameters fail.";
		return false;
	}

	if (std::abs(iterFirst - iterSecond) == 1)
	{
		return true;
	}*/
	return false;
}

bool ModbusProtocol::MP_FuJiServerDriverModbusProtocol::isNeedToSendMessage(QModbusPdu::FunctionCode _functionCode)
{
	return true;
}

int ModbusProtocol::MP_FuJiServerDriverModbusProtocol::zoomParam(MP_MoveParam::MP_ParamType _type, int _val, bool isSend)
{
	if (_type == MP_MoveParam::DecelerationTime || _type == MP_MoveParam::AccelerationTime)
	{
		return isSend ? _val * 10 : _val / 10;
	}

	if (_type == MP_MoveParam::MaxSpeed)
	{
		return isSend ? _val * 100 : _val / 100;
	}

	return _val;
}

int ModbusProtocol::MP_FuJiServerDriverModbusProtocol::zoomParam(MP_DrivenRotaryParam::MP_DrivenRotaryParamType _type, int _val, bool isSend)
{
	if (_type == MP_DrivenRotaryParam::DecelerationTime || _type == MP_DrivenRotaryParam::AccelerationTime)
	{
		return isSend ? _val * 10 : _val / 10;
	}

	if (_type == MP_DrivenRotaryParam::Speed)
	{
		return isSend ? _val * 100 : _val / 100;
	}

	return _val;
}

QModbusPdu::FunctionCode  ModbusProtocol::MP_FuJiServerDriverModbusProtocol::getFunctionCode(FunctionCodeType _functionCodeType)
{
	switch (_functionCodeType)
	{
	case ReadData:
		return QModbusPdu::FunctionCode(0x03);
		break;
	case WriteData:
		return QModbusPdu::FunctionCode(0x10);
		break;
	case ReadWriteData:
		return QModbusPdu::FunctionCode(0x17);
		break;
	case WriteSingleBit:
		return QModbusPdu::FunctionCode(0x05);
		break;
	case ReadBits:
		return QModbusPdu::FunctionCode(0x01);
		break;
	case WriteMultiBits:
		return QModbusPdu::FunctionCode(0x0F);
		break;
	case AfterEncho:
		return QModbusPdu::FunctionCode(0x08);
		break;

	default:
		qDebug() << QString(u8"%1:Function code type(0x%2) is not support!").arg(__FUNCTION__).arg(QString::number(_functionCodeType, 16));
		return QModbusPdu::FunctionCode(0x00);
		break;
	}
}

int ModbusProtocol::MP_FuJiServerDriverModbusProtocol::getMoveParamStartAddress(MP_MoveParam::MP_ParamType _paramType)
{
	int ret = 0x90;
	switch (_paramType)
	{
	case MP_MoveParam::AccelerationTime:
		ret = 0x5103;
		break;

	case MP_MoveParam::DecelerationTime:
		ret = 0x5104;
		break;

	case MP_MoveParam::MaxSpeed:
		ret = 0x5102;
		break;

	case MP_MoveParam::Position:
		ret = 0x5101;
		break;

	default:
		break;
	}
	return ret;
}

int ModbusProtocol::MP_FuJiServerDriverModbusProtocol::calcRegisterCount(MP_MoveParam::MP_ParamType _paramType)
{
	return 2;
}

int ModbusProtocol::MP_FuJiServerDriverModbusProtocol::getDrivenRotaryParamStartAddress(MP_DrivenRotaryParam::MP_DrivenRotaryParamType _paramType)
{
	int ret = 0x90;
	switch (_paramType)
	{
	case MP_DrivenRotaryParam::MP_DrivenRotaryParamType::AccelerationTime:
		ret = 0x4024;
		break;

	case MP_DrivenRotaryParam::MP_DrivenRotaryParamType::DecelerationTime:
		ret = 0x4025;
		break;

	case MP_DrivenRotaryParam::MP_DrivenRotaryParamType::Speed:
		ret = 0x4028;
		break;

	default:
		break;
	}
	return ret;
}

int ModbusProtocol::MP_FuJiServerDriverModbusProtocol::calcRegisterCount(MP_DrivenRotaryParam::MP_DrivenRotaryParamType _paramType)
{
	return 2;
}

std::vector<MP_MoveParam::MP_ParamType> ModbusProtocol::MP_FuJiServerDriverModbusProtocol::makeOrderdMoveParamTypes()
{
	std::vector<MP_MoveParam::MP_ParamType> ret;
	ret.push_back(MP_MoveParam::Position);
	ret.push_back(MP_MoveParam::MaxSpeed);
	ret.push_back(MP_MoveParam::AccelerationTime);
	ret.push_back(MP_MoveParam::DecelerationTime);
	return ret;
}

std::vector<MP_DrivenRotaryParam::MP_DrivenRotaryParamType> ModbusProtocol::MP_FuJiServerDriverModbusProtocol::makeOrderDrivenRotaryTypes()
{
	std::vector<MP_DrivenRotaryParam::MP_DrivenRotaryParamType> ret;
	ret.push_back(MP_DrivenRotaryParam::Speed);
	ret.push_back(MP_DrivenRotaryParam::AccelerationTime);
	ret.push_back(MP_DrivenRotaryParam::DecelerationTime);
	return ret;
}
