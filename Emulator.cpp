#include "stdafx.h"
#include <winsock2.h>

#pragma comment(lib, "wsock32.lib")


#define STUDENT_NUMBER    "15024059"

#define IP_ADDRESS_SERVER "127.0.0.1"

#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.

#define WORD  unsigned short
#define DWORD unsigned long
#define BYTE  unsigned char

#define MAX_FILENAME_SIZE 500
#define MAX_BUFFER_SIZE   500

SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

SOCKET sock;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;

char InputBuffer[MAX_BUFFER_SIZE];

char hex_file[MAX_BUFFER_SIZE];
char trc_file[MAX_BUFFER_SIZE];

//////////////////////////
//   Registers          //
//////////////////////////

#define FLAG_I  0x10 //0001 0000  HEX so it's actually 0x 1 0 (1 zero) not 0x10(ten) (Interupt)
#define FLAG_Z  0x04 //0000 0100 Zero
#define FLAG_N  0x02 //0000 0010 Negative 
#define FLAG_C  0x01 //0000 0001 Carry 
#define REGISTER_M 4
#define REGISTER_A 3
#define REGISTER_B 2
#define REGISTER_H 1   //HIGHBYTE  (ON)
#define REGISTER_L 0	//LOWBYTE   (OFF)
#define REGISTER_X 0
#define REGISTER_Y 1

BYTE Index_Registers[2];

BYTE Registers[5]; //Incremented from 4 20161020
BYTE Flags;
WORD ProgramCounter;
WORD StackPointer;


////////////
// Memory //
////////////

#define MEMORY_SIZE	65536

BYTE Memory[MEMORY_SIZE];

#define TEST_ADDRESS_1  0x01FA
#define TEST_ADDRESS_2  0x01FB
#define TEST_ADDRESS_3  0x01FC
#define TEST_ADDRESS_4  0x01FD
#define TEST_ADDRESS_5  0x01FE
#define TEST_ADDRESS_6  0x01FF
#define TEST_ADDRESS_7  0x0200
#define TEST_ADDRESS_8  0x0201
#define TEST_ADDRESS_9  0x0202
#define TEST_ADDRESS_10  0x0203
#define TEST_ADDRESS_11  0x0204
#define TEST_ADDRESS_12  0x0205


///////////////////////
// Control variables //
///////////////////////

bool memory_in_range = true;
bool halt = false;


///////////////////////
// Disassembly table //
///////////////////////

char opcode_mneumonics[][14] =
{
	"ILLEGAL     ",
	"DECX impl    ",
	"INCX impl    ",
	"DEY impl     ",
	"INCY impl    ",
	"CLC impl     ",
	"STC impl     ",
	"CLI impl     ",
	"STI impl     ",
	"ILLEGAL     ",
	"LDAA  #      ",
	"LDAB  #      ",
	"LX  #,L      ",
	"LX  #,L      ",
	"LDX  #       ",
	"LDY  #       ",

	"JMP abs      ",
	"JCC abs      ",
	"JCS abs      ",
	"JNE abs      ",
	"JEQ abs      ",
	"JMI abs      ",
	"JPL abs      ",
	"JHI abs      ",
	"JLE abs      ",
	"ILLEGAL     ",
	"LDAA abs     ",
	"LDAB abs     ",
	"MVI  #,L     ",
	"MVI  #,H     ",
	"LDX abs      ",
	"LDY abs      ",

	"LODS  #      ",
	"JSR abs      ",
	"CCC abs      ",
	"CCS abs      ",
	"CNE abs      ",
	"CEQ abs      ",
	"CMI abs      ",
	"CPL abs      ",
	"CHI abs      ",
	"CLE abs      ",
	"LDAA abs,X   ",
	"LDAB abs,X   ",
	"NOP impl     ",
	"HLT impl     ",
	"LDX abs,X    ",
	"LDY abs,X    ",

	"LODS abs     ",
	"ADC A,L      ",
	"SBC A,L      ",
	"ADD A,L      ",
	"SUB A,L      ",
	"CMP A,L      ",
	"OR A,L       ",
	"AND A,L      ",
	"XOR A,L      ",
	"BIT A,L      ",
	"LDAA abs,Y   ",
	"LDAB abs,Y   ",
	"ILLEGAL     ",
	"ILLEGAL     ",
	"LDX abs,Y    ",
	"LDY abs,Y    ",

	"LODS abs,X   ",
	"ADC A,H      ",
	"SBC A,H      ",
	"ADD A,H      ",
	"SUB A,H      ",
	"CMP A,H      ",
	"OR A,H       ",
	"AND A,H      ",
	"XOR A,H      ",
	"BIT A,H      ",
	"LDAA (ind)   ",
	"LDAB (ind)   ",
	"RET impl     ",
	"ILLEGAL     ",
	"LDX (ind)    ",
	"LDY (ind)    ",

	"LODS abs,Y   ",
	"ADC A,M      ",
	"SBC A,M      ",
	"ADD A,M      ",
	"SUB A,M      ",
	"CMP A,M      ",
	"OR A,M       ",
	"AND A,M      ",
	"XOR A,M      ",
	"BIT A,M      ",
	"LDAA (ind,X) ",
	"LDAB (ind,X) ",
	"SWI impl     ",
	"RTI impl     ",
	"LDX (ind,X)  ",
	"LDY (ind,X)  ",

	"LODS (ind)   ",
	"ADC B,L      ",
	"SBC B,L      ",
	"ADD B,L      ",
	"SUB B,L      ",
	"CMP B,L      ",
	"OR B,L       ",
	"AND B,L      ",
	"XOR B,L      ",
	"BIT B,L      ",
	"STOS abs     ",
	"MOVE A,A     ",
	"MOVE B,A     ",
	"MOVE L,A     ",
	"MOVE H,A     ",
	"MOVE M,A     ",

	"LODS (ind,X) ",
	"ADC B,H      ",
	"SBC B,H      ",
	"ADD B,H      ",
	"SUB B,H      ",
	"CMP B,H      ",
	"OR B,H       ",
	"AND B,H      ",
	"XOR B,H      ",
	"BIT B,H      ",
	"STOS abs,X   ",
	"MOVE A,B     ",
	"MOVE B,B     ",
	"MOVE L,B     ",
	"MOVE H,B     ",
	"MOVE M,B     ",

	"ILLEGAL     ",
	"ADC B,M      ",
	"SBC B,M      ",
	"ADD B,M      ",
	"SUB B,M      ",
	"CMP B,M      ",
	"OR B,M       ",
	"AND B,M      ",
	"XOR B,M      ",
	"BIT B,M      ",
	"STOS abs,Y   ",
	"MOVE A,L     ",
	"MOVE B,L     ",
	"MOVE L,L     ",
	"MOVE H,L     ",
	"MOVE M,L     ",

	"ILLEGAL     ",
	"ILLEGAL     ",
	"ILLEGAL     ",
	"SBIA  #      ",
	"SBIB  #      ",
	"CPIA  #      ",
	"CPIB  #      ",
	"ORIA  #      ",
	"ORIB  #      ",
	"ILLEGAL     ",
	"STOS (ind)   ",
	"MOVE A,H     ",
	"MOVE B,H     ",
	"MOVE L,H     ",
	"MOVE H,H     ",
	"MOVE M,H     ",

	"INC abs      ",
	"DEC abs      ",
	"RRC abs      ",
	"RLC abs      ",
	"SAL abs      ",
	"SAR abs      ",
	"LSR abs      ",
	"COM abs      ",
	"ROL abs      ",
	"RR abs       ",
	"STOS (ind,X) ",
	"MOVE A,M     ",
	"MOVE B,M     ",
	"MOVE L,M     ",
	"MOVE H,M     ",
	"MOVE -,-     ",

	"INC abs,X    ",
	"DEC abs,X    ",
	"RRC abs,X    ",
	"RLC abs,X    ",
	"SAL abs,X    ",
	"SAR abs,X    ",
	"LSR abs,X    ",
	"COM abs,X    ",
	"ROL abs,X    ",
	"RR abs,X     ",
	"STORA abs    ",
	"STORB abs    ",
	"STOX abs     ",
	"STOY abs     ",
	"PUSH  ,A     ",
	"POP A,       ",

	"INC abs,Y    ",
	"DEC abs,Y    ",
	"RRC abs,Y    ",
	"RLC abs,Y    ",
	"SAL abs,Y    ",
	"SAR abs,Y    ",
	"LSR abs,Y    ",
	"COM abs,Y    ",
	"ROL abs,Y    ",
	"RR abs,Y     ",
	"STORA abs,X  ",
	"STORB abs,X  ",
	"STOX abs,X   ",
	"STOY abs,X   ",
	"PUSH  ,B     ",
	"POP B,       ",

	"INCA A,A     ",
	"DECA A,A     ",
	"RRCA A,A     ",
	"RLCA A,A     ",
	"SALA A,A     ",
	"SARA A,A     ",
	"LSRA A,A     ",
	"COMA A,A     ",
	"ROLA A,A     ",
	"RRA A,A      ",
	"STORA abs,Y  ",
	"STORB abs,Y  ",
	"STOX abs,Y   ",
	"STOY abs,Y   ",
	"PUSH  ,s     ",
	"POP s,       ",

	"INCB B,B     ",
	"DECB B,B     ",
	"RRCB B,B     ",
	"RLCB B,B     ",
	"SALB B,B     ",
	"SARB B,B     ",
	"LSRB B,B     ",
	"COMB B,B     ",
	"ROLB B,B     ",
	"RRB B,B      ",
	"STORA (ind)  ",
	"STORB (ind)  ",
	"STOX (ind)   ",
	"STOY (ind)   ",
	"PUSH  ,L     ",
	"POP L,       ",

	"CAY impl     ",
	"MYA impl     ",
	"CSA impl     ",
	"ABA impl     ",
	"SBA impl     ",
	"AAB impl     ",
	"SAB impl     ",
	"ADCP A,L     ",
	"SBCP A,L     ",
	"XCHG A,L     ",
	"STORA (ind,X)",
	"STORB (ind,X)",
	"STOX (ind,X) ",
	"STOY (ind,X) ",
	"PUSH  ,H     ",
	"POP H,       ",

};

////////////////////////////////////////////////////////////////////////////////
//                           Simulator/Emulator (Start)                       //
////////////////////////////////////////////////////////////////////////////////
BYTE fetch()
{
	BYTE byte = 0;

	if ((ProgramCounter >= 0) && (ProgramCounter <= MEMORY_SIZE))
	{
		memory_in_range = true;
		byte = Memory[ProgramCounter];
		ProgramCounter++;
	}
	else
	{
		memory_in_range = false;
	}
	return byte;
}
void set_flag_z(BYTE inReg) {
	BYTE reg;
	reg = inReg;

	if (reg == 0x00) {
		Flags = Flags | FLAG_Z; //set
	}
	else
	{
		Flags = Flags & (~FLAG_Z); //reset
	}
}
void set_flag_n(WORD inReg) {
	BYTE reg;
	reg = inReg;
	if ((reg & 0x80) == 0x80) {
		Flags = Flags | FLAG_N;	//set N
	}
	else {
		Flags = Flags & (~FLAG_N); //reset N 
	}
}
//CHECK
void set_flag_c(WORD result)
{
	if (result >= 0x100)	//doesnt fit in byte
		Flags = Flags | FLAG_C;
	else
		Flags = Flags & (~FLAG_C);
}


void set_all_flags(BYTE inReg1, BYTE inReg2, WORD result) //SEND REGISTERS AND DATA 
{

	set_flag_c(result);
	set_flag_n(result);
	set_flag_z(result);
}
void set_three_flags(WORD result) //SEND DATA 
{
	set_flag_c(result);
	set_flag_n(result);
	set_flag_z(result);
}

BYTE addRegs(BYTE inReg1, BYTE inReg2)
{
	WORD result = inReg1 + inReg2;
	set_all_flags(inReg1, inReg2, result);
	return (BYTE)result;
}

BYTE subWithCarry(BYTE inReg1, BYTE inReg2)
{
	WORD result;
	BYTE carry = Flags & FLAG_C;
	result = inReg1 - inReg2 - carry;
	set_all_flags(inReg1, ~inReg2 + 1, result);			//reg 2 is data fetched				
	return (BYTE)result;
}

BYTE addWithCarry(BYTE inReg1, BYTE inReg2)
{
	WORD result;
	BYTE carry = Flags & FLAG_C;
	result = inReg1 + inReg2 + carry;
	set_all_flags(inReg1, inReg2, result);
	return (BYTE)result;
}

BYTE subRegs(BYTE inReg1, BYTE inReg2)
{
	WORD result = inReg1 - inReg2;
	set_all_flags(inReg1, ~inReg2 + 1, result);
	return (BYTE)result;
}

BYTE rotateRightCarry(BYTE inReg)
{
	BYTE result, carry = 0x00;
	WORD data;
	if ((Flags & FLAG_C) == 0x01)
		carry = 0x80;
	data = (inReg >> 1) | carry;
	if ((inReg & 0x01) == 0x01)
	{
		data = data | 0x100;
	}
	result = (BYTE)data;
	set_three_flags(data);
	return result;

}

BYTE rotateLeftCarry(BYTE inReg)
{
	BYTE result, carry;
	carry = Flags & FLAG_C;
	WORD data = (WORD)(inReg << 1) | carry;
	result = (BYTE)data;

	set_three_flags(data);
	return result;
}

BYTE rotateLeft(BYTE inReg)
{
	BYTE result;
	result = inReg << 1;
	if ((inReg & 0x80) == 0x80)
		result = result | 0x01;


	set_flag_z(result);
	set_flag_n(result);

	return result;
}
BYTE rotateRight(BYTE inReg)
{
	BYTE result;
	result = inReg >> 1;
	if ((inReg & 0x01) == 0x01)
		result = result | 0x80;

	set_flag_z(result);
	set_flag_n(result);
	return result;
}

WORD getAbsAd() {
	BYTE LB = 0;
	BYTE HB = 0;
	WORD address = 0;

	HB = fetch();
	LB = fetch();
	address += (WORD)((WORD)HB << 8) + LB;

	return address;
}
void push(BYTE reg) {
	Memory[StackPointer] = reg;
	StackPointer--;
}

//Group 1 = Loading Information/Data
void Group_1(BYTE opcode) {
	BYTE LB = 0;
	BYTE HB = 0;
	WORD address = 0;
	WORD data = 0;
	BYTE dataByte = 0;
	WORD temp_word;
	BYTE param1;
	BYTE param2;
	BYTE carry = 0;
	switch (opcode) {



		//LDAA
		/*
		Loads Memory into accumulator
		*/
	case 0x0A: //LDAA Immidiate #
		data = fetch();
		Registers[REGISTER_A] = data;
		break;

	case 0x1A: //LDAA abs
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Registers[REGISTER_A] = Memory[address];
		}

		break;

	case 0x2A://LDAA abs,X
		address += Index_Registers[REGISTER_X];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Registers[REGISTER_A] = Memory[address];
		}
		break;

	case 0x3A: //LDAA abs,Y
		address += Index_Registers[REGISTER_Y];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Registers[REGISTER_A] = Memory[address];
		}

		break;

	case 0x4A:  //(in ) 

		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Registers[REGISTER_A] = Memory[address];
		}
		break;

	case 0x5A: //(indirect) x
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		address += Index_Registers[REGISTER_X];
		if (address >= 0 && address < MEMORY_SIZE) {
			Registers[REGISTER_A] = Memory[address];
		}
		break;

		//LDAB
		/*
		Loads Memory into Accumulator
		*/
	case 0x0B: //LDAB Immidiate #
		data = fetch();
		Registers[REGISTER_B] = data;
		break;

	case 0x1B: //LDAB abs 
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Registers[REGISTER_B] = Memory[address];
		}
		break;

	case 0x2B: //LDAB abs, X 
		address += Index_Registers[REGISTER_X];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Registers[REGISTER_B] = Memory[address];
		}
		break;

	case 0x3B: //LDAB abs,Y
		address += Index_Registers[REGISTER_Y];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Registers[REGISTER_B] = Memory[address];
		}

		break;

	case 0x4B: //(ind)

		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Registers[REGISTER_B] = Memory[address];
		}
		break;

	case 0x5B: //(indirect) x
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		address += Index_Registers[REGISTER_X];
		if (address >= 0 && address < MEMORY_SIZE) {
			Registers[REGISTER_B] = Memory[address];
		}
		break;



		//STORA BEGINS

		/*
		Stores Accumulator
		*/
	case 0xBA: //STORA (abs)
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Registers[REGISTER_A];
		}
		break;

	case 0xCA: //STORA (abs,X)
		address += Index_Registers[REGISTER_X];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Registers[REGISTER_A];
		}
		break;

	case 0xDA: //STORA (abs,Y)
		address += Index_Registers[REGISTER_Y];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Registers[REGISTER_A];
		}
		break;

	case 0xEA: //STORA ((ind))
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Registers[REGISTER_A];
		}
		break;

	case 0xFA: //STORA ((ind,X))
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		address += Index_Registers[REGISTER_X];
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Registers[REGISTER_A];
		}
		break;

		//MVI 
		//L,#	
	case 0x1C:
		data = fetch();
		Registers[REGISTER_L] = data;
		break;

	case 0x1D: //H,#
		data = fetch();
		Registers[REGISTER_H] = data;
		break;

		//LX  

	case 0x0C: //LH,# IMMEDIATE
		data = fetch();
		Registers[REGISTER_L] = data;

		break;


	case 0x0D:  //LH,# IMMEDIUATE
		data = fetch();
		Registers[REGISTER_H] = data;
		//Register[REGISTER_L] = data 
		break;

		//CSA 
		/*
		Transfers Status register(Flags) to Accumulator (Reg A)
		*/
	case 0xF2: //impl
		Registers[REGISTER_A] = Flags;
		break;


		//STORB 
	case 0xBB: //STORB (abs)
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Registers[REGISTER_B];
		}
		break;

	case 0xCB: //STORB (abs,X)
		address += Index_Registers[REGISTER_X];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Registers[REGISTER_B];
		}
		break;

	case 0xDB: //STORB (abs,Y)
		address += Index_Registers[REGISTER_Y];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Registers[REGISTER_B];
		}
		break;

	case 0xEB: //STORB ((ind))
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Registers[REGISTER_B];
		}
		break;

	case 0xFB: //STORB ((ind,X))
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		address += Index_Registers[REGISTER_X];
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Registers[REGISTER_B];
		}
		break;

		//LDX Begins Here 
	case 0x0E: //LDX Immidiate #
		data = fetch();
		Index_Registers[REGISTER_X] = data;
		break;

	case 0x1E: //LDX abs
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Index_Registers[REGISTER_X] = Memory[address];
		}

		break;

	case 0x2E:
		address += Index_Registers[REGISTER_X];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Index_Registers[REGISTER_X] = Memory[address];
		}
		break;

	case 0x3E: //LDX abs,Y
		address += Index_Registers[REGISTER_Y];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Index_Registers[REGISTER_X] = Memory[address];
		}

		break;

	case 0x4E:

		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Index_Registers[REGISTER_X] = Memory[address];
		}
		break;

	case 0x5E: //indirect x
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		address += Index_Registers[REGISTER_X];
		if (address >= 0 && address < MEMORY_SIZE) {
			Index_Registers[REGISTER_X] = Memory[address];
		}
		break;




		//STOX Begins


	case 0xBC:
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Index_Registers[REGISTER_X];
		}
		break;

	case 0xCC:
		address += Index_Registers[REGISTER_X];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Index_Registers[REGISTER_X];
		}
		break;

	case 0xDC:

		address += Index_Registers[REGISTER_Y];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Index_Registers[REGISTER_X];
		}
		break;


	case 0xEC:
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Index_Registers[REGISTER_X];
		}

		break;

	case 0xFC:
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		address += Index_Registers[REGISTER_X];
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Index_Registers[REGISTER_X];
		}
		break;
		//LDY Begins Here 
	case 0x0F: //LDY Immidiate #
		data = fetch();
		Index_Registers[REGISTER_Y] = data;
		break;

	case 0x1F: //LDY abs
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Index_Registers[REGISTER_Y] = Memory[address];
		}

		break;

	case 0x2F:
		address += Index_Registers[REGISTER_X];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Index_Registers[REGISTER_Y] = Memory[address];
		}
		break;

	case 0x3F: //LDY abs,Y
		address += Index_Registers[REGISTER_Y];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Index_Registers[REGISTER_Y] = Memory[address];
		}

		break;

	case 0x4F:

		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Index_Registers[REGISTER_Y] = Memory[address];
		}
		break;

	case 0x5F: //indirect x
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		address += Index_Registers[REGISTER_X];
		if (address >= 0 && address < MEMORY_SIZE) {
			Index_Registers[REGISTER_Y] = Memory[address];
		}
		break;
		//STOY BEGINS
	case 0xBD:
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Index_Registers[REGISTER_Y];
		}
		break;

	case 0xCD:
		address += Index_Registers[REGISTER_X];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Index_Registers[REGISTER_Y];
		}
		break;

	case 0xDD:

		address += Index_Registers[REGISTER_Y];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Index_Registers[REGISTER_Y];
		}
		break;


	case 0xED:
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Index_Registers[REGISTER_Y];
		}

		break;

	case 0xFD:
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		address += Index_Registers[REGISTER_X];
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Index_Registers[REGISTER_Y];
		}
		break;
		//CAY 
		/*
		Transfer Accumulator(REG A) to Register Y
		*/
	case 0xF0:  //impl
		Index_Registers[REGISTER_Y] = Registers[REGISTER_A];
		set_flag_n(Index_Registers[REGISTER_Y]);
		break;

		//MYA 
		/*
		Transfers register Y to Accumulater(Reg A)
		*/
	case 0xF1:   //impl 
		Registers[REGISTER_A] = Index_Registers[REGISTER_Y];
		break;

		//ABA 
		/*
		Adds Accumulator B into Accumlator A
		Refer to PDF to refresh on Flags if need be
		*/
	case 0xF3: //impl 
		data = Registers[REGISTER_A] + Registers[REGISTER_B];
		Registers[REGISTER_A] = (BYTE)data;
		set_three_flags(data);
		break;
		//SBA
		/*
		Subtracts Accumulator B from Accumulator A
		*/
	case 0xF4: //impl 
		data = Registers[REGISTER_A] - Registers[REGISTER_B];
		Registers[REGISTER_A] = (BYTE)data;
		set_three_flags(data);
		break;
		//AAB
		/*
		Adds Accumulator A into Accumulator B
		*/
	case 0xF5: //impl
		data = Registers[REGISTER_B] + Registers[REGISTER_A]; // Sets data to equal the sum of two registers added together
		Registers[REGISTER_B] = (BYTE)data; //Set the register to equal the sum done above (cast into a byte incase)
		set_three_flags(data);	//Set all of the flags by passing the sum to it which bitwise operations will be carried out on
		break;
		//SAB
		/*
		Subtracts Accumulator A from Accumulator B
		*/
	case 0xF6: //impl
		data = Registers[REGISTER_B] - Registers[REGISTER_A];
		Registers[REGISTER_B] = (BYTE)data;
		set_three_flags(data);
		break;


		//LODS

	case 0x20: //#
		data = fetch();
		StackPointer = data << 8; StackPointer += fetch();
		break;

	case 0x30: //abs
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE - 1) {
			StackPointer = (WORD)Memory[address] << 8;
			StackPointer += Memory[address + 1];
		}
		break;

	case 0x40: //abx,X
		address += Index_Registers[REGISTER_X];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE - 1) {
			StackPointer = (WORD)Memory[address] << 8;
			StackPointer += Memory[address + 1];
		}
		break;

	case 0x50: //abs,Y
		address += Index_Registers[REGISTER_Y];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE - 1) {
			StackPointer = (WORD)Memory[address] << 8;
			StackPointer += Memory[address + 1];
		}
		break;

	case 0x60: //(ind)
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE - 1) {
			StackPointer = (WORD)Memory[address] << 8;
			StackPointer += Memory[address + 1];
		}
		break;

	case 0x70: //ind,X
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		address += Index_Registers[REGISTER_X];
		if (address >= 0 && address < MEMORY_SIZE - 1) {
			StackPointer = (WORD)Memory[address] << 8;
			StackPointer += Memory[address + 1];
			break;
		}



	case 0x6A:
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE - 1) {
			//Memory[address] = StackPointer;
			StackPointer = Memory[address];
		}
		break;

	case 0x7A:
		address += Index_Registers[REGISTER_X];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE - 1) {
			//Memory[address] = StackPointer;
			StackPointer = Memory[address];
		}
		break;

	case 0x8A:

		address += Index_Registers[REGISTER_Y];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE - 1) {
			//Memory[address] = StackPointer;
			StackPointer = Memory[address];
		}
		break;


	case 0x9A:
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE - 1) {
			//	Memory[address] = StackPointer;
			StackPointer = Memory[address];
		}

		break;

	case 0xAA:
		HB = fetch();
		LB = fetch();
		address = (WORD)((WORD)HB << 8) + LB;
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
		address += Index_Registers[REGISTER_X];
		if (address >= 0 && address < MEMORY_SIZE - 1) {
			//Memory[address] = StackPointer;
			StackPointer = Memory[address];
		}
		break;

		//NEW ADC
	case 0x31:  // A - L   (L moved to A) 
		Registers[REGISTER_A] = addWithCarry(Registers[REGISTER_A], Registers[REGISTER_L]);
		break;

	case 0x41:   // A-H
		Registers[REGISTER_A] = addWithCarry(Registers[REGISTER_A], Registers[REGISTER_H]);
		break;
	case 0x51: //A-M
		Registers[REGISTER_A] = addWithCarry(Registers[REGISTER_A], Registers[REGISTER_M]);
		break;

	case 0x61: //B-L 
		Registers[REGISTER_B] = addWithCarry(Registers[REGISTER_B], Registers[REGISTER_L]);
		break;

	case 0x71: //B-H
		Registers[REGISTER_B] = addWithCarry(Registers[REGISTER_B], Registers[REGISTER_H]);
		break;

	case 0x81: // B-M
		Registers[REGISTER_B] = addWithCarry(Registers[REGISTER_B], Registers[REGISTER_M]);
		break;


		//CMP
		/*Register compared to Accumulator*/
	case 0x35: // A-L
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_L];
		temp_word = (WORD)param1 - (WORD)param2;
		if (temp_word >= 0x100) {
			Flags = Flags | FLAG_C; // Set carry flag
		}
		else {
			Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		break;


	case 0x45: //A-H
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_H];
		temp_word = (WORD)param1 - (WORD)param2;
		if (temp_word >= 0x100) {
			Flags = Flags | FLAG_C; // Set carry flag
		}
		else {
			Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		break;

	case 0x55: //A-M
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_M];
		temp_word = (WORD)param1 - (WORD)param2;
		if (temp_word >= 0x100) {
			Flags = Flags | FLAG_C; // Set carry flag
		}
		else {
			Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		break;

	case 0x65: //B-L
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_L];
		temp_word = (WORD)param1 - (WORD)param2;
		if (temp_word >= 0x100) {
			Flags = Flags | FLAG_C; // Set carry flag
		}
		else {
			Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		break;


	case 0x75: //B-H
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_H];
		temp_word = (WORD)param1 - (WORD)param2;
		if (temp_word >= 0x100) {
			Flags = Flags | FLAG_C; // Set carry flag
		}
		else {
			Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		break;

	case 0x85: //B-M
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_M];
		temp_word = (WORD)param1 - (WORD)param2;
		if (temp_word >= 0x100) {
			Flags = Flags | FLAG_C; // Set carry flag
		}
		else {
			Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		break;

		//ADD
		/*
		Register is added to accumulator    FLAGS INVOLVED
		*/
	case 0x33: // A- L (L is stored in A) 
		Registers[REGISTER_A] = addRegs(Registers[REGISTER_A], Registers[REGISTER_L]);
		break;

	case 0x43:	//A - H 
		Registers[REGISTER_A] = addRegs(Registers[REGISTER_A], Registers[REGISTER_H]);
		break;

	case 0x53:  //A-M
		Registers[REGISTER_A] = addRegs(Registers[REGISTER_A], Registers[REGISTER_M]);
		break;

	case 0x63: //B-L 
		Registers[REGISTER_B] = addRegs(Registers[REGISTER_B], Registers[REGISTER_L]);
		break;

	case 0x73:  //B-H
		Registers[REGISTER_B] = addRegs(Registers[REGISTER_B], Registers[REGISTER_H]);
		break;

	case 0x83:   //B-M
		Registers[REGISTER_B] = addRegs(Registers[REGISTER_B], Registers[REGISTER_M]);
		break;

		//CLC CHECK
		/*
		Clears Carry flag
		*/
	case 0x05: //impl
		Flags = (Flags & (~FLAG_C));
		break;

		//STC
		/*
		Set Carry flag
		*/
	case 0x06:
		Flags = Flags | FLAG_C;
		break;

		//CLI
		/*
		Clear Interrupt flag
		*/
	case 0x07:
		Flags = (Flags & (~FLAG_I));
		break;

		//STI
		/*
		Set Interupt flag
		*/

	case 0x08:
		Flags = Flags | FLAG_I;
		break;

		//SBC
		/*
		Register subtracted to Accumulator with Carry
		*/
	case 0x32: //A-L   A minus L 
		Registers[REGISTER_A] = subWithCarry(Registers[REGISTER_A], Registers[REGISTER_L]);
		break;

	case 0x42: //A-H
		Registers[REGISTER_A] = subWithCarry(Registers[REGISTER_A], Registers[REGISTER_H]);
		break;


	case 0x52: //A-M 
		Registers[REGISTER_A] = subWithCarry(Registers[REGISTER_A], Registers[REGISTER_M]);
		break;

	case 0x62: //B-L
		Registers[REGISTER_B] = subWithCarry(Registers[REGISTER_B], Registers[REGISTER_L]);
		break;

	case 0x72: // B-H
		Registers[REGISTER_B] = subWithCarry(Registers[REGISTER_B], Registers[REGISTER_H]);
		break;


	case 0x82: //B-M
		Registers[REGISTER_B] = subWithCarry(Registers[REGISTER_B], Registers[REGISTER_M]);
		break;

		//SUB
		/*
		Register Subtracted to Accumulator
		*/
	case 0x34:  // A-L  
		Registers[REGISTER_A] = subRegs(Registers[REGISTER_A], Registers[REGISTER_L]);
		break;

	case 0x44:   //A-H
		Registers[REGISTER_A] = subRegs(Registers[REGISTER_A], Registers[REGISTER_H]);
		break;

	case 0x54: //A-M
		Registers[REGISTER_A] = subRegs(Registers[REGISTER_A], Registers[REGISTER_M]);
		break;

	case 0x64:	//B-L
		Registers[REGISTER_B] = subRegs(Registers[REGISTER_B], Registers[REGISTER_L]);
		break;

	case 0x74: //B-H
		Registers[REGISTER_B] = subRegs(Registers[REGISTER_B], Registers[REGISTER_H]);
		break;

	case 0x84:  //B-M 
		Registers[REGISTER_B] = subRegs(Registers[REGISTER_B], Registers[REGISTER_M]);
		break;

		//OR 
		/*
		Register bitewise inclusive or with Accumulator
		*/

	case 0x36: //A-L 
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_L];

		temp_word = (WORD)param1 | (WORD)param2;

		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);

		Registers[REGISTER_A] = (BYTE)temp_word;
		break;


	case 0x46: // A-H
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_H];

		temp_word = (WORD)param1 | (WORD)param2;

		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);

		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x56: //A-M
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_M];

		temp_word = (WORD)param1 | (WORD)param2;

		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);

		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x66: //B-L
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_L];

		temp_word = (WORD)param1 | (WORD)param2;

		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);

		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0x76: //B-H
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_H];

		temp_word = (WORD)param1 | (WORD)param2;

		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);

		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0x86: //B-M 
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_M];

		temp_word = (WORD)param1 | (WORD)param2;

		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);

		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

		//AND
		/*
		Register bitwise and with Accumulator
		*/

	case 0x37: //A-L

		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_L];

		temp_word = (WORD)param1 & (WORD)param2;
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);

		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x47: //A-H
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_H];

		temp_word = (WORD)param1 & (WORD)param2;
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);

		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x57: // A-M
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_M];

		temp_word = (WORD)param1 & (WORD)param2;
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);

		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x67://B - L 
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_L];

		temp_word = (WORD)param1 & (WORD)param2;
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);

		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0x77: //B-H
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_H];

		temp_word = (WORD)param1 & (WORD)param2;
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);

		Registers[REGISTER_B] = (BYTE)temp_word;

		break;

	case 0x87: //B-M

		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_M];

		temp_word = (WORD)param1 & (WORD)param2;
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);

		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

		//XOR 
		/*
		Register Bitwise exclusive or with Accumulator
		*/
	case 0x38: // A-L
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_L];

		temp_word = (WORD)param1 ^ (WORD)param2;
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);

		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x48: //A-H
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_H];

		temp_word = (WORD)param1 ^ (WORD)param2;
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);

		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x58: //A-M
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_M];

		temp_word = (WORD)param1 ^ (WORD)param2;
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);

		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x68: //B-L
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_L];

		temp_word = (WORD)param1 ^ (WORD)param2;
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);

		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0x78: //B-H
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_H];

		temp_word = (WORD)param1 ^ (WORD)param2;
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);

		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0x88: //B-M
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_M];

		temp_word = (WORD)param1 ^ (WORD)param2;
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);

		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

		//BIT
		/*
		Register Bit tested with Accumulator
		*/
	case 0x39: //A-L

		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_L];

		temp_word = (WORD)param1 & (WORD)param2;
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		break;

	case 0x49: //A-H
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_H];

		temp_word = (WORD)param1 & (WORD)param2;
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		break;

	case 0x59: //A-M
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_M];

		temp_word = (WORD)param1 & (WORD)param2;
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		break;

	case 0x69://B-L
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_L];

		temp_word = (WORD)param1 & (WORD)param2;
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		break;

	case 0x79: //B-H
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_H];

		temp_word = (WORD)param1 & (WORD)param2;
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		break;

	case 0x89: //B-M
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_M];

		temp_word = (WORD)param1 & (WORD)param2;
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		break;

		//ORIA 
		/*
		Data biutwise inclusive or with Accumlator
		*/
	case 0x97: //#
		data = fetch();
		Registers[REGISTER_A] |= data;
		set_flag_z(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);
		break;


		//ORIB 
		/*
		Data bitwise inclusive or with Accumulator
		*/

	case 0x98: //#
		data = fetch();
		Registers[REGISTER_B] |= data;
		set_flag_z(Registers[REGISTER_B]);
		set_flag_n(Registers[REGISTER_B]);
		break;


		//INC
		/*
		Increment memory or Accumulator
		*/
	case 0xA0: //ABS
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] ++;

		}
		set_flag_z((WORD)Memory[address]);
		set_flag_n((WORD)Memory[address]);
		break;
	case 0xB0://abs,x
		address += Index_Registers[REGISTER_X];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] ++;
		}
		set_flag_z((WORD)Memory[address]);
		set_flag_n((WORD)Memory[address]);
		break;

	case 0xC0:  //abs Y
		address += Index_Registers[REGISTER_Y];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] ++;
		}

		set_flag_z((WORD)Memory[address]);
		set_flag_n((WORD)Memory[address]);
		break;
		//INCA
		/*
		Increment Memory or Accumulator
		*/
	case 0xD0: //A (impl)
		Registers[REGISTER_A]++;
		set_flag_z((WORD)Registers[REGISTER_A]);
		set_flag_n((WORD)Registers[REGISTER_A]);
		break;

		//INCB 
		/*
		Increment Memory or Accumulator
		*/
	case 0xE0: //B (impl)
		Registers[REGISTER_B]++;
		set_flag_z((WORD)Registers[REGISTER_B]);
		set_flag_n((WORD)Registers[REGISTER_B]);
		break;

		//INCX 
		/*
		Increments Register X
		*/
	case 0x02:  //impl 
		Index_Registers[REGISTER_X]++;
		set_flag_z((WORD)Index_Registers[REGISTER_X]);
		break;



		//INCY
		/*
		Increments Register Y
		*/
	case 0x04:  //impl 
		Index_Registers[REGISTER_Y]++;
		set_flag_z((WORD)Index_Registers[REGISTER_Y]);
		break;

		//HLT
		/*
		Wait for interupt
		*/
	case 0x2D:
		halt = true;
		break;

		//REPEAT

		//DEC
		/*
		Decrement Memory or
		Accumulator
		*/
	case 0xA1: //ABS
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] --;

		}
		set_flag_z((WORD)Memory[address]);
		set_flag_n((WORD)Memory[address]);
		break;

	case 0xB1://abs,x
		address += Index_Registers[REGISTER_X];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] --;
		}
		set_flag_z((WORD)Memory[address]);
		set_flag_n((WORD)Memory[address]);
		break;

	case 0xC1:  //abs Y
		address += Index_Registers[REGISTER_Y];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] --;
		}

		set_flag_z((WORD)Memory[address]);
		set_flag_n((WORD)Memory[address]);
		break;
		//DECA
		/*
		Decrement Memory or Accumulator
		*/
	case 0xD1: //A (impl)
		Registers[REGISTER_A]--;
		set_flag_z((WORD)Registers[REGISTER_A]);
		set_flag_n((WORD)Registers[REGISTER_A]);
		break;

		//DECB
		/*
		Increment Memory or Accumulator
		*/
	case 0xE1: //B (impl)
		Registers[REGISTER_B]--;
		set_flag_z((WORD)Registers[REGISTER_B]);
		set_flag_n((WORD)Registers[REGISTER_B]);
		break;

		//DECX
		/*
		Decrements Register X
		*/
	case 0x01:  //impl 
		Index_Registers[REGISTER_X]--;
		set_flag_z((WORD)Index_Registers[REGISTER_X]);
		break;



		//DEY
		/*
		Deccrements Register Y
		*/
	case 0x03:  //impl 
		Index_Registers[REGISTER_Y]--;
		set_flag_z((WORD)Index_Registers[REGISTER_Y]);
		break;

		//NOP
		/*
		No Operating (STOP/Halt?)
		*/
	case 0x2C://impl
		halt = true;

		break;


		//RRC (abs)
		/*
		Rotate right through carry Memory or Accumulator
		*/
	case 0xA2: //abs
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			if ((Flags & FLAG_C) == 0x01) {
				carry = 0x80;
			}

			data = (Memory[address] >> 1) | carry;

			if ((Memory[address] & 0x01) == 0x01) {
				data = data | 0x100;
			}
			Memory[address] = (BYTE)data;
		}
		set_three_flags(data);
		break;

	case 0xB2: //abs,x 
		HB = fetch();
		LB = fetch();
		address += Index_Registers[REGISTER_X];

		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			if ((Flags & FLAG_C) == 0x01) {
				carry = 0x80;
			}
			data = (Memory[address] >> 1) | carry;
			if ((Memory[address] & 0x01) == 0x01) {
				data = data | 0x100;
			}
			Memory[address] = (BYTE)data;
		}
		set_three_flags(data);
		break;


	case 0xC2://abs, Y
		HB = fetch();
		LB = fetch();
		address += Index_Registers[REGISTER_Y];

		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			if ((Flags & FLAG_C) == 0x01) {
				carry = 0x80;
			}
			data = (Memory[address] >> 1) | carry;
			if ((Memory[address] & 0x01) == 0x01) {
				data = data | 0x100;
			}
			Memory[address] = (BYTE)data;
		}
		set_three_flags(data);
		break;


		//RRCA
		/*
		Rotate right through carry memory or accumulator
		*/
	case 0xD2: //A

		Registers[REGISTER_A] = rotateRightCarry(Registers[REGISTER_A]);


		break;



		//RCBB
		/*
		Rotate right through carry memory or accumulator
		*/

	case 0xE2: //B 

		Registers[REGISTER_B] = rotateRightCarry(Registers[REGISTER_B]);
		break;

		//RLC 
		/*
		Rotate left through carry Memory or Accumulator
		*/

	case 0xA3://abs
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		//grab the address
		Memory[address] = rotateLeftCarry(Memory[address]);
		break;

	case 0xB3://abs,x
		address += Index_Registers[REGISTER_X];

		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;

		Memory[address] = rotateLeftCarry(Memory[address]);
		break;

	case 0xC3: //abs, y 
		address += Index_Registers[REGISTER_Y];

		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;

		Memory[address] = rotateLeftCarry(Memory[address]);

		break;

		//RLCA
		/*
		Rotate left through carry memory or Accumulator
		*/
	case 0xD3://A
		Registers[REGISTER_A] = rotateLeftCarry(Registers[REGISTER_A]);
		break;

		//RLCB
		/*
		Rotate left through carry memory or accumulator
		*/
	case 0xE3: //B
		Registers[REGISTER_B] = rotateLeftCarry(Registers[REGISTER_B]);
		break;

		//ROL
		/*
		Rotate left without carry Memory or Accumlator
		*/

	case 0xA8: //Abs 
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		Memory[address] = rotateLeft(Memory[address]);
		break;

	case 0xB8:  //abs ,X

		address += Index_Registers[REGISTER_X];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		Memory[address] = rotateLeft(Memory[address]);
		break;

	case 0xC8: //abs, y
		address += Index_Registers[REGISTER_Y];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		Memory[address] = rotateLeft(Memory[address]);
		break;


		//ROLA
		/*
		Rotate left without carry Memory or Accumlator
		*/

	case 0xD8: //A
		Registers[REGISTER_A] = rotateLeft(Registers[REGISTER_A]);
		break;

		//ROLB
		/*
		Rotate left without carry memory or accumulator
		*/
	case 0xE8:
		Registers[REGISTER_B] = rotateLeft(Registers[REGISTER_B]);
		break;


		//RR
		/*
		Rotate right without carry memory or Accumulator
		*/
	case 0xA9: //abs 
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		Memory[address] = rotateRight(Memory[address]);
		break;


	case 0xB9: //abs,x

		address += Index_Registers[REGISTER_X];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		Memory[address] = rotateRight(Memory[address]);
		break;

	case 0xC9: //abs,Y

		address += Index_Registers[REGISTER_Y];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		Memory[address] = rotateRight(Memory[address]);
		break;


		//RRA 
		/*
		Rotate right wihtout carry memory or accumulator
		*/
	case 0xD9://A
		Registers[REGISTER_A] = rotateRight(Registers[REGISTER_A]);
		break;
		//RRB 
		/*
		Rotate right wihtout carry memory or accumulator
		*/
	case 0xE9://B
		Registers[REGISTER_B] = rotateRight(Registers[REGISTER_B]);
		break;


		//SAL
		/*
		Arithmetic Shift left Memory or Accumulator
		*/
	case 0xA4:	//abs
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;

		if (address >= 0 && address < MEMORY_SIZE) {
			data = (WORD)Memory[address] << 1; //Shift

			Memory[address] = (BYTE)data;
		}

		set_three_flags(data);
		break;

	case 0xB4:  //abs x 
		address += Index_Registers[REGISTER_X];

		address += getAbsAd();


		if (address >= 0 && address < MEMORY_SIZE) {
			data = (WORD)Memory[address] << 1; //Shift

			Memory[address] = (BYTE)data;
		}

		set_three_flags(data);

		break;

	case 0xC4:  //abs y 
		address += Index_Registers[REGISTER_Y];

		address += getAbsAd();


		if (address >= 0 && address < MEMORY_SIZE) {
			data = (WORD)Memory[address] << 1; //Shift

			Memory[address] = (BYTE)data;
		}

		set_three_flags(data);
		break;


		//SALA
		/*
		Arithmetic shift Left memory or Accumulator
		*/
	case 0xD4: //A

		data = Registers[REGISTER_A] << 1;
		Registers[REGISTER_A] = (BYTE)data;
		set_three_flags(data);
		break;

		//SALB
		/*
		Arithmetic shift left Memory or AAccumulator
		*/
	case 0xE4:
		data = Registers[REGISTER_B] << 1;
		Registers[REGISTER_B] = (BYTE)data;
		set_three_flags(data);
		break;

		//SAR
		/*
		Arithmetic shift right memory or accumulator
		*/

	case 0xA5: //abs
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			if ((Memory[address] & 0x80) == 0x80) { //keep the sign
				if ((Memory[address] & 0x01) != 0) {
					data = (Memory[address] >> 1) | 0x100;	//make data bigger then 0x100 so the carry is set
					data = (data | 0x80);
				}
				else {
					data = Memory[address] >> 1;
					data = (data | 0x80);
				}
			}
			else {
				if ((Memory[address] & 0x01) != 0) {
					data = (Memory[address] >> 1) | 0x100;	//make data bigger then 0x100 so the carry is set
				}
				else {
					data = Memory[address] >> 1;
				}

			}
			Memory[address] = (BYTE)data;
		}
		set_three_flags(data);
		break;

	case 0xB5: //abs,X
		address += Index_Registers[REGISTER_X];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			if ((Memory[address] & 0x80) == 0x80) { //keep the sign
				if ((Memory[address] & 0x01) != 0) {
					data = (Memory[address] >> 1) | 0x100;	//make data bigger then 0x100 so the carry is set
					data = (data | 0x80);
				}
				else {
					data = Memory[address] >> 1;
					data = (data | 0x80);
				}
			}
			else {
				if ((Memory[address] & 0x01) != 0) {
					data = (Memory[address] >> 1) | 0x100;	//make data bigger then 0x100 so the carry is set
				}
				else {
					data = Memory[address] >> 1;
				}

			}
			Memory[address] = (BYTE)data;
		}
		set_three_flags(data);
		break;


	case 0xC5: //abs,y
		address += Index_Registers[REGISTER_Y];
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			if ((Memory[address] & 0x80) == 0x80) { //keep the sign
				if ((Memory[address] & 0x01) != 0) {
					data = (Memory[address] >> 1) | 0x100;	//make data bigger then 0x100 so the carry is set
					data = (data | 0x80);
				}
				else {
					data = Memory[address] >> 1;
					data = (data | 0x80);
				}
			}
			else {
				if ((Memory[address] & 0x01) != 0) {
					data = (Memory[address] >> 1) | 0x100;	//make data bigger then 0x100 so the carry is set
				}
				else {
					data = Memory[address] >> 1;
				}

			}
			Memory[address] = (BYTE)data;
		}
		set_three_flags(data);
		break;


		//SARA
		/*
		Arithmetic shift right memory or Accumulator
		*/

	case 0xD5: //A
			   //take out if statement for neg flag for normal shift
		if (address >= 0 && address < MEMORY_SIZE) {
			if ((Registers[REGISTER_A] & 0x80) == 0x80) { //keep the sign

				if ((Registers[REGISTER_A] & 0x01) != 0) {
					data = (Registers[REGISTER_A] >> 1) | 0x100;	//make data bigger then 0x100 so the carry is set
					data = (data | 0x80);
				}
				else {
					data = Registers[REGISTER_A] >> 1;
					data = (data | 0x80);
				}
			}
			else {
				if ((Registers[REGISTER_A] & 0x01) != 0) {
					data = (Registers[REGISTER_A] >> 1) | 0x100;	//make data bigger then 0x100 so the carry is set
				}
				else {
					data = Registers[REGISTER_A] >> 1;
				}

			}
			Registers[REGISTER_A] = (BYTE)data;
		}
		set_three_flags(data);
		break;

		//SARB
		/*
		Arithmetic shift right Memory or Accu,ulator
		*/

	case 0xE5:
		if (address >= 0 && address < MEMORY_SIZE) {
			if ((Registers[REGISTER_B] & 0x80) == 0x80) { //keep the sign

				if ((Registers[REGISTER_B] & 0x01) != 0) {
					data = (Registers[REGISTER_B] >> 1) | 0x100;	//make data bigger then 0x100 so the carry is set
					data = (data | 0x80);
				}
				else {
					data = Registers[REGISTER_B] >> 1;
					data = (data | 0x80);
				}
			}
			else {
				if ((Registers[REGISTER_B] & 0x01) != 0) {
					data = (Registers[REGISTER_B] >> 1) | 0x100;	//make data bigger then 0x100 so the carry is set
				}
				else {
					data = Registers[REGISTER_B] >> 1;
				}

			}
			Registers[REGISTER_B] = (BYTE)data;
		}
		set_three_flags(data);
		break;

		//LSR
		/*
		Shift right Memory or Accumulator
		*/

	case 0xA6:	//abs
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			if ((Memory[address] & 0x01) != 0) {
				data = (Memory[address] >> 1) | 0x100;
			}
			else {
				data = Memory[address] >> 1;
			}
			Memory[address] = (BYTE)data;
		}
		set_three_flags(data);

		break;

	case 0xB6:	//abs,X

		address += Index_Registers[REGISTER_X];

		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			if ((Memory[address] & 0x01) != 0) {
				data = (Memory[address] >> 1) | 0x100;
			}
			else {
				data = Memory[address] >> 1;
			}
			Memory[address] = (BYTE)data;
		}
		set_three_flags(data);
		break;

	case 0xC6: //abs,Y
		address += Index_Registers[REGISTER_Y];

		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			if ((Memory[address] & 0x01) != 0) {
				data = (Memory[address] >> 1) | 0x100;
			}
			else {
				data = Memory[address] >> 1;
			}
			Memory[address] = (BYTE)data;
		}
		set_three_flags(data);

		break;

		//LSRA
		/*
		Shif right memory or Accumulator
		*/
	case 0xD6:
		if (address >= 0 && address < MEMORY_SIZE) {
			if ((Registers[REGISTER_A] & 0x01) != 0) {
				data = (Registers[REGISTER_A] >> 1) | 0x100;
			}
			else {
				data = Registers[REGISTER_A] >> 1;
			}
			Registers[REGISTER_A] = (BYTE)data;
		}
		set_three_flags(data);
		break;

		//LSRB
		/*
		Shift right memory or Accumualtor
		*/
	case 0xE6:
		if (address >= 0 && address < MEMORY_SIZE) {
			if ((Registers[REGISTER_B] & 0x01) != 0) {
				data = (Registers[REGISTER_B] >> 1) | 0x100;
			}
			else {
				data = Registers[REGISTER_B] >> 1;
			}
			Registers[REGISTER_B] = (BYTE)data;
		}
		set_three_flags(data);
		break;

		//COM

		/*
		Negate memory or accumulator
		*/

	case 0xA7: //abs 
			   //get ab address
		address = getAbsAd();
		data = ~Memory[address];

		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = (BYTE)data;

		}
		set_three_flags(data);
		break;

	case 0xB7: //abs X
		address += Index_Registers[REGISTER_X];
		address += getAbsAd();
		data = ~Memory[address];

		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = (BYTE)data;

		}
		set_three_flags(data);
		break;

	case 0xC7: //abs Y 
		address += Index_Registers[REGISTER_Y];
		address += getAbsAd();
		data = ~Memory[address];

		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = (BYTE)data;

		}
		set_three_flags(data);
		break;

		//COMA
		/*
		Negate Memory or Accumulator
		*/
	case 0xD7: //A

		data = ~Registers[REGISTER_A];
		if (address >= 0 && address < MEMORY_SIZE) {
			Registers[REGISTER_A] = (BYTE)data;

		}
		set_three_flags(data);
		break;

		//COMB
	case 0xE7: //B 
		data = ~Registers[REGISTER_B];
		if (address >= 0 && address < MEMORY_SIZE) {
			Registers[REGISTER_B] = (BYTE)data;

		}
		set_three_flags(data);

		break;
		//PUSH
		/*
		Push Register onto the stack
		*/

	case 0xBE: //A

		Memory[StackPointer] = Registers[REGISTER_A];
		StackPointer--;
		break;

	case 0xCE: //B
		Memory[StackPointer] = Registers[REGISTER_B];
		StackPointer--;
		break;

	case 0xDE: //FL
		Memory[StackPointer] = Flags;
		StackPointer--;
		break;

	case 0xEE: //L
		Memory[StackPointer] = Registers[REGISTER_L];
		StackPointer--;
		break;
	case 0xFE: //H
		Memory[StackPointer] = Registers[REGISTER_H];
		StackPointer--;
		break;

		//POP
		/*
		Pop the top of the
		Stack into the Register
		*/

	case 0xBF://A
		StackPointer++;

		Registers[REGISTER_A] = Memory[StackPointer];

		break;
	case 0xCF: //B
		StackPointer++;

		Registers[REGISTER_B] = Memory[StackPointer];
		break;

	case 0xDF: //FL
		StackPointer++;
		Flags = Memory[StackPointer];
		break;

	case 0xEF: //L
		StackPointer++;
		Registers[REGISTER_L] = Memory[StackPointer];
		break;

	case 0xFF: //H
		StackPointer++;
		Registers[REGISTER_H] = Memory[StackPointer];
		break;


		//JUMP
	case 0x10: //abs
		address = getAbsAd();

		ProgramCounter = address;
		break;
		/////////////////
		//     JSR     //
		/////////////////

	case 0x21: //JSR (abs)	
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {

			LB = (BYTE)ProgramCounter;
			HB = (BYTE)(ProgramCounter >> 8);

			temp_word = ((WORD)HB << 8) + LB;

			Memory[StackPointer] = temp_word;
			StackPointer--;

			ProgramCounter = address;
		}
		break;

		/////////////////
		//     RET     //
		/////////////////

	case 0x4C: //RET (impl)	
		StackPointer++;
		address = Memory[StackPointer];
		ProgramCounter = address;
		break;




		//SBIA

	case 0x93: //# 
		data = fetch();
		param1 = Registers[REGISTER_A];

		temp_word = (WORD)data - (WORD)param1;

		if ((Flags & FLAG_C) != 0) {
			temp_word--;
		}

		set_three_flags((WORD)temp_word);
		Registers[REGISTER_A] = temp_word;
		break;

		//SBIB

	case 0x94: //# 
		data = fetch();
		param1 = Registers[REGISTER_B];

		temp_word = (WORD)data - (WORD)param1;

		if ((Flags & FLAG_C) != 0) {
			temp_word--;
		}

		set_three_flags((WORD)temp_word);
		Registers[REGISTER_B] = temp_word;
		break;

		//CPIA
		/*
		Data compared to accumulator
		*/
	case 0x95:// # A - Data
		data = fetch();

		param1 = Registers[REGISTER_A];

		temp_word = (WORD)data - (WORD)param1;

		set_three_flags((WORD)temp_word);
		break;

		//CPIB

		/*
		Data compared to accumulator
		*/

	case 0x96:
		data = fetch();

		param1 = Registers[REGISTER_B];

		temp_word = (WORD)data - (WORD)param1;

		set_three_flags((WORD)temp_word);
		break;

		//JCC
		/*
		Jump on Carry clear
		*/
	case 0x11: //abs
		address = getAbsAd();

		if ((Flags & FLAG_C) == 0) {

			ProgramCounter = address;
		}

		break;

		//JCC

		/*
		Jump on carry set
		*/
	case 0x12: //abs 

		address = getAbsAd();
		if ((Flags & FLAG_C) != 0) {
			ProgramCounter = address;
		}

		break;

		//JNE
		/*
		Jump on result not Zero 
		*/
	case 0x13: //abs
			
			address = getAbsAd();

			if ((Flags & FLAG_Z) == 0)
			{
				ProgramCounter = address;

			}
		break;

		//JEQ
		/*
		Jump on result equal to Zero
		*/
	case 0x14:  //abs 
		
		address = getAbsAd();

		if ((Flags & FLAG_Z) != 0)
		{
			ProgramCounter = address;

		}

		break;


		//JMI
		/*
		Jump on Negative Result
		*/
	case 0x15:
		address = getAbsAd();

		if ((Flags & FLAG_N) != 0)
		{

			ProgramCounter = address; 
		}
		break;
		// JPL 
		/*
		Jump on positive reuslt 
		*/
	case 0x16:

		address = getAbsAd();
		if ((Flags & FLAG_N) == 0) {
			
			ProgramCounter = address; 

		}
		break;

	//JHI

	case 0x17:
		address = getAbsAd();
		if (Flags & (FLAG_C | FLAG_Z) != 0)
		{
			ProgramCounter = address;

		}
		
		break;


	//JLE
	case 0x18: //JLE (abs)	
		address = getAbsAd();
		if ((Flags & (FLAG_C | FLAG_Z)) == 0) {
			ProgramCounter = address;
		}
		break;
		
		//CCC
	case 0x22: //abs
		address = getAbsAd();
		if ((Flags & FLAG_C) == 0) {
			push(ProgramCounter);
			ProgramCounter = address;
		}
		break;
		//CCS
	case 0x23: //abs

		address = getAbsAd();
		if ((Flags & FLAG_C) != 0) {
			push(ProgramCounter);
			ProgramCounter = address;
		}
		break; 

		//CNE 
		/*
		Call on result not Zero
		*/

	case 0x24: //abs
		address = getAbsAd();
		if ((Flags & FLAG_Z) == 0) {
			push(ProgramCounter);
			ProgramCounter = address;
		}

		break;

		//CEQ 
		/*
		Call on result equal to Zero
		*/
		case 0x25:
		address = getAbsAd();
		if ((Flags & FLAG_Z) != 0) {
			push(ProgramCounter);
			ProgramCounter = address;
		}
		break;

	//CMI 
		/*
		Call on negative result
		*/

	  case 0x26: //abs
		address = getAbsAd();
		if ((Flags & FLAG_N) != 0) {
			push(ProgramCounter);
			ProgramCounter = address;
		}

		break;

		//CPL 
		/*
		Call on positive result
		*/
			case 0x27: //abs
			address = getAbsAd();
			if ((Flags & FLAG_N) == 0) {
				push(ProgramCounter);
				ProgramCounter = address;
			}

		break;

	}












}
/*
Transfer from one register to another
*also incrementing etc
*/
void Group_2_Move(BYTE opcode)
{
	//Variables for source and destination
	BYTE source = opcode >> 4;
	BYTE destination = opcode & 0x0F;

	//Temporary variables for registers
	int destReg = 0;
	int sourceReg = 0;

	BYTE LB = 0;
	BYTE HB = 0;
	WORD address = 0;
	WORD data = 0;


	switch (destination)
	{
	case 0x0B:
		destReg = REGISTER_A;
		break;

	case 0x0C:
		destReg = REGISTER_B;
		break;

	case 0x0D:
		destReg = REGISTER_L;
		break;

	case 0x0E:
		destReg = REGISTER_H;
		break;

	case 0x0F:
		destReg = REGISTER_M;
		break;

	default:
		Registers[destReg] = Registers[sourceReg];
		if (destReg == REGISTER_M) {
			address = Registers[REGISTER_L];
			address += (WORD)Registers[REGISTER_H] << 4;
			if (address >= 0 && address <= MEMORY_SIZE) {
				Memory[address] = Registers[sourceReg];
			}
		}
		else {
			Registers[destReg] = Registers[sourceReg];
		}
		break;

	}

	switch (source)
	{
	case 0x06:
		sourceReg = REGISTER_A;
		break;

	case 0x07:
		sourceReg = REGISTER_B;
		break;

	case 0x08:
		sourceReg = REGISTER_L;
		break;

	case 0x09:
		sourceReg = REGISTER_H;
		break;

	case 0x0A:
		sourceReg = REGISTER_M;
		break;

	default:
		Registers[sourceReg] = Registers[destReg];
		if (sourceReg == REGISTER_M) {
			address = Registers[REGISTER_L];
			address += (WORD)Registers[REGISTER_H] << 4;
			if (address >= 0 && address <= MEMORY_SIZE) {
				Memory[address] = Registers[destReg];
			}
		}
		else {
			Registers[sourceReg] = Registers[destReg];
		}
		break;

	}

	Registers[destReg] = Registers[sourceReg];
}


void execute(BYTE opcode)
{

	if (((opcode >= 0x6B) && (opcode <= 0x6F))
		|| ((opcode >= 0x7B) && (opcode <= 0x7F))
		|| ((opcode >= 0x8B) && (opcode <= 0x8F))
		|| ((opcode >= 0x9B) && (opcode <= 0x9F))
		|| ((opcode >= 0xAB) && (opcode <= 0xAF)))
	{
		Group_2_Move(opcode);
	}
	else
	{
		Group_1(opcode);
	}
}

void emulate()
{
	BYTE opcode;
	int sanity;

	ProgramCounter = 0;
	halt = false;
	memory_in_range = true;
	sanity = 0;

	printf("                    A  B  L  H  X  Y  SP\n");

	while ((!halt) && (memory_in_range) && (sanity < 200))
	{
		printf("%04X ", ProgramCounter);           // Print current address
		opcode = fetch();
		execute(opcode);

		printf("%s  ", opcode_mneumonics[opcode]);  // Print current opcode

		printf("%02X ", Registers[REGISTER_A]);
		printf("%02X ", Registers[REGISTER_B]);
		printf("%02X ", Registers[REGISTER_L]);
		printf("%02X ", Registers[REGISTER_H]);
		printf("%02X ", Index_Registers[REGISTER_X]);
		printf("%02X ", Index_Registers[REGISTER_Y]);
		printf("%04X ", StackPointer);              // Print Stack Pointer

		if ((Flags & FLAG_I) == FLAG_I)
		{
			printf("I=1 ");
		}
		else
		{
			printf("I=0 ");
		}
		if ((Flags & FLAG_Z) == FLAG_Z)
		{
			printf("Z=1 ");
		}
		else
		{
			printf("Z=0 ");
		}
		if ((Flags & FLAG_N) == FLAG_N)
		{
			printf("N=1 ");
		}
		else
		{
			printf("N=0 ");
		}
		if ((Flags & FLAG_C) == FLAG_C)
		{
			printf("C=1 ");
		}
		else
		{
			printf("C=0 ");
		}

		printf("\n");  // New line
		sanity++;
	}

	printf("\n");  // New line
}


////////////////////////////////////////////////////////////////////////////////
//                            Simulator/Emulator (End)                        //
////////////////////////////////////////////////////////////////////////////////


void initialise_filenames() {
	int i;

	for (i = 0; i<MAX_FILENAME_SIZE; i++) {
		hex_file[i] = '\0';
		trc_file[i] = '\0';
	}
}




int find_dot_position(char *filename) {
	int  dot_position;
	int  i;
	char chr;

	dot_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0') {
		if (chr == '.') {
			dot_position = i;
		}
		i++;
		chr = filename[i];
	}

	return (dot_position);
}


int find_end_position(char *filename) {
	int  end_position;
	int  i;
	char chr;

	end_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0') {
		end_position = i;
		i++;
		chr = filename[i];
	}

	return (end_position);
}


bool file_exists(char *filename) {
	bool exists;
	FILE *ifp;

	exists = false;

	if ((ifp = fopen(filename, "r")) != NULL)
	{
		exists = true;

		fclose(ifp);
	}

	return (exists);
}



void create_file(char *filename) {
	FILE *ofp;

	if ((ofp = fopen(filename, "w")) != NULL) {
		fclose(ofp);
	}
}



bool getline(FILE *fp, char *buffer) {
	bool rc;
	bool collect;
	char c;
	int  i;

	rc = false;
	collect = true;

	i = 0;
	while (collect) {
		c = getc(fp);

		switch (c) {
		case EOF:
			if (i > 0) {
				rc = true;
			}
			collect = false;
			break;

		case '\n':
			if (i > 0) {
				rc = true;
				collect = false;
				buffer[i] = '\0';
			}
			break;

		default:
			buffer[i] = c;
			i++;
			break;
		}
	}

	return (rc);
}






void load_and_run(int args, _TCHAR** argv) {
	char chr;
	int  ln;
	int  dot_position;
	int  end_position;
	long i;
	FILE *ifp;
	long address;
	long load_at;
	int  code;

	// Prompt for the .hex file

	printf("\n");
	printf("Enter the hex filename (.hex): ");

	if (args == 2) {
		ln = 0;
		chr = argv[1][ln];
		while (chr != '\0')
		{
			if (ln < MAX_FILENAME_SIZE)
			{
				hex_file[ln] = chr;
				trc_file[ln] = chr;
				ln++;
			}
			chr = argv[1][ln];
		}
	}
	else {
		ln = 0;
		chr = '\0';
		while (chr != '\n') {
			chr = getchar();

			switch (chr) {
			case '\n':
				break;
			default:
				if (ln < MAX_FILENAME_SIZE) {
					hex_file[ln] = chr;
					trc_file[ln] = chr;
					ln++;
				}
				break;
			}
		}

	}
	// Tidy up the file names

	dot_position = find_dot_position(hex_file);
	if (dot_position == 0) {
		end_position = find_end_position(hex_file);

		hex_file[end_position + 1] = '.';
		hex_file[end_position + 2] = 'h';
		hex_file[end_position + 3] = 'e';
		hex_file[end_position + 4] = 'x';
		hex_file[end_position + 5] = '\0';
	}
	else {
		hex_file[dot_position + 0] = '.';
		hex_file[dot_position + 1] = 'h';
		hex_file[dot_position + 2] = 'e';
		hex_file[dot_position + 3] = 'x';
		hex_file[dot_position + 4] = '\0';
	}

	dot_position = find_dot_position(trc_file);
	if (dot_position == 0) {
		end_position = find_end_position(trc_file);

		trc_file[end_position + 1] = '.';
		trc_file[end_position + 2] = 't';
		trc_file[end_position + 3] = 'r';
		trc_file[end_position + 4] = 'c';
		trc_file[end_position + 5] = '\0';
	}
	else {
		trc_file[dot_position + 0] = '.';
		trc_file[dot_position + 1] = 't';
		trc_file[dot_position + 2] = 'r';
		trc_file[dot_position + 3] = 'c';
		trc_file[dot_position + 4] = '\0';
	}

	if (file_exists(hex_file)) {
		// Clear Registers and Memory

		Registers[REGISTER_A] = 0;
		Registers[REGISTER_B] = 0;
		Registers[REGISTER_L] = 0;
		Registers[REGISTER_H] = 0;
		Index_Registers[REGISTER_X] = 0;
		Index_Registers[REGISTER_Y] = 0;
		Flags = 0;
		ProgramCounter = 0;
		StackPointer = 0;

		for (i = 0; i<MEMORY_SIZE; i++) {
			Memory[i] = 0x00;
		}

		// Load hex file

		if ((ifp = fopen(hex_file, "r")) != NULL) {
			printf("Loading file...\n\n");

			load_at = 0;

			while (getline(ifp, InputBuffer)) {
				if (sscanf(InputBuffer, "L=%x", &address) == 1) {
					load_at = address;
				}
				else if (sscanf(InputBuffer, "%x", &code) == 1) {
					if ((load_at >= 0) && (load_at <= MEMORY_SIZE)) {
						Memory[load_at] = (BYTE)code;
					}
					load_at++;
				}
				else {
					printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
				}
			}

			fclose(ifp);
		}

		// Emulate

		emulate();
	}
	else {
		printf("\n");
		printf("ERROR> Input file %s does not exist!\n", hex_file);
		printf("\n");
	}
}

void building(int args, _TCHAR** argv)
{
	char buffer[1024];
	load_and_run(args, argv);
	sprintf(buffer, "0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X",
		Memory[TEST_ADDRESS_1],
		Memory[TEST_ADDRESS_2],
		Memory[TEST_ADDRESS_3],
		Memory[TEST_ADDRESS_4],
		Memory[TEST_ADDRESS_5],
		Memory[TEST_ADDRESS_6],
		Memory[TEST_ADDRESS_7],
		Memory[TEST_ADDRESS_8],
		Memory[TEST_ADDRESS_9],
		Memory[TEST_ADDRESS_10],
		Memory[TEST_ADDRESS_11],
		Memory[TEST_ADDRESS_12]
		);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
}



void test_and_mark() {
	char buffer[1024];
	bool testing_complete;
	int  len = sizeof(SOCKADDR);
	char chr;
	int  i;
	int  j;
	bool end_of_program;
	long address;
	long load_at;
	int  code;
	int  mark;
	int  passed;

	printf("\n");
	printf("Automatic Testing and Marking\n");
	printf("\n");

	testing_complete = false;

	sprintf(buffer, "Test Student %s", STUDENT_NUMBER);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));

	while (!testing_complete) {
		memset(buffer, '\0', sizeof(buffer));

		if (recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (SOCKADDR *)&client_addr, &len) != SOCKET_ERROR) {
			printf("Incoming Data: %s \n", buffer);

			//if (strcmp(buffer, "Testing complete") == 1)
			if (sscanf(buffer, "Testing complete %d", &mark) == 1) {
				testing_complete = true;
				printf("Current mark = %d\n", mark);

			}
			else if (sscanf(buffer, "Tests passed %d", &passed) == 1) {
				//testing_complete = true;
				printf("Passed = %d\n", passed);

			}
			else if (strcmp(buffer, "Error") == 0) {
				printf("ERROR> Testing abnormally terminated\n");
				testing_complete = true;
			}
			else {
				// Clear Registers and Memory

				Registers[REGISTER_A] = 0;
				Registers[REGISTER_B] = 0;
				Registers[REGISTER_L] = 0;
				Registers[REGISTER_H] = 0;
				Index_Registers[REGISTER_X] = 0;
				Index_Registers[REGISTER_Y] = 0;
				Flags = 0;
				ProgramCounter = 0;
				StackPointer = 0;
				for (i = 0; i<MEMORY_SIZE; i++) {
					Memory[i] = 0;
				}

				// Load hex file

				i = 0;
				j = 0;
				load_at = 0;
				end_of_program = false;
				FILE *ofp;
				fopen_s(&ofp, "branch.txt", "a");

				while (!end_of_program) {
					chr = buffer[i];
					switch (chr) {
					case '\0':
						end_of_program = true;

					case ',':
						if (sscanf(InputBuffer, "L=%x", &address) == 1) {
							load_at = address;
						}
						else if (sscanf(InputBuffer, "%x", &code) == 1) {
							if ((load_at >= 0) && (load_at <= MEMORY_SIZE)) {
								Memory[load_at] = (BYTE)code;
								fprintf(ofp, "%02X\n", (BYTE)code);
							}
							load_at++;
						}
						else {
							printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
						}
						j = 0;
						break;

					default:
						InputBuffer[j] = chr;
						j++;
						break;
					}
					i++;
				}
				fclose(ofp);
				// Emulate

				if (load_at > 1) {
					emulate();
					// Send and store results
					sprintf(buffer, "%02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X",
						Memory[TEST_ADDRESS_1],
						Memory[TEST_ADDRESS_2],
						Memory[TEST_ADDRESS_3],
						Memory[TEST_ADDRESS_4],
						Memory[TEST_ADDRESS_5],
						Memory[TEST_ADDRESS_6],
						Memory[TEST_ADDRESS_7],
						Memory[TEST_ADDRESS_8],
						Memory[TEST_ADDRESS_9],
						Memory[TEST_ADDRESS_10],
						Memory[TEST_ADDRESS_11],
						Memory[TEST_ADDRESS_12]
						);
					sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
				}
			}
		}
	}
}



int _tmain(int argc, _TCHAR* argv[])
{
	char chr;
	char dummy;

	printf("\n");
	printf("Microprocessor Emulator\n");
	printf("UWE Computer and Network Systems Assignment 1\n");
	printf("\n");

	initialise_filenames();

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock) {
		// Creation failed! 
	}

	memset(&server_addr, 0, sizeof(SOCKADDR_IN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	server_addr.sin_port = htons(PORT_SERVER);

	memset(&client_addr, 0, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(PORT_CLIENT);

	chr = '\0';
	while ((chr != 'e') && (chr != 'E'))
	{
		printf("\n");
		printf("Please select option\n");
		printf("L - Load and run a hex file\n");
		printf("T - Have the server test and mark your emulator\n");
		printf("E - Exit\n");
		if (argc == 2) { building(argc, argv); exit(0); }
		printf("Enter option: ");
		chr = getchar();
		if (chr != 0x0A)
		{
			dummy = getchar();  // read in the <CR>
		}
		printf("\n");

		switch (chr)
		{
		case 'L':
		case 'l':
			load_and_run(argc, argv);
			break;

		case 'T':
		case 't':
			test_and_mark();
			break;

		default:
			break;
		}
	}

	closesocket(sock);
	WSACleanup();


	return 0;
}

