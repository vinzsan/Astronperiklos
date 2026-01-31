#ifndef NON_ACPI_HPP
#define NON_ACPI_HPP

#include <cstdint>
#include "./iopl.hpp"

constexpr uint16_t QEMU_PORT = 0x604;
constexpr uint16_t SHUT_QEMU = 0x2000;

namespace N_ACPI {
    static inline void shutdown_vm(){
        IOCTL::out_byte<uint16_t>(QEMU_PORT,SHUT_QEMU);
    }
}

#endif