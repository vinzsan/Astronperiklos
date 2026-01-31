ENTRY(_start)

SECTIONS {
         . = 0x9000;

         .text : {
               *(.kernel_entry)
               *(.text)
         }

         .rodata : { *(.rodata)}
         .data : { *(.data)}
         .bss : { *(.bss)}
}
