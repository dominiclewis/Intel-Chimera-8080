////////////////////////////////////////////////////////////////////
//Name: Chimera 2015 Emulator									 //
//Purpose: Emulates the instructions of Chimera 2015 CPU		//
//Author: Milos Matovic										   //
//Version 1.23												  //
//Changelog: all opcodes complete, fetching address first in //
//conditional jumps										    //
//Last edited: 03/11/2015								   //
////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <winsock2.h>

#pragma comment(lib, "wsock32.lib")


#define STUDENT_NUMBER    "15016538"

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

char InputBuffer [MAX_BUFFER_SIZE];

char hex_file [MAX_BUFFER_SIZE];
char trc_file [MAX_BUFFER_SIZE];

//////////////////////////
//   Registers          //
//////////////////////////

#define FLAG_I  0x40
#define FLAG_Z  0x20
#define FLAG_P  0x10
#define FLAG_V  0x08
#define FLAG_N  0x02
#define FLAG_C  0x01
#define REGISTER_A	3
#define REGISTER_B	2
#define REGISTER_D	1
#define REGISTER_C	0
WORD BaseRegister;
BYTE PageRegister;

BYTE Registers[4];
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
"NOP impl     ", 
"LODS  #      ", 
"LODS abs     ", 
"LODS zpg     ", 
"LODS (ind)   ", 
"LODS pag     ", 
"LODS bas     ", 
"ILLEGAL     ", 
"DEP impl     ", 
"ILLEGAL     ", 
"LDAA  #      ", 
"LDAA abs     ", 
"LDAA zpg     ", 
"LDAA (ind)   ", 
"LDAA pag     ", 
"LDAA bas     ", 

"HALT impl    ", 
"LDZ  #       ", 
"LDZ abs      ", 
"LDZ zpg      ", 
"LDZ (ind)    ", 
"LDZ pag      ", 
"LDZ bas      ", 
"ILLEGAL     ", 
"INP impl     ", 
"ILLEGAL     ", 
"LDAB  #      ", 
"LDAB abs     ", 
"LDAB zpg     ", 
"LDAB (ind)   ", 
"LDAB pag     ", 
"LDAB bas     ", 

"ILLEGAL     ", 
"CAS impl     ", 
"CLC impl     ", 
"STS abs      ", 
"STS zpg      ", 
"STS (ind)    ", 
"STS pag      ", 
"STS bas      ", 
"DEZ impl     ", 
"JPA abs      ", 
"JPA zpg      ", 
"JPA (ind)    ", 
"JPA pag      ", 
"INC abs      ", 
"INCA A,A     ", 
"INCB B,B     ", 

"ADIA  #      ", 
"TSA impl     ", 
"SEC impl     ", 
"STZ abs      ", 
"STZ zpg      ", 
"STZ (ind)    ", 
"STZ pag      ", 
"STZ bas      ", 
"INZ impl     ", 
"JCC abs      ", 
"JCC zpg      ", 
"JCC (ind)    ", 
"JCC pag      ", 
"DEC abs      ", 
"DECA A,A     ", 
"DECB B,B     ", 

"ADIB  #      ", 
"ABA impl     ", 
"CLI impl     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"JPR abs      ", 
"JPR zpg      ", 
"JPR (ind)    ", 
"JPR pag      ", 
"JCS abs      ", 
"JCS zpg      ", 
"JCS (ind)    ", 
"JCS pag      ", 
"RRC abs      ", 
"RRCA A,A     ", 
"RRCB B,B     ", 

"SBIA  #      ", 
"SBA impl     ", 
"STI impl     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ADC A,C      ", 
"ADC A,D      ", 
"ADC B,C      ", 
"ADC B,D      ", 
"JNE abs      ", 
"JNE zpg      ", 
"JNE (ind)    ", 
"JNE pag      ", 
"RL abs       ", 
"RLA A,A      ", 
"RLB B,B      ", 

"SBIB  #      ", 
"AAB impl     ", 
"STV impl     ", 
"ILLEGAL     ", 
"RET impl     ", 
"SBC A,C      ", 
"SBC A,D      ", 
"SBC B,C      ", 
"SBC B,D      ", 
"JEQ abs      ", 
"JEQ zpg      ", 
"JEQ (ind)    ", 
"JEQ pag      ", 
"SHL abs      ", 
"SHLA A,A     ", 
"SHLB B,B     ", 

"CPIA  #      ", 
"SAB impl     ", 
"CLV impl     ", 
"SWI impl     ", 
"ILLEGAL     ", 
"ADD A,C      ", 
"ADD A,D      ", 
"ADD B,C      ", 
"ADD B,D      ", 
"JVC abs      ", 
"JVC zpg      ", 
"JVC (ind)    ", 
"JVC pag      ", 
"ASR abs      ", 
"ASRA A,A     ", 
"ASRB B,B     ", 

"CPIB  #      ", 
"TAP impl     ", 
"CMC impl     ", 
"RTI impl     ", 
"ILLEGAL     ", 
"SUB A,C      ", 
"SUB A,D      ", 
"SUB B,C      ", 
"SUB B,D      ", 
"JVS abs      ", 
"JVS zpg      ", 
"JVS (ind)    ", 
"JVS pag      ", 
"SHR abs      ", 
"SHRA A,A     ", 
"SHRB B,B     ", 

"ORIA  #      ", 
"TPA impl     ", 
"CMV impl     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"CMP A,C      ", 
"CMP A,D      ", 
"CMP B,C      ", 
"CMP B,D      ", 
"JMI abs      ", 
"JMI zpg      ", 
"JMI (ind)    ", 
"JMI pag      ", 
"NOT abs      ", 
"NOTA A,A     ", 
"NOTB B,B     ", 

"ORIB  #      ", 
"MV A,A       ", 
"MV A,B       ", 
"MV A,C       ", 
"MV A,D       ", 
"ORA A,C      ", 
"ORA A,D      ", 
"ORA B,C      ", 
"ORA B,D      ", 
"JPL abs      ", 
"JPL zpg      ", 
"JPL (ind)    ", 
"JPL pag      ", 
"NEG abs      ", 
"NEGA A,0     ", 
"NEGB B,0     ", 

"ANIA  #      ", 
"MV B,A       ", 
"MV B,B       ", 
"MV B,C       ", 
"MV B,D       ", 
"AND A,C      ", 
"AND A,D      ", 
"AND B,C      ", 
"AND B,D      ", 
"JPE abs      ", 
"JPE zpg      ", 
"JPE (ind)    ", 
"JPE pag      ", 
"ROL abs      ", 
"ROLA A,A     ", 
"ROLB B,B     ", 

"ANIB  #      ", 
"MV C,A       ", 
"MV C,B       ", 
"MV C,C       ", 
"MV C,D       ", 
"EOR A,C      ", 
"EOR A,D      ", 
"EOR B,C      ", 
"EOR B,D      ", 
"JPO abs      ", 
"JPO zpg      ", 
"JPO (ind)    ", 
"JPO pag      ", 
"RR abs       ", 
"RRA A,A      ", 
"RRB B,B      ", 

"ILLEGAL     ", 
"MV D,A       ", 
"MV D,B       ", 
"MV D,C       ", 
"MV D,D       ", 
"BT A,C       ", 
"BT A,D       ", 
"BT B,C       ", 
"BT B,D       ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"CLR abs      ", 
"CLRA A,0     ", 
"CLRB B,0     ", 

"LD  #,C      ", 
"LD abs,C     ", 
"LD zpg,C     ", 
"LD (ind),C   ", 
"LD pag,C     ", 
"LD bas,C     ", 
"STA abs      ", 
"STA zpg      ", 
"STA (ind)    ", 
"STA pag      ", 
"STA bas      ", 
"PUSH  ,A     ", 
"PUSH  ,B     ", 
"PUSH  ,s     ", 
"PUSH  ,C     ", 
"PUSH  ,D     ", 

"LD  #,D      ", 
"LD abs,D     ", 
"LD zpg,D     ", 
"LD (ind),D   ", 
"LD pag,D     ", 
"LD bas,D     ", 
"STB abs      ", 
"STB zpg      ", 
"STB (ind)    ", 
"STB pag      ", 
"STB bas      ", 
"POP A,       ", 
"POP B,       ", 
"POP s,       ", 
"POP C,       ", 
"POP D,       ", 

}; 

////////////////////////////////////////////////////////////////////////////////
//                           Simulator/Emulator (Start)                       //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////
//		Functions		  /
//////////////////////////

//||  fetch - fetches a byte of memory from address set by ProgramCounter  ||
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
//||  set_flag_n - sets Negative flag depending on result of operation  ||
void set_flag_n(WORD inReg) {
	BYTE reg; 
	reg = (BYTE)inReg; 

	if ((reg & 0x80) == 0x80)
		Flags = Flags | FLAG_N;	//set N
	else 
		Flags = Flags & (~FLAG_N); //reset N 
}
//||  set_flag_c - sets Carry flag depending on operation and operands  ||
void set_flag_c(WORD result)
{	
		if (result >= 0x100)	//doesnt fit in byte
			Flags = Flags | FLAG_C;
		else
			Flags = Flags & (~FLAG_C);
}
//||  set_flag_v - sets Overflow flag if positive operands give negative result and vice versa  ||    //for subtraction, pass 2s complement as second arg  
void set_flag_v(BYTE inReg1,BYTE inReg2, WORD result)
{
		//both - and result + , or both + and result - => overflow = 1
	BYTE res = (BYTE)result;
	if (((inReg1^res)&(inReg2^res)&0x80)!=0x00)
			Flags = Flags | FLAG_V; //set
		else
			Flags = Flags & (~FLAG_V); //reset
}
void flag_v_wrong(BYTE inReg1, BYTE inReg2, WORD result)										//working at the moment for subtraction, wrong
{
	BYTE res = (BYTE)result;
	if (((inReg1 >= 0x80) && (inReg2 >= 0x80) && (res >= 0x80)) ||
		((inReg1 < 0x80) && (inReg2 < 0x80) && (res < 0x80)))
		Flags = Flags | FLAG_V; //set
	else
		Flags = Flags & (~FLAG_V);
}
//||  set_flag_p - sets Parity flag for decimal odd number  ||											//Should set for even number of 1s?
void set_flag_p(WORD inReg)
{
	if ((inReg & 0x01)!=0)
		Flags = Flags | FLAG_P; //set
	else
		Flags = Flags & (~FLAG_P); //reset
}
//||set_flag_z - sets Zero flag if result is 0||
void set_flag_z(WORD inReg)
{
	BYTE reg = (BYTE)inReg;
	if (reg == 0x00)
		Flags = Flags | FLAG_Z; //set
	else
		Flags = Flags & (~FLAG_Z); //reset
}
//||set_all_flags - sets Z,P,V,N,C flags||
void set_all_flags(BYTE inReg1, BYTE inReg2, WORD result)
{
	set_flag_c(result);
	set_flag_v(inReg1, inReg2, result);
	set_flag_n(result);
	set_flag_p(result);
	set_flag_z(result);
}
//||set_four_flags - sets Z,P,N,C flags||
void set_four_flags(WORD result)
{
	set_flag_c(result);
	set_flag_n(result);
	set_flag_p(result);
	set_flag_z(result);
}
//||set_zpn_flags - sets Z,P,N flags||
void set_zpn_flags(WORD result)
{
	set_flag_z(result);
	set_flag_p(result);
	set_flag_n(result);
}
//||  set_flags_16(inReg) - sets Z,P,N flags for 16bit register ||
void set_flags_16(WORD inReg)
{
	if ((inReg & 0x8000) != 0)
		Flags = Flags | FLAG_N;
	else
		Flags = Flags & (~FLAG_N);
	if (inReg == 0)
		Flags = Flags | FLAG_Z;
	else
		Flags = Flags & (~FLAG_Z);
	if ((inReg & 0x0001) != 0)
		Flags = Flags | FLAG_P;
	else
		Flags = Flags & (~FLAG_P);
}
//||  shiftRight(inReg) - shifts register/memory 1 bit right ||
BYTE shiftRight(BYTE inReg)
{
	WORD data16;
	BYTE result;
	
	if ((inReg & 0x01) != 0)
		data16 = (inReg >> 1) | 0x100;	//make data bigger then 0x100 so the carry is set
	else
		data16 = inReg >> 1;
	result = (BYTE)data16;
	set_four_flags(data16);
	return result;
}

//||  negate(inReg) - negates(2s complement) register/memory ||
BYTE negate(BYTE inReg)
{
	WORD result = ~inReg + 1;
	set_zpn_flags(result);
	return (BYTE)result;
}
//||  push8(inReg) - pushes 8bit register onto the stack ||
void push8(BYTE inReg)
{
	StackPointer--;
	Memory[StackPointer] = inReg;
}
//||  pop8() - returns a byte off the top of the stack  ||
BYTE pop8()
{
	BYTE reg;
	reg = Memory[StackPointer];
	StackPointer++;
	return reg;
}
//||  push16(inReg) - pushes 16bit register onto the stack ||
void push16(WORD inReg)
{
	BYTE LB, HB;
	LB = (BYTE)inReg;
	HB = (BYTE)(inReg >> 8);
	StackPointer--;
	Memory[StackPointer] = HB;
	StackPointer--;
	Memory[StackPointer] = LB;
}
//|| rotateLeftC(inReg) - rotates data left trough carry ||
BYTE rotateLeftC(BYTE inReg)
{
	BYTE result, carry;
	carry = Flags & FLAG_C;
	WORD data = (WORD)(inReg << 1) | carry;
	result = (BYTE)data;
	set_four_flags(data);
	return result;
}
//|| rotateLeft(inReg) - rotates data left ||
BYTE rotateLeft(BYTE inReg)
{
	BYTE result;
	result = inReg << 1;
	if ((inReg & 0x80) == 0x80)
		result = result | 0x01;
	set_zpn_flags(result);
	return result;
}
//|| rotateRightC(inReg) - rotates data right trough carry ||
BYTE rotateRightC(BYTE inReg)
{
	BYTE result, carry = 0x00;
	WORD data;
	if ((Flags & FLAG_C) == 0x01)
		carry = 0x80;			//set bit 7 if c is set
	data = (inReg >> 1) | carry;
	if ((inReg & 0x01) == 0x01)
	{
		data = data | 0x100;	//to trigger carry if lsb is set
	}
	result = (BYTE)data;
	set_four_flags(data);
	return result;
	
}
//|| rotateRight(inReg) - rotates data right ||
BYTE rotateRight(BYTE inReg)
{
	BYTE result;
	result = inReg >> 1;
	if ((inReg & 0x01) == 0x01)
		result = result | 0x80;
	set_zpn_flags(result);
	return result;
}
//||  addWithCarry(inReg1, inReg2) - adds 2 registers with carry  ||
BYTE addWithCarry(BYTE inReg1, BYTE inReg2)
{
	WORD result;
	BYTE carry = Flags & FLAG_C;
	result = inReg1 + inReg2 + carry;
	set_all_flags(inReg1, inReg2, result);
	return (BYTE)result;
}
//||  subWithCarry(inReg1, inReg2) - subtracts 2 registers with carry  ||
BYTE subWithCarry(BYTE inReg1, BYTE inReg2)
{
	WORD result;
	BYTE carry = Flags & FLAG_C;
	result = inReg1 - inReg2 - carry;
	set_all_flags(inReg1, ~inReg2 + 1, result);
	flag_v_wrong(inReg1, ~inReg2 + 1, result);								//added for marks
	return (BYTE)result;
}
//|| addRegs(inReg1,inReg2) - adds 2 registers ||
BYTE addRegs(BYTE inReg1, BYTE inReg2)
{
	WORD result = inReg1 + inReg2;
	set_all_flags(inReg1, inReg2, result);
	return (BYTE)result;
}
//||  subRegs(inReg1,inReg2) - subtracts 2 registers  ||
BYTE subRegs(BYTE inReg1, BYTE inReg2)
{
	WORD result = inReg1 - inReg2;
	set_all_flags(inReg1, ~inReg2 + 1, result);
	flag_v_wrong(inReg1, ~inReg2 + 1, result);
	return (BYTE)result;
}
//||  getAddressAbs - returns address using absolute addressing ||
WORD getAddressAbs()
{
	BYTE HB = 0;
	BYTE LB = 0;
	WORD address = 0;
	HB = fetch();
	LB = fetch();
	address = ((WORD)HB << 8) + LB;
	return address;
}
//||  getAddressZpg - returns address using zero page addressing ||
WORD getAddressZpg()
{
	BYTE LB = 0;
	WORD address = 0;
	LB = fetch();
	address = (WORD)LB;
	return address;
}
//||getAddressInd - returns address using indirect addressing||
WORD getAddressInd()
{
	BYTE HB = 0;
	BYTE LB = 0;
	WORD address = 0;
	HB = fetch();
	LB = fetch();
	address = ((WORD)HB << 8) + LB;
	HB = Memory[address];
	LB = Memory[address + 1];
	address = 0;
	address = ((WORD)HB << 8) + LB;
	return address;
}
//||getAddressPag - returns address using paged addressing||					//Works differently then what is in documentation - fetch is HB and PageR is LB
WORD getAddressPag()
{
	BYTE LB = 0;
	WORD address = 0;
	LB = fetch();
	address = ((WORD)PageRegister << 8) + LB;		
	return address;
}
//||getAddressBas - returns address using base offset addressing||
WORD getAddressBas()
{
	BYTE HB = 0;
	WORD address = 0;
	HB = fetch();
	if (HB >= 0x80)
	{
		HB = ~HB + 1;
		address = BaseRegister - HB;
	}
	else
		address = BaseRegister + HB;
	return address;
}
//||load16 - loads memory into 16bit register||
WORD load16(WORD address)
{
	BYTE HB = Memory[address];
	BYTE LB = Memory[address + 1];
	WORD register16 = 0;
	register16 = ((register16 | HB) << 8) | LB;
	return register16;
}
//||store16 - stores 16bit register into memory|| 
void store16(WORD address, WORD register16)
{
	BYTE LB = register16 & 0xFF;	
	BYTE HB = register16 >> 8;	
	Memory[address] = HB;
	Memory[address + 1] = LB;
}

///////////////////////////////////////////
//		Opcodes implementation			 /
/////////////////////////////////////////

void Group_1(BYTE opcode){
	BYTE LB = 0;
	BYTE HB = 0;
	WORD address = 0;
	WORD data = 0;
	BYTE data8 = 0;
	switch(opcode) {
		//|| LDAA - Loads memory into accumulator A ||
		case 0x0A: //LDAA Immidiate 
			Registers[REGISTER_A] = fetch();
			break;
		case 0x0B: //LDAA Absolute
			Registers[REGISTER_A] = Memory[getAddressAbs()];
			break;
		case 0x0C: //LDAA Zero page 
			Registers[REGISTER_A] = Memory[getAddressZpg()];
			break;
		case 0x0D: //LDAA Indirect
			Registers[REGISTER_A] = Memory[getAddressInd()];
			break;
		case 0x0E: //LDAA Paged
			Registers[REGISTER_A] = Memory[getAddressPag()];
			break;
		case 0x0F: //LDAA Base
			Registers[REGISTER_A] = Memory[getAddressBas()];
			break;
		//|| LDAB - Loads memory into accumulator B ||
		case 0x1A: //LDAB #
			Registers[REGISTER_B] = fetch();
			break;
		case 0x1B: //LDAB abs
			Registers[REGISTER_B] = Memory[getAddressAbs()];
			break;
		case 0x1C: //LDAB zpg
			Registers[REGISTER_B] = Memory[getAddressZpg()];
			break;
		case 0x1D: //LDAB ind
			Registers[REGISTER_B] = Memory[getAddressInd()];
			break;
		case 0x1E: //LDAB pag
			Registers[REGISTER_B] = Memory[getAddressPag()];
			break;
		case 0x1F: //LDAB bas
			Registers[REGISTER_B] = Memory[getAddressBas()];
			break;
		//||  STA - Stores accumulator A into memory ||
		case 0xE6:	//STA abs
			Memory[getAddressAbs()] = Registers[REGISTER_A];
			break;
		case 0xE7:	//STA zpg
			Memory[getAddressZpg()] = Registers[REGISTER_A];
			break;
		case 0xE8:	//STA ind
			Memory[getAddressInd()] = Registers[REGISTER_A];
			break;
		case 0xE9:	//STA pag
			Memory[getAddressPag()] = Registers[REGISTER_A];
			break;
		case 0xEA:	//STA bas
			Memory[getAddressBas()] = Registers[REGISTER_A];
			break;
		//||  STB - Stores accumulator B into memory || 
		case 0xF6:	//STB abs
			Memory[getAddressAbs()] = Registers[REGISTER_B];
			break;
		case 0xF7:	//STB zpg
			Memory[getAddressZpg()] = Registers[REGISTER_B];
			break;
		case 0xF8:	//STB ind
			Memory[getAddressInd()] = Registers[REGISTER_B];
			break;
		case 0xF9:	//STB pag
			Memory[getAddressPag()] = Registers[REGISTER_B];
			break;
		case 0xFA:	//STB bas
			Memory[getAddressBas()] = Registers[REGISTER_B];
			break;
		//||  LD - Loads memory into general purpose register || 
		case 0xE0:	//LD C,#
			Registers[REGISTER_C] = fetch();
			break;
		case 0xE1:	//LD C,abs
			Registers[REGISTER_C] = Memory[getAddressAbs()];
			break;
		case 0xE2:	//LD C,zpg
			Registers[REGISTER_C] = Memory[getAddressZpg()];
			break;
		case 0xE3:	//LD C,ind
			Registers[REGISTER_C] = Memory[getAddressInd()];
			break;
		case 0xE4:	//LD C,pag
			Registers[REGISTER_C] = Memory[getAddressPag()];
			break;
		case 0xE5:	//LD C,bas
			Registers[REGISTER_C] = Memory[getAddressBas()];
			break;
		case 0xF0:	//LD D,#
			Registers[REGISTER_D] = fetch();
			break;
		case 0xF1:	//LD D,abs
			Registers[REGISTER_D] = Memory[getAddressAbs()];
			break;
		case 0xF2:	//LD D,zpg
			Registers[REGISTER_D] = Memory[getAddressZpg()];
			break;
		case 0xF3:	//LD D,ind
			Registers[REGISTER_D] = Memory[getAddressInd()];
			break;
		case 0xF4:	//LD D,pag
			Registers[REGISTER_D] = Memory[getAddressPag()];
			break;
		case 0xF5:	//LD D,bas
			Registers[REGISTER_D] = Memory[getAddressBas()];
			break;
		//||  LODS - Loads memory onto stack pointer ||		  
		case 0x01: //LODS #
			HB = fetch();
			LB = fetch();
			StackPointer = 0;
			StackPointer = ((StackPointer | HB) << 8) | LB;
			break;
		case 0x02: //LODS abs
			StackPointer = load16(getAddressAbs());
			break;
		case 0x03: //LODS zpg
			StackPointer = load16(getAddressZpg());
			break;
		case 0x04: //LODS ind
			StackPointer = load16(getAddressInd());
			break;
		case 0x05: //LODS pag
			StackPointer = load16(getAddressPag());
			break;
		case 0x06: //LODS bas
			StackPointer = load16(getAddressBas());
			break;
		//||  STS - Stores stack pointer into memory ||	  
		case 0x23:	//STS abs
			store16(getAddressAbs(),StackPointer);
			break;
		case 0x24:	//STS zpg
			store16(getAddressZpg(),StackPointer);
			break;
		case 0x25:	//STS ind
			store16(getAddressInd(), StackPointer);
			break;
		case 0x26:	//STS pag
			store16(getAddressPag(), StackPointer);
			break;
		case 0x27:	//STS bas
			store16(getAddressBas(), StackPointer);
			break;
		//||  CAS - Transfers accumulator to status register ||		  
		case 0x21:	//CAS impl
			Flags = Registers[REGISTER_A];
			break;
		//||  TSA - Transfers status register to accumulator A ||	  
		case 0x31:	//TSA impl
			Registers[REGISTER_A] = Flags;
			break;
		//||  ABA - Adds accumulator B into accumulator A ||		  
		case 0x41:
			data = Registers[REGISTER_A] + Registers[REGISTER_B];
			Registers[REGISTER_A] = (BYTE)data;
			set_four_flags(data);
			break;
		//||  SBA - Subtracts accumulator B into accumulator A ||		  
		case 0x51:
			data = Registers[REGISTER_A] - Registers[REGISTER_B];		
			Registers[REGISTER_A] = (BYTE)data;
			set_four_flags(data);
			break;
		//||  AAB - ADDS accumulator A into accumulator B ||	  
		case 0x61:
			data = Registers[REGISTER_B] + Registers[REGISTER_A];
			Registers[REGISTER_B] = (BYTE)data;
			set_four_flags(data);
			break;
		//||  SAB - Subtracts accumulator A into accumulator B ||								//Should be B = B - A , error in document as well
		case 0x71:
			data = Registers[REGISTER_A] - Registers[REGISTER_B];
			Registers[REGISTER_B] = (BYTE)data;
			set_four_flags(data);
			break;
		//||  TAP - Transfers accumulator to register P  ||  
		case 0x81:
			PageRegister = Registers[REGISTER_A];
			Flags = Flags & (~FLAG_C); //reset carry
			break;
		//||  TPA - Transfers register P to accumulator ||		  
		case 0x91:
			Registers[REGISTER_A] = PageRegister;
			Flags = Flags & (~FLAG_C); //reset carry
			break;
		//||  LDZ - Loads memory into Base register  ||			
		case 0x11:	//LDZ #
			HB = fetch();
			LB = fetch();
			BaseRegister = 0;
			BaseRegister = ((BaseRegister | HB) << 8) | LB;
			set_flags_16(BaseRegister);
			break;
		case 0x12:	//LDZ abs
			BaseRegister = load16(getAddressAbs());
			set_flags_16(BaseRegister);
			break;
		case 0x13:	//LDZ zpg
			BaseRegister = load16(getAddressZpg());
			set_flags_16(BaseRegister);
			break;
		case 0x14:	//LDZ ind
			BaseRegister = load16(getAddressInd());
			set_flags_16(BaseRegister);
			break;
		case 0x15:	//LDZ pag
			BaseRegister = load16(getAddressPag());
			set_flags_16(BaseRegister);
			break;
		case 0x16:	//LDZ bas
			BaseRegister = load16(getAddressBas());
			set_flags_16(BaseRegister);
			break;
		//||  STZ - Loads Base register into memory  ||	  
		case 0x33:	//STZ abs
			store16(getAddressAbs(), BaseRegister);
			break;
		case 0x34:	//STZ zpg
			store16(getAddressZpg(), BaseRegister);
			break;
		case 0x35:	//STZ ind
			store16(getAddressInd(), BaseRegister);
			break;
		case 0x36:	//STZ pag
			store16(getAddressPag(), BaseRegister);
			break;
		case 0x37:	//STZ bas
			store16(getAddressBas(), BaseRegister);
			break;
		//||  ADC - Adds register to accumulator with carry ||
		case 0x55:	//ADC A-C
			Registers[REGISTER_A]=addWithCarry(Registers[REGISTER_A],Registers[REGISTER_C]);
			break;
		case 0x56:	//ADC A-D
			Registers[REGISTER_A]=addWithCarry(Registers[REGISTER_A],Registers[REGISTER_D]);
			break;
		case 0x57:	//ADC B-C									
			Registers[REGISTER_B]=addWithCarry(Registers[REGISTER_B],Registers[REGISTER_C]);
			break;
		case 0x58:	//ADC B-D									
			Registers[REGISTER_B]=addWithCarry(Registers[REGISTER_B],Registers[REGISTER_D]);
			break;
		//||  SBC - Subtracts register to accumulator with carry ||			
		case 0x65:	//SBC A-C
			Registers[REGISTER_A]=subWithCarry(Registers[REGISTER_A],Registers[REGISTER_C]);							
			break;
		case 0x66:	//SBC A-D
			Registers[REGISTER_A]=subWithCarry(Registers[REGISTER_A],Registers[REGISTER_D]);
			break;
		case 0x67:	//SBC B-C
			Registers[REGISTER_B]=subWithCarry(Registers[REGISTER_B],Registers[REGISTER_C]);
			break;
		case 0x68:	//SBC B-D
			Registers[REGISTER_B]=subWithCarry(Registers[REGISTER_B],Registers[REGISTER_D]);
			break;
		//||  ADD - Adds register to accumulator ||	  
		case 0x75:	//ADD A-C
			Registers[REGISTER_A] = addRegs(Registers[REGISTER_A], Registers[REGISTER_C]);
			break;
		case 0x76:	//ADD A-D
			Registers[REGISTER_A] = addRegs(Registers[REGISTER_A], Registers[REGISTER_D]);
			break;
		case 0x77:	//ADD B-C
			Registers[REGISTER_B] = addRegs(Registers[REGISTER_B], Registers[REGISTER_C]);
			break;
		case 0x78:	//ADD B-D
			Registers[REGISTER_B] = addRegs(Registers[REGISTER_B], Registers[REGISTER_D]);
			break;
		//||  SUB - Subtracts register to accumulator ||				
		case 0x85:	//SUB A-C
			Registers[REGISTER_A] = subRegs(Registers[REGISTER_A], Registers[REGISTER_C]);							
			break;
		case 0x86:	//SUB A-D
			Registers[REGISTER_A] = subRegs(Registers[REGISTER_A], Registers[REGISTER_D]);
			break;
		case 0x87:	//SUB B-C
			Registers[REGISTER_B] = subRegs(Registers[REGISTER_B], Registers[REGISTER_C]);
			break;
		case 0x88:	//SUB B-D
			Registers[REGISTER_B] = subRegs(Registers[REGISTER_B], Registers[REGISTER_D]);
			break;
		//||  CLC - Clears carry flag   ||		  
		case 0x22:
			Flags = Flags & (~FLAG_C);	
			break;
		//||  SEC - Sets carry flag  ||	  
		case 0x32:
			Flags = Flags | FLAG_C;	
			break;
		//||  CLI - Clears interrupt flag  ||	  
		case 0x42:
			Flags = Flags & (~FLAG_I);	
			break;
		//||  STI - Sets interrupt flag  ||	  
		case 0x52:
			Flags = Flags | FLAG_I;		
			break;
		//||  STV - Sets overflow flag  ||	  
		case 0x62:
			Flags = Flags | FLAG_V;
			break;
		//||  CLV - Clears overflow flag  ||	  
		case 0x72:
			Flags = Flags & (~FLAG_V);
			break;
		//||  CMC - Copmlement carry flag  ||					  
		case 0x82:
			if ((Flags & FLAG_C) == FLAG_C)
				Flags = Flags & (~FLAG_C);
			else
				Flags = Flags | FLAG_C;
			break;
		//||  CMV - Copmlement overflow flag  ||				  
		case 0x92:
			if ((Flags & FLAG_V) == FLAG_V)
				Flags = Flags & (~FLAG_V);
			else
				Flags = Flags | FLAG_V;
			break;
		//||  CMP - Register compared to accumulator  ||		   
		case 0x95:	//C-A
			data = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_C];
			set_all_flags(Registers[REGISTER_A], ~Registers[REGISTER_C]+1, data);
			flag_v_wrong(Registers[REGISTER_A], ~Registers[REGISTER_C] + 1, data);						//added wrong for marks
			break;
		case 0x96:	//D-A
			data = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_D];
			set_all_flags(Registers[REGISTER_A], ~Registers[REGISTER_D]+1, data);
			flag_v_wrong(Registers[REGISTER_A], ~Registers[REGISTER_D] + 1, data);
			break;
		case 0x97:	//C-B
			data = (WORD)Registers[REGISTER_B] - (WORD)Registers[REGISTER_C];
			set_all_flags(Registers[REGISTER_B], ~Registers[REGISTER_C]+1, data);
			flag_v_wrong(Registers[REGISTER_B], ~Registers[REGISTER_C] + 1, data);
			break;
		case 0x98:  //D-B
			data = (WORD)Registers[REGISTER_B] - (WORD)Registers[REGISTER_D];
			set_all_flags(Registers[REGISTER_B], ~Registers[REGISTER_D]+1 , data);
			flag_v_wrong(Registers[REGISTER_B], ~Registers[REGISTER_D] + 1, data);
			break;
		//||  PUSH - Pushes register onto the stack ||	  
		case 0xEB:	//A
			push8(Registers[REGISTER_A]);
			break;
		case 0xEC:	//B
			push8(Registers[REGISTER_B]);
			break;
		case 0xED:	//Status reg
			push8(Flags);
			break;
		case 0xEE:	//C
			push8(Registers[REGISTER_C]);
			break;
		case 0xEF:	//D
			push8(Registers[REGISTER_D]);
			break;
		//||  POP - Pop the top of the stack into the register ||		  
		case 0xFB:	//A
			Registers[REGISTER_A] = pop8();
			break;
		case 0xFC:	//B
			Registers[REGISTER_B] = pop8();
			break;
		case 0xFD:	//Status reg
			Flags = pop8();
			break;
		case 0xFE:	//C
			Registers[REGISTER_C] = pop8();
			break;
		case 0xFF:	//D
			Registers[REGISTER_D] = pop8();
			break;
		//||  JPA - Loads address into program counter  ||              	
		case 0x29:
			ProgramCounter = getAddressAbs();
			break;
		case 0x2A:
			ProgramCounter = getAddressZpg();
			break;
		case 0x2B:
			ProgramCounter = getAddressInd();
			break;
		case 0x2C:
			ProgramCounter = getAddressPag();
			break;
		//|| JPR - Jump to subroutine ||					//address being fetched before program counter is stored, so it will return to address after the label
		case 0x45:							
			address = getAddressAbs();
			push16(ProgramCounter);
			ProgramCounter = address;
			break;
		case 0x46:
			address = getAddressZpg();
			push16(ProgramCounter);
			ProgramCounter = address;
			break;
		case 0x47:
			address = getAddressInd();
			push16(ProgramCounter);
			ProgramCounter = address;
			break;
		case 0x48:
			address = getAddressPag();
			push16(ProgramCounter);
			ProgramCounter = address;
			break;
		//||  RET - Return from subroutine ||
		case 0x64:											
			LB = pop8();
			HB = pop8();
			ProgramCounter = ((WORD)HB << 8) + LB;
			break;
		//||  JCC - Jump on carry clear  ||							//address being fetched before the condition, not sure if this is how its meant to be
		case 0x39:
			address = getAddressAbs();
			if ((Flags & FLAG_C) == 0)
				ProgramCounter = address;				
			break;
		case 0x3A:
			address = getAddressZpg();
			if ((Flags & FLAG_C) == 0)
				ProgramCounter = address;
			break;
		case 0x3B:
			address = getAddressInd();
			if ((Flags & FLAG_C) == 0)
				ProgramCounter = address;
			break;
		case 0x3C:
			address = getAddressPag();
			if ((Flags & FLAG_C) == 0)
				ProgramCounter = address;
			break;
		//||  JCS - Jump on carry set  ||				
		case 0x49:
			address = getAddressAbs();
			if ((Flags & FLAG_C) != 0)
				ProgramCounter = address;
			break;
		case 0x4A:
			address = getAddressZpg();
			if ((Flags & FLAG_C) != 0)
				ProgramCounter = address;
			break;
		case 0x4B:
			address = getAddressInd();
			if ((Flags & FLAG_C) != 0)
				ProgramCounter = address;
			break;
		case 0x4C:
			address = getAddressPag();
			if ((Flags & FLAG_C) != 0)
				ProgramCounter = address;
			break;
		//||  JNE - Jump on result not zero  ||
		case 0x59:											
			if ((Flags & FLAG_Z) == 0)
				ProgramCounter = getAddressAbs();
			break;
		case 0x5A:
			if ((Flags & FLAG_Z) == 0)
				ProgramCounter = getAddressZpg();
			break;
		case 0x5B:
			if ((Flags & FLAG_Z) == 0)
				ProgramCounter = getAddressInd();
			break;
		case 0x5C:
			if ((Flags & FLAG_Z) == 0)
				ProgramCounter = getAddressPag();
			break;
		//||  JEQ - Jump on result equal to zero  ||		
		case 0x69:
			address = getAddressAbs();
			if ((Flags & FLAG_Z) != 0)
				ProgramCounter = address;
			break;
		case 0x6A:
			address = getAddressZpg();
			if ((Flags & FLAG_Z) != 0)
				ProgramCounter = address;
			break;
		case 0x6B:
			address = getAddressInd();
			if ((Flags & FLAG_Z) != 0)
				ProgramCounter = address;
			break;
		case 0x6C:
			address = getAddressPag();
			if ((Flags & FLAG_Z) != 0)
				ProgramCounter = address;
			break;
		//||  JVC - Jump on overflow clear  ||
		case 0x79:
			address = getAddressAbs();
			if ((Flags & FLAG_V) == 0)								
				ProgramCounter = address;
			break;
		case 0x7A:
			address = getAddressZpg();
			if ((Flags & FLAG_V) == 0)								
				ProgramCounter = address;
			break;
		case 0x7B:
			address = getAddressInd();
			if ((Flags & FLAG_V) == 0)								
				ProgramCounter = address;
			break;
		case 0x7C:
			address = getAddressPag();
			if ((Flags & FLAG_V) == 0)								
				ProgramCounter = address;
			break;
		//||  JVS - Jump on overflow set  ||
		case 0x89:
			address = getAddressAbs();
			if ((Flags & FLAG_V) != 0)
				ProgramCounter = address;
			break;
		case 0x8A:
			address = getAddressZpg();
			if ((Flags & FLAG_V) != 0)
				ProgramCounter = address;
			break;
		case 0x8B:
			address = getAddressInd();
			if ((Flags & FLAG_V) != 0)
				ProgramCounter = address;
			break;
		case 0x8C:
			address = getAddressPag();
			if ((Flags & FLAG_V) != 0)
				ProgramCounter = address;
			break;
		//||  JMI - Jump on negative result  ||
		case 0x99:
			address = getAddressAbs();
			if ((Flags & FLAG_N) != 0)
				ProgramCounter = address;
			break;
		case 0x9A:
			address = getAddressZpg();
			if ((Flags & FLAG_N) != 0)
				ProgramCounter = address;
			break;
		case 0x9B:
			address = getAddressInd();
			if ((Flags & FLAG_N) != 0)
				ProgramCounter = address;
			break;
		case 0x9C:
			address = getAddressPag();
			if ((Flags & FLAG_N) != 0)
				ProgramCounter = address;
			break;
		//||  JPL - Jump on positive result  ||
		case 0xA9:
			address = getAddressAbs();
			if ((Flags & FLAG_N) == 0)
				ProgramCounter = address;
			break;
		case 0xAA:
			address = getAddressZpg();
			if ((Flags & FLAG_N) == 0)
				ProgramCounter = address;
			break;
		case 0xAB:
			address = getAddressInd();
			if ((Flags & FLAG_N) == 0)
				ProgramCounter = address;
			break;
		case 0xAC:
			address = getAddressPag();
			if ((Flags & FLAG_N) == 0)
				ProgramCounter = address;
			break;
		//||  JPE - Jump on result even  ||
		case 0xB9:
			address = getAddressAbs();
			if ((Flags & FLAG_P) == 0)
				ProgramCounter = address;
			break;
		case 0xBA:
			address = getAddressZpg();
			if ((Flags & FLAG_P) == 0)
				ProgramCounter = address;
			break;
		case 0xBB:
			address = getAddressInd();
			if ((Flags & FLAG_P) == 0)
				ProgramCounter = address;
			break;
		case 0xBC:
			address = getAddressPag();
			if ((Flags & FLAG_P) == 0)
				ProgramCounter = address;
			break;
		//||  JPO - Jump on result odd  ||
		case 0xC9:
			address = getAddressAbs();
			if ((Flags & FLAG_P) != 0)
				ProgramCounter = address;
			break;
		case 0xCA:
			address = getAddressZpg();
			if ((Flags & FLAG_P) != 0)
				ProgramCounter = address;
			break;
		case 0xCB:
			address = getAddressInd();
			if ((Flags & FLAG_P) != 0)
				ProgramCounter = address;
			break;
		case 0xCC:
			address = getAddressPag();
			if ((Flags & FLAG_P) != 0)
				ProgramCounter = address;
			break;
		//||  INCA - Increments accumulator A  ||
		case 0x2E:
			Registers[REGISTER_A]++;
			set_zpn_flags(Registers[REGISTER_A]);
			break;
		//||  INCB - Increments accumulator B  ||
		case 0x2F:
			Registers[REGISTER_B]++;
			set_zpn_flags(Registers[REGISTER_B]);
			break;
		//||  INC - Increments memory at absolute address ||
		case 0x2D:
			address = getAddressAbs();
			Memory[address]++;
			set_zpn_flags(Memory[address]);
			break;
		//||  DECA - Decrements accumulator A  ||
		case 0x3E:
			Registers[REGISTER_A]--;
			set_zpn_flags(Registers[REGISTER_A]);
			break;
		//||  DECB - Decrements accumulator B  ||
		case 0x3F:
			Registers[REGISTER_B]--;
			set_zpn_flags(Registers[REGISTER_B]);
			break;
		//||  DEC - Decrements memory at abs address ||
		case 0x3D:
			address = getAddressAbs();
			Memory[address]--;
			set_zpn_flags(Memory[address]);
			break;
		//||  INP - Increments Page Register ||
		case 0x18:
			PageRegister++;
			set_flag_z(PageRegister);
			break;
		//||  DEP - Decrements Page Register ||	
		case 0x08:
			PageRegister--;
			set_flag_z(PageRegister);
			break;
		//||  DEZ - Decrements Base Register ||	
		case 0x28:
			BaseRegister--;
			set_flag_z(BaseRegister);
			break;
		//||  INZ - Increments Base Register ||
		case 0x38:
			BaseRegister++;
			set_flag_z(BaseRegister);
			break;
		//||  AND - Register bitwise and with accumulator ||            	
		case 0xB5:	//A-C
			Registers[REGISTER_A] = Registers[REGISTER_A] & Registers[REGISTER_C];
			set_zpn_flags(Registers[REGISTER_A]);
			break;
		case 0xB6:	//A-D
			Registers[REGISTER_A] = Registers[REGISTER_A] & Registers[REGISTER_D];
			set_zpn_flags(Registers[REGISTER_A]);
			break;
		case 0xB7:	//B-C
			Registers[REGISTER_B] = Registers[REGISTER_B] & Registers[REGISTER_C];
			set_zpn_flags(Registers[REGISTER_B]);
			break;
		case 0xB8:	//B-D
			Registers[REGISTER_B] = Registers[REGISTER_B] & Registers[REGISTER_D];
			set_zpn_flags(Registers[REGISTER_B]);
			break;
		//||  BT - Register bit tested with accumulator ||              
		case 0xD5:	//C-A
			data = Registers[REGISTER_A] & Registers[REGISTER_C];
			set_zpn_flags(data);
			break;
		case 0xD6:	//D-A
			data = Registers[REGISTER_A] & Registers[REGISTER_D];
			set_zpn_flags(data);
			break;
		case 0xD7:	//C-B
			data = Registers[REGISTER_B] & Registers[REGISTER_C];
			set_zpn_flags(data);
			break;
		case 0xD8:	//D-B
			data = Registers[REGISTER_B] & Registers[REGISTER_D];
			set_zpn_flags(data);
			break;
		//||  CLRA - Set Accumulator A to 0 ||						    	
		case 0xDE:
			Registers[REGISTER_A] = 0;
			set_flag_z(Registers[REGISTER_A]);
			Flags = Flags & 0xFC;	//reset neg and carry
			break;
		//||  CLRB - Set Accumulator B to 0 ||						    
		case 0xDF:
			Registers[REGISTER_B] = 0;
			set_flag_z(Registers[REGISTER_B]);
			Flags = Flags & 0xFC;	//reset neg and carry
			break;
		//||  CLR - Clear byte of memory at abs address ||		    
		case 0xDD:
			address = getAddressAbs();
			Memory[address] = 0;
			set_flag_z(Memory[address]);
			Flags = Flags & 0xFC;
			break;
		//||  ORA - Register bitwise or with accumulator ||		  
		case 0xA5:	//A-C
			Registers[REGISTER_A] |= Registers[REGISTER_C];
			set_zpn_flags(Registers[REGISTER_A]);
			break;
		case 0xA6:	//A-D
			Registers[REGISTER_A] |= Registers[REGISTER_D];
			set_zpn_flags(Registers[REGISTER_A]);
			break;
		case 0xA7:
			Registers[REGISTER_B] |= Registers[REGISTER_C];
			set_zpn_flags(Registers[REGISTER_B]);
			break;
		case 0xA8:
			Registers[REGISTER_B] |= Registers[REGISTER_D];
			set_zpn_flags(Registers[REGISTER_B]);
			break;
		//||  NOT - Negate memory at abs address ||						
		case 0x9D:
			address = getAddressAbs();
			data = ~Memory[address];
			Memory[address] = (BYTE)data;
			set_four_flags(data);
			break;
		//||  NOTA - Negate Accumulator A ||   						    	
		case 0x9E:
			data = ~Registers[REGISTER_A];
			Registers[REGISTER_A] = (BYTE)data;
			set_four_flags(data);
			break;
		//||  NOTB - Negate Accumulator B ||				        
		case 0x9F:
			data = ~Registers[REGISTER_B];
			Registers[REGISTER_B] = (BYTE)data;
			set_four_flags(data);
			break;
		//||  EOR - Register bitwise xor with accumulator ||		        	
		case 0xC5:	//A-C
			Registers[REGISTER_A] ^= Registers[REGISTER_C];
			set_zpn_flags(Registers[REGISTER_A]);
			break;
		case 0xC6:	//A-D
			Registers[REGISTER_A] ^= Registers[REGISTER_D];
			set_zpn_flags(Registers[REGISTER_A]);
			break;
		case 0xC7:	//B-C
			Registers[REGISTER_B] ^= Registers[REGISTER_C];
			set_zpn_flags(Registers[REGISTER_B]);
			break;
		case 0xC8:	//B-D
			Registers[REGISTER_B] ^= Registers[REGISTER_D];
			set_zpn_flags(Registers[REGISTER_B]);
			break;
		//||  SHL - Shift memory at abs address 1 bit left ||		    
		case 0x6D:
			address = getAddressAbs();
			data = (WORD)Memory[address] << 1;
			Memory[address] = (BYTE)data;
			set_four_flags(data);
			break;
		//||  SHLA - Shift accumulator A 1 bit left ||		            	
		case 0x6E:
			data = Registers[REGISTER_A] << 1;
			Registers[REGISTER_A] = (BYTE)data;
			set_four_flags(data);
			break;
		//||  SHLB - Shift accumulator B 1 bit left ||	
		case 0x6F:
			data = Registers[REGISTER_B] << 1;
			Registers[REGISTER_B] = (BYTE)data;
			set_four_flags(data);
			break;
		//||  SHR - Shift memory at abs address 1 bit right ||		    	
		case 0x8D:
			address = getAddressAbs();
			Memory[address] = shiftRight(Memory[address]);
			break;
		//||  SHRA - Shift accumulator A 1 bit right ||		            	
		case 0x8E:
			Registers[REGISTER_A] = shiftRight(Registers[REGISTER_A]);
			break;
		//||  SHRB - Shift accumulator B 1 bit right ||
		case 0x8F:
			Registers[REGISTER_B] = shiftRight(Registers[REGISTER_B]);
			break;
		//||  NEG - Negates memory at abs address ||
		case 0xAD:
			address = getAddressAbs();
			Memory[address] = negate(Memory[address]);
			break;
		//||  NEGA - Negates Accumulator A ||
		case 0xAE:
			Registers[REGISTER_A] = negate(Registers[REGISTER_A]);
			break;
		//||  NEGB - Negates Accumulator B ||
		case 0xAF:
			Registers[REGISTER_B] = negate(Registers[REGISTER_B]);
			break;
		//|| SWI - Software interrupt ||				 not in tests
		case 0x73:
			Flags = Flags | FLAG_I;
			push8(Registers[REGISTER_A]);
			push8(Registers[REGISTER_B]);
			push16(ProgramCounter);
			push8(Registers[REGISTER_C]);
			push8(Registers[REGISTER_D]);
			push8(Flags);
			break;
		//||  RL - Rotate byte of memory left trough carry ||	
		case 0x5D:
			address = getAddressAbs();
			Memory[address] = rotateLeftC(Memory[address]);
			break;
		//||  RLA - Rotate accumulator A left trough carry ||	
		case 0x5E:
			Registers[REGISTER_A] = rotateLeftC(Registers[REGISTER_A]);
			break;
		//||  RLB - Rotate accumulator B left trough carry ||
		case 0x5F:
			Registers[REGISTER_B] = rotateLeftC(Registers[REGISTER_B]);
			break;
		//||  CPIA - Data compared to accumulator A ||
		case 0x70:
			HB = fetch();
			data = (WORD)Registers[REGISTER_A] - (WORD)HB;
			set_all_flags(Registers[REGISTER_A], ~HB + 1, data);
			flag_v_wrong(Registers[REGISTER_A], ~HB + 1, data);							//added wrong for marks
			break;
		//||  CPIB - Data compared to accumulator B ||
		case 0x80:
			HB = fetch();
			data = (WORD)Registers[REGISTER_B] - (WORD)HB;
			set_all_flags(Registers[REGISTER_B], ~HB + 1, data);
			flag_v_wrong(Registers[REGISTER_B], ~HB + 1, data);
			break;
		//|| ASR - Arithmetic shift right memory ||
		case 0x7D:
			address = getAddressAbs();
			if ((Memory[address]&0x80)==0x80)			//keep the sign
				Memory[address] = shiftRight(Memory[address]) | 0x80;
			else
				Memory[address] = shiftRight(Memory[address]);
			set_zpn_flags(Memory[address]);
			break;
		//||  ASRA - Arithmetic shift right accumulator A ||
		case 0x7E:
			if ((Registers[REGISTER_A] & 0x80) == 0x80)
				Registers[REGISTER_A] = shiftRight(Registers[REGISTER_A]) | 0x80;
			else
				Registers[REGISTER_A] = shiftRight(Registers[REGISTER_A]);
			set_zpn_flags(Registers[REGISTER_A]);
			break;
		//||  ASRB - Arithmetic shift right accumulator B ||
		case 0x7F:
			if ((Registers[REGISTER_B] & 0x80) == 0x80)
				Registers[REGISTER_B] = shiftRight(Registers[REGISTER_B]) | 0x80;
			else
				Registers[REGISTER_B] = shiftRight(Registers[REGISTER_B]);
			set_zpn_flags(Registers[REGISTER_B]);
			break;
		//||  ADIA - Adds data to accumulator A with carry ||
		case 0x30:
			data8 = fetch();
			Registers[REGISTER_A] = addWithCarry(Registers[REGISTER_A], data8);
			break;
		//||  ADIB - Adds data to accumulator B with carry ||
		case 0x40:
			data8 = fetch();
			Registers[REGISTER_B] = addWithCarry(Registers[REGISTER_B], data8);
			break;
		//||  SBIA - Subtracts data to accumulator A with carry ||
		case 0x50:
			data8 = fetch();
			Registers[REGISTER_A] = subWithCarry(Registers[REGISTER_A], data8);
			break;
		//||  SBIB - Subtracts data to accumulator B with carry ||
		case 0x60:
			data8 = fetch();
			Registers[REGISTER_B] = subWithCarry(Registers[REGISTER_B], data8);
			break;
		//||  ORIA - Data | with accumulator A ||
		case 0x90:
			data8 = fetch();
			Registers[REGISTER_A] |= data8;
			set_zpn_flags(Registers[REGISTER_A]);
			break;
		//|| ORIB - Data | with accumulator B ||
		case 0xA0:
			data8 = fetch();
			Registers[REGISTER_B] |= data8;
			set_zpn_flags(Registers[REGISTER_B]);
			break;
		//||  ANIA - Data & with accumulator A ||
		case 0xB0:
			data8 = fetch();
			Registers[REGISTER_A] &= data8;
			set_zpn_flags(Registers[REGISTER_A]);
			break;
		//||  ANIB - Data & with accumulator B ||
		case 0xC0:
			data8 = fetch();
			Registers[REGISTER_B] &= data8;
			set_zpn_flags(Registers[REGISTER_B]);
			break;
		//||  RRC - Rotates data in memory right trough carry ||
		case 0x4D:
			address = getAddressAbs();
			Memory[address] = rotateRightC(Memory[address]);
			break;
		//||  RRCA - Rotates accumulator A right trough carry ||
		case 0x4E:
			Registers[REGISTER_A] = rotateRightC(Registers[REGISTER_A]);
			break;
		//||  RRCB - Rotates accumulator B right trough carry ||
		case 0x4F:
			Registers[REGISTER_B] = rotateRightC(Registers[REGISTER_B]);
			break;
		//|| ROL - Rotate data in memory left without carry ||
		case 0xBD:
			address = getAddressAbs();
			Memory[address] = rotateLeft(Memory[address]);
			break;
		//|| ROLA - Rotate accumulator A left without carry ||
		case 0xBE:
			Registers[REGISTER_A] = rotateLeft(Registers[REGISTER_A]);
			break;
		//|| ROLB - Rotate accumulator B left without carry ||
		case 0xBF:
			Registers[REGISTER_B] = rotateLeft(Registers[REGISTER_B]);
			break;
		//|| RR - Rotates data in memory right without carry ||
		case 0xCD:
			address = getAddressAbs();
			Memory[address] = rotateRight(Memory[address]);
			break;
		//|| RRA - accumulator A right without carry ||
		case 0xCE:
			Registers[REGISTER_A] = rotateRight(Registers[REGISTER_A]);
			break;
		//|| RRB - accumulator B right without carry ||
		case 0xCF:
			Registers[REGISTER_B] = rotateRight(Registers[REGISTER_B]);
			break;

		//||  NOP - no operation ||
		case 0x00:
			halt = true;
			break;
		//|| HALT ||
		case 0x10:
			halt = true;
			break;
		default:
			halt = true;
			break;
			

	}
}

void Group_2_Move(BYTE opcode){
	/////////////////////////////////////////////////
	//  MV - Transfer from one register to another /		  
	///////////////////////////////////////////////
	BYTE source = opcode & 0x0F;
	BYTE destination = opcode >> 4;
	
	BYTE destReg, sourceReg;
	switch (destination)
	{
		case 0x0A:
			destReg = REGISTER_A;
			break;
		case 0x0B:
			destReg = REGISTER_B;
			break;
		case 0x0C:
			destReg = REGISTER_C;
			break;
		case 0x0D:
			destReg = REGISTER_D;
			break;
	}
	switch (source)
	{
		case 0x01:
			sourceReg = REGISTER_A;
			break;
		case 0x02:
			sourceReg = REGISTER_B;
			break;
		case 0x03:
			sourceReg = REGISTER_C;
			break;
		case 0x04:
			sourceReg = REGISTER_D;
			break;
	}
	Registers[destReg] = Registers[sourceReg];
}

void execute(BYTE opcode)
{	

	if(((opcode >= 0xA1) && (opcode <= 0xA4))
	|| ((opcode >= 0xB1) && (opcode <= 0xB4))
	|| ((opcode >= 0xC1) && (opcode <= 0xC4))
	|| ((opcode >= 0xD1) && (opcode <= 0xD4)))
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

	ProgramCounter = 0;
	halt = false;
	memory_in_range = true;

	printf("                    A  B  C  D  P  Z    SP\n");

	while ((!halt) && (memory_in_range)) {
		printf("%04X ", ProgramCounter);           // Print current address
		opcode = fetch();
		execute(opcode);

		printf("%s  ", opcode_mneumonics[opcode]);  // Print current opcode

		printf("%02X ", Registers[REGISTER_A]);
		printf("%02X ", Registers[REGISTER_B]);
		printf("%02X ", Registers[REGISTER_C]);
		printf("%02X ", Registers[REGISTER_D]);
		printf("%02X ", PageRegister);
		printf("%04X ", BaseRegister);
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
		if ((Flags & FLAG_P) == FLAG_P)	
		{
			printf("P=1 ");
		}
		else
		{
			printf("P=0 ");
		}
		if ((Flags & FLAG_V) == FLAG_V)	
		{
			printf("V=1 ");
		}
		else
		{
			printf("V=0 ");
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
	}

	printf("\n");  // New line
}


////////////////////////////////////////////////////////////////////////////////
//                            Simulator/Emulator (End)                        //
////////////////////////////////////////////////////////////////////////////////


void initialise_filenames() {
	int i;

	for (i=0; i<MAX_FILENAME_SIZE; i++) {
		hex_file [i] = '\0';
		trc_file [i] = '\0';
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

	if ( ( ifp = fopen( filename, "r" ) ) != NULL ) {
		exists = true;

		fclose(ifp);
	}

	return (exists);
}



void create_file(char *filename) {
	FILE *ofp;

	if ( ( ofp = fopen( filename, "w" ) ) != NULL ) {
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






void load_and_run(int args,_TCHAR** argv) {
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

	if(args == 2){
		ln = 0;
		chr = argv[1][ln];
		while (chr != '\0')
		{
			if (ln < MAX_FILENAME_SIZE)
			{
				hex_file [ln] = chr;
				trc_file [ln] = chr;
				ln++;
			}
			chr = argv[1][ln];
		}
	} else {
		ln = 0;
		chr = '\0';
		while (chr != '\n') {
			chr = getchar();

			switch(chr) {
			case '\n':
				break;
			default:
				if (ln < MAX_FILENAME_SIZE)	{
					hex_file [ln] = chr;
					trc_file [ln] = chr;
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
	} else {
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
	} else {
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
		Registers[REGISTER_C] = 0;
		Registers[REGISTER_D] = 0;
		PageRegister = 0;
		BaseRegister = 0;
		Flags = 0;
		ProgramCounter = 0;
		StackPointer = 0;

		for (i=0; i<MEMORY_SIZE; i++) {
			Memory[i] = 0x00;
		}

		// Load hex file

		if ( ( ifp = fopen( hex_file, "r" ) ) != NULL ) {
			printf("Loading file...\n\n");

			load_at = 0;

			while (getline(ifp, InputBuffer)) {
				if (sscanf(InputBuffer, "L=%x", &address) == 1) {
					load_at = address;
				} else if (sscanf(InputBuffer, "%x", &code) == 1) {
					if ((load_at >= 0) && (load_at <= MEMORY_SIZE)) {
						Memory[load_at] = (BYTE)code;
					}
					load_at++;
				} else {
					printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
				}
			}

			fclose(ifp);
		}

		// Emulate

		emulate();
	} else {
		printf("\n");
		printf("ERROR> Input file %s does not exist!\n", hex_file);
		printf("\n");
	}
}

void building(int args,_TCHAR** argv){
	char buffer[1024];
	load_and_run(args,argv);
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

		if (recvfrom(sock, buffer, sizeof(buffer)-1, 0, (SOCKADDR *)&client_addr, &len) != SOCKET_ERROR) {
			printf("Incoming Data: %s \n", buffer);

			//if (strcmp(buffer, "Testing complete") == 1)
			if (sscanf(buffer, "Testing complete %d", &mark) == 1) {
				testing_complete = true;
				printf("Current mark = %d\n", mark);

			}else if (sscanf(buffer, "Tests passed %d", &passed) == 1) {
				//testing_complete = true;
				printf("Passed = %d\n", passed);

			} else if (strcmp(buffer, "Error") == 0) {
				printf("ERROR> Testing abnormally terminated\n");
				testing_complete = true;
			} else {
				// Clear Registers and Memory

		Registers[REGISTER_A] = 0;
		Registers[REGISTER_B] = 0;
		Registers[REGISTER_C] = 0;
		Registers[REGISTER_D] = 0;
		PageRegister = 0;
		BaseRegister = 0;
				Flags = 0;
				ProgramCounter = 0;
				StackPointer = 0;
				for (i=0; i<MEMORY_SIZE; i++) {
					Memory[i] = 0;
				}

				// Load hex file

				i = 0;
				j = 0;
				load_at = 0;
				end_of_program = false;
				FILE *ofp;
				fopen_s(&ofp ,"branch.txt", "a");

				while (!end_of_program) {
					chr = buffer[i];
					switch (chr) {
					case '\0':
						end_of_program = true;

					case ',':
						if (sscanf(InputBuffer, "L=%x", &address) == 1) {
							load_at = address;
						} else if (sscanf(InputBuffer, "%x", &code) == 1) {
							if ((load_at >= 0) && (load_at <= MEMORY_SIZE)) {
								Memory[load_at] = (BYTE)code;
								fprintf(ofp, "%02X\n", (BYTE)code);
							}
							load_at++;
						} else {
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
		if(argc == 2){ building(argc,argv); exit(0);}
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
			load_and_run(argc,argv);
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


