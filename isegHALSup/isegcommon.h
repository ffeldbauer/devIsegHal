/****************************************************************************
**
** Copyright (C) 2016 - 2017 iseg Spezialelektronik GmbH
** Contact: http://www.iseg-hv.com
** Contact: sales@iseg-hv.com
**
** This file is part of the isegHAL project
**
** Commercial License Usage
** Licensees holding valid commercial licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained
** in a written agreement between you and iseg. For licensing terms
** and conditions contact sales@iseg-hv.com.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
****************************************************************************/

#ifndef ISEG_COMMON_H
#define ISEG_COMMON_H

#define INTERFACE_NAME_SIZE 128
#define INTERFACE_TYPE_SIZE 128

#define FULLY_QUALIFIED_OBJECT_SIZE 128
#define OBJECT_NAME_SIZE 32
#define VALUE_SIZE 200
#define DATA_TYPE_SIZE 5
#define UNIT_SIZE 4
#define ACCESS_SIZE 3
#define QUALITY_SIZE 4
#define TIME_SIZE 25

#define WRITE_READ_SIZE 20
#define MAX_READ_MESSAGES 10
#define NO_BIT_ARRAY_INDEX 32
#define NO_FLOAT_ARRAY_INDEX NO_BIT_ARRAY_INDEX + 1

#define ISEG_ITEM_QUALITY_INVALID            "000"
#define ISEG_ITEM_QUALITY_INITIALIZE         "001"
#define ISEG_ITEM_QUALITY_OK                 "002"
#define ISEG_ITEM_QUALITY_COMMUNICATION_BAD  "003"
#define ISEG_ITEM_QUALITY_ERROR              "004"

#define UDS_FILE "/tmp/iseghal.uds"

#ifndef UNREFERENCED_PARAMETER
#	define UNREFERENCED_PARAMETER(P) (void)(P)
#endif

enum {
	DataCacheMaxLines = 16,
	DataCacheMaxCrates = 16,
	DataCacheMaxModules = 64,
	DataCacheMaxChannels = 48
};

typedef enum KSupplyVoltages {
	VCC_POSITIVE_EXTERN = 0, ///< +24 Volt external
	VCC_NEGATIVE_EXTERN = 1, ///< -24 Volt external
	VCC_LOGIC_EXTERN = 2,    ///< 5..6 Volt external
	VCC_POSITIVE_OPV = 3,    ///< +12..15 Volt internal
	VCC_NEGATIVE_OPV = 4,    ///< -12..-15 Volt internal
	VCC_LOGIC_5V_INTERN = 5, ///< +5 Volt internal
	VCC_LOGIC_3V_INTERN = 6, ///< +3.3 Volt internal
	VCC_EXTERN_AC_POWER = 7, ///< 230 / 110 Volt AC Power line external
	VCC_BATTERY = 8,         ///< 24 Volt UPS battery
	VCC_ARRAY_SIZE
} KSupplyVoltages;

typedef enum KInternalTemperatures {
	INTERNAL_TEMPERATURE_ARRAY_SIZE = 9
} KInternalTemperatures;

typedef enum IsegLogLevel {
	LogErrors = 1,           ///< Log CAN bus error messages, loss of communication
	LogInformation = 2,      ///< Log state of initialising, switch on/off live insertion mode
	LogApiRead = 4,          ///< Log API read accesses
	LogApiWrite = 8,         ///< Log API write accesses
	LogCanReceive = 32,      ///< Log CAN receive messages (CANRX)
	LogCanReadRequest = 64,  ///< Log CAN read requests messages (CANRQ)
	LogCanWriteRequest = 128 ///< Log CAN write request messages (CANTX)
} IsegLogLevel;

typedef enum IsegResult {
	ISEG_OK = 0,             ///< The function returned without problems
	ISEG_ERROR = 1,          ///< The function returned an error
	ISEG_WRONG_SESSION_NAME, ///< The iseg_connect() parameter name is already in use
	ISEG_WRONG_USER,         ///< The remote iseg_connect() user name is invalid
	ISEG_WRONG_PASSWORD,     ///< The remote iseg_connect() password is invalid
	ISEG_NOT_AUTHORIZED,     ///< The remote access is not allowed due to wrong user name or password
	ISEG_NO_SSL_SUPPORT      ///< The iseg_connect() failed due to missing SSL libraries
} IsegResult;

typedef struct IsegItem {
	char object[FULLY_QUALIFIED_OBJECT_SIZE];	//line.module.channel.item  0.0.1.VoltageSet
	char value[VALUE_SIZE];
	char quality[QUALITY_SIZE];
	char timeStampLastRefreshed[TIME_SIZE];
	char timeStampLastChanged[TIME_SIZE];
} IsegItem;
#define EmptyIsegItem { "" ,"" , "", "", ""}

typedef struct IsegItemProperty {
	char object[FULLY_QUALIFIED_OBJECT_SIZE];	//line.module.channel.item  0.0.1.VoltageSet
	char type[DATA_TYPE_SIZE];	// "R4", "UI4", "STR", "BOOL"
	char unit[UNIT_SIZE];		// "V", "A", "C", "%", "%/s", "s"
	char access[ACCESS_SIZE];	// "R", "W", "RW"
	char quality[QUALITY_SIZE];
} IsegItemProperty;
#define EmptyIsegItemProperty { "", "", "", "", "" }

#define TIME_STAMP 1
#define TIME_OUT_ECH4XA_BACK_PLANE 2000
#define TIME_OUT_MODULE_LOG_ON 3000

#define LOG_PATH "/tmp"

#endif // ISEG_COMMON_H

