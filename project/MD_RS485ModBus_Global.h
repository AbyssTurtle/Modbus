#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(MD_RS485MODBUS_LIB)
#  define MD_RS485MODBUS_EXPORT Q_DECL_EXPORT
# else
#  define MD_RS485MODBUS_EXPORT Q_DECL_IMPORT
# endif
#else
# define MD_RS485MODBUS_EXPORT
#endif
