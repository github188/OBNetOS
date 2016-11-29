/*
 *  rc_ignition.h
 *
 *  This is a part of a RapidControl SDK source code library.
 *
 *  Copyright (C) 2001 WindRiver Systems.
 *  All rights reserved.
 *  Version 3.30
 *
 */


/* WARNING:  This file is generated by a RapidControl Integration
 * Tool.  Any changes made to this file may be overwritten by
 * subsequent uses of the tool. */

#ifndef __IGNITION_HEADER__
#define __IGNITION_HEADER__


/* Access Options Structure */ 
extern DTTypeInfo mAccessInfo;
#define ENUM_ACCESS_ENABLE             (1 << __RLI_ACCESS_LEVEL_SHIFT__)
#define ENUM_ACCESS_CONFIG             (ENUM_ACCESS_ENABLE << 1)
#define ENUM_ACCESS_SUPER              (ENUM_ACCESS_CONFIG << 1)

/*-----------------------------------------------------------------------*/

#ifdef __SNMP_API_ENABLED__
#ifdef __RLI_MIB_TRANSLATION__

extern RLI_SnmpEntry    gRLI_SnmpTbl[];
extern Length           gRLI_SnmpTblSize;

#endif /* __RLI_MIB_TRANSLATION__ */
#endif /* __SNMP_API_ENABLED__ */

/*-----------------------------------------------------------------------*/

extern void Ignite_Database(void);

#ifdef __OCP_ENABLED__
extern void Ignite_Groups();
#endif /* __OCP_ENABLED__ */


#endif /* __IGNITION_HEADER__ */

