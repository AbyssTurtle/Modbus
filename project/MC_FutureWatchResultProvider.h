#pragma once
#include "MC_FutureWatch.h"

class  MC_FutureWatchResultProvider
{
public:
	template < typename T> void setResult(MC_FutureWatch<T>& _watch, const T& _result)
	{
		_watch.setResult(_result);
	}

	template <typename T> void setIsSuccess(MC_FutureWatch<T>& _watch,bool _isSuccess)
	{
		_watch.setIsSuccess(_isSuccess);
	}

	template <typename T> void setFutureWatchFinished(MC_FutureWatch<T>& _watch)
	{
		_watch.setIsFinished(true);
		_watch.onFinished();
	}

	template <typename T> void setErrorInfo(MC_FutureWatch<T>& _watch,const QString& _errorString)
	{
		_watch.setErrorString(_watch.getErrorString() +" " + _errorString);
	}
};

