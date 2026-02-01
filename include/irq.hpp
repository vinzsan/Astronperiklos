#ifndef IRQ_HPP
#define IRQ_HPP

#include <cstdint>

#include "./iopl.hpp"
#include "./kstd.hpp"

static uint32_t static_ticks = 0;

/*
extern "C" __attribute__((naked)) void timer_handler() {
    __asm__ __volatile__(
        "pusha\n\t"
        "incw ticks\n\t"
        "movb $0x20, %al\n\t"
        "outb %al, $0x20\n\t"
        "popa\n\t"
        "iret\n\t"
    );
}

extern "C" __attribute__((naked))
void setup_timer_ivt(void) {
    __asm__ __volatile__(
        "cli\n\t"\
        "movw $timer_handler, %%ax\n\t"
        "movw %%ax, 0x20\n\t"   
        "movw $0x9000, %%ax\n\t"
        "movw %%ax, 0x22\n\t"   
        "sti\n\t"
        "ret\n\t"
        :::"ax"
    );
}
*/

namespace PIT_pack {
    constexpr uint32_t DIVISOR = 1193182;

    inline void pit_setup_init(uint32_t __freq){
        uint32_t div = DIVISOR / __freq;
        IOCTL::out_byte<uint8_t>(0x43,0x36);
        IOCTL::out_byte<uint8_t>(0x40,div & 0xFF);
        IOCTL::out_byte<uint8_t>(0x40,(div >> 8) & 0xFF);
    }
    
    inline void pic_umask_tim(){
        uint8_t mask = IOCTL::in_byte<uint8_t>(0x21);
        mask &= ~0x01;
        IOCTL::out_byte<uint8_t>(0x21,mask);
    }

    inline void send_eoi(){
        IOCTL::out_byte<uint8_t>(0x20,0x20);
    }
    inline void sleep_iter(uint32_t ms){
        uint32_t _result = ms * 100000;
        while(_result > 0){
            __asm__ __volatile__("nop");
            _result--;
        }
    }
}

#endif