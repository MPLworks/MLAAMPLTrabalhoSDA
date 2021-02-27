#ifndef OPC_CLIENT_H
#define OPC_CLIENT_H

/*

* Modifcação do 'SimpleOPCClient_v3.cpp' diponibilizado pelo 
professor Luiz Themystokliz (based on the KEPWARE´s sample client code).

*/

#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <thread>
#include <atlbase.h>    // required for using the "_T" macro
#include <iostream>
#include <ObjIdl.h>

#include "opcda.h"
#include "opcerror.h"
#include "SOCAdviseSink.h"
#include "SOCDataCallback.h"
#include "SOCWrapperFunctions.h"

using namespace std;

#define OPC_SERVER_NAME L"Matrikon.OPC.Simulation.1"
#define VT VT_R4

#pragma comment (lib, "Ws2_32.lib")


IOPCServer* InstantiateServer(wchar_t ServerName[]);
void AddTheGroup(IOPCServer* pIOPCServer, IOPCItemMgt*& pIOPCItemMgt,OPCHANDLE& hServerGroup);
void AddTheItem(IOPCItemMgt* pIOPCItemMgt, OPCHANDLE& hServerItem, int index);
void ReadItem(IUnknown* pGroupIUnknown, OPCHANDLE hServerItem, VARIANT& varValue);
void RemoveItem(IOPCItemMgt* pIOPCItemMgt, OPCHANDLE hServerItem);
void RemoveGroup(IOPCServer* pIOPCServer, OPCHANDLE hServerGroup);
void WriteItem(IUnknown* pGroupIUnknown, DWORD dwCount, OPCHANDLE hServerItem, VARIANT& varValue);
void opcClient(void);
#endif