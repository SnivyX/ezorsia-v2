#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdint>
#include <shellapi.h>
#include <WinSock2.h>

#pragma comment(lib, "comsuppw.lib")

// these make the code harder to reverse engineer
#define JM_XORSTR_DISABLE_AVX_INTRINSICS
#include "xorstr.hpp"
#include "lazy_import.hpp"
#include "MinHook.h"
#include "ini.hpp"
#include "res.hpp"
#include "avatar.hpp"
#include "user.hpp"

#define FMT_HEADER_ONLY
#include "fmt/format.h"

#include <unordered_map>
#include <string>
#include "CInPacket.hpp"

#define HOSTNAME "127.0.0.1"

union key_t
{
	uint32_t dword;
	uint8_t bytes[4];
};

enum server_locale_e
{
	kms = 1,
	kmst,
	jms,
	cms,
	cmst,
	tms,
	thms,
	gms,
	ems,
	bms = 9, // so bms is a copy of ems?
	windia,
	aquila // 11
};

struct chair_data_t
{
	chair_data_t() {}
	chair_data_t(int nBodyRelMoveX, int nBodyRelMoveY) : m_nBodyRelMoveX(nBodyRelMoveX), m_nBodyRelMoveY(nBodyRelMoveY) {}

	int m_nBodyRelMoveX = 0;
	int m_nBodyRelMoveY = 0;
	bool m_bFirstCall = true;
};

key_t g_iv;
uint8_t g_userkey[128]{};
bool g_bInit = false;
decltype(&CreateMutexA) _CreateMutexA;

template <typename T, typename Y>
void hook(T** pTrampoline, Y pDetour, bool bEnable = false)
{
	auto pTarget = *pTrampoline;

	if(MH_CreateHook(pTarget, pDetour, pTrampoline) == MH_OK && bEnable)
	{
		MH_EnableHook(pTarget);
	}
}

#define HOOK(a, b) hook(reinterpret_cast<void**>(&a), b)

auto g_rm = reinterpret_cast<wil::com_ptr_t<IWzResMan>*>(0x00BF14E8);
std::unordered_map<unsigned int, chair_data_t> g_pChairData;

uintptr_t g_pCItemInfo = 0x00BE78D8;
typedef IWzProperty*(__thiscall* CItemInfo__GetItemInfo_t)(uintptr_t ecx, IWzProperty** result, const int nItemID);
auto _CItemInfo__GetItemInfo = reinterpret_cast<CItemInfo__GetItemInfo_t>(0x005DA83C);

typedef void(__thiscall* CClientSocket__ProcessPacket_t)(uintptr_t ecx, CInPacket* iPacket);
auto _CClientSocket__ProcessPacket = reinterpret_cast<CClientSocket__ProcessPacket_t>(0x004965F1);

typedef void(__thiscall* CUserLocal__Update_t)(uintptr_t ecx);
auto _CUserLocal__Update = reinterpret_cast<CUserLocal__Update_t>(0x0094A144);

typedef void(__thiscall* CUser__SetActivePortableChair_t)(CUser* ecx, int nItemID);
auto _CUser__SetActivePortableChair = reinterpret_cast<CUser__SetActivePortableChair_t>(0x0093C7C3);

typedef void(__thiscall* CUser__Update_t)(CUser* ecx);
auto _CUser__Update = reinterpret_cast<CUser__Update_t>(0x00930B27);

typedef void(__thiscall* CAvatar__PrepareActionLayer_t)(CAvatar* ecx, int nActionSpeed, int nWalkSpeed, int bKeyDown);
auto _CAvatar__PrepareActionLayer = reinterpret_cast<CAvatar__PrepareActionLayer_t>(0x00453AD1);

__forceinline void write_buffer(uintptr_t address, const char* aBuffer, size_t dwSize)
{
	unsigned long oldprot;
	LI_FN(VirtualProtect)(reinterpret_cast<void*>(address), dwSize, PAGE_EXECUTE_READWRITE, &oldprot);
	std::copy(aBuffer, aBuffer + dwSize, reinterpret_cast<uint8_t*>(address));
	LI_FN(VirtualProtect)(reinterpret_cast<void*>(address), dwSize, oldprot, nullptr);
}

__declspec(dllexport) void stop_staring_baka()
{
	// this doesn't do anything and the code never runs
	// just for fun, for those who tries to reverse engineer
	// you can get rid of the line if you want but i'd appreciate if it stays
	LI_FN(OutputDebugStringW)(L"https://theomnidesk.com");
}

__forceinline std::string get_ip(std::string_view hostname)
{
	WSADATA data;
	LI_FN(WSAStartup)(MAKEWORD(1, 1), &data);
	auto host = LI_FN(gethostbyname)(hostname.data());
	auto ip = LI_FN(inet_ntoa)(*reinterpret_cast<in_addr*>(host->h_addr_list[0]));
	LI_FN(WSACleanup)();

	return ip;
}

__forceinline uintptr_t get_lib(std::wstring_view lib)
{
	uintptr_t addr = reinterpret_cast<uintptr_t>(LI_FN(GetModuleHandleW)(lib.data()));
	
	if(addr == 0)
	{
		addr = reinterpret_cast<uintptr_t>(LI_FN(LoadLibraryW)(lib.data()));
	}

	return addr;
}

__forceinline uintptr_t get_func(uintptr_t mod, std::string_view fun)
{
	return reinterpret_cast<uintptr_t>(LI_FN(GetProcAddress)(reinterpret_cast<HMODULE>(mod), fun.data()));
}

IWzProperty* get_item_info(const int nItemID)
{
	IWzProperty* pResult;
	_CItemInfo__GetItemInfo(g_pCItemInfo, &pResult, nItemID);

	return pResult;
}

void __fastcall hook_CUserLocal__Update(uintptr_t ecx, uintptr_t)
{
	_CUserLocal__Update(ecx);

	static uint64_t tLastFlush = 0;

	if(LI_FN(GetTickCount64)() - tLastFlush > 15000)
	{
		(*g_rm)->raw_FlushCachedObjects(0);

		tLastFlush = LI_FN(GetTickCount64)();
	}
}

__forceinline void update_user_body_origin(const chair_data_t& pChairData, wil::com_ptr_t<IWzVector2D> pBodyOrigin)
{
	if(pBodyOrigin != nullptr)
	{
		pBodyOrigin->raw_RelMove(pChairData.m_nBodyRelMoveX, pChairData.m_nBodyRelMoveY, 0, 0);
	}
}

void __fastcall hook_CUser__Update(CUser* ecx, uintptr_t)
{
	_CUser__Update(ecx);

	if(g_pChairData.find(ecx->m_dwCharacterId()) != g_pChairData.end())
	{
		auto& pChairData = g_pChairData[ecx->m_dwCharacterId()];

		// prevents the chair from going above
		if(!pChairData.m_bFirstCall)
		{
			update_user_body_origin(pChairData, ecx->GetAvatar()->m_pBodyOrigin());
		}

		pChairData.m_bFirstCall = false;
	}
}

void __fastcall hook_CAvatar__PrepareActionLayer(CAvatar* ecx, uintptr_t, int nActionSpeed, int nWalkSpeed, int bKeyDown)
{
	_CAvatar__PrepareActionLayer(ecx, nActionSpeed, nWalkSpeed, bKeyDown);

	if(g_pChairData.find(ecx->GetCharacterID()) != g_pChairData.end())
	{
		auto& pChairData = g_pChairData[ecx->GetCharacterID()];

		if(!pChairData.m_bFirstCall)
		{
			update_user_body_origin(g_pChairData[ecx->GetCharacterID()], ecx->m_pBodyOrigin());
		}

		pChairData.m_bFirstCall = false;
	}
}

void __fastcall hook_CUser__SetActivePortableChair(CUser* ecx, uintptr_t, int nItemID)
{
	_CUser__SetActivePortableChair(ecx, nItemID);

	if(nItemID != 0)
	{
		int nBodyRelMoveX = 0;
		int nBodyRelMoveY = 0;
		auto pItemInfo = get_item_info(nItemID);

		if(pItemInfo != nullptr)
		{
			auto pBodyRelMove = pItemInfo->get_item<IWzVector2D*>(L"bodyRelMove");

			if(pBodyRelMove != nullptr)
			{
				nBodyRelMoveX = pBodyRelMove->get_x();
				nBodyRelMoveY = pBodyRelMove->get_y();
			}
		}

		// looking to the right - adjust view
		if(!ecx->IsLeft())
		{
			nBodyRelMoveX *= -1;
		}

		g_pChairData[ecx->m_dwCharacterId()] = chair_data_t(nBodyRelMoveX, nBodyRelMoveY);
		update_user_body_origin(g_pChairData[ecx->m_dwCharacterId()], ecx->GetAvatar()->m_pBodyOrigin());

#if CONSOLE
		printf("Chair body rel move: %d/%d (%d)\n", nBodyRelMoveX, nBodyRelMoveY, nItemID);
#endif
	}

	else
	{
		g_pChairData.erase(ecx->m_dwCharacterId());
	}
}

void __fastcall hook_CClientSocket__ProcessPacket(uintptr_t ecx, uintptr_t, CInPacket* iPacket)
{
	uint16_t usType = iPacket->get_type();

	if(usType < ALP_START)
	{
		_CClientSocket__ProcessPacket(ecx, iPacket);

		return;
	}

	iPacket->seek_forward(sizeof(usType));

	switch(usType)
	{
	case ALP_GTOP:
	{
		reinterpret_cast<decltype(&ShellExecuteA)>(get_func(get_lib(xorstr_(L"shell32.dll")), xorstr_("ShellExecuteA")))(nullptr, xorstr_("open"), iPacket->decode<std::string>().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}
	}
}

__forceinline void init()
{
	g_bInit = true;

	// change client locale
	write_to_mem<uint8_t>(0x00495191, server_locale_e::aquila);

	// init keys
	g_iv.bytes[0] = 0xD;
	g_iv.bytes[1] = 0xD;
	g_iv.bytes[2] = 0xD;
	g_iv.bytes[3] = 0xD;

	g_userkey[0 * 16] = 0xD;
	g_userkey[1 * 16] = 0x08;
	g_userkey[2 * 16] = 0x06;
	g_userkey[3 * 16] = 0xB4;
	g_userkey[4 * 16] = 0x1B;
	g_userkey[5 * 16] = 0x0F;
	g_userkey[6 * 16] = 0x0;
	g_userkey[7 * 16] = 0x52;

	const uintptr_t pcom = get_lib(xorstr_(L"PCOM.dll"));
	const uintptr_t zlz = get_lib(xorstr_(L"ZLZ.dll"));

	// overwrite iv in pcom
	for(const auto& address : { 0x6A2E, 0x6AE2, 0x6D38, 0x7192, 0x7290 })
	{
		write_to_mem(pcom + address + 1, &g_iv);
	}

	// overwrite iv in zlz
	for(const auto& address : { 0xB03F, 0xB610 })
	{
		write_to_mem(zlz + address + 1, &g_iv);
	}

	// overwrite userkey in pcom
	for(const auto& address : { 0x1015, 0x108B })
	{
		write_to_mem(pcom + address + 1, g_userkey);
	}

	// overwrite userkey in zlz
	for(const auto& address : { 0x113E, 0x11CE })
	{
		write_to_mem(zlz + address + 1, g_userkey);
	}

	write_to_mem<int>(0x009F5623 + 6, 0); // windowed mode

	// apply patches for cfield::init flush, actionman flush, timed resman flush
	constexpr const uint32_t SWEEPCACHE_DELAY_1 = 0x00411BE2;
	write_to_mem<int>(SWEEPCACHE_DELAY_1 + 2, 5000);

	constexpr const uint32_t SWEEPCACHE_DELAY_2[] = { 0x00411D70, 0x00411E13, 0x00411EC5, 0x00411F68, 0x0041201A, 0x004120BD, 0x00412282, 0x00412303, 0x00412388 };

	for(auto n : SWEEPCACHE_DELAY_2)
	{
		write_to_mem<int>(n + 2, 10000);
	}

	// CField::Init
	constexpr const uint32_t CFIELD_FLUSH = 0x00529320;
	write_to_mem<int>(CFIELD_FLUSH + 1, 0);

	mINI::INIFile file(xorstr_("aquila.ini"));
	mINI::INIStructure cfg;
	file.read(cfg);

	if(!cfg[xorstr_("aquila")].has(xorstr_("hd")))
	{
		cfg[xorstr_("aquila")][xorstr_("hd")] = xorstr_("1");
	}

	if(cfg[xorstr_("aquila")][xorstr_("hd")] != xorstr_("0"))
	{
		change_res(1280, 720);
	}

	file.write(cfg);

	HOOK(_CClientSocket__ProcessPacket, hook_CClientSocket__ProcessPacket);
	HOOK(_CUserLocal__Update, hook_CUserLocal__Update);
	HOOK(_CUser__SetActivePortableChair, hook_CUser__SetActivePortableChair);
	HOOK(_CAvatar__PrepareActionLayer, hook_CAvatar__PrepareActionLayer);
	HOOK(_CUser__Update, hook_CUser__Update);
	MH_EnableHook(MH_ALL_HOOKS);
}

HANDLE __stdcall hook_CreateMutexA(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName)
{
	if(!g_bInit && std::strcmp(lpName, xorstr_("WvsClientMtx")) == 0)
	{
		init();

		return _CreateMutexA(lpMutexAttributes, bInitialOwner, fmt::format("{}{}", xorstr_("AquilaClientMtx-"), LI_FN(GetCurrentProcessId)()).c_str());
	}

	return _CreateMutexA(lpMutexAttributes, bInitialOwner, lpName);
}

int __stdcall DllMain(void* hModule, unsigned int ul_reason_for_call, void* lpReserved)
{
	if(ul_reason_for_call == 1)
	{
		auto ip = get_ip(xorstr_(HOSTNAME));

		for(const auto& address : { 0x00AFE084, 0x00AFE094, 0x00AFE0A4 })
		{
			write_buffer(address, ip.data(), ip.length() + 1);
		}

		_CreateMutexA = LI_FN(CreateMutexA).get();
			
		MH_Initialize();
		HOOK(_CreateMutexA, hook_CreateMutexA);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	return 1;
}
