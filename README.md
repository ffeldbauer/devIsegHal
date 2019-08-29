# EPICS Device Support for iseg Hardware Abstraction Layer

## Introduction
The isegHAL library offers a string based application interface - API. The data
collection from the iseg high voltage modules is done in background. All
communication hand shake is handled by the isegHAL.

This module offers EPICS device support routines to use the isegHAL library
within your EPICS applicaitons.

## Build
Set pathes to `EPICS_BASE` and `ISEGHAL` in `configure/RELEASE.local`.

If the variable `CHECK_TIMESTAMPS` is defined in `configure/RELEASE.local`
the device support will check if the isegHAL has updated the corresponding value
within the last 30 seconds. If not the record is set to a TIMEOUT_ALARM.

## Usage
### Connect to an interface with the isegHalServer
Before loading any records, devIsegHal has to connect to an interface
with the isegHalServer daemon. This is done via the IOC shell command
```
isegHalConnect( "NAME", "INTERFACE" )
```
`NAME` is a user defined name, which is internally used to address this interface
while `INTERFACE` is the actual name of the hardware interface from your operating system
(e.g. "can0" for a CAN interface)

### Records
To make a record use devIsegHal, set its `DTYP` field to "isegHAL".
The `INP` or `OUT` link has the form "@OBJECT IF".
Here OBJECT is a fully qualified object string for the item values
provided by the isegHalServer and IF is the name of the interface as used with the 
`isegHalConnect` command mentioned above.

isegHAL provides its own timestamp of the last change of a value (`timeStampLastChanged`).
To use this timestamp as timestamp of the record, the `TSE` field has to be set to `-2`

Example:
```
record( ai, "ISEG:0:0:2:VoltageMeasure" ) {
  field( DTYP, "isegHAL" )
  field( INP,  "@0.0.2.VoltageMeasure can0" )
  field( TSE,  "-2" )
}
```
If the `EGU` field is not set in the database, the unit-value from the
corresponding IsegItemProperty is copied into this field during initialization.

## Asynchronous Handling
It is possible that control parameters change during operation. For example, if a trip occures
the corresponding `setON` bit in the channel control register will be set to 0.
These changes are monitored by devIsegHal through a polling thread.
Each output record (execpt for stringout records) is automatically registered to this thread
and their values are checked for updates on the isegHAL. If a value has changed
the `VAL` field and timestamp of the record will be set to the new values.

Setting the `SCAN` field of input records to `I/O Intr` will also
register these records for the thread monitoring the values in isegHAL.

The thread goes through the list of registered records, checks each for an update, and
then waits for 5 seconds. This waiting time can be modified using the IOC Shell Commands

## Supported Record Types

| Record type                | isegDataType |
| -------------------------- |:------------:|
| ai/ao records              | R4           |
| bi/bo records              | BOOL         |
| mbbiDirect records         | UI1 & UI4    |
| longin/longout records     | UI1 & UI4    |
| stringin/stringout records | STR          |

*Note: the maximum string length for stringin/out records is limited to 40 characters while the maximal length for the value of an IsegItemValue is 200.
Thus only the first 39 characters of the IsegItemValue are copied to record's VAL field (plus Null-Character for string termination).*

## IOC Shell Commands
Currently there is only on command callable from the IOC shell.
```
devIsegHalSetOpt( "key", "value" )
```

| Key       | Meaning                                    | Value                                                          |
| --------- | ------------------------------------------ |:--------------------------------------------------------------:|
| Intervall | Change the intervall of the polling thread | a value of 0 means no pause between two iterations of the list |
| LogLevel  | Change log level of isegHalServer          | see isegHal Manual                                             |


