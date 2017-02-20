/*
	Chip8/SuperChip8 interface
*/
#pragma once
#include <iostream>

class chip8 {
public:
	unsigned char gfx[128 * 64];
	bool drawFlag;
	bool beepFlag;
	bool exitFlag;
	bool fullscreen;
	bool awaitKey;
	bool debugMode;
	chip8();
private:
	unsigned short opcode, pc, I, sp;
	unsigned short stack[16];
	unsigned char memory[4096];
	unsigned char delay_timer;
	unsigned char sound_timer;
	unsigned char V[16];
	unsigned char RPL[8];
	unsigned char key[16];
	std::string debugIns;
	std::string resetFilePath;

	void(chip8::*Chip8Table[17])(void) = {
		&chip8::cpuSTART, &chip8::cpu1NNN, &chip8::cpu2NNN, &chip8::cpu3XNN, &chip8::cpu4XNN, &chip8::cpu5XY0,
		&chip8::cpu6XNN, &chip8::cpu7XNN, &chip8::cpuARITHMETIC, &chip8::cpu9XY0, &chip8::cpuANNN, &chip8::cpuBNNN,
		&chip8::cpuCNNN, &chip8::cpuDXYN, &chip8::cpuKEYS, &chip8::cpuMEMORY, &chip8::cpuNULL,
	};

	void(chip8::*Chip8Arithmetic[16])(void) = {
		&chip8::cpu8XY0, &chip8::cpu8XY1,&chip8::cpu8XY2, &chip8::cpu8XY3, &chip8::cpu8XY4, &chip8::cpu8XY5,
		&chip8::cpu8XY6, &chip8::cpu8XY7,&chip8::cpuNULL, &chip8::cpuNULL,&chip8::cpuNULL,&chip8::cpuNULL,
		&chip8::cpuNULL,&chip8::cpuNULL,&chip8::cpu8XYE, &chip8::cpuNULL,
	};

	void(chip8::*Chip8Keys[3])(void) = {
		&chip8::cpuEX9E, &chip8::cpuEXA1, &chip8::cpuNULL,
	};

	void(chip8::*Chip8Screen[6])(void) = {
		&chip8::cpu00FB,&chip8::cpu00FC,&chip8::cpu00FD,&chip8::cpu00FE,
		&chip8::cpu00FF,&chip8::cpuNULL
	};

	// Chip Fontset
	unsigned char chip8_fontset[80] ={
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	// Superchip Fontset
	unsigned char schip8_fontset[160] = {
		0x00, 0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00, //0
		0x00, 0x08, 0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x3E, 0x00, //1
		0x00, 0x38, 0x44, 0x04, 0x08, 0x10, 0x20, 0x44, 0x7C, 0x00, //2
		0x00, 0x38, 0x44, 0x04, 0x18, 0x04, 0x04, 0x44, 0x38, 0x00, //3
		0x00, 0x0C, 0x14, 0x24, 0x24, 0x7E, 0x04, 0x04, 0x0E, 0x00, //4
		0x00, 0x3E, 0x20, 0x20, 0x3C, 0x02, 0x02, 0x42, 0x3C, 0x00, //5
		0x00, 0x0E, 0x10, 0x20, 0x3C, 0x22, 0x22, 0x22, 0x1C, 0x00, //6
		0x00, 0x7E, 0x42, 0x02, 0x04, 0x04, 0x08, 0x08, 0x08, 0x00, //7
		0x00, 0x3C, 0x42, 0x42, 0x3C, 0x42, 0x42, 0x42, 0x3C, 0x00, //8
		0x00, 0x3C, 0x42, 0x42, 0x42, 0x3E, 0x02, 0x04, 0x78, 0x00, //9
		0x00, 0x18, 0x08, 0x14, 0x14, 0x14, 0x1C, 0x22, 0x77, 0x00, //A
		0x00, 0x7C, 0x22, 0x22, 0x3C, 0x22, 0x22, 0x22, 0x7C, 0x00, //B
		0x00, 0x1E, 0x22, 0x40, 0x40, 0x40, 0x40, 0x22, 0x1C, 0x00, //C
		0x00, 0x78, 0x24, 0x22, 0x22, 0x22, 0x22, 0x24, 0x78, 0x00, //D
		0x00, 0x7E, 0x22, 0x28, 0x38, 0x28, 0x20, 0x22, 0x7E, 0x00, //E
		0x00, 0x7E, 0x22, 0x28, 0x38, 0x28, 0x20, 0x20, 0x70, 0x00  //F
	};

public:
	void initialize();
	bool loadGame(const char* game);
	void emulateCycle();
	void setKey(char k);
	void clearKey();
	void reset();
	bool noKeyWait();

private:
	void fetch();
	void execute();

	//////[Opcodes]////////////////////

	// 0NNN: Calls RCA 1802 program at address NNN.
	void cpu0NNN();
	// *00CN: Scroll display N lines down
	void cpu00CN();
	// 00E0: Clears the screen  
	void cpu00E0();
	// 00EE: Returns from subroutine 
	void cpu00EE();
	//*00FB:  Scroll display 4 pixels right
	void cpu00FB();
	//*00FC:  Scroll display 4 pixels left
	void cpu00FC();
	//*00FD:  Exit CHIP interpreter
	void cpu00FD();
	//*00FE:  Disable extended screen mode
	void cpu00FE();
	//*00FF:  Enable extended screen mode for fullscreen graphics
	void cpu00FF();
	// 1NNN: Jumps to address NNN.
	void cpu1NNN();
	// 2NNN: Calls subroutine at NNN.
	void cpu2NNN();
	// 3NNN: Skips the next instruction if VX equals NN.
	void cpu3XNN();
	// 4NNN: Skips the next instruction if VX doesn't equal NN.
	void cpu4XNN();
	// 5NNN: Skips the next instruction if VX equals VY.
	void cpu5XY0();
	// 6NNN: Sets VX to NN.
	void cpu6XNN();
	// 7NNN: Adds NN to VX.
	void cpu7XNN();
	// 8XY0: Sets VX to the value of VY.
	void cpu8XY0();
	// 8XY1: Sets VX to VX or VY.
	void cpu8XY1();
	// 8XY2: Sets VX to VX and VY.
	void cpu8XY2();
	// 8XY3: Sets VX to VX xor VY.
	void cpu8XY3();
	// 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
	void cpu8XY4();
	// 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
	void cpu8XY5();
	// 8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.
	void cpu8XY6();
	// 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
	void cpu8XY7();
	// 8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
	void cpu8XYE();
	// 9XY0: Skips the next instruction if VX doesn't equal VY.
	void cpu9XY0();
	// ANNN: Sets I to the address NNN
	void cpuANNN();
	// BNNN: Jumps to the address NNN plus V0.
	void cpuBNNN();
	// CNNN: Sets VX to a random number, masked by NN.
	void cpuCNNN();
	// DXYN: Sprites stored in memory at location in index register (I), maximum 8bits wide. Wraps around the screen. If when drawn, clears pixel, register VF is set to 1 otherwise it is zero. All drawing is XOR drawing (i.e. it toggles the screen pixels) 
	void cpuDXYN();
	// EX9E: Skips the next instruction if the key stored in VX is pressed
	void cpuEX9E();
	// EXA1: Skips the next instruction if the key stored in VX isn't pressed.
	void cpuEXA1();
	// *F0NN: I = 28bit address
	void cpuF0NN();
	// FX07:  Sets VX to the value of the delay timer.
	void cpuFX07();
	// FX0A: A key press is awaited, and then stored in VX.
	void cpuFX0A();
	// FX15: Sets the delay timer to VX.
	void cpuFX15();
	// FX18: Sets the sound timer to VX.
	void cpuFX18();
	// *FX1E: I += VX
	void cpuFX1E();
	// FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
	void cpuFX29();
	// *FX30: Point I to 10-byte font sprite for digit VX (0..9)
	void cpuFX30();
	// FX33: Stores the Binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. (See wiki for more info)
	void cpuFX33();
	// FX55: Stores V0 to VX in memory starting at address I.
	void cpuFX55();
	// FX65: Fills V0 to VX with values from memory starting at address I.
	void cpuFX65();
	// *FX75: Store V0..VX in RPL user flags (X <= 7)
	void cpuFX75();
	// *FX85: Read V0..VX from RPL user flags (X <= 7) 
	void cpuFX85();
	////////////// Function pointers ////////////////////////////

	// Null opcode
	void cpuNULL();
	// Opcode category
	void cpuARITHMETIC();
	// Beginning opcodes
	void cpuSTART();
	// Memory stuff
	void cpuMEMORY();
	// Keys
	void cpuKEYS();
};
	
