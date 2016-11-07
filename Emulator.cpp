#include "stdafx.h"
#include <winsock2.h>
/*
Author: Dominic Lewis(15024059)
Created: 17/10/2016
Revised: 02/11/2016 - Adding Comments
Description: Emulates the Intel 8080 Processor
User advice: None
*/
#pragma comment(lib, "wsock32.lib")


#define STUDENT_NUMBER    "15024059"

#define IP_ADDRESS_SERVER "127.0.0.1"

#define PORT_SERVER 0x1984 
#define PORT_CLIENT 0x1985 

#define WORD  unsigned short
#define DWORD unsigned long
#define BYTE  unsigned char

#define MAX_FILENAME_SIZE 500
#define MAX_BUFFER_SIZE   500

SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

SOCKET sock;

WSADATA data;

char InputBuffer[MAX_BUFFER_SIZE];

char hex_file[MAX_BUFFER_SIZE];
char trc_file[MAX_BUFFER_SIZE];

//////////////////////////
//   Registers          //
//////////////////////////

#define FLAG_I  0x10 
#define FLAG_Z  0x04 
#define FLAG_N  0x02 
#define FLAG_C  0x01 
#define REGISTER_M 4
#define REGISTER_A 3
#define REGISTER_B 2
#define REGISTER_H 1   
#define REGISTER_L 0	
#define REGISTER_X 0
#define REGISTER_Y 1

BYTE Index_Registers[2];

BYTE Registers[5];
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
/*
Function: fetch
Description: Fetches from address a byte of memory which is set by the ProgramCounter
Paramaters: NONE
Returns: byte
Warnings: None
*/
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
/*
Function: fetch
Description: Fetches from address a byte of memory which is set by the ProgramCounter
Paramaters: NONE
Returns: byte
Warnings: None
*/
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
/*
Function: set_flag_n
Description: Sets flag n based on the result of the bitwise or ( | ) operation carried out inside the function
Paramaters: WORD inReg - This is passed to the function to ensure the correct register is stored and used in the operations
Returns: none (void)
Warnings: None
*/void set_flag_n(WORD inReg) {
	BYTE reg;
	reg = inReg;
	if ((reg & 0x80) == 0x80) {
		Flags = Flags | FLAG_N;
	}
	else {
		Flags = Flags & (~FLAG_N);
	}
}
/*
Function: set_flag_C
Description: Sets flag C based on the result of the bitwise or ( | ) operation carried out inside the function OR the result of the bitwise & in conjunction with the negation of FLAG_C 
Paramaters: WORD inReg - This is passed to the function to ensure the correct register is stored and used in the operations
Returns: none (void)
Warnings: None
*/

void set_flag_c(WORD result)
{
	if (result >= 0x100)
		Flags = Flags | FLAG_C;
	else
		Flags = Flags & (~FLAG_C);
}
/*
Function: set_three_flags
Description: Sets flag n,z and c based on result
Paramaters: WORD result, this is passed to the function to ensure the flags are set correctly.
Returns: none (void)
Warnings: None
*/

void set_three_flags(WORD result)
{
	set_flag_c(result);
	set_flag_n(result);
	set_flag_z(result);
}
/*
Function: add_regs
Description: Adds the contents of two registers, then sets the flag based on it and returns that result to the calling function
Paramaters: WORD result, this is passed to the function to ensure the flags are set correctly.
Returns: BYTE result: The result of the operation
Warnings: None
*/

BYTE add_regs(BYTE inReg1, BYTE inReg2)
{
	WORD result = inReg1 + inReg2;
	set_three_flags(result);
	return (BYTE)result;
}
/*
Function: sub_With_carry
Description: Verifies whether or not a carry should take place then subtracts the second register away from the first then also subtracts the carry
Paramters: BYTE inReg1, BYTE inReg2. These two paramaters represent the first and second register that will be used in the sum. 
Returns: BYTE result: The result of the operation
Warnings: None
*/
BYTE sub_with_carry(BYTE inReg1, BYTE inReg2)
{
	WORD result;
	BYTE carry = Flags & FLAG_C;
	result = inReg1 - inReg2 - carry;
	set_three_flags(result);
	return (BYTE)result;
}
/*
Function: add_with_carry
Description: Verifies whether or not a carry should take place then adds the second register to the first then also adds the carry
Paramters: BYTE inReg1, BYTE inReg2. These two paramaters represent the first and second register that will be used in the sum.
Returns: BYTE result: The result of the operation
Warnings: None
*/

BYTE add_with_carry(BYTE inReg1, BYTE inReg2)
{
	WORD result;
	BYTE carry = Flags & FLAG_C;
	result = inReg1 + inReg2 + carry;
	set_three_flags (result);
	return (BYTE)result;
}
/*
Function: sub_regs
Description: Subracts one register away from another
Paramters: BYTE inReg1, BYTE inReg2 : These two paramaters represent the first and second register that will be used in the sum.
Returns: BYTE result : The result of the operation
Warnings: None
*/
BYTE sub_regs(BYTE inReg1, BYTE inReg2)
{
	WORD result = inReg1 - inReg2;
	set_three_flags (result);
	return (BYTE)result;
}
/*
Function: rotate_right_carry
Description: Checks if a carry is necessary then rotates the bits right whilst incorporating the carry and also sets flag_c, n and z
Paramters: BYTE inReg
Returns: BYTE result: The result of the operation
Warnings: None
*/
BYTE rotate_right_carry(BYTE inReg)
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
/*
Function: rotate_left_carry
Description: Checks if a carry is necessary then rotates the bits left whilst incorporating the carry and also sets flag_c, n and z
Paramters: BYTE inReg
Returns: BYTE result: The result of the operation
Warnings: None
*/
BYTE rotate_left_carry(BYTE inReg)
{
	BYTE result, carry;
	carry = Flags & FLAG_C;
	WORD data = (WORD)(inReg << 1) | carry;
	result = (BYTE)data;

	set_three_flags(data);
	return result;
}
/*
Function: rotate_left
Description: Rotates the bits left and also sets flag n and z to match the result
Paramters: BYTE inReg
Returns: BYTE result: The result of the operation
Warnings: None
*/
BYTE rotate_left(BYTE inReg)
{
	BYTE result;
	result = inReg << 1;
	if ((inReg & 0x80) == 0x80)
		result = result | 0x01;


	set_flag_z(result);
	set_flag_n(result);

	return result;
}
/*
Function: rotate_right
Description: Rotates the bits right and also sets flag n and z to match the result
Paramters: BYTE inReg
Returns: BYTE result: The result of the operation
Warnings: None
*/
BYTE rotate_right(BYTE inReg)
{
	BYTE result;
	result = inReg >> 1;
	if ((inReg & 0x01) == 0x01)
		result = result | 0x80;

	set_flag_z(result);
	set_flag_n(result);
	return result;
}
/*
Function: get_abs_ad
Description: Fetches the address for absolute addressing=
Paramters: None 
Returns: WORD address : The address to be used 
Warnings: None
*/
WORD get_abs_ad() {
	BYTE LB = 0;
	BYTE HB = 0;
	WORD address = 0;

	HB = fetch();
	LB = fetch();
	address += (WORD)((WORD)HB << 8) + LB;

	return address;
}
/*
Function: compare_accumulator
Description: Compares what is held in the/an accumulator to what is held in a register also calls set_three_flags with temp_word being used to set the flags. 
Paramters: BYTE inReg - The register which will be used in the comarpsion
Returns: None (void) 
Warnings: None
*/
void compare_accumulator(BYTE inReg) {
	BYTE param1 = 0;
	WORD data = 0;
	WORD temp_word;
	data = fetch();
	param1 = Registers[inReg];
	temp_word = (WORD)data - (WORD)param1;
	set_three_flags((WORD)temp_word);
}
/*
Function: push
Description: Pushes a registers contents onto the top of the stack 
Paramters: BYTE reg : the register in consideration
Returns: None (void)
Warnings: None
*/
void push(BYTE reg) {
	Memory[StackPointer] = reg;
	StackPointer--;
}

/*
Function: negate_mem_or_accumulator
Description: Depending on choices it carries out a variety of functions which work towards negating whatever is stored in the memory or an accumulator. It also calls set_three_flags passing data 
Paramters: BYTE inReg : This is the reg which will be used, WORD data, WORD address, BYTE choice: This variable controls which control flow is executed 
Returns: None (void)
Warnings: None
*/
void negate_mem_or_accumulator(BYTE inReg, WORD data, WORD address, BYTE choice) {
	if (choice == 0) {
		address += Index_Registers[inReg];
		address += get_abs_ad();
		data = ~Memory[address];

		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = (BYTE)data;

		}

	}
	else if (choice == 1)
	{
		address = get_abs_ad();
		data = ~Memory[address];

		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = (BYTE)data;

		}
	}

	else {
		data = ~Registers[inReg];
		if (address >= 0 && address < MEMORY_SIZE) {
			Registers[inReg] = (BYTE)data;

		}
	}
	set_three_flags(data);
}
/*
Function: compare
Description:Compares the contents of two registers, setting a carry if necessary, also calls set_flag_n & _Z passing temp_word
Paramters:BYTE reg1, BYTE reg2 : These are the registers in conisderation
Returns: None (void)
Warnings: None
*/
void compare(BYTE reg1, BYTE reg2)
{
	WORD temp_word = 0;
	BYTE param1 = 0;
	BYTE param2 = 0;

	param1 = Registers[reg1];
	param2 = Registers[reg2];
	temp_word = (WORD)param1 - (WORD)param2;

	if (temp_word >= 0x100) {
		Flags = Flags | FLAG_C; // Set carry flag
	}
	else {
		Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
	}
	set_flag_n((BYTE)temp_word);
	set_flag_z((BYTE)temp_word);

}
/*
Function: default_switch()
Description: The default case in the switches written here for resuability 
Paramters:
Returns: None (void)
Warnings: None
*/
void default_switch() {

	int destReg = 0;
	int sourceReg = 0;
	WORD address = 0;


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
}
/*
Function: source_as_reg_index
Description: Moves whatever is in the source register to the correct destination register
Paramters:int destReg, int sourceReg: These are the registers in consideration=
Returns: None (void)
Warnings: None
*/
void source_as_reg_index(int destReg, int sourceReg) {
	Registers[destReg] = Registers[sourceReg];
}
/*
Function: check_address
Description: Validates the address to ensure there are no overflows and the correct operation has been carried out on it
Paramters:WORD address, BYTE reg1, BYTE reg2 : These are the registers being used, BYTE HB, BYTE LB, BYTE option: option is used to specify which control flow to utilise
Returns: None (void)
Warnings: For reg2 if there is no appropriate Register in consideration simply pass it REGISTER_A as reg2 and no additonal operations will be carried out (REGISTER_B also works) 
*/
void check_address(WORD address, BYTE reg1, BYTE reg2, BYTE HB, BYTE LB, BYTE option)
{

	if ((option == 1) || (option == 3))
	{
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;

	}


	if (reg2 == 0) {
		address += Index_Registers[REGISTER_X];
	}
	else if (reg2 == 1) {
		address += Index_Registers[REGISTER_Y];
	}

	if ((option != 2) && (option != 3) && (option != 4) && (option != 5)) {
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Registers[reg1] = Memory[address];

		}
	}
	else if ((option == 2) || (option == 3)) {
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Registers[reg1];
		}

	}
	else if (option == 4) {
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] ++;
		}
	}
	else if (option == 5) {
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] --;
		}
	}
}
/*
Function: index_check_address
Description: Validates the address to ensure there are no overflows and the correct operation has been carried out on it. This is an extension of check address as if left into one function it became unwieldy. 
Paramters:WORD address, BYTE reg1, BYTE reg2 : These are the registers being used, BYTE HB, BYTE LB, BYTE option: option is used to specify which control flow to utilise
Returns: None (void)
Warnings: For reg2 if there is no appropriate Register in consideration simply pass it REGISTER_A as reg2 and no additonal operations will be carried out(REGISTER_B also works) 
*/
void index_check_address(WORD address, BYTE reg1, BYTE reg2, BYTE HB, BYTE LB, BYTE option)
{


	if ((option == 1) || (option == 4)) {
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
	}



	if (reg2 == 0) {
		address += Index_Registers[REGISTER_X];
	}
	else if (reg2 == 1) {
		address += Index_Registers[REGISTER_Y];
	}

	if ((option == 0) || (option == 1))
	{
		if (address >= 0 && address < MEMORY_SIZE) {
			Index_Registers[reg1] = Memory[address];
		}

	}
	else if ((option == 3) || (option == 4)) {
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Index_Registers[reg1];
		}
	}

}
/*
Function: load_stackpointer
Description: Loads the stackpointer with the relevant address and also ensures that the address being used is correct. 
Paramters:WORD address, BYTE reg1: This is the register being used, BYTE HB, BYTE LB, BYTE option: option is used to specify which control flow to utilise
Returns: None (void)
Warnings: For reg1 if there is no appropriate one pass in REGISTER A or REGISTER B 
*/

void load_stackpointer(WORD address, WORD data, BYTE reg1, BYTE HB, BYTE LB, BYTE option)
{


	if (option == 1) {

		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;
	}
	if (reg1 == 0) {
		address += Index_Registers[REGISTER_X];
	}
	else if (reg1 == 1) {
		address += Index_Registers[REGISTER_Y];
	}

	if ((option == 0) || (option == 1)) {
		if (address >= 0 && address < MEMORY_SIZE - 1) {
			StackPointer = (WORD)Memory[address] << 8;
			StackPointer += Memory[address + 1];
		}

	}



}

/*
Function: store_stackpointer
Description: Stores the Stackpointer in memory and also ensures the correct address is used
Paramters:WORD address, BYTE reg1 This is the register being used, BYTE HB, BYTE LB, BYTE option: option is used to specify which control flow to utilise
Returns: None (void)
Warnings: For reg1 if there is no appropriate one pass in REGISTER A or REGISTER B 
*/
void store_stackpointer(WORD address, WORD data, BYTE reg1, BYTE HB, BYTE LB, BYTE option) {

	if (option == 1)
	{
		HB = Memory[address];
		LB = Memory[address + 1];
		address = (WORD)((WORD)HB << 8) + LB;

	}

	if (reg1 == 0) {
		address += Index_Registers[REGISTER_X];
	}
	else if (reg1 == 1) {
		address += Index_Registers[REGISTER_Y];
	}


	if ((option == 0) || (option == 1)) {

		if (address >= 0 && address < MEMORY_SIZE - 1) {
			Memory[address] = (BYTE)(StackPointer >> 8);
			Memory[address + 1] = (BYTE)(StackPointer);
		}

	}

}
/*
Function: rotate_right_through
Description: Rotates the bits right through a carry if necessary or the accumulators. Also corrects address value if necessary.
Paramters:WORD address, BYTE reg1: This is the registers being used, BYTE HB, BYTE LB, BYTE option: option is used to specify which control flow to utilise
Returns: WORD data: returns this to allow the calling block to set the flags correctly. 
Warnings: For reg1 if there is no appropriate one pass in REGISTER A or REGISTER B 
*/
WORD rotate_right_through(WORD address, WORD data, BYTE reg1, BYTE carry, BYTE option)
{

	if (reg1 == 0)
	{

		address += Index_Registers[REGISTER_X];

	}

	//RRC LOGIC 
	if (reg1 == 1)
	{
		address += Index_Registers[REGISTER_Y];

	}

	if (option == 0) {
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
	}
	//RRC END 
	return data;

}
/*
Function: airthmetic_shift
Description: Carries out arithmetic shift left or right and rectifies address in Registers if necessary
Paramters:WORD address, BYTE reg1, BYTE reg2 : These are the registers being used, BYTE HB, BYTE LB, BYTE option: option is used to specify which control flow to utilise
Returns: WORD data : This allows the calling block to set the flags based on this data
Warnings: For reg2 if there is no appropriate Register in consideration simply pass it REGISTER_A as reg2 and no additonal operations will be carried out(REGISTER_B also works) 
*/
WORD arithmetic_shift(WORD address, WORD data, BYTE reg1, BYTE reg2, BYTE option)
{
	if (reg2 == 0) {
		address += Index_Registers[REGISTER_X];

	}
	else if (reg2 == 1) {

		address += Index_Registers[REGISTER_Y];
	}


	//shift_left
	if (option == 0) {
		if (address >= 0 && address < MEMORY_SIZE) {
			data = (WORD)Memory[address] << 1; //Shift

			Memory[address] = (BYTE)data;
		}


	} //shift left end  
	else if (option == 1) { 	//Shift right

		if (address >= 0 && address < MEMORY_SIZE) {
			if ((Memory[address] & 0x80) == 0x80) {
				if ((Memory[address] & 0x01) != 0) {
					data = (Memory[address] >> 1) | 0x100;
					data = (data | 0x80);
				}
				else {
					data = Memory[address] >> 1;
					data = (data | 0x80);
				}
			}
			else {
				if ((Memory[address] & 0x01) != 0) {
					data = (Memory[address] >> 1) | 0x100;
				}
				else {
					data = Memory[address] >> 1;
				}

			}
			Memory[address] = (BYTE)data;
		}
	}//shift right end
	else if (option == 3)
	{
		if (address >= 0 && address < MEMORY_SIZE) {
			if ((Registers[reg1] & 0x80) == 0x80) {
				if ((Registers[reg1] & 0x01) != 0) {
					data = (Registers[reg1] >> 1) | 0x100;
					data = (data | 0x80);
				}
				else {
					data = Registers[reg1] >> 1;
					data = (data | 0x80);
				}
			}
			else {
				if ((Registers[reg1] & 0x01) != 0) {
					data = (Registers[reg1] >> 1) | 0x100;
				}
				else {
					data = Registers[reg1] >> 1;
				}

			}
			Registers[reg1] = (BYTE)data;
		}

	}
	return data;

}
/*
Function: shift_right
Description: Carries out shift right and rectifies address in Registers if necessary
Paramters:WORD address, BYTE reg1, BYTE reg2 : These are the registers being used, BYTE HB, BYTE LB, BYTE option: option is used to specify which control flow to utilise
Returns: WORD data : This allows the calling block to set the flags based on this data
Warnings:For reg2 if there is no appropriate Register in consideration simply pass it REGISTER_A as reg2 and no additonal operations will be carried out(REGISTER_B also works) 
*/
WORD shift_right(WORD address, WORD data, BYTE reg1, BYTE reg2, BYTE option)
{
	if (reg2 == 0)
	{
		address += Index_Registers[REGISTER_X];
	}

	if (reg2 == 1)
	{
		address += Index_Registers[REGISTER_Y];

	}

	if (option == 0) {

		if (address >= 0 && address < MEMORY_SIZE) {
			if ((Memory[address] & 0x01) != 0) {
				data = (Memory[address] >> 1) | 0x100;
			}
			else {
				data = Memory[address] >> 1;
			}
			Memory[address] = (BYTE)data;
		}


	}

	if (option == 1)
	{
		if (address >= 0 && address < MEMORY_SIZE) {
			if ((Registers[reg1] & 0x01) != 0) {
				data = (Registers[reg1] >> 1) | 0x100;
			}
			else {
				data = Registers[reg1] >> 1;
			}
			Registers[reg1] = (BYTE)data;
		}



	}
	return data;
}
/*
Function: sub_to_accumulator_carry
Description: Carries out to an accumulator also checks if a carry is necessary
Paramters:BYTE reg1 : The register(Accumulator) in consideration
Returns:WORD temp_word: This is passed back to the calling function which it is used for setting the flags
Warnings: None
*/
WORD sub_to_accumulator_carry(BYTE reg1)
{
	WORD data = 0, temp_word;
	BYTE param1;

	data = fetch();
	param1 = Registers[reg1];
	temp_word = (WORD)data - (WORD)param1;

	if ((Flags & FLAG_C) != 0) {
		temp_word--;
	}

	return temp_word;

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

		////////////////
		//    LDAA     //
		////////////////
		/*
		Loads Memory into
		Accumulator
		*/

	case 0x0A: //IMMEDIATE ADDRESSING (#)
		//Fetch the address from memory 
		data = fetch();
		//Store the address in the stated Register
		Registers[REGISTER_A] = data;
		break;

	case 0x1A: ///ABSOLUTE ADDRESSING(abs)
		//Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_A, REGISTER_A, HB, LB, 0);
		break;

	case 0x2A: //INDEXED ABSOLUTE ADDRESSING(abs,X)
		//Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_A, REGISTER_X, HB, LB, 0);
		break;

	case 0x3A: //INDEXED ABSOLUTE ADDRESSING(abs,Y)
	  //Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_A, REGISTER_Y, HB, LB, 0);

		break;

	case 0x4A:  //INDIRECT ADDRESSING(ind)
		//Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_A, REGISTER_A, HB, LB, 1);
		break;
		
	case 0x5A: //INDEXED INDIRECT ADDRESSING((ind,X))
		//Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_A, REGISTER_X, HB, LB, 1);
		break;

		////////////////
		//    LDAB     //
		////////////////
		/*
		Loads Memory into
		Accumulator
		*/

	case 0x0B: //IMMEDIATE ADDRESSING (#)
		//fetch address from memory
		data = fetch();
		//Store the address in the stated Register
		Registers[REGISTER_B] = data;
		break;

	case 0x1B: //ABSOLUTE ADDRESSING(abs)
	    //Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_B, REGISTER_B, HB, LB, 0);
		break;

	case 0x2B: //INDEXED ABSOLUTE ADDRESSING(abs,X)
	     //Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_B, REGISTER_X, HB, LB, 0);
		break;

	case 0x3B: //INDEXED ABSOLUTE ADDRESSING(abs,Y)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_B, REGISTER_Y, HB, LB, 0);

		break;

	case 0x4B: //INDIRECT ADDRESSING(ind)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_B, REGISTER_B, HB, LB, 1);
		break;

	case 0x5B: //INDEXED INDIRECT ADDRESSING((ind,X))
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_B, REGISTER_X, HB, LB, 1);
		break;

		////////////////
		//   STORA    //
		////////////////
		/*
		Stores Accumulator
		into Memory
		*/

	case 0xBA: //ABSOLUTE ADDRESSING(abs)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_A, REGISTER_A, HB, LB, 2);
		break;

	case 0xCA: //INDEXED ABSOLUTE ADDRESSING(abs,X)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_A, REGISTER_X, HB, LB, 2);
		break;

	case 0xDA: //INDEXED ABSOLUTE ADDRESSING(abs,Y)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_A, REGISTER_Y, HB, LB, 2);
		break;

	case 0xEA: //INDIRECT ADDRESSING(ind)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_A, REGISTER_A, HB, LB, 3);
		break;

	case 0xFA: //INDEXED INDIRECT ADDRESSING((ind,X))
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_A, REGISTER_X, HB, LB, 3);
		break;

		////////////////
		//    MVI     //
		////////////////
		/*
		Loads Memory into
		register
		*/
			
	case 0x1C: //L,#
		//Fetch address from memory
		data = fetch();
		//Store address in stated Register
		Registers[REGISTER_L] = data;
		break;

	case 0x1D: //H,#
		//Fetch address from Memory
		data = fetch();
		//Store address in state Register
		Registers[REGISTER_H] = data;
		break;

		////////////////
		//     LX     //
		////////////////
		/*
		Loads Memory into
		register pair
		*/

	case 0x0D: //LH,#
		//Fetch address from memory and store it 
		Registers[REGISTER_H] = fetch();
		Registers[REGISTER_L] = fetch();
		break;

	case 0x0C: //LH,#
		//Fetch address from memory and store it
		Registers[REGISTER_H] = fetch();
		Registers[REGISTER_L] = fetch();
		break;

		////////////////
		//     CSA     //
		////////////////
		/*
		Transfers Status register(Flags) to Accumulator (Reg A)
		*/

	case 0xF2: //IMPLIED ADDRESSING(impl)
		//Move the status register into the accumulator
		Registers[REGISTER_A] = Flags;
		break;

		////////////////
		//    STORB    //
		////////////////
		/*
		Stores Accumulator
		into Memory
		*/

	case 0xBB: //ABSOLUTE ADDRESSING(abs)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_B, REGISTER_B, HB, LB, 2);
		break;

	case 0xCB: //INDEXED ABSOLUTE ADDRESSING(abs,X)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_B, REGISTER_X, HB, LB, 2);
		break;

	case 0xDB: //INDEXED ABSOLUTE ADDRESSING(abs,Y)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_B, REGISTER_Y, HB, LB, 2);
		break;

	case 0xEB: //INDIRECT ADDRESSING(ind)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_B, REGISTER_B, HB, LB, 3);
		break;

	case 0xFB: //INDEXED INDIRECT ADDRESSING((ind,X))
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_B, REGISTER_X, HB, LB, 3);
		break;

		////////////////
		//    LDX     //
		////////////////
		/*
		Loads Memory into
		register X
		*/

	case 0x0E: //IMMEDIATE ADDRESSING (#)
		//Fetch an address
		data = fetch();
		//Store address inside the stated index_register
		Index_Registers[REGISTER_X] = data;
		break;

	case 0x1E: //ABSOLUTE ADDRESSING(abs)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_X, REGISTER_A, HB, LB, 0);
		break;

	case 0x2E: //INDEXED ABSOLUTE ADDRESSING(abs,X)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_X, REGISTER_X, HB, LB, 0);
		break;

	case 0x3E: //INDEXED ABSOLUTE ADDRESSING(abs,Y)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_X, REGISTER_Y, HB, LB, 0);
		break;

	case 0x4E: //INDIRECT ADDRESSING(ind)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_X, REGISTER_A, HB, LB, 1);
		break;

	case 0x5E: //INDEXED INDIRECT ADDRESSING((ind,X))
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_X, REGISTER_X, HB, LB, 1);

		break;

		////////////////
		//     STOX    //
		////////////////
		/*
		Stores register X into
		Memory
		*/

	case 0xBC: //ABSOLUTE ADDRESSING(abs)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_X, REGISTER_A, HB, LB, 3);
		break;

	case 0xCC: //INDEXED ABSOLUTE ADDRESSING(abs,X)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_X, REGISTER_X, HB, LB, 3);
		break;

	case 0xDC: //INDEXED ABSOLUTE ADDRESSING(abs,Y)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_X, REGISTER_Y, HB, LB, 3);
		break;

	case 0xEC: //INDIRECT ADDRESSING(ind)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_X, REGISTER_A, HB, LB, 4);
		break;

	case 0xFC: //INDEXED INDIRECT ADDRESSING((ind,X))
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_X, REGISTER_X, HB, LB, 4);
		break;

		////////////////
		//     LDY     //
		////////////////
		/*
		Loads Memory into
		register Y
		*/

	case 0x0F: //IMMEDIATE ADDRESSING (#)
		//Grab the address from memory 
		data = fetch();
		//Move the address into index_register
		Index_Registers[REGISTER_Y] = data;
		break;

	case 0x1F: //ABSOLUTE ADDRESSING(abs)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_Y, REGISTER_A, HB, LB, 0);
		break;

	case 0x2F: //INDEXED ABSOLUTE ADDRESSING(abs,X)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_Y, REGISTER_X, HB, LB, 0);
		break;

	case 0x3F: //INDEXED ABSOLUTE ADDRESSING(abs,Y)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_Y, REGISTER_Y, HB, LB, 0);

		break;

	case 0x4F: //INDIRECT ADDRESSING(ind)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_Y, REGISTER_A, HB, LB, 1);
		break;

	case 0x5F: //INDEXED INDIRECT ADDRESSING((ind,X))
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_Y, REGISTER_X, HB, LB, 1);
		break;

		////////////////
		//    STOY     //
		////////////////
		/*
		Stores register Y into
		Memory
		*/

	case 0xBD: //ABSOLUTE ADDRESSING(abs)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_Y, REGISTER_A, HB, LB, 3);
		break;

	case 0xCD: //INDEXED ABSOLUTE ADDRESSING(abs,X)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_Y, REGISTER_X, HB, LB, 3);
		break;

	case 0xDD: //INDEXED ABSOLUTE ADDRESSING(abs,y)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_Y, REGISTER_Y, HB, LB, 3);
		break;

	case 0xED: //INDIRECT ADDRESSING(ind)
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_Y, REGISTER_A, HB, LB, 4);
		break;

	case 0xFD: //INDEXED INDIRECT ADDRESSING((ind,X))
			   //Get valid address
		address += get_abs_ad();
		//Validate address
		index_check_address(address, REGISTER_Y, REGISTER_X, HB, LB, 4);
		break;

		////////////////
		//     CAY     //
		////////////////
		/*
		Transfer Accumulator(REG A) to Register Y
		*/

	case 0xF0:  //IMPLIED ADDRESSING(impl)
		//Move what is currently stored in the Accumulator(Register A) into Register Y
		Index_Registers[REGISTER_Y] = Registers[REGISTER_A];
		//Set the negative flag based on what is inside the register
		set_flag_n(Index_Registers[REGISTER_Y]); 
		break;

		////////////////
		//     MYA     //
		////////////////
		/*
		Transfers register Y to Accumulater(Reg A)
		*/

	case 0xF1: //IMPLIED ADDRESSING(impl)
		//Transfer what is stored in Register Y into the Accumulator (Register_A)
		Registers[REGISTER_A] = Index_Registers[REGISTER_Y];
		break;

		////////////////
		//     ABA     //
		//////////////// 
		/*
		Adds Accumulator B into Accumlator A
		Refer to PDF to refresh on Flags if need be
		*/

	case 0xF3: //IMPLIED ADDRESSING(impl)
		//Create a store for both of the Registers in consideration to use
		data = Registers[REGISTER_A] + Registers[REGISTER_B];
		//Once added set Register A to be the sum of them 
		Registers[REGISTER_A] = (BYTE)data;
		//Set the flags (z,n,c) based on the result of the operation 
		set_three_flags(data);
		break;

		////////////////
		//     SBA     //
		////////////////
		/*
		Subtracts Accumulator B from Accumulator A
		*/

	case 0xF4: //IMPLIED ADDRESSING(impl)
		//Create a store for both of the registers in consideration to use 
		data = Registers[REGISTER_A] - Registers[REGISTER_B];
		//Set Register A to equal the result of the operation
		Registers[REGISTER_A] = (BYTE)data;
		///set the flags based on the sum(z,n,c)
		set_three_flags(data);
		break;

		////////////////
		//     AAB     //
		////////////////
		/*
		Adds Accumulator A into Accumulator B
		*/

	case 0xF5: //IMPLIED ADDRESSING(impl)
			 //Create a store forboth of the registers in consideration to use
		data = Registers[REGISTER_B] + Registers[REGISTER_A]; 
		//Once added set Register B to be the sum of them 
		Registers[REGISTER_B] = (BYTE)data;
		//Set the flags (z,n,c) based on the result of the operation 
		set_three_flags(data);	
		break;

		//SAB
		/*
		Subtracts Accumulator A from Accumulator B
		*/

	case 0xF6: //IMPLIED ADDRESSING(impl)
			   //Create a store for both of the registers in consideration to use 
		data = Registers[REGISTER_B] - Registers[REGISTER_A];
		//Set Register B to equal the result of the operation
		Registers[REGISTER_B] = (BYTE)data;
		//Set the flags (z,n,c) based on the result of the operation 
		set_three_flags(data);
		break;

		////////////////
		//    LODS    //
		////////////////
		/*
		Loads Memory into
		Stackpointer
		*/

	case 0x20: //IMMEDIATE ADDRESSING (#)
		//Fetch the address from memory
		data = fetch();
		//Set the stackpointer to be the result of the fetch and shift it then fetch the next address
		StackPointer = data << 8; StackPointer += fetch();
		break;

	case 0x30: //ABSOLUTE ADDRESSING(abs)
		//Get a valid adress
		address += get_abs_ad();
		//Load stackpointer with address
		load_stackpointer(address, data, REGISTER_A, HB, LB, 0);
		break;

	case 0x40: //INDEXED ABSOLUTE ADDRESSING(abs,X)
			   //Get a valid adress
		address += get_abs_ad();
		//Load stackpointer with address
		load_stackpointer(address, data, REGISTER_X, HB, LB, 0);
		break;

	case 0x50: //INDEXED ABSOLUTE ADDRESSING(abs,Y)
			   //Get a valid adress
		address += get_abs_ad();
		//Load stackpointer with address
		load_stackpointer(address, data, REGISTER_Y, HB, LB, 0);
		break;

	case 0x60: //INDIRECT ADDRESSING(ind)
			   //Get a valid adress
		address += get_abs_ad();
		//Load stackpointer with address
		load_stackpointer(address, data, REGISTER_A, HB, LB, 1);
		break;

	case 0x70: ///INDEXED INDIRECT ADDRESSING((ind,X))
			   //Get a valid adress
		address += get_abs_ad();
		//Load stackpointer with address
		load_stackpointer(address, data, REGISTER_X, HB, LB, 1);
		break;

		//////////////////
		//     STOS     //
		//////////////////
		/*
		Stores Stackpointer
		into Memory
		*/

	case 0x6A: //ABSOLUTE ADDRESSING(abs)
			   //Get a valid adress
		address += get_abs_ad();
		//Store the stackpointer in memory
		store_stackpointer(address, data, REGISTER_A, HB, LB, 0);
		break;

	case 0x7A: //INDEXED ABSOLUTE ADDRESSING(abs,X)
			   //Get a valid adress
		address += get_abs_ad();
		//Store the stackpointer in memory
		store_stackpointer(address, data, REGISTER_X, HB, LB, 0);
		break;

	case 0x8A: //INDEXED ABSOLUTE ADDRESSING(abs,y)
			   //Get a valid adress
		address += get_abs_ad();
		//Store the stackpointer in memory
		store_stackpointer(address, data, REGISTER_Y, HB, LB, 0);
		break;

	case 0x9A: //INDIRECT ADDRESSING(ind)
			   //Get a valid adress
		address += get_abs_ad();
		//Store the stackpointer in memory
		store_stackpointer(address, data, REGISTER_A, HB, LB, 1);
		break;

	case 0xAA: //INDEXED INDIRECT ADDRESSING((ind,X))
			   //Get a valid adress
		address += get_abs_ad();
		//Store the stackpointer in memory
		store_stackpointer(address, data, REGISTER_X, HB, LB, 1);
		break;

		////////////////
		//     ADC     //
		////////////////
		/*
		Register added to
		Accumulator with
		Carry
		*/

	case 0x31: //A-L   
		//Call function which adds the stated registers together then returns the answer to the destination Register
		Registers[REGISTER_A] = add_with_carry(Registers[REGISTER_A], Registers[REGISTER_L]);
		break;

	case 0x41: // A-H
			   //Call function which adds the stated registers together then returns the answer to the destination Register
		Registers[REGISTER_A] = add_with_carry(Registers[REGISTER_A], Registers[REGISTER_H]);
		break;
	case 0x51: //A-M
			   //Call function which adds the stated registers together then returns the answer to the destination Register
		Registers[REGISTER_A] = add_with_carry(Registers[REGISTER_A], Registers[REGISTER_M]);
		break;

	case 0x61: //B-L 
			   //Call function which adds the stated registers together then returns the answer to the destination Register
		Registers[REGISTER_B] = add_with_carry(Registers[REGISTER_B], Registers[REGISTER_L]);
		break;

	case 0x71: //B-H
			   //Call function which adds the stated registers together then returns the answer to the destination Register
		Registers[REGISTER_B] = add_with_carry(Registers[REGISTER_B], Registers[REGISTER_H]);
		break;

	case 0x81: // B-M
			   //Call function which adds the stated registers together then returns the answer to the destination Register
		Registers[REGISTER_B] = add_with_carry(Registers[REGISTER_B], Registers[REGISTER_M]);
		break;

		////////////////
		//     CMP     //
		////////////////
		/*
		Register compared to Accumulator
		*/

	case 0x35: // A-L
		//Call function which compares two registers together
		compare(REGISTER_A, REGISTER_L);
		break;


	case 0x45: //A-H
			   //Call function which compares two registers together
		compare(REGISTER_A, REGISTER_H);
		break;

	case 0x55: //A-M
			   //Call function which compares two registers together
		compare(REGISTER_A, REGISTER_M);
		break;

	case 0x65: //B-L
			   //Call function which compares two registers together
		compare(REGISTER_B, REGISTER_L);
		break;


	case 0x75: //B-H
			   //Call function which compares two registers together
		compare(REGISTER_B, REGISTER_H);
		break;

	case 0x85: //B-M
			   //Call function which compares two registers together
		compare(REGISTER_B, REGISTER_M);
		break;

		////////////////
		//     ADD     //
		////////////////
		/*
		Register is added to accumulator 
		*/

	case 0x33: // A- L
			   //Call function which adds the stated registers together then returns the answer to the destination Register
		Registers[REGISTER_A] = add_regs(Registers[REGISTER_A], Registers[REGISTER_L]);
		break;

	case 0x43:	//A - H 
				//Call function which adds the stated registers together then returns the answer to the destination Register
		Registers[REGISTER_A] = add_regs(Registers[REGISTER_A], Registers[REGISTER_H]);
		break;

	case 0x53:  //A-M
				//Call function which adds the stated registers together then returns the answer to the destination Register
		Registers[REGISTER_A] = add_regs(Registers[REGISTER_A], Registers[REGISTER_M]);
		break;

	case 0x63: //B-L 
			   //Call function which adds the stated registers together then returns the answer to the destination Register
		Registers[REGISTER_B] = add_regs(Registers[REGISTER_B], Registers[REGISTER_L]);
		break;

	case 0x73:  //B-H
				//Call function which adds the stated registers together then returns the answer to the destination Register
		Registers[REGISTER_B] = add_regs(Registers[REGISTER_B], Registers[REGISTER_H]);
		break;

	case 0x83:   //B-M
				 //Call function which adds the stated registers together then returns the answer to the destination Register
		Registers[REGISTER_B] = add_regs(Registers[REGISTER_B], Registers[REGISTER_M]);
		break;

		////////////////
		//     CLC     //
		////////////////
		/*
		Clears Carry flag
		*/

	case 0x05: //IMPLIED ADDRESSING(impl)
		//Clears a carry 
		Flags = (Flags & (~FLAG_C));
		break;

		////////////////
		//     STC     //
		////////////////
		/*
		Set Carry flag
		*/
	case 0x06: //IMPLIED ADDRESSING(impl)
			   //Sets a carry
		Flags = Flags | FLAG_C;
		break;

		////////////////
		//     CLI     //
		////////////////
		/*
		Clear Interrupt flag
		*/
		
	case 0x07: //IMPLIED ADDRESSING(impl)
		//Clears an interrupt 
		Flags = (Flags & (~FLAG_I));
		break;

		////////////////
		//     STI     //
		////////////////
		/*
		Set Interupt flag
		*/

	case 0x08: //IMPLIED ADDRESSING(impl)
		//Sets an interrupt 
		Flags = Flags | FLAG_I;
		break;

		////////////////
		//     SBC     //
		////////////////
		/*
		Register subtracted to Accumulator with Carry
		*/

	case 0x32: //A-L   
		//Calls a function which subtracts the stated registers away from each other and returns the result to the destination register
		Registers[REGISTER_A] = sub_with_carry(Registers[REGISTER_A], Registers[REGISTER_L]);
		break;

	case 0x42: //A-H
			   //Calls a function which subtracts the stated registers away from each other and returns the result to the destination register
		Registers[REGISTER_A] = sub_with_carry(Registers[REGISTER_A], Registers[REGISTER_H]);
		break;

	case 0x52: //A-M 
			   //Calls a function which subtracts the stated registers away from each other and returns the result to the destination register
		Registers[REGISTER_A] = sub_with_carry(Registers[REGISTER_A], Registers[REGISTER_M]);
		break;

	case 0x62: //B-L
			   //Calls a function which subtracts the stated registers away from each other and returns the result to the destination register
		Registers[REGISTER_B] = sub_with_carry(Registers[REGISTER_B], Registers[REGISTER_L]);
		break;

	case 0x72: // B-H
			   //Calls a function which subtracts the stated registers away from each other and returns the result to the destination register
		Registers[REGISTER_B] = sub_with_carry(Registers[REGISTER_B], Registers[REGISTER_H]);
		break;

	case 0x82: //B-M
			   //Calls a function which subtracts the stated registers away from each other and returns the result to the destination register
		Registers[REGISTER_B] = sub_with_carry(Registers[REGISTER_B], Registers[REGISTER_M]);
		break;

		////////////////
		//     SUB     //
		////////////////
		/*
		Register Subtracted to Accumulator
		*/

	case 0x34:  // A-L  
				//Calls a function which subtracts the stated registers away from each other and returns the result to the destination register
		Registers[REGISTER_A] = sub_regs(Registers[REGISTER_A], Registers[REGISTER_L]);
		break;

	case 0x44:   //A-H
				 //Calls a function which subtracts the stated registers away from each other and returns the result to the destination register
		Registers[REGISTER_A] = sub_regs(Registers[REGISTER_A], Registers[REGISTER_H]);
		break;

	case 0x54: //A-M
			   //Calls a function which subtracts the stated registers away from each other and returns the result to the destination register
		Registers[REGISTER_A] = sub_regs(Registers[REGISTER_A], Registers[REGISTER_M]);
		break;

	case 0x64:	//B-L
				//Calls a function which subtracts the stated registers away from each other and returns the result to the destination register
		Registers[REGISTER_B] = sub_regs(Registers[REGISTER_B], Registers[REGISTER_L]);
		break;

	case 0x74: //B-H
			   //Calls a function which subtracts the stated registers away from each other and returns the result to the destination register
		Registers[REGISTER_B] = sub_regs(Registers[REGISTER_B], Registers[REGISTER_H]);
		break;

	case 0x84:  //B-M 
				//Calls a function which subtracts the stated registers away from each other and returns the result to the destination register
		Registers[REGISTER_B] = sub_regs(Registers[REGISTER_B], Registers[REGISTER_M]);
		break;

		////////////////
		//     OR     //
		////////////////
		/*
		Register bitewise inclusive OR with Accumulator
		*/

	case 0x36: //A-L 
		//Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_L];
		//Create a store for the paramaters once they have been inclusively or'd
		temp_word = (WORD)param1 | (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		//Move the result of the operation into the destination register
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x46: // A-H
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_H];
		//Create a store for the paramaters once they have been inclusively or'd
		temp_word = (WORD)param1 | (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		//Move the result of the operation into the destination register
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x56: //A-M
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_M];
		//Create a store for the paramaters once they have been inclusively or'd
		temp_word = (WORD)param1 | (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		//Move the result of the operation into the destination register
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x66: //B-L
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_L];
		//Create a store for the paramaters once they have been inclusively or'd
		temp_word = (WORD)param1 | (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		//Move the result of the operation into the destination register
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0x76: //B-H
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_H];
		//Create a store for the paramaters once they have been inclusively or'd
		temp_word = (WORD)param1 | (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		//Move the result of the operation into the destination register
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0x86: //B-M 
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_M];
		//Create a store for the paramaters once they have been inclusively or'd
		temp_word = (WORD)param1 | (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		//Move the result of the operation into the destination register
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

		////////////////
		//     AND     //
		////////////////
		/*
		Register bitwise and with Accumulator
		*/

	case 0x37: //A-L
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_L];
		//Create a store for the paramaters once they have been and'd 
		temp_word = (WORD)param1 & (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		//Move the result of the operation into the destination register
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x47: //A-H
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_H];
		//Create a store for the paramaters once they have been and'd
		temp_word = (WORD)param1 & (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		//Move the result of the operation into the destination register
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x57: // A-M
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_M];
		//Create a store for the paramaters once they have been and'd 
		temp_word = (WORD)param1 & (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		//Move the result of the operation into the destination register
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x67://B - L 
			  //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_L];
		//Create a store for the paramaters once they have been and'd 
		temp_word = (WORD)param1 & (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		//Move the result of the operation into the destination register
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0x77: //B-H
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_H];
		//Create a store for the paramaters once they have been and'd 
		temp_word = (WORD)param1 & (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		//Move the result of the operation into the destination register
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0x87: //B-M
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_M];
		//Create a store for the paramaters once they have been and'd 
		temp_word = (WORD)param1 & (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		//Move the result of the operation into the destination register
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

		////////////////
		//     XOR    //
		////////////////
		/*
		Register Bitwise exclusive or with Accumulator
		*/

	case 0x38: // A-L
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_L];
		//Create a store for the paramters once they have been exclusively or'd
		temp_word = (WORD)param1 ^ (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		//Move the result of the operation into the destination register
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x48: //A-H
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_H];
		//Create a store for the paramters once they have been exclusively or'd
		temp_word = (WORD)param1 ^ (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		//Move the result of the operation into the destination register
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x58: //A-M
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_M];
		//Create a store for the paramters once they have been exclusively or'd
		temp_word = (WORD)param1 ^ (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		//Move the result of the operation into the destination register
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x68: //B-L
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_L];
		//Create a store for the paramters once they have been exclusively or'd
		temp_word = (WORD)param1 ^ (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		//Move the result of the operation into the destination register
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0x78: //B-H
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_H];
		//Create a store for the paramters once they have been exclusively or'd
		temp_word = (WORD)param1 ^ (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		//Move the result of the operation into the destination register
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0x88: //B-M
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_M];
		//Create a store for the paramters once they have been exclusively or'd
		temp_word = (WORD)param1 ^ (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		//Move the result of the operation into the destination register
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

		////////////////
		//     BIT    //
		////////////////
		/*
		Register Bit tested with Accumulator
		*/

	case 0x39: //A-L
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_L];
		//Create a store for the paramters once they have been and'd
		temp_word = (WORD)param1 & (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		break;

	case 0x49: //A-H
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_H];
		//Create a store for the paramters once they have been and'd
		temp_word = (WORD)param1 & (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		break;

	case 0x59: //A-M
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_M];
		//Create a store for the paramters once they have been and'd
		temp_word = (WORD)param1 & (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		break;

	case 0x69://B-L
			  //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_L];
		//Create a store for the paramters once they have been and'd
		temp_word = (WORD)param1 & (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		break;

	case 0x79: //B-H
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_H];
		//Create a store for the paramters once they have been and'd
		temp_word = (WORD)param1 & (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		break;

	case 0x89: //B-M
			   //Loads the paramater variables which the register contents
		param1 = Registers[REGISTER_B];
		param2 = Registers[REGISTER_M];
		//Create a store for the paramters once they have been and'd
		temp_word = (WORD)param1 & (WORD)param2;
		//Set flag z and n based on the result
		set_flag_z((WORD)temp_word);
		set_flag_n((WORD)temp_word);
		break;

		////////////////
		//     ORIA    //
		//////////////// 
		/*
		Data bitwise inclusive or with Accumlator
		*/

	case 0x97: //IMMEDIATE ADDRESSING (#)
		//Fetch the address 
		data = fetch();
		//Set the register to be the result of a bitwise or
		Registers[REGISTER_A] |= data;
		//Set flag z and n based on the result
		set_flag_z(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);
		break;

		////////////////
		//     ORIB    //
		////////////////
		/*
		Data bitwise inclusive or with Accumulator
		*/

	case 0x98: //IMMEDIATE ADDRESSING (#)
			   //Fetch the address 
		data = fetch();
		//Set the register to be the result of a bitwise or
		Registers[REGISTER_B] |= data;
		//Set flag z and n based on the result
		set_flag_z(Registers[REGISTER_B]);
		set_flag_n(Registers[REGISTER_B]);
		break;

		////////////////
		//     INC     //
		////////////////
		/*
		Increment memory or Accumulator
		*/

	case 0xA0: //ABSOLUTE ADDRESSING(abs)
		//Get a valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_A, REGISTER_A, HB, LB, 4);
		//Set flag z and n based on the result
		set_flag_z((WORD)Memory[address]);
		set_flag_n((WORD)Memory[address]);
		break;
	case 0xB0: //INDEXED ABSOLUTE ADDRESSING(abs,X)
			   //Get a valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_A, REGISTER_X, HB, LB, 4);
		//Set flag z and n based on the result
		set_flag_z((WORD)Memory[address]);
		set_flag_n((WORD)Memory[address]);
		break;

	case 0xC0:  //INDEXED ABSOLUTE ADDRESSING(abs,y)
				//Get a valid address
		address += get_abs_ad();
		//Validate address
		check_address(address, REGISTER_A, REGISTER_Y, HB, LB, 4);
		//Set flag z and n based on the result
		set_flag_z((WORD)Memory[address]);
		set_flag_n((WORD)Memory[address]);
		break;

		////////////////
		//     INCA     //
		////////////////
		/*
		Increment Memory or Accumulator
		*/

	case 0xD0: //IMPLIED ADDRESSING(impl)
		//Increment the address in the register
		Registers[REGISTER_A]++;
		//Set the flag z and n based on this
		set_flag_z((WORD)Registers[REGISTER_A]);
		set_flag_n((WORD)Registers[REGISTER_A]);
		break;

		////////////////
		//     INCB     //
		////////////////
		/*
		Increment Memory or Accumulator
		*/

	case 0xE0: //IMPLIED ADDRESSING(impl)
			   //Increment the address in the register
		Registers[REGISTER_B]++;
		//Set the flag z and n based on this
		set_flag_z((WORD)Registers[REGISTER_B]);
		set_flag_n((WORD)Registers[REGISTER_B]);
		break;

		////////////////
		//     INCX     //
		//////////////// 
		/*
		Increments Register X
		*/

	case 0x02:  //IMPLIED ADDRESSING(impl) 
		//Increment what is stored in the register
		Index_Registers[REGISTER_X]++;
		//Set the flag z based on this
		set_flag_z((WORD)Index_Registers[REGISTER_X]);
		break;

		////////////////
		//     INCY     //
		////////////////
		/*
		Increments Register Y
		*/

	case 0x04:  //IMPLIED ADDRESSING(impl) 
				//Increment what is stored in the register
		Index_Registers[REGISTER_Y]++;
		//Set the flag z based on this
		set_flag_z((WORD)Index_Registers[REGISTER_Y]);
		break;

		////////////////
		//     HLT     //
		////////////////
		/*
		Wait for interupt
		*/

	case 0x2D: //IMPLIED ADDRESSING(impl)
		// Set the halt variable to true 
		halt = true;
		break;

		////////////////
		//     DEC     //
		////////////////
		/*
		Decrement Memory or
		Accumulator
		*/

	case 0xA1: //ABSOLUTE ADDRESSING(abs)
		//Get the address
		address += get_abs_ad();
		//Verify the address
		check_address(address, REGISTER_A, REGISTER_A, HB, LB, 5);
		//Set the flags z and n based on the verification of the address
		set_flag_z((WORD)Memory[address]);
		set_flag_n((WORD)Memory[address]);
		break;

	case 0xB1://INDEXED ABSOLUTE ADDRESSING(abs,x)
			  //Get the address
		address += get_abs_ad();
		//Verify the address
		check_address(address, REGISTER_A, REGISTER_X, HB, LB, 5);
		//Set the flags z and n based on the verification of the address
		set_flag_z((WORD)Memory[address]);
		set_flag_n((WORD)Memory[address]);
		break;

	case 0xC1: //INDEXED ABSOLUTE ADDRESSING(abs,y)
			   //Get the address
		address += get_abs_ad();
		//Verify the address
		check_address(address, REGISTER_A, REGISTER_Y, HB, LB, 5);
		//Set the flags z and n based on the verification of the address
		set_flag_z((WORD)Memory[address]);
		set_flag_n((WORD)Memory[address]);
		break;

		////////////////
		//    DECA     //
		////////////////
		/*
		Decrement Memory or Accumulator
		*/

	case 0xD1: //IMPLIED ADDRESSING(impl)
		//Decrement the register
		Registers[REGISTER_A]--;
		//Set the flags z and n based on the Register
		set_flag_z((WORD)Registers[REGISTER_A]);
		set_flag_n((WORD)Registers[REGISTER_A]);
		break;

		////////////////
		//    DECB     //
		////////////////
		/*
		Increment Memory or Accumulator
		*/

	case 0xE1: //IMPLIED ADDRESSING(impl)
			   //Decrement the register
		Registers[REGISTER_B]--;
		//Set the flags z and n based on the Register
		set_flag_z((WORD)Registers[REGISTER_B]);
		set_flag_n((WORD)Registers[REGISTER_B]);
		break;

		////////////////
		//     DECX     //
		////////////////
		/*
		Decrements Register X
		*/

	case 0x01: //IMPLIED ADDRESSING(impl)
			   //Decrement the register
		Index_Registers[REGISTER_X]--;
		//Set the flag z based on the register
		set_flag_z((WORD)Index_Registers[REGISTER_X]);
		break;

		////////////////
		//     DEY     //
		////////////////
		/*
		Deccrements Register Y
		*/

	case 0x03: //IMPLIED ADDRESSING(impl)
			   //Decrement the register
		Index_Registers[REGISTER_Y]--;
		//Set the flag z based on the register
		set_flag_z((WORD)Index_Registers[REGISTER_Y]);
		break;

		////////////////
		//     NOP     //
		////////////////
		/*
		No Operating
		*/
		
	case 0x2C: //IMPLIED ADDRESSING(impl)
		//Set the halt variable to be true
		halt = true;
		break;

		////////////////
		//    RRC     //
		////////////////
		/*
		Rotate right through carry Memory or Accumulator
		*/

	case 0xA2:  //ABSOLUTE ADDRESSING(abs)
		//Get address
		address += get_abs_ad();
		//Set data to be the result of the rotate function
		data = rotate_right_through(address, data, REGISTER_A, carry, 0);
		//Set flags based on the return of the rotate function (z,n,c)
		set_three_flags(data);
		break;

	case 0xB2: //INDEXED ABSOLUTE ADDRESSING(abs,x)
			   //Get address
		address += get_abs_ad();
		//Set data to be the result of the rotate function
		data = rotate_right_through(address, data, REGISTER_X, carry, 0);
		//Set flags based on the return of the rotate function(z,n,c)
		set_three_flags(data);
		break;

	case 0xC2://INDEXED ABSOLUTE ADDRESSING(abs,y)
			  //Get address
		address += get_abs_ad();
		//Set data to be the result of the rotate function
		data = rotate_right_through(address, data, REGISTER_Y, carry, 0);
		//Set flags based on the return of the rotate function(z,n,c)
		set_three_flags(data);
		break;

		////////////////
		//     RRCA     //
		////////////////
		/*
		Rotate right through carry memory or accumulator
		*/

	case 0xD2: //A
		//Set Register to be the result of the rotate function
		Registers[REGISTER_A] = rotate_right_carry(Registers[REGISTER_A]);

		break;

		////////////////
		//     RCBB     //
		////////////////
		/*
		Rotate right through carry memory or accumulator
		*/

	case 0xE2: //B 
			   //Set Register to be the result of the rotate function
		Registers[REGISTER_B] = rotate_right_carry(Registers[REGISTER_B]);
		break;

		////////////////
		//     RLC     //
		////////////////
		/*
		Rotate left through carry Memory or Accumulator
		*/

	case 0xA3:  //ABSOLUTE ADDRESSING(abs)
				//Grab the address
		address += get_abs_ad();
		//Set the address in memory to be the result of the rotate function
		Memory[address] = rotate_left_carry(Memory[address]);
		break;

	case 0xB3: //INDEXED ABSOLUTE ADDRESSING(abs,x)
			   //Grab the address
		address += Index_Registers[REGISTER_X];
		address += get_abs_ad();
		//Set the address in memory to be the result of the rotate function
		Memory[address] = rotate_left_carry(Memory[address]);
		break;

	case 0xC3:  //INDEXED ABSOLUTE ADDRESSING(abs,y)
				//Grab the address
		address += Index_Registers[REGISTER_Y];
		address += get_abs_ad();
		//Set the address in memory to be the result of the rotate function
		Memory[address] = rotate_left_carry(Memory[address]);

		break;

		////////////////
		//     RLCA     //
		////////////////
		/*
		Rotate left through carry memory or Accumulator
		*/

	case 0xD3://A
		//Set the register to be the result of the rotate function
		Registers[REGISTER_A] = rotate_left_carry(Registers[REGISTER_A]);
		break;

		////////////////
		//     RLCB     //
		////////////////
		/*
		Rotate left through carry memory or accumulator
		*/

	case 0xE3: //B
			   //Set the register to be the result of the rotate function
		Registers[REGISTER_B] = rotate_left_carry(Registers[REGISTER_B]);
		break;

		////////////////
		//     ROL     //
		////////////////
		/*
		Rotate left without carry Memory or Accumlator
		*/

	case 0xA8: //ABSOLUTE ADDRESSING(abs)
		//Grab the address
		address += get_abs_ad();
		//Set the address in memory to be the result of the rotate function
		Memory[address] = rotate_left(Memory[address]);
		break;

	case 0xB8:  //INDEXED ABSOLUTE ADDRESSING(abs,X)
				//Grab the address
		address += Index_Registers[REGISTER_X];
		address += get_abs_ad();
		//Set the address in memory to be the result of the rotate function
		Memory[address] = rotate_left(Memory[address]);
		break;

	case 0xC8: //INDEXED ABSOLUTE ADDRESSING(abs,y)
			   //Grab the address
		address += Index_Registers[REGISTER_Y];
		address += get_abs_ad();
		//Set the address in memory to be the result of the rotate function
		Memory[address] = rotate_left(Memory[address]);
		break;

		////////////////
		//     ROLA     //
		////////////////
		/*
		Rotate left without carry Memory or Accumlator
		*/

	case 0xD8: //A
			   //Set the register to be the result of the rotate function
		Registers[REGISTER_A] = rotate_left(Registers[REGISTER_A]);
		break;

		////////////////
		//     ROLB     //
		////////////////
		/*
		Rotate left without carry memory or accumulator
		*/

	case 0xE8: //B
			   //Set the register to be the result of the rotate function
		Registers[REGISTER_B] = rotate_left(Registers[REGISTER_B]);
		break;

		////////////////
		//     RR     //
		////////////////
		/*
		Rotate right without carry memory or Accumulator
		*/

	case 0xA9:  //ABSOLUTE ADDRESSING(abs)
		//Get the address
		address += get_abs_ad();
		//Set the address in memory to be the result of the rotate function
		Memory[address] = rotate_right(Memory[address]);
		break;

	case 0xB9: //INDEXED ABSOLUTE ADDRESSING(abs,X)
			   //Get the address
		address += Index_Registers[REGISTER_X];
		address += get_abs_ad();
		//Set the address in memory to be the result of the rotate function
		Memory[address] = rotate_right(Memory[address]);
		break;

	case 0xC9: //INDEXED ABSOLUTE ADDRESSING(abs,y)
			   //Get the address
		address += Index_Registers[REGISTER_Y];
		address += get_abs_ad();
		//Set the address in memory to be the result of the rotate function
		Memory[address] = rotate_right(Memory[address]);
		break;

		////////////////
		//     RRA     //
		////////////////
		/*
		Rotate right wihtout carry memory or accumulator
		*/

	case 0xD9: //A
			   //Set the register to be the result of the rotate function
		Registers[REGISTER_A] = rotate_right(Registers[REGISTER_A]);
		break;

		////////////////
		//     RRB     //
		//////////////// 
		/*
		Rotate right wihtout carry memory or accumulator
		*/

	case 0xE9: //B
			   //Set the register to be the result of the rotate function
		Registers[REGISTER_B] = rotate_right(Registers[REGISTER_B]);
		break;

		////////////////
		//     SAL     //
		////////////////
		/*
		Arithmetic Shift left Memory or Accumulator
		*/

	case 0xA4:	 //ABSOLUTE ADDRESSING(abs)
		//Get the address
		address += get_abs_ad();
		//Set data to be the result of the arithmetic_shift function
		data = arithmetic_shift(address, data, REGISTER_A, REGISTER_A, 0);
		//Set flag z,n,c based on the result of data
		set_three_flags(data);
		break;
	case 0xB4:   //INDEXED ABSOLUTE ADDRESSING(abs,X)
				 //Get the address
		address += get_abs_ad();
		//Set data to be the result of the arithmetic_shift function
		data = arithmetic_shift(address, data, REGISTER_A, REGISTER_X, 0);
		//Set flag z,n,c based on the result of data
		set_three_flags(data);
		break;

	case 0xC4:  ///INDEXED ABSOLUTE ADDRESSING(abs,y)
				//Get the address
		address += get_abs_ad();
		//Set data to be the result of the arithmetic_shift function
		data = arithmetic_shift(address, data, REGISTER_A, REGISTER_Y, 0);
		//Set flag z,n,c based on the result of data
		set_three_flags(data);
		break;

		////////////////
		//     SALA     //
		////////////////
		/*
		Arithmetic shift Left memory or Accumulator
		*/

	case 0xD4: //A
		//Shift whats in the register 1 place to the left
		data = Registers[REGISTER_A] << 1;
		//Once inside data set the register to be the (BYTE) result of the shift
		Registers[REGISTER_A] = (BYTE)data;
		//Set the flags based on the shift
		set_three_flags(data);
		break;

		////////////////
		//    SALB     //
		////////////////
		/*
		Arithmetic shift left Memory or AAccumulator
		*/

	case 0xE4: //B
			   //Shift whats in the register 1 place to the left
		data = Registers[REGISTER_B] << 1;
		//Once inside data set the register to be the (BYTE) result of the shift
		Registers[REGISTER_B] = (BYTE)data;
		//Set the flags based on the shift
		set_three_flags(data);
		break;

		////////////////
		//     SAR     //
		////////////////
		/*
		Arithmetic shift right memory or accumulator
		*/

	case 0xA5: //ABSOLUTE ADDRESSING(abs)
		//Grab the address
		address += get_abs_ad();
		//Set data to be the return of the airhtmetic_shift function
		data = arithmetic_shift(address, data, REGISTER_A, REGISTER_A, 1);
		//Set the flags based on data (z,n,c)
		set_three_flags(data);
		break;

	case 0xB5: ///INDEXED ABSOLUTE ADDRESSING(abs,X)
			   //Grab the address
		address += get_abs_ad();
		//Set data to be the return of the airhtmetic_shift function
		data = arithmetic_shift(address, data, REGISTER_A, REGISTER_X, 1);
		//Set the flags based on data (z,n,c)
		set_three_flags(data);
		break;


	case 0xC5: //INDEXED ABSOLUTE ADDRESSING(abs,y)
			   //Grab the address
		address += get_abs_ad();
		//Set data to be the return of the airhtmetic_shift function
		data = arithmetic_shift(address, data, REGISTER_A, REGISTER_Y, 1);
		//Set the flags based on data (z,n,c)
		set_three_flags(data);
		break;

		////////////////
		//     SARA     //
		////////////////
		/*
		Arithmetic shift right memory or Accumulator
		*/

	case 0xD5: //A
			   //Set data to be the result of the  aithrmetic_shift function
		data = arithmetic_shift(address, data, REGISTER_A, REGISTER_A, 3);
		//Set the flags based on data (z,n,c)
		set_three_flags(data);
		break;

		////////////////
		//     SARB    //
		////////////////
		/*
		Arithmetic shift right Memory or Accumulator
		*/

	case 0xE5: //B
			   //Set data to be the result of the  aithrmetic_shift function
		data = arithmetic_shift(address, data, REGISTER_B, REGISTER_A, 3);
		//Set the flags based on data (z,n,c)
		set_three_flags(data);
		break;

		////////////////
		//     LSR     //
		////////////////
		/*
		Shift right Memory or Accumulator
		*/

	case 0xA6:	//ABSOLUTE ADDRESSING(abs)
		//Grab the address
		address += get_abs_ad();
		//Set data based on the shift right function
		data = shift_right(address, data, REGISTER_A, REGISTER_A, 0);
		//Set the flags based on data (z,n,c)
		set_three_flags(data);

		break;

	case 0xB6:	//INDEXED ABSOLUTE ADDRESSING(abs,X)
				//Grab the address
		address += get_abs_ad();
		//Set data based on the shift right function
		data = shift_right(address, data, REGISTER_A, REGISTER_X, 0);
		//Set the flags based on data (z,n,c)
		set_three_flags(data);
		break;

	case 0xC6: //INDEXED ABSOLUTE ADDRESSING(abs,Y)
			   //Grab the address
		address += get_abs_ad();
		//Set data based on the shift right function
		data = shift_right(address, data, REGISTER_A, REGISTER_Y, 0);
		//Set the flags based on data (z,n,c)
		set_three_flags(data);
		break;

		////////////////
		//    LSRA     //
		////////////////
		/*
		Shif right memory or Accumulator
		*/

	case 0xD6://A
			  //Set data based on the shift right function
		data = shift_right(address, data, REGISTER_A, REGISTER_A, 1);
		//Set the flags based on data (z,n,c)
		set_three_flags(data);
		break;

		////////////////
		//     LSRB     //
		////////////////
		/*
		Shift right memory or Accumualtor
		*/

	case 0xE6://B
			  //Set data based on the shift right function
		data = shift_right(address, data, REGISTER_B, REGISTER_A, 1);
		//Set the flags based on data (z,n,c)
		set_three_flags(data);
		break;

		////////////////
		//     COM     //
		////////////////	
		/*
		Negate memory or accumulator
		*/

	case 0xA7: //ABSOLUTE ADDRESSING(abs)
		//Call negation function which negates the Register (or memory)
		negate_mem_or_accumulator(REGISTER_A, data, address, 1);

		break;

	case 0xB7: //INDEXED ABSOLUTE ADDRESSING(abs,X)
			   //Call negation function which negates the Register (or memory)
		negate_mem_or_accumulator(REGISTER_X, data, address, 0);
		break;

	case 0xC7: // INDEXED ABSOLUTE ADDRESSING(abs, Y)
			   //Call negation function which negates the Register (or memory)
		negate_mem_or_accumulator(REGISTER_Y, data, address, 0);
		break;

		////////////////
		//    COMA     //
		////////////////
		/*
		Negate Memory or Accumulator
		*/

	case 0xD7: //A
			   //Call negation function which negates the Register (or memory)
		negate_mem_or_accumulator(REGISTER_A, data, address, 2);
		break;

		////////////////
		//    COMB     //
		////////////////
		/*
		Negate Memory or Accumulator
		*/

	case 0xE7: //B 
			   //Call negation function which negates the Register (or memory)
		negate_mem_or_accumulator(REGISTER_B, data, address, 2);
		break;

		////////////////
		//    PUSH     //
		////////////////
		/*
		Push Register onto the stack
		*/

	case 0xBE: //A
		//Move whatever is in the source register onto the top of the stack
		Memory[StackPointer] = Registers[REGISTER_A];
		//Decrement the stackpointer
		StackPointer--;
		break;

	case 0xCE: //B
			   //Move whatever is in the source register onto the top of the stack
		Memory[StackPointer] = Registers[REGISTER_B];
		//Decrement the stackpointer
		StackPointer--;
		break;

	case 0xDE: //FL
			   //Move whatever is in the source variable onto the top of the stack
		Memory[StackPointer] = Flags;
		//Decrement the stackpointer
		StackPointer--;
		break;

	case 0xEE: //L
			   //Move whatever is in the source variable onto the top of the stack
		Memory[StackPointer] = Registers[REGISTER_L];
		//Decrement the stackpointer
		StackPointer--;
		break;
	case 0xFE: //H
			   //Move whatever is in the source variable onto the top of the stack
		Memory[StackPointer] = Registers[REGISTER_H];
		//Decrement the stackpointer
		StackPointer--;
		break;

		////////////////
		//     POP     //
		////////////////
		/*
		Pop the top of the
		Stack into the Register
		*/

	case 0xBF://A
		//Move the stackpointer 
		StackPointer++;
		//Pop off of the top of the stack into a register
		Registers[REGISTER_A] = Memory[StackPointer];

		break;
	case 0xCF: //B
			   //Move the stackpointer
		StackPointer++;
		//Pop off of the top of the stack into a register
		Registers[REGISTER_B] = Memory[StackPointer];
		break;

	case 0xDF: //FL
			   //Move the stackpointer
		StackPointer++;
		//Pop off of the top of the stack into the variable
		Flags = Memory[StackPointer];
		break;

	case 0xEF: //L
			   //Move the stackpointer
		StackPointer++;
		//Pop off of the top of the stack into a register
		Registers[REGISTER_L] = Memory[StackPointer];
		break;

	case 0xFF: //H
			   //Move the stackpointer
		StackPointer++;
		//Pop off of the top of the stack into a register
		Registers[REGISTER_H] = Memory[StackPointer];
		break;

		////////////////
		//     JUMP     //
		////////////////
		/*
		Loads Memory in ProgramCounter
		*/

	case 0x10: //ABSOLUTE ADDRESSING(abs)
		//Grab address
		address = get_abs_ad();
		//Move address into program counter
		ProgramCounter = address;
		break;

		/////////////////
		//     JSR     //
		/////////////////
		/*
		Jump to subroutine
		*/

	case 0x21: //ABSOLUTE ADDRESSING(abs)
		//Grab address
		address += get_abs_ad();
		//Validate the address range
		if (address >= 0 && address < MEMORY_SIZE) {

			LB = (BYTE)ProgramCounter;
			HB = (BYTE)(ProgramCounter >> 8);

			temp_word = ((WORD)HB << 8) + LB;
			//Add to the stack
			Memory[StackPointer] = temp_word;
			//Decrement the stackpointer
			StackPointer--;
			//Set the programcounter to match the address
			ProgramCounter = address;
		}
		break;

		/////////////////
		//     RET     //
		/////////////////
		/*
		Return from subroutine
		*/

	case 0x4C: //IMPLIED ADDRESSING(impl)
		//Increment the stackpointer
		StackPointer++;
		//Set the address to be what is on the top of the stack
		address = Memory[StackPointer];
		//Move the address into the ProgramCounter
		ProgramCounter = address;
		break;

		////////////////
		//     SBIA     //
		////////////////
		/*
		Data subtracted to Accumulator with Carry
		*/

	case 0x93: ///IMMEDIATE ADDRESSING (#)
		//Set temp_word to be the result of the sub_to_accumulator_carry
		temp_word = sub_to_accumulator_carry(REGISTER_A);
		//Set flag z,n,c based on the operation
		set_three_flags((WORD)temp_word);
		//Move the operation result into the Register
		Registers[REGISTER_A] = temp_word;
		break;

		////////////////
		//    SBIB     //
		////////////////
		/*
		Data subtracted to Accumulator with Carry
		*/

	case 0x94: //IMMEDIATE ADDRESSING (#)
			   //Set temp_word to be the result of the sub_to_accumulator_carry
		temp_word = sub_to_accumulator_carry( REGISTER_B);
		//Set flag z,n,c based on the operation
		set_three_flags((WORD)temp_word);
		//Move the operation result into the Register
		Registers[REGISTER_B] = temp_word;
		break;

		////////////////
		//     CPIA    //
		////////////////
		/*
		Data compared to accumulator
		*/

	case 0x95://IMMEDIATE ADDRESSING (#)
		//Compare the stated register using compare_accumulator function
		compare_accumulator(REGISTER_A);
		break;

		////////////////
		//     CPIB     //
		////////////////
		/*
		Data compared to accumulator
		*/

	case 0x96: //IMMEDIATE ADDRESSING (#)
			   //Compare the stated register using compare_accumulator function
		compare_accumulator(REGISTER_B);

		break;

		////////////////
		//     JCC     //
		////////////////
		/*
		Jump on Carry clear
		*/

	case 0x11: //ABSOLUTE ADDRESSING(abs)
		//get address
		address = get_abs_ad();
		//Check if there is no carry
		if ((Flags & FLAG_C) == 0) {
			//Set the program counter to have the current address
			ProgramCounter = address;
		}

		break;

		////////////////
		//     JCS     //
		////////////////
		/*
		Jump on carry set
		*/

	case 0x12: //ABSOLUTE ADDRESSING(abs)
			   //get address
		address = get_abs_ad();
		//Check if there is a carry
		if ((Flags & FLAG_C) != 0) {
			//Set the program counter to have the current address
			ProgramCounter = address;
		}

		break;

		////////////////
		//     JNE     //
		////////////////
		/*
		Jump on result not Zero
		*/

	case 0x13: //ABSOLUTE ADDRESSING(abs)
		//get address
		address = get_abs_ad();
		//Check if there is no Zero flag set
		if ((Flags & FLAG_Z) == 0)
		{
			//Set the program counter to have the current address
			ProgramCounter = address;

		}
		break;

		////////////////
		//     JEQ     //
		////////////////
		/*
		Jump on result equal to Zero
		*/

	case 0x14:  //ABSOLUTE ADDRESSING(abs)
		//get address
		address = get_abs_ad();
		// check that the zero flag is set
		if ((Flags & FLAG_Z) != 0)
		{
			//Set the program counter to have the current address
			ProgramCounter = address;
		}

		break;

		////////////////
		//     JMI     //
		////////////////
		/*
		Jump on Negative Result
		*/

	case 0x15: //ABSOLUTE ADDRESSING(abs)
		//get address
		address = get_abs_ad();
		//Check the negative flag is set
		if ((Flags & FLAG_N) != 0)
		{
			//Set the program counter to have the current address
			ProgramCounter = address;
		}
		break;

		////////////////
		//     JPL     //
		////////////////
		/*
		Jump on positive reuslt
		*/

	case 0x16: //ABSOLUTE ADDRESSING(abs)
		//get address
		address = get_abs_ad();
		//Check there the negative flag isn't set
		if ((Flags & FLAG_N) == 0) {
			//Set the program counter to have the current address
			ProgramCounter = address;

		}
		break;

		////////////////
		//     JHI     //
		////////////////
		/*
		Jump on result same or lower
		*/

	case 0x17: //ABSOLUTE ADDRESSING(abs)
		//get address
		address = get_abs_ad();
		//Check what the result of the bitwise operations
		if (Flags & (FLAG_C | FLAG_Z) != 0)
		{
			//Set the program counter to have the current address
			ProgramCounter = address;

		}

		break;

		////////////////
		//     JLE     //
		////////////////
		/*
		Jump on result higher
		*/

	case 0x18: //ABSOLUTE ADDRESSING(abs)
		//get address
		address = get_abs_ad();
		//Check what the result of the bitwise operations
		if ((Flags & (FLAG_C | FLAG_Z)) == 0) {
			//Set the program counter to have the current address
			ProgramCounter = address;
		}
		break;

		////////////////
		//     CCC     //
		////////////////
		/*
		Call on Carry clear
		*/

	case 0x22: //ABSOLUTE ADDRESSING(abs)
		//Get the address
		address = get_abs_ad();
		//Check that a carry is not set
		if ((Flags & FLAG_C) == 0) {
			//Push program counter onto the top of the stack
			push(ProgramCounter);
			//Set the program counter to have the current address
			ProgramCounter = address;
		}
		break;

		////////////////
		//     CCS     //
		////////////////
		/*
		Call on Carry set
		*/

	case 0x23: //ABSOLUTE ADDRESSING(abs)
		//Get the address
		address = get_abs_ad();
		//Check a carry is set
		if ((Flags & FLAG_C) != 0) {
			//Push program counter onto the top of the stack
			push(ProgramCounter);
			//Set the program counter to have the current address
			ProgramCounter = address;
		}
		break;

		////////////////
		//     CNE     //
		////////////////
		/*
		Call on result not Zero
		*/

	case 0x24: //ABSOLUTE ADDRESSING(abs)
		//Get the address
		address = get_abs_ad();
		//Check the zero flag is not set
		if ((Flags & FLAG_Z) == 0) {
			//Push program counter onto the top of the stack
			push(ProgramCounter);
			//Set the program counter to have the current address
			ProgramCounter = address;
		}

		break;

		////////////////
		//     CEQ     //
		////////////////
		/*
		Call on result equal to Zero
		*/

	case 0x25: //ABSOLUTE ADDRESSING(abs)
		//Get the address
		address = get_abs_ad();
		//Check the zero flag is set
		if ((Flags & FLAG_Z) != 0) {
			//Push program counter onto the top of the stack
			push(ProgramCounter);
			//Set the program counter to have the current address
			ProgramCounter = address;
		}
		break;

		////////////////
		//     CMI     //
		////////////////
		/*
		Call on negative result
		*/

	case 0x26: //ABSOLUTE ADDRESSING(abs)
		//get the address
		address = get_abs_ad();
		//Check the negative flag is set
		if ((Flags & FLAG_N) != 0) {
			//Push program counter onto the top of the stack
			push(ProgramCounter);
			//Set the program counter to have the current address
			ProgramCounter = address;
		}
		break;

		////////////////
		//     CPL     //
		//////////////// 
		/*
		Call on positive result
		*/

	case 0x27: //ABSOLUTE ADDRESSING(abs)
		//Get the address
		address = get_abs_ad();
		//Check the negative flag is not set
		if ((Flags & FLAG_N) == 0) {
			//Push program counter onto the top of the stack
			push(ProgramCounter);
			//Set the program counter to have the current address
			ProgramCounter = address;
		}
		break;
	
		////////////////
		//     CHI     //
		////////////////
		/*
		Call on result same or lower
		*/

	case 0x28: //ABSOLUTE ADDRESSING(abs)
		//Get the address
		address = get_abs_ad();
		//Check what the result of the bitwise operations
		if (Flags & (FLAG_C | FLAG_Z) != 0) {
			//Push program counter onto the top of the stack
			push(ProgramCounter);
			//Set the program counter to have the current address
			ProgramCounter = address;
		}
		break;

		////////////////
		//     CLE     //
		////////////////
		/*
		Call on result higher
		*/

	case 0x29: //ABSOLUTE ADDRESSING(abs)
		//Get the address
		address = get_abs_ad();
		//Check what the result of the bitwise operations
		if ((Flags & (FLAG_C | FLAG_Z)) == 0) {
			//Push program counter onto the top of the stack
			push(ProgramCounter);
			//Set the program counter to have the current address
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
		default_switch();
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
		default_switch();
		break;

	}

	source_as_reg_index(destReg, sourceReg);
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

