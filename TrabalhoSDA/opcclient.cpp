#include <atlbase.h>    // required for using the "_T" macro
#include <iostream>
#include <ObjIdl.h>

#include "opcda.h"
#include "opcerror.h"
#include "opcclient.h"
#include "SOCAdviseSink.h"
#include "SOCDataCallback.h"
#include "SOCWrapperFunctions.h"

UINT OPC_DATA_TIME = RegisterClipboardFormat(_T("OPCSTMFORMATDATATIME"));