/*===========================================================================
FILE:
	QMI.h

DESCRIPTION:
	Qualcomm QMI driver header

FUNCTIONS:
	Generic QMUX functions
		ParseQMUX
		FillQMUX

	Generic QMI functions
		GetTLV
		ValidQMIMessage
		GetQMIMessageID

	Get sizes of buffers needed by QMI requests
		QMUXHeaderSize
		QMICTLGetClientIDReqSize
		QMICTLReleaseClientIDReqSize
		QMICTLReadyReqSize
		QMIWDSSetEventReportReqSize
		QMIWDSGetPKGSRVCStatusReqSize
		QMIDMSGetMEIDReqSize
		QMICTLSyncReqSize

	Fill Buffers with QMI requests
		QMICTLGetClientIDReq
		QMICTLReleaseClientIDReq
		QMICTLReadyReq
		QMIWDSSetEventReportReq
		QMIWDSGetPKGSRVCStatusReq
		QMIDMSGetMEIDReq
		QMICTLSetDataFormatReq
		QMICTLSyncReq

	Parse data from QMI responses
		QMICTLGetClientIDResp
		QMICTLReleaseClientIDResp
		QMIWDSEventResp
		QMIDMSGetMEIDResp

Copyright (c) 2011, Code Aurora Forum. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
	* Redistributions of source code must retain the above copyright
		notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.
	* Neither the name of Code Aurora Forum nor
		the names of its contributors may be used to endorse or promote
		products derived from this software without specific prior written
		permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
===========================================================================*/

#pragma once

/*=========================================================================*/
// Definitions
/*=========================================================================*/

extern int quec_debug;
// DBG macro
#define DBG( format, arg... ) do { \
	if (quec_debug == 1) { \
		printk(KERN_INFO "GobiNet::%s " format, __FUNCTION__, ## arg); \
	} } while (0)

#if 0
#define VDBG( format, arg... ) do { \
	if (debug == 1) { \
		printk(KERN_INFO "GobiNet::%s " format, __FUNCTION__, ## arg); \
	} } while (0)
#else
#define VDBG(format, arg...) do { } while (0)
#endif

#define INFO(format, arg...) do { \
		printk(KERN_INFO "GobiNet::%s " format, __FUNCTION__, ## arg); \
	} while (0)

// QMI Service Types
#define QMICTL 0
#define QMIWDS 1
#define QMIDMS 2
#define QMINAS 3
#define QMIUIM 11
#define QMIWDA 0x1A

#define u8        unsigned char
#define u16       unsigned short
#define u32       unsigned int
#define u64       unsigned long long

#define bool      u8
#define true      1
#define false     0

#define ENOMEM    12
#define EFAULT    14
#define EINVAL    22
#ifndef ENOMSG
#define ENOMSG    42
#endif
#define ENODATA   61

#define TLV_TYPE_LINK_PROTO 0x10

/*=========================================================================*/
// Struct sQMUX
//
//    Structure that defines a QMUX header
/*=========================================================================*/
typedef struct sQMUX {
	/* T\F, always 1 */
	u8  mTF;
	/* Size of message */
	u16 mLength;
	/* Control flag */
	u8  mCtrlFlag;
	/* Service Type */
	u8  mQMIService;
	/* Client ID */
	u8  mQMIClientID;

} __attribute__((__packed__)) sQMUX;
