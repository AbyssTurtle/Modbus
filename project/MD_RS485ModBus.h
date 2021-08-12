/************************************************************************/
/* 文 件 名:MD_RS485ModBus.h
/* 生成日期：2019-01-25
/* 版    本：V1.0.0.1
/* 描    述：Modbus485通信：目前支持研控步进(参考YKD2405PR总线型步进驱动器用户手册_V1.3)
/* 更新日志：
/************************************************************************/

#pragma once

#include <QModbusDevice> 
#include <QModbusPdu>
#include <QObject>
#include <QMutex>
#include <memory>
#include <string>

#include "MD_RS485ModBus_global.h"
#include "MD_ModbusSerialParameters.h"
#include "MC_FutureWatch.h"
#include "MP_MoveParam.h"


namespace Modbus
{
	enum MP_ProtocolType
	{
		YaKo_StepMotorDriver,//研控步进
		FuJi_ServerDriver, //富士伺服
	};

	class MD_RS485MODBUS_EXPORT MD_RS485ModBus: public QObject
	{
		Q_OBJECT
	public:
		MD_RS485ModBus(QObject* parent = nullptr);
		~MD_RS485ModBus();
		
		//打开设备
		virtual void openDevice(const MD_ModbusSerialParameters& _param);
		//关闭设备
		virtual void closeDevice();
		//设备是连接状态
		virtual QModbusDevice::State deviceConnectState();
		//电机使能
		virtual MC_FutureWatch<bool>* enableDrivenMachine(int _index, bool _enabled);
		//停止电机
		virtual MC_FutureWatch<bool>* stopDrivenMachine(int _index, bool _isValid);
		//回原
		virtual MC_FutureWatch<bool>* goHome(int _index, bool _isValid);
		//清除报警
		virtual MC_FutureWatch<bool>* clearAlarm(int _index, bool _isValid);
		//设置运动参数
		virtual MC_FutureWatch<bool>* setMoveParam(int _index, const std::map<MP_MoveParam::MP_ParamType, int>& _param);
		//电机正转
		virtual MC_FutureWatch<bool>* runDrivenPositiveRotary(int _index, bool _isValid);
		//电机反转
		virtual MC_FutureWatch<bool>* runDrivenNegativeRotary(int _index, bool _isValid);
		//设置电机转正反转参数
		virtual MC_FutureWatch<bool>* setDrivenRotaryParam(int _index, const std::map<MP_DrivenRotaryParam::MP_DrivenRotaryParamType, int>& _param);
		//设置定位动动模式(相对/绝对)
		virtual MC_FutureWatch<bool>* startRunPositionModel(int _index, bool _isAbsolutePosition);
		//定位运动
		virtual MC_FutureWatch<bool>* startRunPosition(int _index, bool _isValid);
		//读当前位置
		virtual MC_FutureWatch<int>* readCurrentPosition(int _index);
		//读当前到位信号
		virtual MC_FutureWatch<bool>* readIsInPlace(int _index);
		//读回原完成
		virtual MC_FutureWatch<bool>* readIsGoHomeFinish(int _index);
		//读电机使能
		virtual MC_FutureWatch<bool>* readIsDrivenMachineEnabled(int _index);
		//读是否报警
		virtual MC_FutureWatch<bool>* readIsAlarm(int _index);
		//读电机是否运行中
		virtual MC_FutureWatch<bool>* readIsDrivenMachineRunnging(int _index);
		//读移动参数
		virtual MC_FutureWatch<int>*  readMoveParam(int _index, MP_MoveParam::MP_ParamType _type);
		//发送报文
		virtual MC_FutureWatch<std::pair<QModbusPdu::FunctionCode, QByteArray>>*
			sendMessage(int _index, QModbusPdu::FunctionCode _functionCode, const QByteArray &_payload);
		//获取连接状态文字描述
		virtual QString getConnectStateString(QModbusDevice::State _state);
		//设置协议类型
		virtual void setProtocolType(int _index, MP_ProtocolType _type);
		//获取协议类型
		virtual MP_ProtocolType getProtocolType(int _index);
		//获取当前错误码/报警码
		virtual MC_FutureWatch<int>* readErrorCode(int _index);
		//获取报警记录
		virtual MC_FutureWatch<std::vector<int>>* readErrorCodeRecord(int _index);

		class MI_Impl;
	signals:
		/** modbus连接信号变化信号 */
		void connectStateChanged(QModbusDevice::State _state);

		/** modbus 出错信号 */
		void errorOccurred(const QString& _error);
	private:
		std::shared_ptr<MI_Impl> m_impl; //实现
	};
}

