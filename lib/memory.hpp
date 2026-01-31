#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cstdint>

extern "C" {
    extern void *memset(void *__src,int i,uint32_t n);
    extern void *memcpy(void *__dest,void *__src,uint32_t n);
}

namespace PAE {
    constexpr uint32_t EFER = 0xC0000080;
    constexpr uint32_t NXE  = (1u << 11);

    static inline void enable_nxe() {
        uint32_t lo, hi;
        __asm__ __volatile__ (
            "rdmsr\n\t"
            "or %[bit], %%eax\n\t"
            "wrmsr\n\t"
            : "=a"(lo), "=d"(hi)
            : "c"(EFER), [bit]"r"(NXE)
            : "memory"
        );
    }
}

#endif