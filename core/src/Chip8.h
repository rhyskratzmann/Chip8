#pragma once
#include <cstdint>
#include <string>


//Based on http://devernay.free.fr/hacks/chip8/C8TECH10.HTM specification
//header to define Chip8 interpreter

static constexpr int MEMORY_SIZE = 4096;
static constexpr int ROM_ADDRESS = 0x200;
static constexpr int FONT_ADDRESS = 0x50;
static constexpr int DISPLAY_HEIGHT = 32;
static constexpr int DISPLAY_WIDTH = 64;
static constexpr int KEYBOARD_SIZE = 16;


/**
 * CHIP-8 interpreter core.
 *
 * Provides the virtual machine state (memory, registers, timers, display, stack)
 * and opcode handlers that implement the CHIP-8 instruction set.
 *
 * Documentation style:
 * - Brief line: "<opcode> - <mnemonic> <variables>"
 * - Followed by a short description of effects (PC changes, flags, memory/display).
 */
class Chip8 {
    public:
		Chip8();

        /**
         * Executes a single emulation cycle (fetch, decode, execute, timers).
         *
         * Advances the interpreter by one instruction and updates timers.
         */
        void Cycle();

        /**
         * Loads a ROM binary into memory starting at ROM address (0x200).
         *
         * @param path Path to the ROM file to load.
         */
        void Load_ROM(const char* path);
	//Display is using packed bits
	uint8_t display[DISPLAY_HEIGHT * DISPLAY_WIDTH / 8]{};
	uint8_t delayTimer{};
	uint8_t soundTimer{};
	//Keyboard
	uint8_t keyboard[KEYBOARD_SIZE]{};
private:
        uint8_t memory[MEMORY_SIZE]{};
        //Registers
        uint8_t registers[16]{}; //16 8-bit registers
        uint16_t i{}; //16-bit index register

        uint16_t programCounter{};
        uint8_t stackPointer{};
        uint16_t stack[16]{};
        int8_t waitingForKeyRelease{-1}; // -1 = not waiting, 0-15 = waiting for key to be released



        uint8_t fontset[80] =
	        {
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

        // Instructions
        // nnn or addr - A 12-bit value, the lowest 12 bits of the instruction
        // n or nibble - A 4-bit value, the lowest 4 bits of the instruction
        // x - A 4-bit value, the lower 4 bits of the high byte of the instruction
        // y - A 4-bit value, the upper 4 bits of the low byte of the instruction
        // kk or byte - an 8-bit value, the lowest 8 bits of the instruction

        /**
         * Decodes the provided opcode into its fields (nnn, n, x, y, kk).
         *
         * @param opcode Raw 16-bit opcode fetched from memory.
         */
	     void execute_instruction(uint16_t opcode);

        /**
         * 0nnn - SYS nnn
         * Jump to a machine code routine at address nnn (ignored on modern interpreters).
         * @param nnn Target address.
         */
        void SYS_addr(uint16_t nnn);

        /**
         * 00E0 - CLS
         * Clear the display buffer (all pixels set to 0).
         */
        void CLS();

        /**
         * 00EE - RET
         * Return from a subroutine (pop PC from stack).
         */
        void RET();

        /**
         * 1nnn - JP addr
         * Jump to address nnn.
         * @param nnn Target address.
         */
        void JP_addr(uint16_t nnn);

        /**
         * 2nnn - CALL addr
         * Call subroutine at nnn (push PC; set PC = nnn).
         * @param nnn Target address.
         */
        void CALL_addr(uint16_t nnn);

        /**
         * 3xkk - SE Vx, byte
         * Skip next instruction if Vx == kk.
         * @param x Register index.
         * @param kk Immediate byte.
         */
        void SE_Vx_byte(uint8_t x, uint8_t kk);

        /**
         * 4xkk - SNE Vx, byte
         * Skip next instruction if Vx != kk.
         * @param x Register index.
         * @param kk Immediate byte.
         */
        void SNE_Vx_byte(uint8_t x, uint8_t kk);

        /**
         * 5xy0 - SE Vx, Vy
         * Skip next instruction if Vx == Vy.
         * @param x Register index.
         * @param y Register index.
         */
        void SE_Vx_Vy(uint8_t x, uint8_t y);

        /**
         * 6xkk - LD Vx, byte
         * Set Vx = kk.
         * @param x Register index.
         * @param kk Immediate byte.
         */
        void LD_Vx_byte(uint8_t x, uint8_t kk);

        /**
         * 7xkk - ADD Vx, byte
         * Set Vx = Vx + kk (no carry).
         * @param x Register index.
         * @param kk Immediate byte.
         */
        void ADD_Vx_byte(uint8_t x, uint8_t kk);

        /**
         * 8xy0 - LD Vx, Vy
         * Set Vx = Vy.
         * @param x Destination register index.
         * @param y Source register index.
         */
        void LD_Vx_Vy(uint8_t x, uint8_t y);

        /**
         * 8xy1 - OR Vx, Vy
         * Set Vx = Vx OR Vy. VF is not affected on most interpreters.
         * @param x Destination register index.
         * @param y Source register index.
         */
        void OR_Vx_Vy(uint8_t x, uint8_t y);

        /**
         * 8xy2 - AND Vx, Vy
         * Set Vx = Vx AND Vy. VF is not affected on most interpreters.
         * @param x Destination register index.
         * @param y Source register index.
         */
        void AND_Vx_Vy(uint8_t x, uint8_t y);

        /**
         * 8xy3 - XOR Vx, Vy
         * Set Vx = Vx XOR Vy. VF is not affected on most interpreters.
         * @param x Destination register index.
         * @param y Source register index.
         */
        void XOR_Vx_Vy(uint8_t x, uint8_t y);

        /**
         * 8xy4 - ADD Vx, Vy
         * Set Vx = Vx + Vy; VF = carry.
         * @param x Destination register index.
         * @param y Source register index.
         */
        void ADD_Vx_Vy(uint8_t x, uint8_t y);

        /**
         * 8xy5 - SUB Vx, Vy
         * Set Vx = Vx - Vy; VF = NOT borrow.
         * @param x Destination register index.
         * @param y Source register index.
         */
        void SUB_Vx_Vy(uint8_t x, uint8_t y);

        /**
         * 8xy6 - SHR Vx {, Vy}
         * Set Vx = Vx >> 1; VF = least-significant bit prior to shift.
         * @param x Register index (target).
         * @param y Register index (often ignored on modern interpreters).
         */
        void SHR_Vx_Vy(uint8_t x, uint8_t y);

        /**
         * 8xy7 - SUBN Vx, Vy
         * Set Vx = Vy - Vx; VF = NOT borrow.
         * @param x Destination register index.
         * @param y Source register index.
         */
        void SUBN_Vx_Vy(uint8_t x, uint8_t y);

        /**
         * 8xyE - SHL Vx {, Vy}
         * Set Vx = Vx << 1; VF = most-significant bit prior to shift.
         * @param x Register index (target).
         * @param y Register index (often ignored on modern interpreters).
         */
        void SHL_Vx_Vy(uint8_t x, uint8_t y);

        /**
         * 9xy0 - SNE Vx, Vy
         * Skip next instruction if Vx != Vy.
         * @param x Register index.
         * @param y Register index.
         */
        void SNE_Vx_Vy(uint8_t x, uint8_t y);

        /**
         * Annn - LD I, addr
         * Set I = nnn.
         * @param nnn Address immediate.
         */
        void LD_I_addr(uint16_t nnn);

        /**
         * Bnnn - JP V0, addr
         * Jump to nnn + V0.
         * @param nnn Base address.
         */
        void JP_V0_addr(uint16_t nnn);

        /**
         * Cxkk - RND Vx, byte
         * Set Vx = random_byte() AND kk.
         * @param x Register index.
         * @param kk Mask byte.
         */
        void RND_Vx_byte(uint8_t x, uint8_t kk);

        /**
         * Dxyn - DRW Vx, Vy, nibble
         * Display n-byte sprite at (Vx, Vy); set VF = collision.
         * @param x X register index.
         * @param y Y register index.
         * @param n Sprite height in bytes (nibble).
         */
        void DRW_Vx_Vy_nibble(uint8_t x, uint8_t y, uint8_t n);

        /**
         * Ex9E - SKP Vx
         * Skip next instruction if key with the value of Vx is pressed.
         * @param x Register index holding key value.
         */
        void SKP_Vx(uint8_t x);

        /**
         * ExA1 - SKNP Vx
         * Skip next instruction if key with the value of Vx is not pressed.
         * @param x Register index holding key value.
         */
        void SKNP_Vx(uint8_t x);

        /**
         * Fx07 - LD Vx, DT
         * Set Vx = delay timer value.
         * @param x Register index.
         */
        void LD_Vx_DT(uint8_t x);

        /**
         * Fx0A - LD Vx, K
         * Wait for a key press, store the value of the key in Vx.
         * @param x Register index.
         */
        void LD_Vx_K(uint8_t x);

        /**
         * Fx15 - LD DT, Vx
         * Set delay timer = Vx.
         * @param x Register index.
         */
        void LD_DT_Vx(uint8_t x);

        /**
         * Fx18 - LD ST, Vx
         * Set sound timer = Vx.
         * @param x Register index.
         */
        void LD_ST_Vx(uint8_t x);

        /**
         * Fx1E - ADD I, Vx
         * Set I = I + Vx.
         * @param x Register index.
         */
        void ADD_I_Vx(uint8_t x);

        /**
         * Fx29 - LD F, Vx
         * Set I = location of sprite for digit Vx.
         * @param x Register index (0x0-0xF digit).
         */
        void LD_F_Vx(uint8_t x);

        /**
         * Fx33 - LD B, Vx
         * Store BCD representation of Vx in memory at I..I+2.
         * @param x Register index.
         */
        void LD_B_Vx(uint8_t x);

        /**
         * Fx55 - LD [I], Vx
         * Store registers V0 through Vx in memory starting at I.
         * @param x Last register index to store (inclusive).
         */
        void LD_I_Vx(uint8_t x);

        /**
         * Fx65 - LD Vx, [I]
         * Read registers V0 through Vx from memory starting at I.
         * @param x Last register index to read (inclusive).
         */
        void LD_Vx_I(uint8_t x);
};
