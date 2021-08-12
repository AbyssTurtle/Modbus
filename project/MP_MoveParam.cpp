#include "MP_MoveParam.h"

QString MP_MoveParam::getParamTypeString(MP_MoveParam::MP_ParamType _type)
{
	QString ret;
	switch(_type)
	{
		case MP_MoveParam::StartSpeed:
			ret = u8"start speed";
			break;
		case MP_MoveParam::AccelerationTime:
			ret = u8"acceleration time";
			break;
		case MP_MoveParam::DecelerationTime:
			ret = u8"deceleration time";
			break;
		case MP_MoveParam::MaxSpeed:
			ret = u8"max speed";
			break;
		case MP_MoveParam::Position:
			ret = u8"position";
			break;
		default:
			break;
	}
	return ret;
}


