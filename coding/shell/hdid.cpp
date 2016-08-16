// -----------------------------------------------------------------------
//
// 文 件 名 ：hdid.cpp
// 创 建 者 ：ducheng
// 创建时间 ：2015-7-15 17:01:23
// 功能描述 ：计算HDID from wps
//
// -----------------------------------------------------------------------

#include "hdid.h"

#include <Wbemidl.h>
#include <algorithm>
#include <atlbase.h>
#pragma comment(lib, "wbemuuid")

#include <IPTypes.h>

#define CURL_STATICLIB
#include "../curl/include/curl/curl.h"

#pragma push_macro("CoInitialize")
#pragma push_macro("CoUninitialize")
#pragma push_macro("CoCreateInstance")

#undef CoInitialize
#undef CoUninitialize
#undef CoCreateInstance

// -----------------------------------------------------------------------

std::string KSWMIDiskSNInfo::GetHardDiskSN()
{
	static char s_szHdidBuf[1024] = {0};
	if (*s_szHdidBuf)
		return s_szHdidBuf;

	HANDLE hThread = ::CreateThread(NULL, 0, &KSWMIDiskSNInfo::ThreadProc, 
		reinterpret_cast<void*>(s_szHdidBuf), 0, NULL);
	BOOL bOk = (::WaitForSingleObject(hThread, 3000) == WAIT_OBJECT_0);
	::CloseHandle(hThread);
	return (bOk) ? s_szHdidBuf : std::string();
}

DWORD WINAPI KSWMIDiskSNInfo::ThreadProc(LPVOID lpParam)
{
	char* pszBuf = reinterpret_cast<char*>(lpParam);
	std::string hdid;
	::CoInitialize(NULL);
	hdid = _GetHardDiskSNByWMI();
	::CoUninitialize();

	if (pszBuf && !hdid.empty())
		strncpy(pszBuf, hdid.c_str(), 1023);
	return 0;
}

std::string KSWMIDiskSNInfo::_GetProperty(IWbemClassObject* pClsObj, const WCHAR* szName)
{
	CComVariant vtProp;
	HRESULT hr = pClsObj->Get(szName, 0, &vtProp, 0, 0);
	if (FAILED(hr))
		return std::string();

	if (vtProp.vt != VT_BSTR)
		return std::string();

	USES_CONVERSION;
	return std::string(W2A(vtProp.bstrVal));
}

std::string KSWMIDiskSNInfo::_GetHardDiskSNByWMI()
{
	HRESULT hres =  ::CoInitializeSecurity(
		NULL, 
		-1,                          // COM authentication
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities 
		NULL                         // Reserved
		);

	CComPtr<IWbemLocator> spLoc;
	hres = ::CoCreateInstance(
		CLSID_WbemLocator,             
		0, 
		CLSCTX_INPROC_SERVER, 
		IID_IWbemLocator, (void **)&spLoc);
	if (FAILED(hres))
		return std::string();

	CComPtr<IWbemServices> spSvc;
	hres = spLoc->ConnectServer(
		L"ROOT\\CIMV2",			 // Object path of WMI namespace
		NULL,                    // User name. NULL = current user
		NULL,                    // User password. NULL = current
		0,                       // Locale. NULL indicates current
		NULL,                    // Security flags.
		0,                       // Authority (for example, Kerberos)
		0,                       // Context object 
		&spSvc                   // pointer to IWbemServices proxy
		);
	if (FAILED(hres))
		return std::string();

	hres = ::CoSetProxyBlanket(
		spSvc,                       // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		NULL,                        // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities 
		);
	if (FAILED(hres))
		return std::string();

	CComPtr<IEnumWbemClassObject> spEnumerator;
	hres = spSvc->ExecQuery(
		L"WQL", 
		L"SELECT * FROM Win32_DiskDrive",
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
		NULL,
		&spEnumerator);
	if (FAILED(hres))
		return std::string();

	std::string strModel, strFirmwareRevison, strSerialNumber;
	while (spEnumerator)
	{
		CComPtr<IWbemClassObject> spClsObj;
		ULONG ulReturn = 0;

		HRESULT hr = spEnumerator->Next(WBEM_INFINITE, 1, 
			&spClsObj, &ulReturn);
		if (FAILED(hr) || 0 == ulReturn)
			break;

		strSerialNumber = _GetProperty(spClsObj, L"SerialNumber");
		if (strSerialNumber.empty())
			continue;

		strModel = _GetProperty(spClsObj, L"Model");
		strFirmwareRevison = _GetProperty(spClsObj, L"FirmwareRevision");
		break;	
	}
	if (strSerialNumber.empty())
		return std::string();

	char buf[30];
	memset(buf, ' ', sizeof(buf));
	if (!strModel.empty())
		memcpy(buf, strModel.c_str(), std::min(4, (int)strModel.length()));
	if (!strFirmwareRevison.empty())
	{
		memcpy(buf + 4, strFirmwareRevison.c_str(), 
			std::min(4, (int)strFirmwareRevison.length()));
	}
	memcpy(buf + 8, strSerialNumber.c_str(), 
		std::min(20, (int)strSerialNumber.length()));
	buf[28] = 0;
	return std::string(buf);
}


// -----------------------------------------------------------------------
//				implementation
//////////////////////////////////////////////////////////////////////////
#define DFP_GET_VERSION         0x00074080 
#define DFP_SEND_DRIVE_COMMAND  0x0007c084 
#define DFP_RECEIVE_DRIVE_DATA  0x0007c088 

#pragma pack(1) 
typedef struct _GETVERSIONOUTPARAMS
{ 
	BYTE bVersion; // Binary driver version. 
	BYTE bRevision; // Binary driver revision. 
	BYTE bReserved; // Not used. 
	BYTE bIDEDeviceMap; // Bit map of IDE devices. 
	DWORD fCapabilities; // Bit mask of driver capabilities. 
	DWORD dwReserved[4]; // For future use. 
} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS; 

typedef struct _IDEREGS_
{ 
	BYTE bFeaturesReg; // Used for specifying SMART "commands". 
	BYTE bSectorCountReg; // IDE sector count register 
	BYTE bSectorNumberReg; // IDE sector number register 
	BYTE bCylLowReg; // IDE low order cylinder value 
	BYTE bCylHighReg; // IDE high order cylinder value 
	BYTE bDriveHeadReg; // IDE drive/head register 
	BYTE bCommandReg; // Actual IDE command. 
	BYTE bReserved; // reserved for future use. Must be zero. 
} IDEREGS_, *PIDEREGS_, *LPIDEREGS_; 
// 
typedef struct _SENDCMDINPARAMS_
{ 
	DWORD cBufferSize; // Buffer size in bytes 
	IDEREGS_ irDriveRegs; // Structure with drive register values. 
	BYTE bDriveNumber; // Physical drive number to send 
	// command to (0,1,2,3). 
	BYTE bReserved[3]; // Reserved for future expansion. 
	DWORD dwReserved[4]; // For future use. 
	//BYTE bBuffer[1]; // Input buffer. 
} SENDCMDINPARAMS_, *PSENDCMDINPARAMS_, *LPSENDCMDINPARAMS_; 
// 
typedef struct _DRIVERSTATUS_
{ 
	BYTE bDriverError; // Error code from driver, 
	// or 0 if no error. 
	BYTE bIDEStatus; // Contents of IDE Error register. 
	// Only valid when bDriverError 
	// is SMART_IDE_ERROR. 
	BYTE bReserved[2]; // Reserved for future expansion. 
	DWORD dwReserved[2]; // Reserved for future expansion. 
} DRIVERSTATUS_, *PDRIVERSTATUS_, *LPDRIVERSTATUS_; 
// 
typedef struct _SENDCMDOUTPARAMS_
{ 
	DWORD cBufferSize; // Size of bBuffer in bytes 
	DRIVERSTATUS_ DriverStatus; // Driver status structure. 
	BYTE bBuffer[512]; // Buffer of arbitrary length 
	// in which to store the data read from the drive. 
} SENDCMDOUTPARAMS_, *PSENDCMDOUTPARAMS_, *LPSENDCMDOUTPARAMS_;

// 
typedef struct _IDSECTOR
{ 
	USHORT wGenConfig; 
	USHORT wNumCyls; 
	USHORT wReserved; 
	USHORT wNumHeads; 
	USHORT wBytesPerTrack; 
	USHORT wBytesPerSector; 
	USHORT wSectorsPerTrack; 
	USHORT wVendorUnique[3]; 
	CHAR sSerialNumber[20]; 
	USHORT wBufferType; 
	USHORT wBufferSize; 
	USHORT wECCSize; 
	CHAR sFirmwareRev[8]; 
	CHAR sModelNumber[40]; 
	USHORT wMoreVendorUnique; 
	USHORT wDoubleWordIO; 
	USHORT wCapabilities; 
	USHORT wReserved1; 
	USHORT wPIOTiming; 
	USHORT wDMATiming; 
	USHORT wBS; 
	USHORT wNumCurrentCyls; 
	USHORT wNumCurrentHeads; 
	USHORT wNumCurrentSectorsPerTrack; 
	ULONG ulCurrentSectorCapacity; 
	USHORT wMultSectorStuff; 
	ULONG ulTotalAddressableSectors; 
	USHORT wSingleWordDMA; 
	USHORT wMultiWordDMA; 
	BYTE bReserved[128]; 
} IDSECTOR, *PIDSECTOR; 
#pragma pack() 
//////////////////////////////////////////////////////////////////////////

void KSHardSNInfo::ChangeByteOrder(PCHAR szString, USHORT uscStrSize) 
{
	USHORT i; 
	CHAR temp; 

	for (i = 0; i < uscStrSize; i+=2) 
	{ 
		temp = szString[i]; 
		szString[i] = szString[i+1]; 
		szString[i+1] = temp; 
	} 
} 

BOOL KSHardSNInfo::hdid9x(std::string& strSN)
{ 
	GETVERSIONOUTPARAMS vers; 
	SENDCMDINPARAMS_ in; 
	SENDCMDOUTPARAMS_ out; 
	HANDLE h; 
	DWORD i; 
	BYTE j;


	ZeroMemory(&vers,sizeof(vers)); 
	//We start in 95/98/Me 
	h = CreateFileA("\\\\.\\Smartvsd", 0, 0, 0, CREATE_NEW, 0, 0); 

	if (INVALID_HANDLE_VALUE == h)
	{ 
		//cout<<"open smartvsd.vxd failed"<<endl; 
		//strcpy(g_szErrorInfo, "open smartvsd.vxd failed");
		return FALSE;
	} 

	if (!DeviceIoControl(h,DFP_GET_VERSION,0,0,&vers,sizeof(vers),&i,0))
	{ 
		//printf("getLastError = %d\n", GetLastError());
		//strcpy(g_szErrorInfo, "DeviceIoControl failed:DFP_GET_VERSION"); 
		CloseHandle(h); 
		return  FALSE; 
	} 

	//If IDE identify command not supported, fails 
	if (!(vers.fCapabilities&1))
	{ 
		//strcpy(g_szErrorInfo, "Error: IDE identify command not supported."); 
		CloseHandle(h); 
		return FALSE; 
	} 

	//Display IDE drive number detected 
	//DetectIDE(vers.bIDEDeviceMap); 
	//Identify the IDE drives 
	for (j = 0; j < 4; j++)
	{ 
		PIDSECTOR phdinfo;
		char s[41]; 
		char sPreModuleNumber[5] = {0};
		char sPreFirmwareRev[5] = {0};		

		ZeroMemory(&in,sizeof(in)); 
		ZeroMemory(&out,sizeof(out)); 

		if ( j & 1 )
		{ 
			in.irDriveRegs.bDriveHeadReg=0xb0; 
		}
		else
		{ 
			in.irDriveRegs.bDriveHeadReg = 0xa0; 
		} 
		if (vers.fCapabilities&(16>>j))
		{ 
			//We don't detect a ATAPI device. 
			//cout<<"Drive "<<(int)(j+1)<<" is a ATAPI device, we don't detect it"<< endl; 
			continue; 
		}else
		{ 
			in.irDriveRegs.bCommandReg = 0xec; 
		}

		in.bDriveNumber = j; 
		in.irDriveRegs.bSectorCountReg = 1; 
		in.irDriveRegs.bSectorNumberReg = 1; 
		in.cBufferSize = 512; 

		if (!DeviceIoControl(h,DFP_RECEIVE_DRIVE_DATA,&in,sizeof(in),&out,sizeof(out),&i,0))
		{ 
			//cout<<"DeviceIoControl failed:DFP_RECEIVE_DRIVE_DATA"<<endl; 
			//strcpy(g_szErrorInfo, "DeviceIoControl failed:DFP_RECEIVE_DRIVE_DATA");
			CloseHandle(h); 
			return FALSE; 
		}

		phdinfo = (PIDSECTOR)out.bBuffer;

		memcpy(s,phdinfo->sModelNumber, 40); 
		s[40]=0; 
		ChangeByteOrder(s,40); 
		//cout<<endl<<"Module Number:"<< s <<endl; 
		strncpy(sPreModuleNumber, s, 4);
		sPreModuleNumber[4] = 0;

		memcpy(s, phdinfo->sFirmwareRev, 8);
		s[8] = 0;
		ChangeByteOrder(s, 8);
		//cout<<"\tFirmware rev:"<< s <<endl; 
		strncpy(sPreFirmwareRev, s, 4);
		sPreFirmwareRev[4] = 0;

		memcpy(s, phdinfo->sSerialNumber, 20);
		s[20] = 0;
		ChangeByteOrder(s,20);
		//cout<<"\tSerial Number:"<<s<<endl;
		char szHardSN[1024] = {0};
		sprintf(szHardSN, "%s%s%s", sPreModuleNumber, sPreFirmwareRev, s);
		strSN = szHardSN;
		//Close handle before quit 
		CloseHandle(h);
		return TRUE;
		//cout<<"\tCapacity:"<<phdinfo->ulTotalAddressableSectors/2/1024<<"M"<<endl<<endl; 
	}

	return FALSE;
} 

BOOL KSHardSNInfo::hdidnt(std::string& strSN)
{ 
	GETVERSIONOUTPARAMS vers; 
	SENDCMDINPARAMS_ in; 
	SENDCMDOUTPARAMS_ out; 
	HANDLE h; 
	DWORD i; 
	BYTE j;

	char hd[80]; 
	PIDSECTOR phdinfo; 
	char s[41]; 

	char sPreModuleNumber[5] = {0};
	char sPreFirmwareRev[5] = {0};		

	ZeroMemory(&vers,sizeof(vers)); 
	//We start in NT/Win2000 
	for (j = 0; j < 4; j++)
	{ 

		sprintf(hd,"\\\\.\\PhysicalDrive%d",j);
		h = CreateFileA(hd,GENERIC_READ | GENERIC_WRITE, 
			FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0); 

		if (INVALID_HANDLE_VALUE == h)
		{
			continue; 
		} 

		if (!DeviceIoControl(h,DFP_GET_VERSION,0,0,&vers,sizeof(vers),&i,0))
		{ 
			CloseHandle(h); 
			continue; 
		} 

		//If IDE identify command not supported, fails 
		if (!(vers.fCapabilities&1))
		{ 
			//cout<<"Error: IDE identify command not supported."; 
			//strcpy(g_szErrorInfo, "Error: IDE identify command not supported.");
// 			ASSERT(FALSE);
			CloseHandle(h); 
			return FALSE; 
		}

		//Identify the IDE drives 
		ZeroMemory(&in,sizeof(in)); 
		ZeroMemory(&out,sizeof(out)); 

		if (j & 1)
		{ 
			in.irDriveRegs.bDriveHeadReg = 0xb0; 
		}
		else
		{ 
			in.irDriveRegs.bDriveHeadReg = 0xa0; 
		} 

		if (vers.fCapabilities & (16 >> j ))
		{ 
			//We don't detect a ATAPI device. 
			//cout<<"Drive "<<(int)(j+1)<<" is a ATAPI device, we don't detect it"<<endl; 
			continue; 
		}
		else
		{ 
			in.irDriveRegs.bCommandReg = 0xec; 
		} 

		in.bDriveNumber = j; 
		in.irDriveRegs.bSectorCountReg = 1; 
		in.irDriveRegs.bSectorNumberReg = 1; 
		in.cBufferSize = 512;
		if (!DeviceIoControl(h,DFP_RECEIVE_DRIVE_DATA,&in,sizeof(in),&out,sizeof(out),&i,0))
		{ 
			//cout<<"DeviceIoControl failed:DFP_RECEIVE_DRIVE_DATA"<<endl; 
			//strcpy(g_szErrorInfo, "DeviceIoControl failed:DFP_RECEIVE_DRIVE_DATA");
// 			ASSERT(FALSE);
			CloseHandle(h); 
			return FALSE; 
		} 

		phdinfo = (PIDSECTOR)out.bBuffer; 

		memcpy(s,phdinfo->sModelNumber, 40); 
		s[40] = 0; 
		ChangeByteOrder(s, 40); 
		//cout<<endl<<"Module Number:"<<s<<endl; 
		strncpy(sPreModuleNumber, s, 4);
		sPreModuleNumber[4] = 0;

		memcpy(s, phdinfo->sFirmwareRev, 8); 
		s[8] = 0; 
		ChangeByteOrder(s, 8); 
		//cout<<"\tFirmware rev:"<<s<<endl; 
		strncpy(sPreFirmwareRev, s, 4);
		sPreFirmwareRev[4] = 0;

		memcpy(s,phdinfo->sSerialNumber, 20); 
		s[20] = 0; 
		ChangeByteOrder(s,20); 
		//strcpy(g_szHardSN, s);
		char szHardSN[1024] = {0};
		sprintf(szHardSN, "%s%s%s", sPreModuleNumber, sPreFirmwareRev, s);
		strSN = szHardSN;

		//cout<<"\tCapacity:"<<phdinfo->ulTotalAddressableSectors/2/1024<<"M"<<endl<<endl; 

		CloseHandle(h); 
		return TRUE;
	}
	return FALSE;
} 


std::string KSHardSNInfo::GetHardSN()
{
	OSVERSIONINFO VersionInfo;

	ZeroMemory(&VersionInfo, sizeof(VersionInfo)); 
	VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo); 

	GetVersionEx(&VersionInfo); 
	std::string strHardSN;
	switch (VersionInfo.dwPlatformId)
	{ 
	case VER_PLATFORM_WIN32s: 
		//cout<<"Win32s is not supported by this programm."<<endl; 
		{		
			//strcpy(g_szErrorInfo, "Win32s is not supported by this programm.");
			//return ""; 
		}
	case VER_PLATFORM_WIN32_WINDOWS: 
		{		
			hdid9x(strHardSN);
		}
	case VER_PLATFORM_WIN32_NT: 
		{
			hdidnt(strHardSN);
		}
	}

	if (strHardSN.empty())
		strHardSN = KSWMIDiskSNInfo::GetHardDiskSN();
	return strHardSN;
}

// -----------------------------------------------------------------------
typedef DWORD (WINAPI *GetAdaptersInfoProc)(PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen);

class KIphlpapidll
{
public:
	KIphlpapidll(void) : m_hIphlpapi(NULL)
	{
	}

	~KIphlpapidll(void)
	{
		if (m_hIphlpapi) ::FreeLibrary(m_hIphlpapi);
	}

	bool Init(void)
	{
		bool bRet = false;
		m_hIphlpapi = ::LoadLibraryA("Iphlpapi.dll");
		if (m_hIphlpapi)
		{
			m_pGetAdaptersInfo = (GetAdaptersInfoProc)::GetProcAddress(m_hIphlpapi, "GetAdaptersInfo");
			bRet = m_pGetAdaptersInfo != NULL;
		}

		return bRet;
	}

	DWORD GetAdaptersInfo(PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen)
	{
		return m_pGetAdaptersInfo(pAdapterInfo, pOutBufLen);
	}

private:
	HMODULE				m_hIphlpapi;
	GetAdaptersInfoProc m_pGetAdaptersInfo;
};

// -------------------------------------------------------------------------

#define IPCONFIG_OUTPUT_SIZE 4096

size_t KSNICInfo::FindPhysicalAddress(std::string& strMacAddr, size_t nInitPos)
{
	size_t nIdx = strMacAddr.find('-', nInitPos);
	if(std::string::npos == nIdx)
		return std::string::npos;
	if(nIdx > 1 && (strMacAddr.length() - nIdx) > 14 &&
		strMacAddr[nIdx + 3] == '-' && strMacAddr[nIdx + 6] == '-' &&
		strMacAddr[nIdx + 9] == '-' && strMacAddr[nIdx + 12] == '-' &&
		(isdigit(strMacAddr[nIdx - 1]) || isalpha(strMacAddr[nIdx - 1])) &&
		(isdigit(strMacAddr[nIdx - 2]) || isalpha(strMacAddr[nIdx - 2])) &&
		(isdigit(strMacAddr[nIdx + 1]) || isalpha(strMacAddr[nIdx + 1])) &&
		(isdigit(strMacAddr[nIdx + 2]) || isalpha(strMacAddr[nIdx + 2])) &&
		(isdigit(strMacAddr[nIdx + 4]) || isalpha(strMacAddr[nIdx + 4])) &&
		(isdigit(strMacAddr[nIdx + 5]) || isalpha(strMacAddr[nIdx + 5])) &&
		(isdigit(strMacAddr[nIdx + 7]) || isalpha(strMacAddr[nIdx + 7])) &&
		(isdigit(strMacAddr[nIdx + 8]) || isalpha(strMacAddr[nIdx + 8])) &&
		(isdigit(strMacAddr[nIdx + 10]) || isalpha(strMacAddr[nIdx + 10])) &&
		(isdigit(strMacAddr[nIdx + 11]) || isalpha(strMacAddr[nIdx + 11])) &&
		(isdigit(strMacAddr[nIdx + 13]) || isalpha(strMacAddr[nIdx + 13])) &&
		(isdigit(strMacAddr[nIdx + 14]) || isalpha(strMacAddr[nIdx + 14])))
	{
		return nIdx - 2;
	}
	else
		return FindPhysicalAddress(strMacAddr, nIdx + 1);
}

void KSNICInfo::GetAllPhysicalAdapters(std::vector<std::string>& adapters)
{
	const char* const szAdapterClassKey = 
		"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}";
	const DWORD NCF_PHYSICAL = 0x4;

	HKEY hKey;
	LONG lRetVal = ::RegOpenKeyExA(HKEY_LOCAL_MACHINE, szAdapterClassKey, 0,
		KEY_READ, &hKey);
	if (lRetVal != ERROR_SUCCESS)
		return;

	char szBuffer[256];
	for (DWORD dwIndex = 0, dwSize = 256;
		::RegEnumKeyExA(hKey, dwIndex, szBuffer, &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS;
		dwSize = 256, ++dwIndex)
	{
		HKEY hSubKey;
		lRetVal = ::RegOpenKeyExA(hKey, szBuffer, 0, KEY_READ, &hSubKey);
		if (lRetVal != ERROR_SUCCESS)
			continue;

		DWORD dwCharacteristics = 0;
		DWORD dwType = 0;
		dwSize = sizeof(DWORD);
		lRetVal = ::RegQueryValueExA(hSubKey, "Characteristics", NULL, &dwType,
			(BYTE*)&dwCharacteristics, &dwSize);
		if (lRetVal != ERROR_SUCCESS ||
			(dwCharacteristics & NCF_PHYSICAL) != NCF_PHYSICAL)
		{
			::RegCloseKey(hSubKey);
			continue;
		}

		dwType = 0;
		dwSize = 256 * sizeof(char);
		lRetVal = ::RegQueryValueExA(hSubKey, "ComponentId", NULL, &dwType, 
			(LPBYTE)szBuffer, &dwSize);
		if (lRetVal != ERROR_SUCCESS || strnicmp(szBuffer, "PCI", 3))
		{
			::RegCloseKey(hSubKey);
			continue;
		}

		dwType = 0;
		dwSize = 256 * sizeof(char);
		lRetVal = ::RegQueryValueExA(hSubKey, "NetCfgInstanceId", NULL, &dwType, 
			(LPBYTE)szBuffer, &dwSize);
		if (lRetVal == ERROR_SUCCESS)
			adapters.push_back(szBuffer);
		::RegCloseKey(hSubKey);
	}
}

std::string KSNICInfo::GetMacAddress()
{
	std::vector<std::string> vecMacAddress;
	KIphlpapidll iphlpapidll;
	if (iphlpapidll.Init())
	{
		IP_ADAPTER_INFO info = {0};
		ULONG uLen = sizeof(IP_ADAPTER_INFO);
		DWORD dwError = iphlpapidll.GetAdaptersInfo(&info, &uLen);

		PIP_ADAPTER_INFO vecInfos = new IP_ADAPTER_INFO[uLen / sizeof(IP_ADAPTER_INFO) + 1];
		ZeroMemory(vecInfos, (uLen / sizeof(IP_ADAPTER_INFO) + 1) * sizeof(IP_ADAPTER_INFO));
		if (dwError == ERROR_BUFFER_OVERFLOW)
		{
			dwError = iphlpapidll.GetAdaptersInfo(vecInfos, &uLen);
		}
		else
			memcpy(vecInfos, &info, uLen);

		if (dwError == 0)
		{
			std::vector<std::string> vecPhysicalAdapters;
			GetAllPhysicalAdapters(vecPhysicalAdapters);

			for (PIP_ADAPTER_INFO pInfo = vecInfos; 
				pInfo != NULL;
				pInfo = pInfo->Next)
			{
				if (pInfo->Type != 6/*MIB_IF_TYPE_ETHERNET*/)
					continue;

				std::string strAdapterName = pInfo->AdapterName;
				auto it = std::find(vecPhysicalAdapters.begin(), 
					vecPhysicalAdapters.end(), strAdapterName);
				if (it == vecPhysicalAdapters.end())
					continue;

				char szAddr[18] = {0};
				sprintf(szAddr, "%02X-%02X-%02X-%02X-%02X-%02X",
					(int)pInfo->Address[0],
					(int)pInfo->Address[1],
					(int)pInfo->Address[2],
					(int)pInfo->Address[3],
					(int)pInfo->Address[4],
					(int)pInfo->Address[5]);
				vecMacAddress.push_back(szAddr);
			}
		}
		delete [] vecInfos;
	}
	if (vecMacAddress.empty())
		return std::string();

	std::sort(vecMacAddress.begin(), vecMacAddress.end());
	std::string strMacAddress;
	for (auto it = vecMacAddress.begin(); it != vecMacAddress.end(); ++ it)
	{
		if (!strMacAddress.empty())
			strMacAddress.append("+");
		strMacAddress.append(*it);
	}
	return strMacAddress;
}

#pragma pop_macro("CoInitialize")
#pragma pop_macro("CoUninitialize")
#pragma pop_macro("CoCreateInstance")

void GenerateHDID(std::wstring& strHDID)
{
	std::string strHardwareInfo = KSHardSNInfo::GetHardSN() + "|" + KSNICInfo::GetMacAddress();

	CHAR pSig[16];
	Curl_md5it((unsigned char*)pSig, (unsigned char*)strHardwareInfo.c_str());

	WCHAR pwstrSig[33] = {0};
	for(int i = 0; i < 16; ++i)
		_snwprintf(&pwstrSig[i * 2], 2, L"%02X", (unsigned char)pSig[i]);

	strHDID = pwstrSig;
}
