[BITS 16]
[org 0x7C00]

DEFAULT_STACK_LIMIT_OFFSET equ 0x8000
DEFAULT_SECTOR_COUNT equ 25
BOOT_NEXT_SECTOR_offset equ 0x9000
BOOT_NEXT_SECTOR_segment equ 0x0000

  mov byte [ds:BOOT_DRIVE],dl

_start:

  cli
  xor ax,ax
  mov es,ax
  mov ds,ax
  mov ss,ax
  mov fs,ax
  mov gs,ax

  mov sp,DEFAULT_STACK_LIMIT_OFFSET
  cld

  mov ax,0x0003
  int 0x10

  in al,0x92
  or al,0x02
  out 0x92,al

  mov ah,0x02
  mov al,DEFAULT_SECTOR_COUNT
  xor ch,ch
  mov cl,2
  xor dh,dh

  mov dl,byte [ds:BOOT_DRIVE]
  mov bx,BOOT_NEXT_SECTOR_segment
  mov es,bx
  mov bx,BOOT_NEXT_SECTOR_offset
  int 0x13

  jc .ERROR_failed_load

  lea si,[ds:L1]
  call print

  jmp BOOT_NEXT_SECTOR_segment:BOOT_NEXT_SECTOR_offset

.ERROR_failed_load:

  lea si,[ds:L2]
  call print

hang:

  hlt

  jmp hang

print:

  mov ah,0x0E

.iter:

  lodsb
  or al,al
  jz .done
  int 0x10
  jmp .iter

.done:

  ret

L1: db "[INFO] : load next sector",0x0A,0x0D,0
L2: db "[INFO] : ERROR failed to boot",0x0A,0x0D,0

BOOT_DRIVE: db 0

times 510 - ($ - $$) db 0
dw 0xAA55


