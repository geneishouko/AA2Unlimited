#pragma once
#include <Windows.h>

#include "../CharacterStruct.h"

namespace ExtClass {

#pragma pack(push, 1)
/*
 * Represents the data of one of the two participants in h
 */
class HParticipant
{
public:
	CharacterStruct* m_charPtr;
	BYTE m_unknown1[0x1C];
	DWORD m_nClimedInFront;
	BYTE m_unknown2[4];
	DWORD m_nClimedInRear;
	BYTE m_unknown3[0xC];
	DWORD m_nClim;
	DWORD m_nClimTogether;
	BYTE m_unknown4[0x14];
	DWORD m_nPosChanges;
	BYTE m_unknown5[8];
	DWORD m_nClimedOutside;
	BYTE m_unknown6[0xC];
	void* m_someSoundPointer;
	BYTE m_unknown7[0x14];
	DWORD m_nCremeOnFront;
	DWORD m_nCremeOn1;
	DWORD m_nCremeOn2;
	DWORD m_nCremeOnBack;
	DWORD m_nCremeOn3;
	WORD m_unknown8;
	WORD m_clothesState;
	BYTE m_bClothesSlipped;
	BYTE m_unknown9;
	BYTE m_shoesOffState; //0 = all on, 1 = shoe off, 2 = shoe & knees off, 3 = shoe off again?!
public:
	HParticipant() = delete;
	~HParticipant() = delete;
#define LUA_CLASS ExtClass::HParticipant
	static inline void bindLua() {
	LUA_BIND(m_charPtr)
	LUA_BIND(m_shoesOffState)
	LUA_BIND(m_bClothesSlipped)
	LUA_BIND(m_clothesState)
	LUA_BIND(m_nCremeOn3)
	LUA_BIND(m_nCremeOnBack)
	LUA_BIND(m_nCremeOn2)
	LUA_BIND(m_nCremeOn1)
	LUA_BIND(m_nCremeOnFront)
	LUA_BIND(m_nClimedOutside)

	LUA_BIND(m_nPosChanges)
	LUA_BIND(m_nClimTogether)
	LUA_BIND(m_nClim)
	LUA_BIND(m_nClimedInRear)
	LUA_BIND(m_nClimedInFront)
	}
#undef LUA_CLASS
};

static_assert(sizeof(HParticipant) == 0xA3,"HParticipant struct must have size of A3");

#pragma pack(pop)
}