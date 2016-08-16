// -----------------------------------------------------------------------
//
// 文 件 名 ：hdid.h
// 创 建 者 ：ducheng
// 创建时间 ：2015-7-15 17:01:23
// 功能描述 ：计算HDID from wps
//
// -----------------------------------------------------------------------
#ifndef SHELL_PWOERWORD_SHARED_HDID_H
#define SHELL_PWOERWORD_SHARED_HDID_H

#include <string>
#include <vector>
#include <BaseTyps.h>
#include <WbemCli.h>
interface IWbemClassObject;
class KSWMIDiskSNInfo
{
public:
	static std::string GetHardDiskSN();

private:
	static std::string _GetProperty(IWbemClassObject* pClsObj, const WCHAR* szName);
	static std::string _GetHardDiskSNByWMI();

	static DWORD WINAPI ThreadProc(LPVOID lpParam);
};

class KSHardSNInfo
{
public:
	static std::string GetHardSN();
private:
	static BOOL hdidnt(std::string& strSN);
	static BOOL hdid9x(std::string& strSN);
	static void ChangeByteOrder(PCHAR szString, USHORT uscStrSize);
};

// -------------------------------------------------------------------------
class KSNICInfo
{
public:
	static std::string GetMacAddress();
private:
	static size_t FindPhysicalAddress(std::string& strMacAddr, size_t nInitPos);
	static void GetAllPhysicalAdapters(std::vector<std::string>& adapters);
};

void GenerateHDID(std::wstring& strHDID);

#endif //FRAMEWORK_INFOCOL_SRC_HDID_H
