cmake_minimum_required(VERSION 3.10)
project(kernel LANGUAGES C CXX ASM_NASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FREESTANDING_FLAGS 
    -ffreestanding
    -fno-exceptions
    -fno-rtti
    -nostdlib
    -fno-stack-protector
    -fno-pie
    -fno-builtin
    -m16
)

enable_language(ASM_NASM)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)

add_library(memory_obj OBJECT ../lib/memory.cc)
target_compile_options(memory_obj PRIVATE ${FREESTANDING_FLAGS})
set_target_properties(memory_obj PROPERTIES POSITION_INDEPENDENT_CODE OFF)

file(GLOB SRC_FILES "src/*.cc" "src/*.cpp" "src/*.c" "src/*.s")

add_executable(kernel.elf ${SRC_FILES} $<TARGET_OBJECTS:memory_obj>)

target_compile_options(kernel.elf PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${FREESTANDING_FLAGS}>
    $<$<COMPILE_LANGUAGE:CXX>:${FREESTANDING_FLAGS}>
)

target_link_options(kernel.elf PRIVATE
    -T ${CMAKE_SOURCE_DIR}/link.ld
    -nostdlib
    -ffreestanding
    -Wl,-m elf_i386 -z noexecstack
)
add_custom_command(TARGET kernel.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary kernel.elf kernel.bin
    COMMENT "[INFO] : Creating raw kernel binary"
)
add_custom_target(bootloader
    COMMAND nasm -f bin ${CMAKE_SOURCE_DIR}/src/main.asm -o boot.bin
    COMMENT "[INFO] : Building bootloader"
)
add_custom_target(image ALL
    COMMAND dd if=/dev/zero of=os.img bs=512 count=2880
    COMMAND dd if=boot.bin of=os.img conv=notrunc
    COMMAND dd if=kernel.bin of=os.img seek=1 conv=notrunc
    DEPENDS kernel.elf bootloader
    COMMENT "[INFO] : Creating bootable OS image"
)

add_custom_target(run
    COMMAND qemu-system-i386 -drive format=raw,file=os.img
    DEPENDS image
)
