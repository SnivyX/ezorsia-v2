//### Readme:
/* 
Heads up about the quickslot tooltips on EzorsiaV2. There's a faulty approximation used in Client.cpp on line 121:
Memory::WriteInt(dwQuickSlotCWndVPos + 2, (600 - m_nGameHeight) / 2 - 427); //0x008DE8EE: lea edi,[eax-427]
 
According to my testing this line should just be 
Memory::WriteInt(dwQuickSlotCWndVPos + 2, -500 );

similar to the line which immediately follows it 
Memory::WriteInt(dwQuickSlotCWndHPos + 2, -798); //0x008DE8E5: lea ebx,[eax-647]

This way, the tooltips will display correctly and the offset will not be janky in resolutions beyond 768p. YMMV, need someone to confirm the fix works.
Implemented by editing the subtraction at 0x008DE8EE.
*/

//### ARRAYS - These go at the top of the file, or in a header or somewhere. Not inside Client::Functions
//### CLIENT EDITS - These go inside a function in the client class. ("Client::UpdateGame", etc.)
//### CODECAVES CLIENT EDITS - Same as Client Edits
//### CODECAVES - In a header or somewhere, as long as they are "#include#" in the file you call them from. 
//### Enjoy, have fun, be nice, report bugs, and please credit if you decide to use others' edits.

// ARRAYS ----
unsigned char Array_aDefaultQKM[] = {
	42, 0, 0, 0,
	82, 0, 0, 0,
	71, 0, 0, 0,
	73, 0, 0, 0,
	2, 0, 0, 0,
	3, 0, 0, 0,
	4, 0, 0, 0,
	5, 0, 0, 0,
	6, 0, 0, 0,
	30, 0, 0, 0,
	31, 0, 0, 0,
	32, 0, 0, 0,
	33, 0, 0, 0,
	29, 0, 0, 0,
	83, 0, 0, 0,
	79, 0, 0, 0,
	81, 0, 0, 0,
	16, 0, 0, 0,
	17, 0, 0, 0,
	18, 0, 0, 0,
	19, 0, 0, 0,
	20, 0, 0, 0,
	44, 0, 0, 0,
	45, 0, 0, 0,
	46, 0, 0, 0,
	47, 0, 0, 0,
	52, 0, 0, 0
};

// 0x00BE2DB0 confirmed, s_ptShortKeyPos
unsigned char Array_ptShortKeyPos[] = {
	7, 0, 0, 0,
	8, 0, 0, 0,
	42, 0, 0, 0,
	8, 0, 0, 0,
	77, 0, 0, 0,
	8, 0, 0, 0,
	112, 0, 0, 0,
	8, 0, 0, 0,
	147, 0, 0, 0,
	8, 0, 0, 0,
	182, 0, 0, 0,
	8, 0, 0, 0,
	217, 0, 0, 0,
	8, 0, 0, 0,
	252, 0, 0, 0,
	8, 0, 0, 0,
	287, 1, 0, 0,
	8, 0, 0, 0,
	322, 1, 0, 0,
	8, 0, 0, 0,
	357, 1, 0, 0,
	8, 0, 0, 0,
	392, 1, 0, 0,
	8, 0, 0, 0,
	427, 1, 0, 0,
	8, 0, 0, 0,
	7, 0, 0, 0,
	41, 0, 0, 0,
	42, 0, 0, 0,
	41, 0, 0, 0,
	77, 0, 0, 0,
	41, 0, 0, 0,
	112, 0, 0, 0,
	41, 0, 0, 0,
	147, 0, 0, 0,
	41, 0, 0, 0,
	182, 0, 0, 0,
	41, 0, 0, 0,
	217, 0, 0, 0,
	41, 0, 0, 0,
	252, 0, 0, 0,
	41, 0, 0, 0,
	287, 1, 0, 0,
	41, 0, 0, 0,
	322, 1, 0, 0,
	41, 0, 0, 0,
	357, 1, 0, 0,
	41, 0, 0, 0,
	392, 1, 0, 0,
	41, 0, 0, 0,
	427, 1, 0, 0,
	41, 0, 0, 0
};

// s_aDefaultQKM_0
unsigned char Array_aDefaultQKM_0[] = { 
	42, 0, 0, 0, 
	82, 0, 0, 0, 
	71, 0, 0, 0, 
	73, 0, 0, 0, //4
	29, 0, 0, 0, 
	83, 0, 0, 0, 
	79, 0, 0, 0, 
	81, 0, 0, 0, //8
	42, 0, 0, 0, 
	82, 0, 0, 0, 
	71, 0, 0, 0, 
	73, 0, 0, 0, //12
	29, 0, 0, 0, 
	83, 0, 0, 0, 
	79, 0, 0, 0, 
	81, 0, 0, 0, //16
	84, 0, 0, 0,
	85, 0, 0, 0,
	86, 0, 0, 0,
	87, 0, 0, 0, //20 
	88, 0, 0, 0,
	89, 0, 0, 0,
	29, 0, 0, 0,
	29, 0, 0, 0, //24
	29, 0, 0, 0,
	29, 0, 0, 0,
	29, 0, 0, 0,
};

unsigned char Array_Expanded[312] = {  4, 4, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 0, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 1, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 2, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 3, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 5, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 6, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 7, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 8, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 10, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 11, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 12, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 13, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 14, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 15, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 16, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 17, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 23, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 24, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 25, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 26, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	4, 27, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	5, 50, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	5, 51, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	5, 52, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	5, 53, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0 };

unsigned char cooldown_Array[124] = { 255, 255, 255, 255, 255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255,255, 255, 255, 255 };

// CLIENT EDITS ----
	// CUIStatusBar::OnCreate
	Memory::WriteByte(0x008D155C + 1, 0xF0); // Draw rest of quickslot bar
	Memory::WriteByte(0x008D155C + 2, 0x03);
	Memory::WriteByte(0x008D182E + 1, 0xF0); // Draw rest of hotkeys
	Memory::WriteByte(0x008D182E + 2, 0x03);
	Memory::WriteByte(0x008D1AC0 + 1, 0xF0); // Draw rest of cooldowns, who tf knows why. TY Rulax
	Memory::WriteByte(0x008D1AC0 + 2, 0x03);

	//----CQuickslotKeyMappedMan::CQuickslotKeyMappedMan?????
	Memory::WriteInt(0x0072B7CE + 1, (DWORD)&Array_aDefaultQKM_0); 
	Memory::WriteInt(0x0072B8EB +1, (DWORD)&Array_aDefaultQKM_0); 
	
	//----CUIStatusBar::CQuickSlot::CompareValidateFuncKeyMappedInfo
	Memory::WriteByte(0x008DD916, 0x1A); // increase 8 --> 26
	Memory::WriteByte(0x008DD8AD, 0x1A); // increase 8 --> 26
	Memory::WriteByte(0x008DD8FD, 0xBB); 
	Memory::WriteInt(0x008DD8FD + 1, (DWORD)&Array_Expanded);
	Memory::WriteByte(0x008DD8FD + 5, 0x90); //Errant byte
	Memory::WriteByte(0x008DD898, 0xB8);
	Memory::WriteInt(0x008DD898 + 1, (DWORD)&Array_Expanded);
	Memory::WriteByte(0x008DD898 + 5, 0x90); //Errant Byte

	//----CUIStatusBar::CQuickSlot::Draw
	Memory::WriteByte(0x008DE75E+3, 0x6C);
	Memory::WriteByte(0x008DDF99, 0xB8); 	
	Memory::WriteInt(0x008DDF99 + 1, (DWORD)&Array_Expanded); 
	Memory::FillBytes(0x008DDF99 + 5, 0x90, 3); // Nopping errant operations

	//----CUIStatusBar::OnMouseMove
	Memory::WriteByte(0x008D7F1E + 1, 0x34);
	Memory::WriteByte(0x008D7F1E + 2, 0x85);
	Memory::WriteInt(0x008D7F1E + 3, (DWORD)&Array_Expanded); 
	
	//----CUIStatusBar::CQuickSlot::GetPosByIndex
	Memory::WriteInt(0x008DE94D + 2, (DWORD)&Array_ptShortKeyPos); 
	Memory::WriteInt(0x008DE955 + 2, (DWORD)&Array_ptShortKeyPos + 4); 
	Memory::WriteByte(0x008DE941 + 2, 0x1A); //change cmp 8 --> cmp 26
	
	//CUIStatusBar::GetShortCutIndexByPos
	Memory::WriteInt(0x008DE8F4 + 1, (DWORD)&Array_ptShortKeyPos + 4);
	Memory::WriteByte(0x008DE926 + 1, 0x3E);

	//CUIStatusBar::CQuickSlot::DrawSkillCooltime
	Memory::WriteByte(0x008E099F + 3, 0x1A);
	Memory::WriteByte(0x008E069D, 0xBE);
	Memory::WriteInt(0x008E069D + 1, (DWORD)&cooldown_Array); //Pass enlarged FFFFF array
	Memory::WriteByte(0x008E069D + 5, 0x90); //Errant byte
	Memory::WriteByte(0x008E06A3, 0xBF);
	Memory::WriteInt(0x008E06A3 + 1, (DWORD)&Array_Expanded + 1);
	Memory::WriteByte(0x008E06A3 + 5, 0x90);
	
	//----CDraggableMenu::OnDropped
	Memory::WriteByte(0x004F928A + 2, 0x1A); //change cmp 8 --> cmp 26
	//----CDraggableMenu::MapFuncKey
	Memory::WriteByte(0x004F93F9 + 2, 0x1A); //change cmp 8 --> cmp 26
	//----CUIKeyConfig::OnDestroy
	Memory::WriteByte(0x00833797 + 2, 0x6C); // Updates the offset to 108 (triple) (old->24h)
	Memory::WriteByte(0x00833841 + 2, 0x6C); // Updates the offset to 108 (triple) (old->24h)
	Memory::WriteByte(0x00833791 + 1, 0x68); // push 68h (triple)
	Memory::WriteByte(0x0083383B + 1, 0x68); // push 68h (triple)
	//----CUIKeyConfig::~CUIKeyConfig
	Memory::WriteByte(0x0083287F + 2, 0x6C); // triple the base value at this hex (old->24h)
	Memory::WriteByte(0x00832882 + 1, 0x68); // push 68h (triple)
	//----CQuickslotKeyMappedMan::SaveQuickslotKeyMap
	Memory::WriteByte(0x0072B8C0 + 2, 0x6C); // triple the base value at this hex (old->24h)
	Memory::WriteByte(0x0072B8A0 + 1, 0x68); // push 68h, (triple) //CQuickslotKeyMappedMan::SaveQuickslotKeyMap
	Memory::WriteByte(0x0072B8BD + 1, 0x68); // push 68h, (triple) //CQuickslotKeyMappedMan::SaveQuickslotKeyMap
	//----CQuickslotKeyMappedMan::OnInit
	Memory::WriteByte(0x0072B861 + 1, 0x68); // push 68h (triple) (these ones might have to be just 60)
	Memory::WriteByte(0x0072B867 + 2, 0x6C); // triple the base value at this hex (old->24h)
	//----CUIKeyConfig::CNoticeDlg::OnChildNotify????
	Memory::WriteByte(0x00836A1E + 1, 0x68); // push 68h (triple)
	Memory::WriteByte(0x00836A21 + 2, 0x6C); // triple the base value at this hex (old->24h)


// CODECAVES CLIENT EDITS ---- 
	Memory::CodeCave(CompareValidateFuncKeyMappedInfo_cave, 0x8DD8B8, 5);
	Memory::CodeCave(sub_9FA0CB_cave, 0x9FA0DB, 5);
	Memory::CodeCave(sDefaultQuickslotKeyMap_cave, 0x72B7BC, 5);
	Memory::CodeCave(DefaultQuickslotKeyMap_cave, 0x72B8E6, 5);
	Memory::CodeCave(Restore_Array_Expanded, 0x008CFDFD, 6); //restores the skill array to 0s

// CODECAVES --- 
DWORD Array_aDefaultQKM_Address = (DWORD)&Array_aDefaultQKM;
DWORD Array_mystery_Address = (DWORD)&Array_Expanded;
DWORD Array_mystery_Address_plus = (DWORD)&Array_Expanded+1;
DWORD cooldown_Array_Address = (DWORD)&cooldown_Array;

DWORD CompareValidate_Retn = 0x8DD8BD;
_declspec(naked) void CompareValidateFuncKeyMappedInfo_cave()
{
	_asm
	{
		push 0x138;
		push 0x0;
		push eax;
		pushad;
		popad;
		jmp CompareValidate_Retn
		//push 0x8DD8BD;
		//ret;
	}
}

DWORD sub_9FA0CB_cave_retn_1 = 0x9FA0E1;
_declspec(naked) void sub_9FA0CB_cave()
{
	_asm {
		test eax, eax;
		jne label;
		push 0xD4;
		pushad;
		popad;
		// -> ZAllocEx<ZAllocAnonSelector>::Alloc(ZAllocEx<ZAllocAnonSelector>::_s_alloc, 0x44u);
		//push 0x9FA0E1;
		//ret;
		jmp sub_9FA0CB_cave_retn_1
	label:
		push 0x138;
		push 0x0;
		push eax;
		pushad;
		popad;
		// -> memset(this + 0xD20, 0, 0x60u);
		//push 0x8DD8BD;
		//ret;
		jmp CompareValidate_Retn
	}
}
//DWORD sDefaultQuickslotKeyMap_cave_retn = 0x72B7C2;
_declspec(naked) void sDefaultQuickslotKeyMap_cave()
{
	_asm {
		push ebx;
		push esi;
		push edi;
		xor edx, edx;
		mov ebx, ecx;
		call label;
		nop;
		lea edi, dword ptr ds : [ebx + 0x4] ;
		mov ecx, 0x1A;
		mov esi, Array_aDefaultQKM_Address;
		rep movsd;
		lea edi, dword ptr ds : [ebx + 0x6C] ;
		mov ecx, 0x1A;
		mov esi, Array_aDefaultQKM_Address;
		rep movsd;
		pop edi;
		pop esi;
		pop ebx;
		ret;
		// 0xBF8EE8
	label:
		push esi;
		mov esi, ecx;
		lea eax, dword ptr ds : [esi + 0x4] ;
		// -> _DWORD *__fastcall sub_72B7BC(_DWORD *a1)
		push 0x72B7C2;
		ret;
		//jmp sDefaultQuickslotKeyMap_cave_retn
	}
}
_declspec(naked) void DefaultQuickslotKeyMap_cave()
{
	_asm {
		push esi;
		push edi;
		lea eax, dword ptr ds : [ecx + 0x4] ;
		mov esi, Array_aDefaultQKM_Address;
		mov ecx, 0x1A;
		mov edi, eax;
		rep movsd;
		pop edi;
		pop esi;
		ret;
	}
}