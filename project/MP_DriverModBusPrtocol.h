#pragma once
#include <utility>
#include <QModbusPdu> 
#include "MP_MoveParam.h"
namespace ModbusProtocol
{
	using AduData = std::pair <QModbusPdu::FunctionCode, QByteArray>;
	class MP_DriverModBusProtocol
	{
	
	public:

		virtual AduData getWriteEnableDrivenMachineAdu(bool _enabled) = 0;
		virtual AduData getWriteStopDrivenMachineAdu(bool _isValid) = 0;
		virtual AduData getWriteGoHomeAdu(bool _isValid) = 0;
		virtual AduData getWriteClearAlarmAdu(bool _isValid) = 0;

		virtual std::vector<AduData> getWriteMoveParamAdu(const std::map<MP_MoveParam::MP_ParamType, int>& _param) = 0;
		
		virtual AduData getWriteRunDrivenPositiveRotaryAdu(bool _isValid) = 0;
		virtual AduData getWriteRunDrivenNegativeRotaryAdu(bool _isValid) = 0;
		virtual std::vector<AduData>   getWriteDrivenRotaryParamAdu(const std::map<MP_DrivenRotaryParam:: MP_DrivenRotaryParamType,int>& _param) = 0;
		virtual AduData getWriteRunPositionModeAdu(bool _isAbsolutePosition) = 0;
		virtual AduData getWriteRunPoisitonAdu(bool _isValid) = 0;

		virtual AduData getReadCurrentPositionAdu() = 0;
		virtual int getPositonFromReturnMessage(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody) = 0;

		virtual AduData getReadIsInPlaceAdu() = 0;
		virtual bool checkIsReturnMessageIndicateIsInPlace(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody) = 0;

		virtual AduData getReadIsGoHomeFinishAdu() = 0;
		virtual bool checkIsReturnMessageIndicateIsGoHomeFinish(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody) = 0;

		virtual AduData getReadIsDrivenMachineEnabledAdu() = 0;
		virtual bool checkIsReturnMessageIndicateIsDrivenMachineEnabled(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody) = 0;

		virtual AduData getReadIsAlarmAdu() = 0;
		virtual bool checkIsReturnMessageIndicateIsAlarm(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody) = 0;

		virtual AduData getReadErrorCodeAdu() = 0;
		virtual int getReadErrorCodeFromReturnMessage(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody)= 0;

		virtual AduData getReadErrorCodeRecordAdu() = 0;
		virtual std::vector<int> getReadErrorCodeRecordFromReturnMessage(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody) = 0;

		virtual AduData getReadIsDrivenMachineRunngingAdu() = 0;
		virtual bool checkIsReturnMessageIndicateIsDrivenMachineRunning(QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody) = 0;

		virtual bool checkIsReturnMessageIndicateSuccess(
			QModbusPdu::FunctionCode _sendFunctionCode, const QByteArray& _sendDataBody,
			QModbusPdu::FunctionCode _returnFunctionCode, const QByteArray& _returnDataBody) = 0;

		virtual AduData getReadMoveParamAdu(MP_MoveParam::MP_ParamType _moveParamType) = 0;
		virtual int getReadMoveParamFromReturnMessage(MP_MoveParam::MP_ParamType _moveParamType, QModbusPdu::FunctionCode _returnedFunctionCode, const QByteArray& _returnDataBody) = 0;

		virtual QString dealWithError(QModbusPdu::FunctionCode _sendFunctionCode, const QByteArray& _sendDataBody,
			QModbusPdu::FunctionCode _returnFunctionCode, const QByteArray& _returnDataBody) = 0;
		virtual bool isMoveParamContinuous(MP_MoveParam::MP_ParamType _firstType, MP_MoveParam::MP_ParamType _secondType) = 0;
		virtual bool isDrivenRotaryContinuous(MP_DrivenRotaryParam::MP_DrivenRotaryParamType _firstType, MP_DrivenRotaryParam::MP_DrivenRotaryParamType _secondType) = 0;

		virtual bool isNeedToSendMessage(QModbusPdu::FunctionCode _function) = 0;

		
	protected:

		QByteArray intToByteArray(int _n, int _base = 10, int _size = 4);
	};

	template <typename Key, typename Value>
	std::vector<std::pair<Key, Value>> reorderKeys(const std::map<Key,Value>& _inputMap,const std::vector<Key>& _orderedKeys) 
	{
		using ReturnType = std::vector<std::pair<Key, Value> >;
		ReturnType ret;
		if (_orderedKeys.empty()) 
		{
			for (const auto& var: _inputMap) 
			{
				ret.emplace_back(var);
			}
		}
		else 
		{
			for ( auto var : _orderedKeys)
			{
				auto it{ _inputMap.find(var) };
				if ( it!= _inputMap.end()) //find it
				{
					ret.emplace_back(std::make_pair(var,it->second));
				}
			}
		}
	
		return ret;
	}

	template <typename Key, typename Value,class Comp>
	std::vector<std::vector<std::pair<Key, Value>>>
	slipByOrder(const std::vector<std::pair<Key, Value>>& _orderedInput,Comp _compIsContinuous)
	{
		typedef std::vector<std::vector<std::pair<Key, Value>>> ReturnType;
		if(_orderedInput.empty())
		{
			return ReturnType();
		}
		ReturnType ret;
		ReturnType::value_type temp;

		bool isFirst = true;
		auto lastElem = _orderedInput.begin()->first;
		for(const auto& var : _orderedInput)
		{
			if(isFirst)
			{
				isFirst = false;
			}
			else
			{
				if(!_compIsContinuous(var.first, lastElem)) //不连续
				{
					ret.push_back(temp);
					temp.clear();
				}
			}

			lastElem = var.first;
			temp.push_back(std::make_pair(var.first, var.second));
		}

		if(!temp.empty())
		{
			ret.push_back(temp);
		}

		return ret;
	}
}


