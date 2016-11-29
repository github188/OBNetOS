/*  
 *  rc_skt_prop.c
 *
 *  This is a part of the OpenControl SDK source code library. 
 *
 *  Copyright (C) 1998 Rapid Logic, Inc.
 *  All rights reserved.
 *
 */

/*----------------------------------------------------------------------
 *
 * NAME CHANGE NOTICE:
 *
 * On May 11th, 1999, Rapid Logic changed its corporate naming scheme.
 * The changes are as follows:
 *
 *      OLD NAME                        NEW NAME
 *
 *      OpenControl                     RapidControl
 *      WebControl                      RapidControl for Web
 *      JavaControl                     RapidControl for Applets
 *      MIBway                          MIBway for RapidControl
 *
 *      OpenControl Backplane (OCB)     RapidControl Backplane (RCB)
 *      OpenControl Protocol (OCP)      RapidControl Protocol (RCP)
 *      MagicMarkup                     RapidMark
 *
 * The source code portion of our product family -- of which this file 
 * is a member -- will fully reflect this new naming scheme in an upcoming
 * release.
 *
 *
 * RapidControl, RapidControl for Web, RapidControl Backplane,
 * RapidControl for Applets, MIBway, RapidControl Protocol, and
 * RapidMark are trademarks of Rapid Logic, Inc.  All rights reserved.
 *
 */

/* This file is designed to serve as a guide to porting our OpenControl
 * Products to your system.
 *
 * To port OpenControl to your system, you must first:
 *
 * 1) Modify the code in rc_prop_os.c.  The code in that file is a 
 * series of wrapper functions to the basic system services required by 
 * OpenControl.  The code in each of the wrapper functions has been taken
 * from rc_posix.c, just to serve as a guide.  However, in addition, each
 * wrapper function has a comment in it describing the specific system service 
 * that must be fulfilled by that call. Possible return values are specified 
 * as well.
 *
 * 2) Modify the code in the file rc_skt_prop.c (this file).  The code in this file  
 * is a connection server to handle packets as they are presented by the socket.
 * The code in that file has been taken from rc_sktposix.c, to serve as a guide.
 * Specific system calls or data structures that need to be modified are 
 * highlighted by comments.  In addition, each function has a comment in
 * it describing the specific service that must be fulfilled by that call, along
 * with possible return values.
 * 
 * 3) Modify the code in rc_os_spec.h that is bracketed by the compiler flag
 * #ifdef __CUSTOMER_SPECIFIC_OS__ 
 *
 * ...C Source Code...
 *
 * #endif 
 *
 * In that code block, you will need to specify the typedefs for
 * OS_SPECIFIC_SOCKET_HANDLE, OS_SPECIFIC_THREAD_HANDLE, and OS_SPECIFIC_MUTEX. 
 *
 * 4) In the Integration Tool, Go to 
 * Build | rc_options.h... | Device Real-Time Operating System | Host OS:
 * and select "Proprietary / Other".  This will cause the compiler flag
 * __CUSTOMER_SPECIFIC_OS__ to be #defined in rc_options.h, thereby 
 * causing the code in this file to be compiled and linked in with your 
 * system image.
 *
 * Note that the Integration Tool allows you to customize the compiler flag
 * __CUSTOMER_SPECIFIC_OS__.  If you do modify the flag, make sure that you 
 * change the flag in rc_prop_os.c, rc_skt_prop.c, and rc_os_spec.h.  Otherwise,
 * the relevant code won't get compiled in.
 * 
 * At that point, this OpenControl Product should be fully ported to your system. 
 */
/*

$History: rc_skt_prop.c $
 * 
 * *****************  Version 22  *****************
 * User: Leech        Date: 6/21/00    Time: 11:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments
 * 
 * *****************  Version 21  *****************
 * User: Epeterson    Date: 4/25/00    Time: 5:22p
 * Include history and enable auto archiving feature from VSS


*/
#include "rc_options.h"

#ifdef  __CUSTOMER_SPECIFIC_OS__

#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_errors.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_msghdlr.h"
#include "rc_socks.h"
#include "rc_environ.h"
#include "rc_database.h"

#ifdef __OCP_ENABLED__
#include "rca_ocp.h"
#include "rca_ocpd.h"
#endif	/* __OCP_ENABLED__ */

#include "rc_access.h"

/* Place whatever necessary header files and #defines for your host environment 
   here.  The directives below are taken from rc_sktposix.c */

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <thread.h>
#include <errno.h>

/* in order to build correctly you will need to link in the following libraries (POSIX)
 * pthread
 * xnet
 * posix4
 */

/* Change this constant to the equivalent for your system (ANY_PORT should direct the underlying socket
 * bind call to bind the socket handle to any available port */
#define     kANY_PORT                       0


ProcessComChannel g_pccObjects[kHTTPD_QUEUE_SIZE];


/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_Initialize( void )
{
    /* Generally most systems will not require this call (Win32 is an exception).
     * necessary, place any socket initialization calls in here */
    return OK;
}



/*-----------------------------------------------------------------------*/

static OS_SPECIFIC_SOCKET_HANDLE SOCKET_TcpCreate()
{
    /* This function must provide a handle to a TCP socket.  In most cases you
     * will need to change the socket() call below and change the if statement
     * to check for the appropriate error value for your system.
     *
     * The return value of this wrapper must be OK if the system call
     * completed successfully, and SYS_ERROR_SOCKET_CREATE if there
     * was a problem.
     */

    OS_SPECIFIC_SOCKET_HANDLE   soc = socket(AF_INET, SOCK_STREAM, 0);
    char*                       pMsg;

    if (-1 == soc)
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_CREATE, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
    }

    return soc;
}



/*-----------------------------------------------------------------------*/

static OS_SPECIFIC_DGRAM_HANDLE SOCKET_UdpCreate()
{
    /* This function must provide a handle to a UDP socket.  In most cases you
     * will need to change the socket() call below and change the if statement
     * to check for the appropriate error value for your system.
     *
     * The return value of this wrapper must be OK if the system call
     * completed successfully, and SYS_ERROR_SOCKET_CREATE if there
     * was a problem.
     */

    OS_SPECIFIC_DGRAM_HANDLE   soc = socket(AF_INET, SOCK_DGRAM, 0);
    char*                      pMsg;

    if (-1 == soc)
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_CREATE, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
    }

    return soc;
}


/*-----------------------------------------------------------------------*/

static RLSTATUS BindSocketToAddr(OS_SPECIFIC_SOCKET_HANDLE soc, ubyte4 port)
{
    /* This function must bind a valid IP network socket to an acceptable device
     * IP address and UDP/TCP port.  The INADDR_ANY constant may need to be 
     * replaced as well.

     * The return value of this wrapper must be OK if the system call
     * completed successfully, and ERROR_GENERAL_ACCESS_DENIED or 
     * SYS_ERROR_SOCKET_BIND if there is a problem.
     */

    struct sockaddr_in  LocalAddr;
    RLSTATUS            status;
    u_long              ulHostAddress;
    char                *pMsg;

    /* ask the system to allocate a port and bind to INADDR_ANY */
    /* get system to allocate a port number by binding a host address */
    LocalAddr.sin_family        = AF_INET;
    LocalAddr.sin_addr.s_addr   = htonl(INADDR_ANY);
    LocalAddr.sin_port          = htons((u_short)port);

    if (0 != bind(soc, (struct sockaddr *)&LocalAddr, sizeof(LocalAddr)))
    {
        printf ("Bind failed w/ %d\n",errno);
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_BIND, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
        close(soc);

        status = ERROR_GENERAL_ACCESS_DENIED;
    }
    else
        status = OK;

    return status;
}



/*-----------------------------------------------------------------------*/

static RLSTATUS 
SOCKET_UdpBind(OS_SPECIFIC_DGRAM_HANDLE *pSoc, ubyte4 port)
{
    /* no modifications needed here */
    RLSTATUS status;

    /* Create a stream socket */
    *pSoc = SOCKET_UdpCreate();
    if (-1 == *pSoc)
        return ERROR_GENERAL_ACCESS_DENIED;

    /* N.B. Since OS_SPECIFIC_DGRAM_HANDLE and OS_SPECIFIC_SOCKET_HANDLE
     * are typedef'ed to the same value, we'll go ahead and reuse the 
     * function BindSocketToAddr.  However, in the future, if Windows changes
     * their general DGRAM/SOCKET API, we'll need to reinvestigate this function 
     */
    status = BindSocketToAddr(*pSoc, port);
    if (OK != status)
        return ERROR_GENERAL_ACCESS_DENIED;

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS 
SOCKET_UdpBindFixed(OS_SPECIFIC_DGRAM_HANDLE *pSoc, ubyte4 port)
{
    /* no modifications needed here */
    return SOCKET_UdpBind(pSoc, port);
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS 
SOCKET_UdpBindAny(OS_SPECIFIC_DGRAM_HANDLE *pSoc)
{
    /* no modifications needed here */
    return SOCKET_UdpBind(pSoc, kANY_PORT);
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_OpenServer(OS_SPECIFIC_SOCKET_HANDLE *pSoc, ubyte4 port)
{
    /* This function opens a network socket and sets it to listen for 
     * incoming server requests.  The listen() should be modified to 
     * correspond to the appropriate call on the host system 
     *
     * The return value of this wrapper must be OK if the system call
     * completed successfully, and ERROR_GENERAL_ACCESS_DENIED 
     * if there is a problem.
     */

    RLSTATUS status;
    char    *pMsg;

    SOCKET_Initialize();

    /* Create a stream socket */
    *pSoc = SOCKET_TcpCreate();

    if (-1 == *pSoc)
        return ERROR_GENERAL_ACCESS_DENIED;

    status = BindSocketToAddr(*pSoc, port);
    if (OK != status)
        return ERROR_GENERAL_ACCESS_DENIED;

    /* Create a incoming Client connect request queue */
    if (0 != listen(*pSoc, kTCP_LISTEN_BACKLOG))
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_LISTEN, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
        close(*pSoc);

        return ERROR_GENERAL_ACCESS_DENIED;
    }

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS
SOCKET_UdpSendTo(OS_SPECIFIC_DGRAM_HANDLE sock, UdpParams *pParams)
{
    /* This function directs packet data to be sent to a valid UDP socket.  
     * You will need to map the sendto() call and the address struct 
     * to the equivalents on your system.
     *
     * The return value of this wrapper must be OK if the system call
     * completed successfully (negative otherwise)
     */

    struct sockaddr_in *pAddrTo;

    if ( NULL == (pAddrTo = (struct sockaddr_in *) RC_MALLOC(sizeof(struct sockaddr_in))) )
        return ERROR_MEMMGR_NO_MEMORY;

    pAddrTo->sin_family = AF_INET;
    pAddrTo->sin_addr.S_un.S_addr = pParams->clientAddr;
    pAddrTo->sin_port = HTON2(pParams->clientPort);
    
    if (-1 == sendto(sock, pParams->pSendPacket, 
                                pParams->sendPacketLength, 0, 
                                (struct sockaddr *)pAddrTo, 
                                  sizeof(struct sockaddr)))
    {
        RC_FREE(pAddrTo);
        return SYS_ERROR_SOCKET_GENERAL;
    }

    RC_FREE(pAddrTo);
    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS 
SOCKET_UdpReceive(OS_SPECIFIC_DGRAM_HANDLE sock, UdpParams *pParams)
{
    /* This function reads packet data from a valid UDP socket.  
     * You will need to map the recvfrom() call and the address struct 
     * to the equivalents on your system.
     *
     * The return value of this wrapper must be OK if the system call
     * completed successfully (negative otherwise)
     */

    struct sockaddr_in *pAddr;
    sbyte4 iAddrLen;
    sbyte4 RecvResult;

    iAddrLen = sizeof(struct sockaddr_in);
    if ( NULL == (pAddr = (struct sockaddr_in *) RC_MALLOC(iAddrLen)) )
        return ERROR_MEMMGR_NO_MEMORY;

    RecvResult = recvfrom(sock, pParams->pRecPacket, kMaxUdpPacketSize, 
                                    0, (struct sockaddr *)pAddr, &iAddrLen );
    if (-1 == RecvResult)
	{
        RC_FREE(pAddr);
		return ERROR_GENERAL;
	}

    pParams->clientAddr = pAddr->sin_addr.S_un.S_addr;
    pParams->clientPort = NTOH2(pAddr->sin_port);
    pParams->recvPacketLength = (ubyte4)RecvResult;

    RC_FREE(pAddr);

#ifdef __ENABLE_LAN_IP_FILTER__
	if (TRUE != ACCESS_ValidIpAddress(pParams->clientAddr))
		return ERROR_GENERAL;
#endif

	return OK;
}



/*-----------------------------------------------------------------------*/

#if defined( __MULTI_THREADED_SERVER_ENABLED__) || defined(__OCP_MULTI_THREADED_SERVER_ENABLED__)

static void * PersistentConnectionHandler(void *pccObject)
{
    /* This function serves as the entry point for the worker threads 
     * spawned by the main WebControl task. You will need to map the
     * pthread_exit() call to your system equivalent for ending a 
     * thread gracefully. 
     */

    RLSTATUS status = 0;
#ifdef __RLI_DEBUG_SOCK__
    printf("PersistentConnectionHandler: Thread start.\n");
#endif

    /* process the connection */
	((ProcessComChannel *)pccObject)->pfuncConnHandler(pccObject); 

#ifdef __RLI_DEBUG_SOCK__
    printf("PersistentConnectionHandler: closing socket(threadState=%d)!!!!!!!!!!!!!\n", ((ProcessComChannel *)pccObject)->ThreadState);
#endif

    /* close down the socket */
    close(((ProcessComChannel *)pccObject)->sock);

    /* mark the com channel as free */
    ((ProcessComChannel *)pccObject)->InUse = FALSE;

#ifdef __RLI_DEBUG_SOCK__
    printf("PersistentConnectionHandler: Thread end!!!!!!!!!!!!!!!!\n");
#endif

    pthread_exit(&status);
}

#endif



/*-----------------------------------------------------------------------*/

extern RLSTATUS SOCKET_Accept(OS_SPECIFIC_SOCKET_HANDLE *soc, 
                              OS_SPECIFIC_SOCKET_HANDLE *accSoc,
                              ubyte4 port)
{
    /* This function accepts incoming requests to the server on a socket
     * that has been set-up with a listen() call (see SOCKET_OpenServer).
     *
     * Items to replace/modify:
     * accept()
     * close()
     * struct sockaddr_in
     *
     * This function should return OK if the accept was successful or
     * SYS_ERROR_SOCKET_ACCEPT otherwise.
     */

    struct sockaddr_in ClientAddr;
    int     iAddrLen;
    char*   pMsg;

    iAddrLen = sizeof(ClientAddr);
    memset(&ClientAddr, 0x00, iAddrLen);

    *accSoc = accept(*soc, (struct sockaddr *)&ClientAddr, &iAddrLen);

    if (-1 == *accSoc)
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_ACCEPT, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);

        close(*accSoc);
        close(*soc);

        SOCKET_OpenServer(soc, port);
        return SYS_ERROR_SOCKET_ACCEPT;
    }

    return OK;
}



/*-----------------------------------------------------------------------*/

#ifdef __MASTER_SLAVE_SNMP_STACK__

extern RLSTATUS SOCKET_CreateMasterInterface()
{
    /* Please contact Rapid Logic for futher information if this feature is necessary. */
    return ERROR_GENERAL_NOT_FOUND;        
}

#endif



/*-----------------------------------------------------------------------*/

extern ubyte4 SOCKET_GetClientsAddr(OS_SPECIFIC_SOCKET_HANDLE ClientSock)
{
    /* this function extracts the client's IP address from a connected socket.
     *
     * Items to replace/modify:
     * getpeername()
     * the IP address type (ubyte4 in this example -
     * struct sockaddr_in

     * This function should return the IP address of the client as an unsigned
     * 4 bytes.
     */

    struct sockaddr_in  ClientAddr;
    int                 iAddrLen;
    ubyte4              clientIpAddr;

    iAddrLen     = sizeof(ClientAddr);
    clientIpAddr = 0;

    if (OK == getpeername(ClientSock, (struct sockaddr *)&ClientAddr, &iAddrLen))
        clientIpAddr = ClientAddr.sin_addr.s_addr;

    return clientIpAddr;
}



/*-----------------------------------------------------------------------*/

#if defined( __MULTI_THREADED_SERVER_ENABLED__) || defined(__OCP_MULTI_THREADED_SERVER_ENABLED__)

extern RLSTATUS SOCKET_CreateTask(ProcessComChannel* pPCC)
{
    /* This function spawns the worker threads to respond to HTTP requests 
     * from browsers.  
     *    
     * Items to replace/modify:
     * Thread ID type (pthread_t in this example)
     * pthread_create()
     * The check of the return value of the thread create function.
     *
     * This function should return OK if successful, SYS_ERROR_SOCKET_CREATE_TASK 
     * otherwise
     */

    pthread_t threadID;
    
    int Result = pthread_create(&threadID, NULL, PersistentConnectionHandler, pPCC);
    
    if (0 != Result)
    {
	    return SYS_ERROR_SOCKET_CREATE_TASK;
    }
    return OK;
}

#endif



/*-----------------------------------------------------------------------*/

extern RLSTATUS
SOCKET_Close( OS_SPECIFIC_SOCKET_HANDLE sock )
{
    /* This function closes a network socket.
     *
     * Items to replace/modify:
     * close()
     * The check of the return value of the close() function.
     *
     * This function should return OK if successful, ERROR_GENERAL_ACCESS_DENIED 
     * otherwise
     */

  RLSTATUS err;

    err = close(sock);
    if (-1 == err )
        return ERROR_GENERAL_ACCESS_DENIED;

    else
        return OK;
}



/*-----------------------------------------------------------------------*/

static RLSTATUS
SOCKET_ConnectToServer(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pName, ubyte4 port )
{
    /* This function connects a network socket to a remote IP address.  This is used
     * by JavaControl and SMTP to connect a socket to a Java broker running on a remote
     * machine and a remote SMTP server respectively.
     *
     * Items to replace/modify:
     * connect()
     * struct sockaddr_in
     */

    RLSTATUS            status;
    struct sockaddr_in  RemAddr;
    u_long              ulHostAddress;
    int                 err;
    sbyte               *pMsg;

    MEMSET(&RemAddr, 0x00, sizeof(RemAddr));

    status = CONVERT_StrTo(pName, &ulHostAddress, kDTipaddress);

    if ( OK > status )
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_CONNECT, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);

        return ERROR_GENERAL_ACCESS_DENIED;
    }

    RemAddr.sin_family      = AF_INET;
    RemAddr.sin_addr.s_addr = ulHostAddress;
    RemAddr.sin_port        = htons((u_short)port);

    /* Create a Client connection */
    err = connect( sock, (struct sockaddr *)&RemAddr, sizeof(RemAddr));
    if ( -1 == err )
    {
        MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_CONNECT, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);

        return ERROR_GENERAL_ACCESS_DENIED;
    }

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS
SOCKET_Connect(OS_SPECIFIC_SOCKET_HANDLE *pSoc, sbyte *pName, ubyte4 port )
{
    /* no modifications needed here */
    RLSTATUS status;

    SOCKET_Initialize();

    /* Create a stream socket */
    *pSoc = SOCKET_TcpCreate();
    if (-1 == *pSoc)
        return ERROR_GENERAL_ACCESS_DENIED;

    status = BindSocketToAddr(*pSoc, kANY_PORT);
    if (OK != status)
        return ERROR_GENERAL_ACCESS_DENIED;

    /* Create a Client connection */
    status = SOCKET_ConnectToServer( *pSoc, pName, port );

    return status;
}

#endif /* __CUSTOMER_SPECIFIC_OS__ */
