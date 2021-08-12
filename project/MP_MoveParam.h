/************************************************************************/
/* 文 件 名:MC_FutureWatch.h
/* 生成日期：2019-01-25
/* 版    本：V1.0.0.1
/* 描    述：Modbus485通信运动参数
/* 更新日志：
/************************************************************************/
#pragma once
#include <QString>

//运动参数
struct MP_MoveParam
{
	enum  MP_ParamType
	{
		StartSpeed = 1, //起始速度  //富士不支持起始速度
		AccelerationTime = 2,//加速度
		DecelerationTime = 3,//减速度
		MaxSpeed = 4, //最大速度
		Position = 5, //位置
	}; 

	//获取参数描述字符串
	QString getParamTypeString(MP_MoveParam::MP_ParamType _type); 
};

//电机参数
struct MP_DrivenRotaryParam
{
	enum MP_DrivenRotaryParamType
	{
		Speed, //速度
		AccelerationTime, //加速度
		DecelerationTime, //减速度
	};
};

/**
 * #研控步进#
 * startSpeed		< 起始速度: 2 -300 r / min(RW), 默认值： 5(r / min).
 * AccelerationTime < 加速时间： 0-2000ms(RW), 默认值：100 ms. 
 * DecelerationTime  < 减速时间: 0-2000 ms(RW), 默认值： 100 (ms).
 * MaxSpeed 	    < 最大速度：-3000-3000 r/min(RW), 默认值：60/min，低细分设置时，最大速度最大为 3000r/min，高细分设置时，输出频 率最大为 200KHz
 * Position	    < 总脉冲：32767~32768 (RW),默认值： 5000. 
 */





