/*
	Chip8/SuperChip8 implementation
*/
#include "Chip8.hpp"
#include <iostream>
#include <cstdio>
#include <fstream>
#include <iomanip>

// Function pointers

chip8::chip8() {
	initialize();
}
void chip8::initialize() {

    pc = 0x200;  // Program counter starts at 0x200
    opcode = 0;      // Reset current opcode	
    I = 0;      // Reset index register
    sp = 0;      // Reset stack pointer

    // Clear display
	memset(gfx, 0x0, 128 * 64);
    // Clear stack
	memset(stack, 0x0, 16);
    // Clear registers V0-VF
	memset(V, 0x0, 16);
    // Clear memory
	memset(memory, 0x0, 4096);
	// Clear RPL
	memset(RPL, 0x0, 8);
	
	
	

    // Load fontset
    for (int i = 0; i < 80; ++i)
        memory[i] = chip8_fontset[i];

    // Reset timers
    delay_timer = 0;
    sound_timer = 0;

	// Flags
	drawFlag = false;
	beepFlag = false;
	exitFlag = false;
	fullscreen = false;
	awaitKey = false;
	debugMode = false;
}


bool chip8::loadGame(const char* game) {
	// attempts to open file
	std::ifstream in(game, std::ios::in | std::ios::binary);
	if (!in) return false;

	// get size of file:
	in.seekg(0, in.end);
	int size = (int)in.tellg();

	// invalid size
	if (size > 0xFFF - 0x200) return false;

	// allocate memory:
	char* buffer = (char*)(&(memory[0x200]));

	// read data:
	in.seekg(0, in.beg);
	in.read(buffer, size);

	// save filename of file for resetting
	resetFilePath = std::string(game);

	// close file
	in.close();
	return true;
}

void chip8::emulateCycle() {
    execute();
	if(debugMode)
		std::cout << std::hex << opcode << std::endl;

    // Update timers
    if (delay_timer > 0)
        --delay_timer;

    if (sound_timer > 0){
		if (sound_timer == 1)
			beepFlag = true;
        --sound_timer;
    }
}
void chip8::fetch(){
    opcode = memory[pc] << 8 | memory[pc + 1];
}

void chip8::execute() {
    fetch();
	void(chip8::*chip8call)(void);
	chip8call = Chip8Table[(opcode & 0xF000) >> 12];
	(this->*chip8call)();
}

void chip8::reset() {
	std::ifstream in(resetFilePath, std::ios::in | std::ios::binary);
	if (in) {
		in.close();
		bool isDebug = debugMode;
		initialize();
		loadGame(resetFilePath.c_str());
		debugMode = isDebug;
	}
}

void chip8::setKey(char k) {
	// Clear keys
	memset(key, 0x0, 16);
	if (k >= 0x0 && k <= 0xF)
		key[k] = 1;
}

void chip8::clearKey() {
	memset(key, 0x0, 16);
}

bool chip8::noKeyWait() {
	// returns true if opcode != FX0A
	return ((opcode & 0xF00F) != 0xF00A);
}

////////////// Opcodes ////////////////////////////

// 0NNN: Calls RCA 1802 program at address NNN.
void chip8::cpu0NNN(){
	pc += 2;
}

// *00CN: Scroll display N lines down
void chip8::cpu00CN() {
	uint8_t w = 64 + 64 * (uint8_t)(fullscreen);
	uint8_t h = 32 + 32 * (uint8_t)(fullscreen);
	uint8_t n = opcode & 0x000F;
	uint16_t size = w * h;
	for (uint16_t i = size - 1; i > w * n - 1; i--)
		gfx[i] = gfx[i - w * n];
	// padding
	for (uint16_t i = 0; i < w * n; i++)
		gfx[i] = 0;
	drawFlag = true;
	pc += 2;
}

// 00E0: Clears the screen  
void chip8::cpu00E0(){
	memset(gfx, 0x0, 128 * 64);
	drawFlag = true;
	pc += 2;
}

// 00EE: Returns from subroutine 
void chip8::cpu00EE() {
	--sp;
    pc = stack[sp];
    pc += 2;
}

//*00FB:  Scroll display 4 pixels right
void chip8::cpu00FB() {
	uint8_t w = 64 + 64 * (uint8_t)(fullscreen);
	uint8_t h = 32 + 32 * (uint8_t)(fullscreen);
	uint16_t size = w * h;
	// Increment by row
	for (uint16_t i = 0; i < size; i += w) {
		// Now by columns in row
		for (uint16_t j = i + w - 1; j > i + 3; j--)
			gfx[j] = gfx[j - 4];
		// padding
		for (uint16_t j = i; j < i + 4; j++)
			gfx[j] = 0;
	}
	drawFlag = true;
	pc += 2;
}
//*00FC:  Scroll display 4 pixels left
void chip8::cpu00FC() {
	uint8_t w = 64 + 64 * (uint8_t)(fullscreen);
	uint8_t h = 32 + 32 * (uint8_t)(fullscreen);
	uint16_t size = w * h;
	// Increment by row
	for (uint16_t i = 0; i < size; i += w) {
		// Now by columns in row
		for (uint16_t j = i; j < i + w - 4; j++)
			gfx[j] = gfx[j + 4];
		// padding
		for (uint16_t j = i + w - 5; j < i + w; j++)
			gfx[j] = 0;
	}
	drawFlag = true;
	pc += 2;
}
//*00FD:  Exit CHIP interpreter
void chip8::cpu00FD() {
	exitFlag = true;
	pc += 2;
}
//*00FE:  Disable extended screen mode
void chip8::cpu00FE() {
	fullscreen = false;
	drawFlag = true;
	pc += 2;
}
//*00FF:  Enable extended screen mode for fullscreen graphics
void chip8::cpu00FF() {
	fullscreen = true;
	drawFlag = true;
	pc += 2;
}
// 1NNN: Jumps to address NNN.
void chip8::cpu1NNN() {
    pc = opcode & 0x0FFF;
}

// 2NNN: Calls subroutine at NNN.
void chip8::cpu2NNN() {
    stack[sp] = pc;
    ++sp;
    pc = opcode & 0x0FFF;
}
// 3NNN: Skips the next instruction if VX equals NN.
void chip8::cpu3XNN() {
	pc += 2 + 2 * (uint8_t)(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF));
}
// 4NNN: Skips the next instruction if VX doesn't equal NN.
void chip8::cpu4XNN() {
	pc += 2 + 2 * (uint8_t)(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF));

}
// 5NNN:  Skips the next instruction if VX equals VY.
void chip8::cpu5XY0() {
	pc += 2 + 2 * (uint8_t)(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]);
}
// 6NNN: Sets VX to NN.
void chip8::cpu6XNN(){
	V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
	pc += 2;

}
// 7NNN: Adds NN to VX.
void chip8::cpu7XNN() {
	V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
	pc += 2;

}
// 8XY0: Sets VX to the value of VY.
void chip8::cpu8XY0() {
	V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
	pc += 2;
}
// 8XY1: Sets VX to VX or VY.
void chip8::cpu8XY1() {
	V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
	pc += 2;
}
// 8XY2: Sets VX to VX and VY.
void chip8::cpu8XY2() {
	V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
	pc += 2;
}
// 8XY3: Sets VX to VX xor VY.
void chip8::cpu8XY3() {
	V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
	pc += 2;
}
// 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
void chip8::cpu8XY4() {
	uint16_t x = (opcode & 0x0F00) >> 8;
	uint16_t y = (opcode & 0x00F0) >> 4;
	V[0xF] = 1 * (uint8_t)(V[y] > 0xFF - V[x]);
    V[x] += V[y];
    pc += 2;
}
// 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
void chip8::cpu8XY5() {
	uint16_t x = (opcode & 0x0F00) >> 8;
	uint16_t y = (opcode & 0x00F0) >> 4;
	V[0xF] = 1 * (uint8_t)(V[x] >= V[y]);
	V[x] -= V[y];
	pc += 2;
}
// 8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.
void chip8::cpu8XY6() {
	uint16_t x = (opcode & 0x0F00) >> 8;
	V[0xF] = (V[x] & 0x1);
	V[x] >>= 1;
	pc += 2;
}
// 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
void chip8::cpu8XY7() {
	uint16_t x = (opcode & 0x0F00) >> 8;
	uint16_t y = (opcode & 0x00F0) >> 4;
	V[0xF] = 1 * (uint8_t)(V[y] >= V[x]);
	V[x] = V[y] - V[x];
	pc += 2;
}
// 8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
void chip8::cpu8XYE() {
	uint16_t x = (opcode & 0x0F00) >> 8;
	V[0xF] = (V[x] & 0x80) >> 7;
	V[x] <<= 1;
	pc += 2;

}
// 9XY0: Skips the next instruction if VX doesn't equal VY.
void chip8::cpu9XY0() {
	pc += 2 + 2 * (uint8_t)(V[(opcode & 0x00F0)] != V[(opcode & 0x0F00)]);
}
// ANNN: Sets I to the address NNN
void chip8::cpuANNN() {
    I = opcode & 0x0FFF;
    pc += 2;
}
// BNNN: Jumps to the address NNN plus V0.
void chip8::cpuBNNN() {
	pc = (opcode & 0x0FFF) + V[0];

}
// CNNN: Sets VX to a random number, masked by NN.
void chip8::cpuCNNN() {
	uint8_t x = (opcode & 0x0F00) >> 8;
	uint8_t mask = opcode & 0x00FF;
	V[x] = (rand() % 0xFF) & mask;
	pc += 2;
}
//DXYN: Sprites stored in memory at location in index register (I), maximum 8bits wide. Wraps around the screen. If when drawn, clears pixel, register VF is set to 1 otherwise it is zero. All drawing is XOR drawing (i.e. it toggles the screen pixels) Show N-byte sprite from M(I) at coords (VX,VY), VF = collision. If N = 0 and extended mode, show 16x16 sprite.
void chip8::cpuDXYN() {
	uint8_t x = V[(opcode & 0x0F00) >> 8];
	uint8_t y = V[(opcode & 0x00F0) >> 4];
	uint8_t n = opcode & 0x000F;
	uint16_t w = 64 + 64 * (uint8_t)(fullscreen);
	uint16_t h = 32 + 32 * (uint8_t)(fullscreen);
	uint16_t size = w * h;
	uint8_t bigSprite = (uint8_t)(fullscreen && n == 0x0);
	uint16_t spr_width = 0x8 + 0x8 * bigSprite;
	uint16_t spr_height = (n == 0x0) ? 0x10 : n;
	uint16_t pixel;
	uint16_t shift = bigSprite ? 0x8000 : 0x80;

    V[0xF] = 0;
    for (uint16_t yline = 0; yline < spr_height; yline++){
		if (bigSprite) {
			pixel = memory[I + yline * 2];
			pixel <<= 0x8;
			pixel |= memory[I + (yline * 2) + 1];
		}
		else
			pixel = memory[I + yline];
        for (uint16_t xline = 0; xline < spr_width; xline++){
			if (exitFlag) {
				std::cout << "asdf" << std::endl;
			}
            if ((pixel & (shift >> xline)) != 0){
				uint16_t idx = (x + xline + ((y + yline) * w)) % size;
                if (gfx[idx] == 1)
                    V[0xF] = 1;
                gfx[idx] ^= 1;
            }
        }
    }
    drawFlag = true;
    pc += 2;
}
// EX9E: Skips the next instruction if the key stored in VX is pressed
void chip8::cpuEX9E() {
	uint16_t x = (opcode & 0x0F00) >> 8;
	pc += 2 + 2 * (uint8_t)(key[V[x]] != 0);
	key[V[x]] = 0;
}

// EXA1: Skips the next instruction if the key stored in VX isn't pressed.
void chip8::cpuEXA1() {
	uint16_t x = (opcode & 0x0F00) >> 8;
	pc += 2 + 2 * (uint8_t)(key[V[x]] == 0);
}
// *F0NN: I = 28bit address
void chip8::cpuF0NN() {
	I = opcode & 0x00FF;
	pc += 2;
}
// FX07:  Sets VX to the value of the delay timer.
void chip8::cpuFX07() {
	uint16_t x = (opcode & 0x0F00) >> 8;
	V[x] = delay_timer;
	pc += 2;
}
// FX0A: A key press is awaited, and then stored in VX.
void chip8::cpuFX0A() {
	uint8_t x = (opcode & 0x0F00) >> 8;
	int8_t c = -1;
	for (uint8_t i = 0; i < 0x10; i++) {
		if (key[i] != 0)
			c = i;
	}
	if (c > -1) {
		V[x] = c;
		pc += 2;
		memset(key, 0x0, 16);
	}
	else {
		awaitKey = true;
	}
}
// FX15: Sets the delay timer to VX.
void chip8::cpuFX15() {
	delay_timer = V[(opcode & 0x0F00) >> 8];
	pc += 2;

}
// FX18: Sets the sound timer to VX.
void chip8::cpuFX18() {
	sound_timer = V[(opcode & 0x0F00) >> 8];
	pc += 2;
}
// *FX1E: I += VX 
void chip8::cpuFX1E() {
	I += V[(opcode & 0x0F00) >> 8];
	if (I > 0xFFF) {
		V[0xF] = 1;
	}
	pc += 2;
}

// FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
void chip8::cpuFX29() {
	I = V[(opcode & 0x0F00) >> 8] * 0x5;
	pc += 2;
}

// *FX30: Point I to 10-byte font sprite for digit VX (0..9)
void chip8::cpuFX30() {
	I = V[(opcode & 0x0F00) >> 8] * 0xA;
	pc += 2;
}
// FX33: Stores the Binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. (See wiki for more info)
void chip8::cpuFX33() {
	uint16_t x = (opcode & 0x0F00) >> 8;
	memory[I] = V[x] / 100;
	memory[I + 1] = (V[x] / 10) % 10;
	memory[I + 2] = (V[x] % 100) % 10;
	pc += 2;
}

// FX55: Stores V0 to VX in memory starting at address I.
void chip8::cpuFX55() {
	uint16_t x = (opcode & 0x0F00) >> 8;
	for (uint8_t i = 0; i <= x; i++) {
		memory[I + i] = V[i];
	}
	pc += 2;

}
// FX65: Fills V0 to VX with values from memory starting at address I.
void chip8::cpuFX65() {
	uint16_t x = (opcode & 0x0F00) >> 8;
	for (uint8_t i = 0; i <= x; i++) {
		V[i] = memory[I + i];
	}
	pc += 2;
}

// *FX75: Store V0..VX in RPL user flags (X <= 7)
void chip8::cpuFX75() {
	uint16_t x = (opcode & 0x0F00) >> 8;
	x = x <= 7 ? x : 7;
	for (uint8_t i = 0; i <= x; i++) {
		RPL[i] = V[i];
	}
	pc += 2;
}
// *FX85: Read V0..VX from RPL user flags (X <= 7) 
void chip8::cpuFX85() {
	uint16_t x = (opcode & 0x0F00) >> 8;
	x = x <= 7 ? x : 7;
	for (uint8_t i = 0; i <= x; i++) {
		V[i] = RPL[i];
	}
	pc += 2;
}

////////////// Function pointers ////////////////////////////

void chip8::cpuNULL() {
    std::cout << "Unknown opcode: " << opcode << std::endl;
	pc += 2;
}

void chip8::cpuARITHMETIC() {
	void(chip8::*chip8call)(void);
	chip8call = Chip8Arithmetic[(opcode & 0x000F)];
	(this->*chip8call)();
}

void chip8::cpuKEYS() {
	void(chip8::*chip8call)(void);
	chip8call = Chip8Keys[((opcode & 0x00F0) >> 4) - 0x9];
	(this->*chip8call)();
}

void chip8::cpuSTART() {
	if ((opcode & 0x00F0) >> 4 == 0xF) {
		void(chip8::*chip8call)(void);
		chip8call = Chip8Screen[(opcode & 0x000F) - 0xB];
		(this->*chip8call)();
	}
	else if ((opcode & 0x00F0) >> 4 == 0xC) {
		cpu00CN();
	}
	else {
		switch (opcode & 0x000F) {
		case 0x0000: cpu00E0(); break;
		case 0x000E: cpu00EE(); break;
		default: cpuNULL(); break;
		}
	}
}

void chip8::cpuMEMORY() {
	switch (opcode & 0x00FF) {
	case 0x07: cpuFX07(); break;
	case 0x0A: cpuFX0A();break;
	case 0x15: cpuFX15(); break;
	case 0x18: cpuFX18(); break;
	case 0x1E: cpuFX1E(); break;
	case 0x29: cpuFX29(); break;
	case 0x30: cpuFX30(); break;
	case 0x33: cpuFX33(); break;
	case 0x55: cpuFX55(); break;
	case 0x65: cpuFX65(); break;
	case 0x75: cpuFX75(); break;
	case 0x85: cpuFX85(); break;
	default: {
		if ((opcode & 0xFF00) >> 8 == 0x00F0)
			cpuF0NN();
		else
			cpuNULL();
		break;
	}
}
}