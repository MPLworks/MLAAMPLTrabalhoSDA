#include <atlbase.h>    // required for using the "_T" macro
#include <iostream>
#include <ObjIdl.h>

#include "opcda.h"
#include "opcerror.h"
#include "opcclient.h"
#include "SOCAdviseSink.h"
#include "SOCDataCallback.h"
#include "SOCWrapperFunctions.h"

#define VT VT_R4
wchar_t ITEM_ID[] = L"Saw-toothed Waves.Real4";

wchar_t ServerName[] = L"Matrikon.OPC.Simulation.1";
UINT OPC_DATA_TIME = RegisterClipboardFormat(_T("OPCSTMFORMATDATATIME"));


wchar_t ITEM_ID0[] = L"Random.UInt1";
wchar_t ITEM_ID1[] = L"Random.Real";
wchar_t ITEM_ID2[] = L"Saw-toothed Waves.Real4";
wchar_t ITEM_ID3[] = L"Square Waves.Real4";
wchar_t ITEM_ID4[] = L"Bucket Brigade.Real4";
wchar_t ITEM_ID5[] = L"Bucket Brigade.UInt1";
wchar_t ITEM_ID6[] = L"Bucket Brigade.UInt2";
wchar_t ITEM_ID7[] = L"Bucket Brigade.UInt4";
wchar_t ITEM_ID8[] = L"Bucket Brigade.Real8";

#define VT0 VT_UI1
#define VT1 VT_R4
#define VT2 VT_R4
#define VT3 VT_R8
#define VT4 VT_R4
#define VT5 VT_UI1
#define VT6 VT_UI2
#define VT7 VT_UI4
#define VT8 VT_R8


/*
wchar_t* ITEM_IDS[9] = { (LPWSTR)"Random.UInt1",(LPWSTR)"Random.Real",
(LPWSTR)"Saw-toothed Waves.Real4",(LPWSTR)"Square Waves.Real4",(LPWSTR)"Bucket Brigade.Real4",
(LPWSTR)"Bucket Brigade.UInt1",(LPWSTR)"Bucket Brigade.UInt2",(LPWSTR)"Bucket Brigade.UInt4",
(LPWSTR)"Bucket Brigade.Real8" };

int TypesVT[9] = { VT_UI1,VT_R4,VT_R4,VT_R4,VT_R4,VT_UI1,VT_UI2,VT_UI4,VT_R8 };*/


IOPCServer* InstantiateServer(wchar_t ServerName[])
{
	CLSID CLSID_OPCServer;
	HRESULT hr;

	// get the CLSID from the OPC Server Name:
	hr = CLSIDFromString(ServerName, &CLSID_OPCServer);
	_ASSERT(!FAILED(hr));

	//queue of the class instances to create
	LONG cmq = 1; // nbr of class instance to create.
	MULTI_QI queue[1] =
	{ {&IID_IOPCServer,
	NULL,
	0} };

	//Server info:
//COSERVERINFO CoServerInfo =
//{
//	/*dwReserved1*/ 0,
//	/*pwszName*/ REMOTE_SERVER_NAME,
//	/*COAUTHINFO*/  NULL,
//	/*dwReserved2*/ 0
//}; 


	// create an instance of the IOPCServer
	hr = CoCreateInstanceEx(CLSID_OPCServer, NULL, CLSCTX_SERVER,
		/*&CoServerInfo*/NULL, cmq, queue);
	_ASSERT(!hr);

	// return a pointer to the IOPCServer interface:
	return(IOPCServer*)queue[0].pItf;
}

void AddTheGroup(IOPCServer* pIOPCServer, IOPCItemMgt*& pIOPCItemMgt,OPCHANDLE& hServerGroup)
{
	DWORD dwUpdateRate = 0;
	OPCHANDLE hClientGroup = 0;

	// Add an OPC group and get a pointer to the IUnknown I/F:
	HRESULT hr = pIOPCServer->AddGroup(/*szName*/ L"Group1",
		/*bActive*/ FALSE,
		/*dwRequestedUpdateRate*/ 1000,
		/*hClientGroup*/ hClientGroup,
		/*pTimeBias*/ 0,
		/*pPercentDeadband*/ 0,
		/*dwLCID*/0,
		/*phServerGroup*/&hServerGroup,
		&dwUpdateRate,
		/*riid*/ IID_IOPCItemMgt,
		/*ppUnk*/ (IUnknown**)&pIOPCItemMgt);
	_ASSERT(!FAILED(hr));
}

void AddTheItem(IOPCItemMgt* pIOPCItemMgt, OPCHANDLE& hServerItem,int indice, LPWSTR* vec, int* vec2){
	HRESULT hr;

	// Array of items to add:

		OPCITEMDEF ItemArray[1] =
	{ {
			/*szAccessPath*/ (LPWSTR)"",
			/*szItemID*/ vec[indice],
			/*bActive*/ TRUE,
			/*hClient*/ indice,
			/*dwBlobSize*/ 0,
			/*pBlob*/ NULL,
			/*vtRequestedDataType*/ vec2[indice],
			/*wReserved*/0
			} };

	//Add Result:
	OPCITEMRESULT* pAddResult = NULL;
	HRESULT* pErrors = NULL;

	// Add an Item to the previous Group:
	hr = pIOPCItemMgt->AddItems(1, ItemArray, &pAddResult, &pErrors);
	if (hr != S_OK) {
		printf("Failed call to AddItems function. Error code = %x\n", hr);
		exit(0);
	}

	// Server handle for the added item:
	hServerItem = pAddResult[0].hServer;

	// release memory allocated by the server:
	CoTaskMemFree(pAddResult->pBlob);

	CoTaskMemFree(pAddResult);
	pAddResult = NULL;

	CoTaskMemFree(pErrors);
	pErrors = NULL;
}

void ReadItem(IUnknown* pGroupIUnknown, OPCHANDLE hServerItem, VARIANT& varValue)
{
	// value of the item:
	OPCITEMSTATE* pValue = NULL;

	//get a pointer to the IOPCSyncIOInterface:
	IOPCSyncIO* pIOPCSyncIO;
	pGroupIUnknown->QueryInterface(__uuidof(pIOPCSyncIO), (void**)&pIOPCSyncIO);

	// read the item value from the device:
	HRESULT* pErrors = NULL; //to store error code(s)
	HRESULT hr = pIOPCSyncIO->Read(OPC_DS_DEVICE, 1, &hServerItem, &pValue, &pErrors);
	_ASSERT(!hr);
	_ASSERT(pValue != NULL);

	varValue = pValue[0].vDataValue;

	//Release memeory allocated by the OPC server:
	CoTaskMemFree(pErrors);
	pErrors = NULL;

	CoTaskMemFree(pValue);
	pValue = NULL;

	// release the reference to the IOPCSyncIO interface:
	pIOPCSyncIO->Release();
}

void RemoveItem(IOPCItemMgt* pIOPCItemMgt, OPCHANDLE hServerItem)
{
	// server handle of items to remove:
	OPCHANDLE hServerArray[1];
	hServerArray[0] = hServerItem;

	//Remove the item:
	HRESULT* pErrors; // to store error code(s)
	HRESULT hr = pIOPCItemMgt->RemoveItems(1, hServerArray, &pErrors);
	_ASSERT(!hr);

	//release memory allocated by the server:
	CoTaskMemFree(pErrors);
	pErrors = NULL;
}

void RemoveGroup(IOPCServer* pIOPCServer, OPCHANDLE hServerGroup)
{
	// Remove the group:
	HRESULT hr = pIOPCServer->RemoveGroup(hServerGroup, FALSE);
	if (hr != S_OK) {
		if (hr == OPC_S_INUSE)
			printf("Failed to remove OPC group: object still has references to it.\n");
		else printf("Failed to remove OPC group. Error code = %x\n", hr);
		exit(0);
	}
}

void WriteItem(IUnknown* pGroupIUnknown, DWORD Count, OPCHANDLE hServerItem, VARIANT& varValue)
{
	// Recebe o ponteiro IOPCSyncIO
	IOPCSyncIO* pIOPCSyncIO;
	pGroupIUnknown->QueryInterface(__uuidof(pIOPCSyncIO), (void**)&pIOPCSyncIO);

	// Escreve o item => processo similar ao de leitura 
	HRESULT* pErrors = NULL; //to store error code(s)
	HRESULT hr = pIOPCSyncIO->Write(Count, &hServerItem, &varValue, &pErrors);
	_ASSERT(!hr);

	CoTaskMemFree(pErrors);
	pErrors = NULL;

	// Libera o ponteiro;
	pIOPCSyncIO->Release();
}


void opcClient(void) {
	IOPCServer* pIOPCServer = NULL;   //pointer to IOPServer interface
	IOPCItemMgt* pIOPCItemMgt = NULL; //pointer to IOPCItemMgt interface
	OPCHANDLE hServerItem[9]; // server handle to the group
	OPCHANDLE hServerGroup; // server handle to the group
	

	int i;
	char buf[100];

	// Have to be done before using microsoft COM library:
	printf("Initializing the COM environment...\n");
	CoInitialize(NULL);

	// Let's instantiante the IOPCServer interface and get a pointer of it:
	printf("Instantiating the MATRIKON OPC Server for Simulation...\n");
	pIOPCServer = InstantiateServer(ServerName);

	// Testa a Coneção
	if (pIOPCServer != NULL) {
		cout << " Conexao com Matrikon Realizada!\n";
	}
	else {
		cout << " Erro na Conexão com Matrikon!\n";
	}

	// Add the OPC group the OPC server and get an handle to the IOPCItemMgt
	//interface:
	printf("Adding a group in the INACTIVE state for the moment...\n");
	AddTheGroup(pIOPCServer, pIOPCItemMgt, hServerGroup);

	// Add the OPC item. First we have to convert from wchar_t* to char*
	// in order to print the item name in the console.

	int vec2[9] = { VT0,VT1,VT2,VT3,VT4,VT5,VT6,VT7,VT8};
	LPWSTR vec[9] = { ITEM_ID0,ITEM_ID1, ITEM_ID2, ITEM_ID3, ITEM_ID4, ITEM_ID5, ITEM_ID6, ITEM_ID7, ITEM_ID8};

	for (i = 0; i < 9; i++) {
		printf("ADDing the OPC item %i...\n",i);
		AddTheItem(pIOPCItemMgt, hServerItem[i], i, vec, vec2);
	}


	/* ordem dos itens
	
	 0 -  Taxa de recuperação de minério (kg/min)         - Random.UInt1             - leitura
	 1 -  Potência atual consumida (kW)                   - Random.Real              - leitura
	 2 -  Temperatura motor de translação (C)             - Saw-toothed Waves.Real4  - leitura 
	 3 -  Temperatura motor da roda de caçambas (C)       - Square Waves.Real4       - leitura
	 4 -  Velocidade de translação (cm/s)                 - Bucket Brigade.Real4     - Escrita
	 5 -  Coordenada espacial X do ponto de ataque (cm)   - Bucket Brigade.UInt1     - Escrita
	 6 -  Coordenada espacial Y do ponto de ataque (cm)   - Bucket Brigade.UInt2     - Escrita
	 7 -  Coordenada espacial Z do ponto de ataque (cm)   - Bucket Brigade.UInt4     - Escrita
	 8 -  Taxa de recuperação de minério (kg/min)         - Bucket Brigade.Real8     - Escrita
	*/

    //Leitura:

	//Synchronous read of the device´s item value.

		VARIANT varValueR[4]; //to store the read value
		VariantInit(&varValueR[0]); // 0 -  Taxa de recuperação de minério
		VariantInit(&varValueR[1]); // 1 -  Potência atual consumida (kW) 
		VariantInit(&varValueR[2]); // 2 -  Temperatura motor de translação (C)
		VariantInit(&varValueR[3]); // 3 -  Temperatura motor da roda de caçambas (C)

		ReadItem(pIOPCItemMgt, hServerItem[0], varValueR[0]);
		ReadItem(pIOPCItemMgt, hServerItem[1], varValueR[1]);
		ReadItem(pIOPCItemMgt, hServerItem[2], varValueR[2]);
		ReadItem(pIOPCItemMgt, hServerItem[3], varValueR[3]);

		printf("Read value: %06d\n", varValueR[0].intVal);
		printf("Read value: %06.2f\n", varValueR[1].fltVal);
		printf("Read value: %06.2f\n", varValueR[2].fltVal);
		printf("Read value: %06.2f\n", varValueR[3].fltVal);



	//Escrita:

		VARIANT varValueW[5]; //to store the write value
		VariantInit(&varValueW[0]); //  4 -  Velocidade de translação (cm/s) 
		VariantInit(&varValueW[1]); //  5 -  Coordenada espacial X do ponto de ataque (cm) 
		VariantInit(&varValueW[2]); //  6 -  Coordenada espacial Y do ponto de ataque (cm)
		VariantInit(&varValueW[3]); //  7 -  Coordenada espacial Z do ponto de ataque (cm)
		VariantInit(&varValueW[4]); //  8 -  Taxa de recuperação de minério (kg/min) 


		varValueW[0].fltVal =321.12;
		varValueW[1].intVal = 2;
		varValueW[2].intVal = 3;
		varValueW[3].intVal = 4;
		varValueW[4].fltVal = 35.412;

		varValueW[0].vt = vec2[4];
		varValueW[1].vt = vec2[5];
		varValueW[2].vt = vec2[6];
		varValueW[3].vt = vec2[7];
		varValueW[4].vt = vec2[8];



		WriteItem(pIOPCItemMgt, 1, hServerItem[4], varValueW[0]);
		WriteItem(pIOPCItemMgt, 1, hServerItem[5], varValueW[1]);
		WriteItem(pIOPCItemMgt, 1, hServerItem[6], varValueW[2]);
		WriteItem(pIOPCItemMgt, 1, hServerItem[7], varValueW[3]);
		WriteItem(pIOPCItemMgt, 1, hServerItem[8], varValueW[4]);

		varValueW[0].fltVal = 1;
		varValueW[1].intVal = 10;
		varValueW[2].intVal = 10;
		varValueW[3].intVal = 10;
		varValueW[4].fltVal = 1;

		
		ReadItem(pIOPCItemMgt, hServerItem[4], varValueW[0]);
		ReadItem(pIOPCItemMgt, hServerItem[5], varValueW[1]);
		ReadItem(pIOPCItemMgt, hServerItem[6], varValueW[2]);
		ReadItem(pIOPCItemMgt, hServerItem[7], varValueW[3]);
		ReadItem(pIOPCItemMgt, hServerItem[8], varValueW[4]);
	
		printf("Read value: %06.2f\n", varValueW[0].fltVal);
		printf("Read value: %06d\n",   varValueW[1].intVal);
		printf("Read value: %06d\n",   varValueW[2].intVal);
		printf("Read value: %06d\n",   varValueW[3].intVal);
		printf("Read value: %06.2f\n", varValueW[4].fltVal);








	// Establish a callback asynchronous read by means of the old IAdviseSink()
	// (OPC DA 1.0) method. We first instantiate a new SOCAdviseSink object and
	// adjusts its reference count, and then call a wrapper function to
	// setup the callback.
	IDataObject* pIDataObject = NULL; //pointer to IDataObject interface
	DWORD tkAsyncConnection = 0;
	SOCAdviseSink* pSOCAdviseSink = new SOCAdviseSink();
	pSOCAdviseSink->AddRef();
	printf("Setting up the IAdviseSink callback connection...\n");
	SetAdviseSink(pIOPCItemMgt, pSOCAdviseSink, pIDataObject, &tkAsyncConnection);

	// Change the group to the ACTIVE state so that we can receive the
   // server´s callback notification
	printf("Changing the group state to ACTIVE...\n");
	SetGroupActive(pIOPCItemMgt);

	int bRet;
	MSG msg;
	DWORD ticks1, ticks2;
	ticks1 = GetTickCount();
	printf("Waiting for IAdviseSink callback notifications during 10 seconds...\n");


	// Cancel the callback and release its reference
	printf("Cancelling the IAdviseSink callback...\n");
	CancelAdviseSink(pIDataObject, tkAsyncConnection);
	pSOCAdviseSink->Release();

	// Remove the OPC item:
	for (i = 0; i < 9; i++) {
		printf("Removing the OPC item %i...\n",i);
		RemoveItem(pIOPCItemMgt, hServerItem[i]);
	}

	// Remove the OPC group:
	printf("Removing the OPC group object...\n");
	pIOPCItemMgt->Release();
	RemoveGroup(pIOPCServer, hServerGroup);

	// release the interface references:
	printf("Removing the OPC server object...\n");
	pIOPCServer->Release();

	//close the COM library:
	printf("Releasing the COM environment...\n");
	CoUninitialize();


}