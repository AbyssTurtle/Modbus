#pragma once
#include <utility>
#include <QModbusPdu>
#include "MP_MoveParam.h"
#include "MP_DriverModBusPrtocol.h"
namespace ModbusProtocol
{
	class MP_FuJiServerDriverModbusProtocol:public MP_DriverModBusProtocol
	{
	public:
		enum FunctionCodeType
		{
			///#数据操作#
			ReadData = 0x03, //读取各种数据
			WriteData = 0x10,//写入各种数据
			ReadWriteData = 0x17,//读取各种数据
			///#位操作#
			ReadBits = 0x01,//读入位数据
			WriteSingleBit = 0x05,//写入单一数据
			WriteMultiBits = 0x0F,//写入位数据
			///#维护#
			AfterEncho = 0x08,//后回波

			UnknowType,
		};
		MP_FuJiServerDriverModbusProtocol();
		virtual ~MP_FuJiServerDriverModbusProtocol();

		AduData getWriteEnableDrivenMachineAdu(bool _enabled) override;
		AduData getWriteStopDrivenMachineAdu(bool _isValid) override;
		AduData getWriteGoHomeAdu(bool _isValid) override;
		AduData getWriteClearAlarmAdu(bool _isValid) override;

		std::vector<AduData> getWriteMoveParamAdu(const std::map<MP_MoveParam::MP_ParamType, int>& _param) override;
		AduData getWriteRunDrivenPositiveRotaryAdu(bool _isValid) override;
		AduData getWriteRunDrivenNegativeRotaryAdu(bool _isValid) override;
		std::vector<AduData> getWriteDrivenRotaryParamAdu(const std::map<MP_DrivenRotaryParam::MP_DrivenRotaryParamType, int>& _param) override;
		AduData getWriteRunPositionModeAdu(bool _isAbsolutePosition) override;
		AduData getWriteRunPoisitonAdu(bool _isValid);
		AduData getReadCurrentPositionAdu() override;
		int getPositonFromReturnMessage(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody) override;

		AduData getReadIsInPlaceAdu() override;
		bool checkIsReturnMessageIndicateIsInPlace(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody) override;
		AduData getReadIsGoHomeFinishAdu() override;
		bool checkIsReturnMessageIndicateIsGoHomeFinish(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody) override;
		AduData getReadIsDrivenMachineEnabledAdu() override;
		bool checkIsReturnMessageIndicateIsDrivenMachineEnabled(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody) override;
		AduData getReadIsAlarmAdu() override;
		bool checkIsReturnMessageIndicateIsAlarm(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody) override;

		AduData getReadErrorCodeAdu() override;
		int getReadErrorCodeFromReturnMessage(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody) override;
		AduData getReadErrorCodeRecordAdu() override;
		std::vector<int> getReadErrorCodeRecordFromReturnMessage(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody) override;

		AduData getReadIsDrivenMachineRunngingAdu() override;
		bool checkIsReturnMessageIndicateIsDrivenMachineRunning(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody) override;

		bool checkIsReturnMessageIndicateSuccess(
			QModbusPdu::FunctionCode _sendFunctionCode, const QByteArray& _sendDataBody,
			QModbusPdu::FunctionCode _returnFunctionCode, const QByteArray& _returnDataBody) override;

		AduData getReadMoveParamAdu(MP_MoveParam::MP_ParamType _moveParamType) override;
		int getReadMoveParamFromReturnMessage(MP_MoveParam::MP_ParamType _moveParamType, QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody) override;
		QString dealWithError(QModbusPdu::FunctionCode _sendFunctionCode, const QByteArray& _sendDataBody,
							  QModbusPdu::FunctionCode _returnFunctionCode, const QByteArray& _returnDataBody) override;

		bool isMoveParamContinuous(MP_MoveParam::MP_ParamType _firstType, MP_MoveParam::MP_ParamType _secondType) override;
		bool isDrivenRotaryContinuous(MP_DrivenRotaryParam::MP_DrivenRotaryParamType _firstType, MP_DrivenRotaryParam::MP_DrivenRotaryParamType _secondType) override;
		bool isNeedToSendMessage(QModbusPdu::FunctionCode _functionCode) override;
	private:
		int zoomParam(MP_MoveParam::MP_ParamType _type, int _val, bool isSend);
		int zoomParam(MP_DrivenRotaryParam::MP_DrivenRotaryParamType _type, int _val, bool isSend);
		bool getSingleBitData(QModbusPdu::FunctionCode _functionCode,const QByteArray& _returnDataBody);
		QModbusPdu::FunctionCode getFunctionCode(FunctionCodeType _functionCodeType);
		int getMoveParamStartAddress(MP_MoveParam::MP_ParamType _paramType);
		int calcRegisterCount(MP_MoveParam::MP_ParamType _paramType);

		int getDrivenRotaryParamStartAddress(MP_DrivenRotaryParam::MP_DrivenRotaryParamType _paramType);
		int calcRegisterCount(MP_DrivenRotaryParam::MP_DrivenRotaryParamType _paramType);
		int getIntFromResultMessage(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody);

		static std::vector<MP_MoveParam::MP_ParamType> m_orderedMoveParamTypes;
		static std::vector<MP_MoveParam::MP_ParamType> makeOrderdMoveParamTypes();
		static std::vector<MP_DrivenRotaryParam::MP_DrivenRotaryParamType> m_orderedDrivenRotaryParamTypes;
		static std::vector<MP_DrivenRotaryParam::MP_DrivenRotaryParamType> makeOrderDrivenRotaryTypes();

	};

}

