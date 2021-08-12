/************************************************************************/
/* 文 件 名:MC_FutureWatch.h
/* 生成日期：2019-01-25
/* 版    本：V1.0.0.1
/* 描    述：Modbus485通信用于异步取回复结果
/* 更新日志：
/************************************************************************/
#pragma once

#include <QString>
#include <QObject>
#include <memory>
#include <functional>
#include "MD_RS485ModBus_Global.h"

class MC_FutureWatchResultProvider;
class MD_RS485MODBUS_EXPORT MC_FutureWatchBase : public QObject
{
	Q_OBJECT
public:
	explicit MC_FutureWatchBase(QObject* _parent = Q_NULLPTR):QObject(_parent)
	{
	}

	virtual ~MC_FutureWatchBase()
	{
	}
	
public slots:

//是否完成
bool getIsFinished() const
{
	return m_isFinish;
}


//获取错误提示信息
QString getErrorString() const 
{
	return m_errorString;
}

//设置错误信息
void setErrorString(const QString& _errorString)
{
	m_errorString = _errorString;
}

//是否成功
bool getIsSuccess()const
{
	return m_isSuccess;
}

signals:

//完成信号
void finished();

private slots: 

//设置是否完成
void setIsFinished(bool _isFinished)
{
	m_isFinish = _isFinished;
}
//设置是否成功
void setIsSuccess(bool _isSuccess)
{
	m_isSuccess = _isSuccess;
}
//完成时调用
void onFinished()
{
	QMetaObject::invokeMethod(this, [this]() 
	{
		emit finished();
	},Qt::QueuedConnection);
	
}

private:
	bool m_isFinish = false; //通信是否完成
	bool m_isSuccess = false; //是否成功
	QString m_errorString; //错误提示信息

	friend class MC_FutureWatchResultProvider;
};


template < typename T >
class MC_FutureWatch : public MC_FutureWatchBase
{
public:
	explicit MC_FutureWatch(std::function<void()> _fun = []()
	{}, QObject *_parent = Q_NULLPTR):MC_FutureWatchBase(_parent), m_function(_fun)
	{
	}

	virtual ~MC_FutureWatch()
	{
		m_function();
	}

	T getResult()
	{
		return m_result;
	}

	const T& getResult() const 
	{
		return m_result;
	}

private:
	T  m_result = T{};

	//设置结果
	void setResult(const T& _result)
	{
		m_result = _result;
	}

	std::function<void()> m_function; //析构时调用

	friend class MC_FutureWatchResultProvider;
};

template <>
class MC_FutureWatch<bool> :public MC_FutureWatchBase
{
public:
	MC_FutureWatch(std::function<void()> _fun = []()
	{}, QObject *_parent = Q_NULLPTR):MC_FutureWatchBase(_parent), m_function(_fun)
	{
	}

	virtual ~MC_FutureWatch()
	{
		m_function();
	}

	//获取结果
	bool getResult()
	{
		return m_result;
	}

private:
	bool  m_result = false;
	//设置结果
	void setResult(bool _result)
	{
		m_result = _result;
	}

	std::function<void()> m_function;

	friend class MC_FutureWatchResultProvider;
};

template <> class MC_FutureWatch<void>:public MC_FutureWatchBase
{
public:
	MC_FutureWatch(std::function<void()> _fun = []()
	{}, QObject* _parent = Q_NULLPTR):MC_FutureWatchBase(_parent),m_funtion(_fun)
	{
	}

	virtual ~MC_FutureWatch()
	{
	}
private:
	std::function<void()> m_funtion;
};
