#include "Chip8.h"
#include <cstring>
#include <fstream>

Chip8::Chip8() {
    programCounter = ROM_ADDRESS;
    for (unsigned int i = 0; i < 80; ++i)
    {
        memory[FONT_ADDRESS + i] = fontset[i];
    }
}

void Chip8::Load_ROM(const char* path) {
    std::ifstream ifs(path, std::ios::binary);
    if (ifs.is_open()) {
        ifs.seekg(0, std::ios::end);
        const std::streampos fileSize = ifs.tellg();
        //Handle oversized ROMs
        if (fileSize > MEMORY_SIZE - ROM_ADDRESS){
            return;
        }
        ifs.seekg(0, std::ios::beg);
        char buffer[MEMORY_SIZE];
        ifs.read(buffer, fileSize);
        ifs.close();
        memcpy(memory + ROM_ADDRESS, buffer, fileSize);
    }
}

void Chip8::Cycle() {
    auto opcode = memory[programCounter] << 8 | memory[programCounter + 1];
    programCounter += 2;
    execute_instruction(opcode);
    if (delayTimer > 0)
    {
        --delayTimer;
    }
    if (soundTimer > 0)
    {
        --soundTimer;
    }
}

void Chip8::execute_instruction(uint16_t opcode) {

    //WARNING: This does additional work for instructions like jp_addr.
    //MULTIPLE cpu instructions spent decoding unused opcode segments. Definitely won't matter on desktop
    uint16_t addr = opcode & 0x0FFFu;
    uint8_t nibble = opcode & 0x0Fu;
    uint8_t x = (opcode & 0x0F00u) >> 8;
    uint8_t y = (opcode & 0x00F0u) >> 4;
    uint8_t byte = opcode & 0x00FFu;

    //TODO: investigate alternatives to nested switches.
    //TODO: Not sure if there is any reason to change this. I just don't like the look
    switch ((opcode & 0xF000) >> 12) {
        case 0: switch (byte) {
            case 0xE0: CLS(); break;
            case 0xEE: RET(); break;
            default: break;
        }
        default: break;

        case 1: JP_addr(addr); break;
        case 2: CALL_addr(addr); break;
        case 3: SE_Vx_byte(x, byte); break;
        case 4: SNE_Vx_byte(x, byte); break;
        case 5: SE_Vx_Vy(x, y); break;
        case 6: LD_Vx_byte(x, byte); break;

        case 7: ADD_Vx_byte(x, byte); break;
        case 8: switch (nibble) {
            case 0: LD_Vx_Vy(x, y); break;
            case 1: OR_Vx_Vy(x, y); break;
            case 2: AND_Vx_Vy(x, y); break;
            case 3: XOR_Vx_Vy(x, y); break;
            case 4: ADD_Vx_Vy(x, y); break;
            case 5: SUB_Vx_Vy(x, y); break;
            case 6: SHR_Vx_Vy(x, y); break;
            case 7: SUBN_Vx_Vy(x, y); break;
            case 0xE: SHL_Vx_Vy(x, y); break;
            default: break;
        } break;
        case 9: SNE_Vx_Vy(x, y); break;
        case 0xA: LD_I_addr(addr); break;
        case 0xB: JP_V0_addr(addr); break;
        case 0xC: RND_Vx_byte(x, byte); break;
        case 0xD: DRW_Vx_Vy_nibble(x, y, nibble); break;
        case 0xE: switch (byte) {
            case 0x9E: SKP_Vx(x); break;
            case 0xA1: SKNP_Vx(x); break;
            default: break;
        } break;
        case 0xF: switch (byte) {
            case 0x07: LD_Vx_DT(x); break;
            case 0x0A: LD_Vx_K(x); break;
            case 0x15: LD_DT_Vx(x); break;
            case 0x18: LD_ST_Vx(x); break;
            case 0x1E: ADD_I_Vx(x); break;
            case 0x29: LD_F_Vx(x); break;
            case 0x33: LD_B_Vx(x); break;
            case 0x55: LD_I_Vx(x); break;
            case 0x65: LD_Vx_I(x); break;
            default: break;
        } break;
    }
}

void Chip8::SYS_addr(uint16_t nnn) {
    (void)nnn;
}

void Chip8::CLS() {
    memset(display, 0, (DISPLAY_WIDTH * DISPLAY_HEIGHT) / 8);
}

void Chip8::RET() {
    programCounter = stack[stackPointer];
    stackPointer --;
}

void Chip8::JP_addr(uint16_t nnn) {
    programCounter = nnn;
}
void Chip8::CALL_addr(uint16_t nnn) {
    stackPointer ++;
    stack[stackPointer] = programCounter;
    programCounter = nnn;
}

void Chip8::SE_Vx_byte(uint8_t x, uint8_t kk) {
    if (registers[x] == kk) {
        programCounter += 2;
    }
}
void Chip8::SNE_Vx_byte(uint8_t x, uint8_t kk) {
    if (registers[x] != kk) {
        programCounter += 2;
    }
}
void Chip8::SE_Vx_Vy(uint8_t x, uint8_t y) {
    if (registers[x] == registers[y]) {
        programCounter += 2;
    }
}

void Chip8::LD_Vx_byte(uint8_t x, uint8_t kk) {
    registers[x] = kk;
}

void Chip8::ADD_Vx_byte(uint8_t x, uint8_t kk) {
    registers[x] += kk;
}

void Chip8::LD_Vx_Vy(uint8_t x, uint8_t y) {
    registers[x] = registers[y];
}
void Chip8::OR_Vx_Vy(uint8_t x, uint8_t y) {
    registers[x] |= registers[y];
}
void Chip8::AND_Vx_Vy(uint8_t x, uint8_t y) {
    registers[x] &= registers[y];
}
void Chip8::XOR_Vx_Vy(uint8_t x, uint8_t y) {
    registers[x] ^= registers[y];
}
void Chip8::ADD_Vx_Vy(uint8_t x, uint8_t y) {
    const uint8_t temp { registers[x] };
    registers[x] += registers[y];
    registers[0xF] = temp > registers[x] ? 1 : 0;
}

void Chip8::SUB_Vx_Vy(uint8_t x, uint8_t y) {
    const uint8_t temp {registers [x]};
    registers[x] -= registers[y];
    registers[0xF] = temp >= registers[y] ? 1 : 0; //Flag must be set last to handle case where x = F
}

void Chip8::SHR_Vx_Vy(uint8_t x, uint8_t y) {
    const uint8_t temp {registers[x]};
    registers[y] = registers[x];
    registers[x] >>= 1;
    registers[0xF] = temp & 0x1;
}

void Chip8::SUBN_Vx_Vy(uint8_t x, uint8_t y) {
    const uint8_t temp {registers[x]};
    registers[x] = registers[y] - registers[x];
    registers[0xF] = registers[y] >= temp;
}

void Chip8::SHL_Vx_Vy(uint8_t x, uint8_t y) {
    const uint8_t temp {registers[x]};
    registers[y] = registers[x];
    registers[x] <<= 1;
    registers[0xF] = temp >> 7;
}

void Chip8::SNE_Vx_Vy(uint8_t x, uint8_t y) {
    programCounter += registers[x] != registers[y] ? 2 : 0;
}

void Chip8::LD_I_addr(uint16_t nnn) {
    i = nnn;
}

void Chip8::DRW_Vx_Vy_nibble(uint8_t x, uint8_t y, uint8_t n) {
    auto vx = registers[x] % DISPLAY_WIDTH;
    auto vy = registers[y] % DISPLAY_HEIGHT;
    for (int row = 0; row < n; row++) {
        auto spriteByte = memory[i + row];
        //index of first bit
        auto bitIndex = vx + (vy + row) % DISPLAY_HEIGHT * DISPLAY_WIDTH;
        //first byte
        auto byte = bitIndex >> 3;
        auto bitOffset = vx & 7;

        auto leftSprite = spriteByte >> bitOffset;
        bool collisionLeft = display[byte] & leftSprite;
        display[byte] ^= leftSprite;
        bool collisionRight {};
        if (bitOffset) {
            auto rightByte = byte + 1 - ((byte & 7) == 7) * 8;
            auto rightSprite = spriteByte << (8 - bitOffset);
            collisionRight = display[rightByte] & rightSprite;
            display[rightByte] ^= rightSprite;
        }
        registers[0xF] |=  collisionLeft | collisionRight;
    }
}

void Chip8::SKP_Vx(uint8_t x) {
    if (keyboard[registers[x]]) {
        programCounter += 2;
    }
}

void Chip8::SKNP_Vx(uint8_t x) {
    if (!keyboard[registers[x]]) {
        programCounter += 2;
    }
}

void Chip8::LD_Vx_DT(uint8_t x) {
    registers[x] = delayTimer;
}

void Chip8::LD_Vx_K(uint8_t x) {
    if (waitingForKeyRelease >= 0) {
        if (keyboard[waitingForKeyRelease]) {
            programCounter -= 2;
        } else {
            waitingForKeyRelease = -1;
        }
    } else {
        auto key {-1};
        for (int i = 0; i < KEYBOARD_SIZE; i++) {
            if (keyboard[i]) {
                waitingForKeyRelease = i;
                break;
            }
        }
        if (key == -1) {
            programCounter -= 2;
        } else {
            registers[x] = key;
            waitingForKeyRelease = key;
            programCounter -= 2;
        }
    }
}

void Chip8::LD_DT_Vx(uint8_t x) {
    delayTimer = registers[x];
}

void Chip8::LD_ST_Vx(uint8_t x) {
    soundTimer = registers[x];
}

void Chip8::ADD_I_Vx(uint8_t x) {
    i += registers[x];
}

void Chip8::LD_F_Vx(uint8_t x) {
    //TODO find a start point for fontset, allegedly 0x50
    i = 0x50 + registers[x] * 5;
}

void Chip8::LD_B_Vx(uint8_t x) {
    auto value = registers[x];
    memory[i + 2] = value % 10;
    value /= 10;

    memory[i + 1] = value % 10;
    value /= 10;

    memory[i] = value % 10;
}

void Chip8::LD_I_Vx(uint8_t x) {
    memcpy(memory + i, &registers, sizeof(registers[0]) * (x + 1));
}
void Chip8::LD_Vx_I(uint8_t x) {
    memcpy(registers, memory + i, sizeof(registers[0]) * (x + 1));
}

void Chip8::JP_V0_addr(uint16_t nnn) {
    programCounter = nnn + registers[0x0];
}

void Chip8::RND_Vx_byte(uint8_t x, uint8_t kk) {
    registers[x] = rand() % 256 & kk;
}



