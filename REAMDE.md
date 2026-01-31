# x86-16 Shell OS

## Daftar Isi

- [Tentang Proyek](#tentang-proyek)
- [Fitur Utama](#fitur-utama)
- [Arsitektur Sistem](#arsitektur-sistem)
- [Prasyarat](#prasyarat)
- [Instalasi](#instalasi)
- [Cara Menggunakan](#cara-menggunakan)
- [Perintah yang Tersedia](#perintah-yang-tersedia)
- [Struktur Proyek](#struktur-proyek)
- [Detail Teknis](#detail-teknis)
- [Pengembangan](#pengembangan)
- [Troubleshooting](#troubleshooting)
- [Kontribusi](#kontribusi)
- [Lisensi](#lisensi)

---

## Tentang Proyek

**x86-16 Shell OS** adalah proyek sistem operasi minimal yang berjalan dalam mode real 16-bit (x86-16) dengan antarmuka shell interaktif berbasis teks. Proyek ini dirancang sebagai pembelajaran mendalam tentang pemrograman sistem tingkat rendah, bootloader, dan interaksi langsung dengan perangkat keras x86.

Meskipun disebut sebagai "shell" dan bukan sistem operasi lengkap, proyek ini mencakup komponen-komponen fundamental dari sebuah OS seperti bootloader, kernel space, manajemen interrupt, driver keyboard, dan output VGA text mode. Proyek ini ditulis dalam kombinasi Assembly x86 (untuk bootloader) dan C++ dengan inline assembly (untuk kernel).

### Tujuan Pembelajaran

Proyek ini cocok untuk:
- Memahami proses booting komputer dari awal (POST hingga kernel loading)
- Belajar pemrograman bare-metal tanpa sistem operasi host
- Menguasai Assembly x86 dan inline assembly dalam C++
- Memahami interaksi langsung dengan hardware (VGA, keyboard, PIT, dll)
- Eksplorasi arsitektur x86 mode real dan protected mode
- Praktik system programming dan embedded development

---

## Fitur Utama

### 1. **Bootloader Custom**
- Bootloader 512-byte yang ditulis dalam Assembly murni
- Mendukung loading multi-sector dari disk
- Deteksi dan error handling untuk boot failure
- Aktivasi A20 gate untuk akses extended memory
- Konfigurasi segment registers dan stack

### 2. **Shell Interaktif**
- Command-line interface dengan prompt yang menampilkan nomor baris
- Input keyboard real-time dengan dukungan shift key
- Buffer input yang aman dengan ukuran 4KB
- Parsing dan eksekusi perintah dinamis
- Echo karakter yang diketik pengguna

### 3. **Driver Keyboard**
- Pembacaan scan code langsung dari port 0x60
- Konversi scan code ke ASCII (lowercase dan uppercase)
- Dukungan tombol shift untuk huruf kapital dan simbol
- Handling key press dan key release events
- Non-blocking keyboard input

### 4. **VGA Text Mode Output**
- Output teks ke VGA framebuffer (0xB8000)
- Dukungan warna teks dan background (16 warna)
- Scrolling otomatis saat mencapai batas layar
- Cursor hardware yang dapat diposisikan
- Clear screen dengan warna custom
- Fungsi `write_screen()` dan `putchar_screen()`

### 5. **Sistem Perintah (Commands)**
Perintah bawaan yang dapat dijalankan:
- `help` - Menampilkan bantuan
- `shutdown` - Mematikan VM (QEMU)
- `clear` - Membersihkan layar
- `version` - Menampilkan versi OS
- `start` - Menampilkan ASCII art pohon natal

### 6. **Hardware Abstraction**
- A20 Gate management untuk akses memori extended
- PIT (Programmable Interval Timer) setup
- Port I/O operations (in/out instructions)
- VGA cursor control register manipulation

### 7. **Utility Functions**
- String comparison (`strcompare`)
- Integer to string conversion (`int2str`)
- Memory operations
- Color macros untuk output berwarna

---

## Arsitektur Sistem

### Diagram Boot Process

```
┌─────────────────────┐
│   BIOS POST         │
│   (Power-On Self    │
│    Test)            │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│  BIOS membaca       │
│  sektor 1 (512 byte)│
│  ke 0x7C00          │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│  Bootloader         │
│  (src/main.asm)     │
│  - Setup segments   │
│  - Enable A20       │
│  - Load 25 sectors  │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│  Jump ke 0x9000     │
│  (Kernel entry)     │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│  Kernel (_start)    │
│  (src/kernel.cc)    │
│  - Setup stack      │
│  - Init hardware    │
│  - Clear screen     │
│  - Enter shell loop │
└─────────────────────┘
```

### Memory Layout

```
0x0000 - 0x03FF   : Interrupt Vector Table (IVT)
0x0400 - 0x04FF   : BIOS Data Area (BDA)
0x0500 - 0x7BFF   : Free conventional memory
0x7C00 - 0x7DFF   : Bootloader (512 bytes)
0x7E00 - 0x8FFF   : Free memory / stack grows down
0x9000 - 0x????   : Kernel code & data
0xA0000- 0xBFFFF  : Video memory
0xB8000- 0xBFFFF  : VGA text mode buffer (80x25)
0xC0000- 0xFFFFF  : BIOS ROM
```

### Komponen Arsitektur

#### 1. Bootloader (`src/main.asm`)
- **Ukuran**: Tepat 512 bytes dengan magic number `0xAA55`
- **Fungsi**:
  - Inisialisasi segment registers (DS, ES, SS, FS, GS)
  - Setup stack pointer di `0x8000`
  - Aktivasi A20 gate via port `0x92`
  - Load 25 sektor kernel dari disk menggunakan INT 13h
  - Jump ke alamat kernel di `0x9000:0x9000`
- **Error Handling**: Pesan error jika disk read gagal

#### 2. Kernel (`src/kernel.cc`)
- **Entry Point**: Fungsi `_start()` dengan atribut `section(".kernel_entry")`
- **Inisialisasi**:
  - Disable interrupt (`cli`)
  - Setup segment registers
  - Setup stack di `0x9000`
  - Enable NX (No-Execute) bit untuk keamanan
  - Konfigurasi PIT untuk timer
  - Clear VGA screen
- **Main Loop**: REPL (Read-Eval-Print Loop) untuk shell

#### 3. Header Files

**`include/kstd.hpp`**: Standard library custom
- Namespace `kstd::str` untuk string operations
- Namespace `kstd::KeyboardStatus` untuk keyboard driver
- Namespace `kstd::VGA_framebuffer` untuk VGA operations
- Namespace `kstd::arch` untuk arsitektur-specific code

**`include/iopl.hpp`**: I/O Port Operations
- Template functions untuk `in` dan `out` instructions
- Type-safe port I/O

**`include/irq.hpp`**: Interrupt & Timer
- PIT (Programmable Interval Timer) setup
- PIC (Programmable Interrupt Controller) configuration
- IRQ handling utilities

**`include/non_acpi.hpp`**: Non-ACPI Power Management
- Shutdown untuk QEMU via port `0x604`

---

## Prasyarat

Untuk mengompilasi dan menjalankan proyek ini, Anda memerlukan:

### Perangkat Lunak Wajib

1. **Compiler & Toolchain**
   ```bash
   # GCC dengan dukungan multilib (untuk 32-bit compilation)
   sudo apt install gcc g++ gcc-multilib g++-multilib
   
   # Atau untuk distro berbasis Arch
   sudo pacman -S gcc gcc-multilib
   ```

2. **Assembler**
   ```bash
   # NASM (Netwide Assembler)
   sudo apt install nasm
   
   # Atau
   sudo pacman -S nasm
   ```

3. **Build System**
   ```bash
   # CMake (versi 3.10 atau lebih baru)
   sudo apt install cmake
   
   # Atau
   sudo pacman -S cmake
   ```

4. **Binary Utilities**
   ```bash
   # GNU Binutils
   sudo apt install binutils
   
   # Atau sudah terinstall dengan gcc
   ```

5. **Emulator (Opsional tapi Sangat Direkomendasikan)**
   ```bash
   # QEMU untuk menjalankan OS image
   sudo apt install qemu-system-x86
   
   # Atau untuk Arch
   sudo pacman -S qemu-full
   ```

### Versi yang Direkomendasikan
- GCC: 9.0 atau lebih baru
- NASM: 2.14 atau lebih baru
- CMake: 3.10 atau lebih baru
- QEMU: 4.0 atau lebih baru

### Sistem Operasi Host
Proyek ini telah diuji pada:
- Ubuntu 20.04 LTS / 22.04 LTS
- Arch Linux
- Debian 11+
- Fedora 35+

**Catatan untuk Windows**: Disarankan menggunakan WSL2 (Windows Subsystem for Linux) atau virtual machine Linux.

**Catatan untuk macOS**: Memerlukan Homebrew untuk instalasi tools:
```bash
brew install nasm cmake qemu gcc
```

---

## Instalasi

### 1. Clone atau Download Proyek

```bash
# Jika menggunakan git
git clone <repository-url>
cd x86-16-shell-os

# Atau extract file zip/tar
unzip x86-16-shell-os.zip
cd x86-16-shell-os
```

### 2. Verifikasi Struktur Direktori

Pastikan struktur direktori seperti ini:

```
.
├── CMakeLists.txt
├── link.ld
├── src/
│   ├── main.asm
│   └── kernel.cc
├── include/
│   ├── kstd.hpp
│   ├── iopl.hpp
│   ├── irq.hpp
│   └── non_acpi.hpp
└── lib/
    └── memory.cc (jika ada)
```

### 3. Build Project

```bash
# Buat direktori build
mkdir build
cd build

# Generate build files dengan CMake
cmake ..

# Compile semua target
make

# Atau hanya build image OS
make image
```

### 4. Output yang Dihasilkan

Setelah build berhasil, Anda akan mendapatkan:
- `boot.bin` - Bootloader (512 bytes)
- `kernel.elf` - Kernel dalam format ELF
- `kernel.bin` - Kernel binary raw
- `os.img` - Bootable disk image (1.44MB floppy format)

---

## Cara Menggunakan

### Menjalankan di QEMU (Emulator)

#### Cara 1: Menggunakan Make Target

```bash
# Dari direktori build
make run
```

Ini akan otomatis menjalankan QEMU dengan konfigurasi:
```bash
qemu-system-i386 -drive format=raw,file=os.img
```

#### Cara 2: Manual dengan Opsi Tambahan

```bash
# Dengan tampilan grafis
qemu-system-i386 -drive format=raw,file=os.img

# Dengan serial output (untuk debugging)
qemu-system-i386 -drive format=raw,file=os.img -serial stdio

# Dengan KVM acceleration (Linux host)
qemu-system-i386 -drive format=raw,file=os.img -enable-kvm

# Dengan memory tertentu
qemu-system-i386 -drive format=raw,file=os.img -m 16M
```

### Menjalankan di Hardware Real (Opsional)

**PERINGATAN**: Booting di hardware real dapat berisiko. Lakukan dengan hati-hati!

#### Menggunakan USB Flash Drive

```bash
# HATI-HATI: Ini akan menghapus semua data di USB!
# Pastikan /dev/sdX adalah USB drive Anda

# Cek device USB
lsblk

# Write image ke USB (jalankan sebagai root)
sudo dd if=os.img of=/dev/sdX bs=512 status=progress

# Sync untuk memastikan write selesai
sudo sync
```

Kemudian:
1. Restart komputer
2. Masuk ke BIOS/UEFI (biasanya F2, F12, Del, atau ESC)
3. Pilih boot dari USB
4. OS akan booting

#### Menggunakan CD/DVD (ISO)

Untuk membuat ISO bootable:
```bash
# Install xorriso
sudo apt install xorriso

# Buat ISO
xorriso -as mkisofs -b os.img -no-emul-boot -boot-load-size 4 \
        -o bootable.iso os.img

# Burn ke CD atau mount di VM
```

### Berinteraksi dengan Shell

Setelah boot berhasil, Anda akan melihat:

```
[INFO] A20 Gate : <nilai>
[INFO] A20 Gate After : <nilai_baru>


Welcome to x86-16 shell
Shell [0] : >> _
```

Ketik perintah dan tekan Enter untuk menjalankan.

---

## Perintah yang Tersedia

### 1. `help`

**Deskripsi**: Menampilkan pesan bantuan yang menjelaskan perintah-perintah yang tersedia.

**Penggunaan**:
```
Shell [0] : >> help
```

**Output**:
```
Simple x86_16 shell (not an OS dawg)

-help (to show ts message)
```

---

### 2. `shutdown`

**Deskripsi**: Mematikan virtual machine. Perintah ini bekerja dengan mengirim sinyal shutdown ke QEMU melalui port I/O `0x604`.

**Penggunaan**:
```
Shell [0] : >> shutdown
```

**Catatan**: 
- Hanya bekerja di QEMU/emulator
- Tidak akan bekerja di hardware real
- Menggunakan mekanisme non-ACPI

**Detail Teknis**:
```cpp
// Mengirim 0x2000 ke port 0x604 (QEMU shutdown port)
IOCTL::out_byte<uint16_t>(0x604, 0x2000);
```

---

### 3. `clear`

**Deskripsi**: Membersihkan layar dan mereset posisi cursor ke (0,0). Juga mereset counter baris shell ke 0.

**Penggunaan**:
```
Shell [5] : >> clear
```

**Efek**:
- Layar menjadi kosong
- Cursor kembali ke kiri atas
- Counter shell kembali ke [0]

---

### 4. `version`

**Deskripsi**: Menampilkan informasi versi dari OS shell ini.

**Penggunaan**:
```
Shell [0] : >> version
```

**Output**:
```
version 0.1 minor update
```

---

### 5. `start`

**Deskripsi**: Menampilkan ASCII art berupa pohon natal berwarna hijau.

**Penggunaan**:
```
Shell [0] : >> start
```

**Output**:
```
         
        █
       ███
      █████
     ███████
    █████████
   ███████████
  █████████████
 ███████████████
█████████████████
        |
        |
```

**Detail Implementasi**:
- Tinggi pohon: 10 baris
- Warna daun: Hijau (color code `0x2A`)
- Warna batang: Merah (color code `0x4`)
- Otomatis center di tengah layar (80 kolom)

---

### Perintah Tidak Dikenali

Jika Anda mengetik perintah yang tidak ada dalam daftar, shell akan mem-echo kembali input Anda:

```
Shell [0] : >> hello
hello
Shell [1] : >>
```

---

## Struktur Proyek

```
x86-16-shell-os/
│
├── CMakeLists.txt              # Build configuration
├── link.ld                     # Linker script untuk memory layout
├── README.md                   # Dokumentasi (file ini)
│
├── src/                        # Source code
│   ├── main.asm                # Bootloader 16-bit
│   └── kernel.cc               # Kernel utama dalam C++
│
├── include/                    # Header files
│   ├── kstd.hpp                # Standard library (VGA, keyboard, string)
│   ├── iopl.hpp                # I/O port operations
│   ├── irq.hpp                 # Interrupt & timer handling
│   └── non_acpi.hpp            # Power management non-ACPI
│
├── lib/                        # Library implementations
│   └── memory.cc               # Memory management functions
│
└── build/                      # Build output (generated)
    ├── boot.bin                # Compiled bootloader
    ├── kernel.bin              # Raw kernel binary
    ├── kernel.elf              # ELF format kernel
    └── os.img                  # Bootable disk image
```

### Penjelasan File

#### `CMakeLists.txt`
File konfigurasi CMake yang mengatur:
- Target kompilasi: `kernel.elf`, `bootloader`, `image`
- Compiler flags untuk freestanding environment
- Linker options dengan custom linker script
- Custom commands untuk membuat binary dan disk image
- Target `run` untuk menjalankan di QEMU

#### `link.ld`
Linker script yang mendefinisikan:
- Entry point di `_start`
- Load address kernel di `0x9000`
- Layout sections: `.text`, `.rodata`, `.data`, `.bss`
- Urutan loading sections

#### `src/main.asm`
Bootloader dalam Assembly NASM:
- Mode 16-bit dengan origin `0x7C00`
- Setup segment registers dan stack
- Enable A20 gate
- Load 25 sectors dari disk ke memory `0x9000`
- Jump ke kernel entry point
- Boot signature `0xAA55` di byte 510-511

#### `src/kernel.cc`
Kernel utama dalam C++ dengan inline assembly:
- Entry point `_start()` dengan setup awal
- Command parsing dan execution
- Keyboard input handling
- VGA text output
- Main shell loop (REPL)
- Command handlers untuk setiap perintah

#### `include/kstd.hpp`
Custom standard library yang berisi:
- **VGA_framebuffer**: Fungsi untuk manipulasi VGA text mode
  - `write_screen()`: Menulis string ke layar
  - `putchar_screen()`: Menulis satu karakter
  - `clear_screen()`: Membersihkan layar
  - `set_cursor_screen()`: Set posisi cursor
  - `scroll_cursor_screen()`: Scroll layar satu baris
- **KeyboardStatus**: Driver keyboard
  - `read_non_blocking()`: Membaca scan code dari keyboard
- **str**: String utilities
  - `strcompare()`: Membandingkan string
  - `int2str()`: Konversi integer ke string
- **arch**: Arsitektur-specific functions
  - `a20_gate_set()`: Enable/disable A20 gate

#### `include/iopl.hpp`
Template functions untuk I/O port operations:
- `in_byte<T>()`: Membaca dari I/O port
- `out_byte<T>()`: Menulis ke I/O port
- Type-safe dengan C++ templates

#### `include/irq.hpp`
Interrupt dan timer management:
- PIT (Programmable Interval Timer) setup
- IRQ configuration
- Timer interrupt handler (commented out)

#### `include/non_acpi.hpp`
Power management non-ACPI:
- Fungsi shutdown untuk QEMU via port `0x604`

---

## Detail Teknis

### Mode Operasi

Proyek ini berjalan dalam **Real Mode 16-bit**, yaitu:
- Segmented memory model (segment:offset)
- Akses langsung ke hardware
- Maksimal 1 MB memory (dengan A20 gate bisa sampai ~1 MB lebih)
- Tidak ada memory protection
- Tidak ada multitasking atau multi-user

### Kompilasi dengan GCC untuk 16-bit

Normalnya GCC menghasilkan kode 32-bit atau 64-bit. Untuk menghasilkan kode 16-bit:
```cpp
__asm__(".code16gcc");  // Directive untuk GCC
```

Dan compiler flags:
```cmake
-m16              # Generate 16-bit code
-ffreestanding    # No standard library
-fno-exceptions   # Disable C++ exceptions
-fno-rtti         # Disable RTTI
-nostdlib         # No standard library linking
```

### VGA Text Mode

VGA text mode `80x25` memiliki karakteristik:
- **Framebuffer address**: `0xB8000`
- **Size**: 80 kolom × 25 baris = 2000 karakter
- **Memory size**: 2000 × 2 bytes = 4000 bytes
- **Format per karakter**: 2 bytes
  - Byte 0: ASCII character
  - Byte 1: Attribute (color)

**Attribute byte format**:
```
Bit:  7 6 5 4 | 3 2 1 0
      B F F F | B B B B
      ↑ └─┬─┘ | └──┬──┘
      │   │   |    └─── Background color (0-7)
      │   │   └──────── Blinking bit (jika enabled)
      │   └──────────── Foreground color (0-15)
      └──────────────── Blinking/Bright background
```

**Warna yang tersedia**:
```
0 = Black        8 = Dark Gray
1 = Blue         9 = Light Blue
2 = Green        A = Light Green
3 = Cyan         B = Light Cyan
4 = Red          C = Light Red
5 = Magenta      D = Light Magenta
6 = Brown        E = Yellow
7 = Light Gray   F = White
```

Contoh:
- `0x0F` = White text on black background
- `0x1F` = White text on blue background (alert)
- `0x4F` = White text on red background (error)
- `0x2A` = Light green text on green background

### Keyboard Scan Code

Keyboard mengirimkan **scan codes** (bukan ASCII) melalui port `0x60`:

**Make code** (key press): Scan code normal
**Break code** (key release): Scan code + 0x80

Contoh:
- 'A' key press: `0x1E`
- 'A' key release: `0x9E` (`0x1E + 0x80`)
- Left Shift press: `0x2A`
- Left Shift release: `0xAA`

**Konversi Scan Code ke ASCII**:
```cpp
// Array lookup table
char scancode_to_ascii_lower_case[128];
char scancode_to_ascii_upper_case[128];

// Usage
char ascii = scancode_to_ascii_lower_case[scan_code];
```

### A20 Gate

**A20 Gate** adalah mekanisme untuk mengakses memory di atas 1 MB.

Di era IBM PC/XT (8086), address bus hanya 20-bit = 1 MB max.
Pada 8086, address `0xFFFF:0xFFFF` = `0x10FFEF` wraps around ke `0x0FFEF`.

Software lama bergantung pada wrap-around ini. Ketika 286 (24-bit) dan 386 (32-bit) keluar, untuk backward compatibility, A20 line disabled by default.

**Enable A20 via port 0x92** (Fast A20 gate):
```cpp
uint8_t value = in_byte(0x92);
value |= 0x02;           // Set bit 1
out_byte(0x92, value);   // Enable A20
```

Alternatif metode:
1. **Keyboard Controller** (port `0x64`, `0x60`) - slower, compatible
2. **Fast A20** (port `0x92`) - faster, used here
3. **BIOS INT 15h** - tidak tersedia setelah boot

### PIT (Programmable Interval Timer)

PIT chip 8253/8254 digunakan untuk:
- System timer (IRQ 0)
- Speaker tones
- Real-time clock

**Channel 0** (IRQ 0 timer):
- Port `0x40`: Data port
- Port `0x43`: Command port
- Default frequency: 1.193182 MHz

**Setup PIT**:
```cpp
uint32_t divisor = 1193182 / desired_frequency;
out_byte(0x43, 0x36);           // Command: Channel 0, mode 3
out_byte(0x40, divisor & 0xFF); // Low byte
out_byte(0x40, divisor >> 8);   // High byte
```

Mode 3 = Square wave generator

### Interrupt Vector Table (IVT)

Di Real Mode, IVT berada di `0x0000` - `0x03FF` (1024 bytes).
Setiap entry 4 bytes (segment:offset).

**Entry format**:
```
Offset 0: IP (Instruction Pointer) low byte
Offset 1: IP high byte
Offset 2: CS (Code Segment) low byte
Offset 3: CS high byte
```

**IRQ Mapping**:
- IRQ 0 (Timer): INT 0x08
- IRQ 1 (Keyboard): INT 0x09
- IRQ 2: INT 0x0A
- ...dll

### Memory Segmentation

Real mode menggunakan **segment:offset** addressing:
```
Physical Address = (Segment << 4) + Offset
                 = (Segment * 16) + Offset
```

Contoh:
```
0x7C00:0x0000 = 0x7C00 * 16 + 0 = 0x7C000
0x0000:0x7C00 = 0x0000 * 16 + 0x7C00 = 0x7C00
```

Kedua alamat berbeda di Real Mode tetapi menunjuk lokasi fisik yang sama!

**Segment Registers**:
- **CS**: Code Segment (untuk fetch instructions)
- **DS**: Data Segment (default untuk data access)
- **ES**: Extra Segment (untuk string operations, dll)
- **SS**: Stack Segment (untuk stack operations)
- **FS, GS**: General purpose segments (386+)

### Stack dalam Real Mode

Stack tumbuh **ke bawah** (dari high address ke low address):
```
      High Memory
      ┌──────────┐
SP -> │   Free   │  ← Stack grows downward
      ├──────────┤
      │   Data   │
      ├──────────┤
      │   Data   │
      └──────────┘
      Low Memory
```

**Operasi Stack**:
- `PUSH`: Decrement SP, lalu store
- `POP`: Load, lalu increment SP

Di proyek ini, stack di-setup di `0x9000`:
```asm
mov sp, 0x9000
```

### Calling Convention

Proyek ini menggunakan `extern "C"` untuk beberapa fungsi agar:
- Tidak ada name mangling C++
- Compatible dengan assembly
- Dapat di-call dari inline assembly

```cpp
extern "C" void function_name() {
    // Implementation
}
```

### NX Bit (No-Execute)

PAE (Physical Address Extension) mode dapat enable NX bit:
```cpp
PAE::enable_nxe();
```

Ini mencegah eksekusi kode dari data segments (security feature).

### Disk I/O (INT 13h)

BIOS interrupt `INT 13h` untuk disk operations:

**Read Sectors (AH=02h)**:
```
Input:
  AH = 02h          (function: read sectors)
  AL = sector count (1-128)
  CH = cylinder number (0-1023, low 8 bits)
  CL = sector number (1-63, bits 0-5)
       + high 2 bits of cylinder (bits 6-7)
  DH = head number (0-255)
  DL = drive number (00h=first floppy, 80h=first HDD)
  ES:BX = buffer address

Output:
  CF = 0 if successful
  AH = status
  AL = sectors read
```

Contoh di bootloader:
```asm
mov ah, 0x02           ; Function: Read
mov al, 25             ; Read 25 sectors
xor ch, ch             ; Cylinder 0
mov cl, 2              ; Start from sector 2
xor dh, dh             ; Head 0
mov dl, [BOOT_DRIVE]   ; Drive number
mov bx, 0x9000         ; Buffer ES:BX
int 0x13               ; Call BIOS
jc .error              ; Jump if carry flag set
```

---

## Pengembangan

### Menambah Perintah Baru

Untuk menambah perintah custom ke shell:

1. **Tambahkan enum** di `CMD_command_list`:
```cpp
enum class CMD_command_list {
  CMD_START,
  CMD_VM_SHUTDOWN,
  CMD_HELP,
  CMD_CLEAR_SCREEN,
  CMD_VERSION,
  CMD_YOUR_NEW_COMMAND,  // ← Tambahkan ini
};
```

2. **Buat handler function**:
```cpp
inline void your_new_command_handler(CMD_metadata_args *args) {
    kstd::VGA_framebuffer::write_screen("Hello from my new command!\n");
    
    // Akses arguments jika perlu
    uint32_t *line_num = (uint32_t *)args->args1;
    
    // Your logic here...
}
```

3. **Register di `cmd_list_f` array**:
```cpp
static CMD_list_func cmd_list_f[] = {
  {"help", CMD_command_list::CMD_HELP, help_handler},
  {"shutdown", CMD_command_list::CMD_VM_SHUTDOWN, shutdown_handler},
  {"clear", CMD_command_list::CMD_CLEAR_SCREEN, clear_handler},
  {"version", CMD_command_list::CMD_VERSION, version_handler},
  {"start", CMD_command_list::CMD_START, start_handler},
  {"mycmd", CMD_command_list::CMD_YOUR_NEW_COMMAND, your_new_command_handler}, // ← Tambahkan
};
```

4. **Rebuild**:
```bash
cd build
make
make run
```

### Menambah Fungsi Utility

Tambahkan di namespace `kstd` dalam `include/kstd.hpp`:

```cpp
namespace kstd {
    namespace math {
        inline int add(int a, int b) {
            return a + b;
        }
        
        inline int multiply(int a, int b) {
            int result = 0;
            for(int i = 0; i < b; i++) {
                result += a;
            }
            return result;
        }
    }
}
```

Gunakan:
```cpp
int sum = kstd::math::add(5, 3);
```

### Menambah Driver Hardware

Contoh: Driver untuk PC speaker

1. **Buat header file** `include/speaker.hpp`:
```cpp
#ifndef SPEAKER_HPP
#define SPEAKER_HPP

#include "./iopl.hpp"

namespace Speaker {
    inline void beep(uint32_t frequency, uint32_t duration_ms) {
        uint32_t divisor = 1193180 / frequency;
        
        // Set PIT channel 2 untuk speaker
        IOCTL::out_byte<uint8_t>(0x43, 0xB6);
        IOCTL::out_byte<uint8_t>(0x42, divisor & 0xFF);
        IOCTL::out_byte<uint8_t>(0x42, (divisor >> 8) & 0xFF);
        
        // Enable speaker
        uint8_t tmp = IOCTL::in_byte<uint8_t>(0x61);
        IOCTL::out_byte<uint8_t>(0x61, tmp | 0x03);
        
        // Delay (simplified)
        for(volatile uint32_t i = 0; i < duration_ms * 1000; i++);
        
        // Disable speaker
        IOCTL::out_byte<uint8_t>(0x61, tmp);
    }
}

#endif
```

2. **Gunakan di kernel**:
```cpp
#include "../include/speaker.hpp"

// Di command handler
inline void beep_handler(CMD_metadata_args *args) {
    Speaker::beep(1000, 200);  // 1000 Hz, 200ms
    kstd::VGA_framebuffer::write_screen("Beep!\n");
}
```

### Debugging

#### 1. Serial Output untuk Debugging

Tambahkan fungsi serial output:
```cpp
namespace Serial {
    constexpr uint16_t COM1 = 0x3F8;
    
    inline void init() {
        IOCTL::out_byte<uint8_t>(COM1 + 1, 0x00);
        IOCTL::out_byte<uint8_t>(COM1 + 3, 0x80);
        IOCTL::out_byte<uint8_t>(COM1 + 0, 0x03);
        IOCTL::out_byte<uint8_t>(COM1 + 1, 0x00);
        IOCTL::out_byte<uint8_t>(COM1 + 3, 0x03);
        IOCTL::out_byte<uint8_t>(COM1 + 2, 0xC7);
        IOCTL::out_byte<uint8_t>(COM1 + 4, 0x0B);
    }
    
    inline void putchar(char c) {
        while((IOCTL::in_byte<uint8_t>(COM1 + 5) & 0x20) == 0);
        IOCTL::out_byte<uint8_t>(COM1, c);
    }
    
    inline void print(const char *str) {
        for(int i = 0; str[i]; i++) {
            putchar(str[i]);
        }
    }
}
```

Jalankan QEMU dengan serial:
```bash
qemu-system-i386 -drive format=raw,file=os.img -serial stdio
```

#### 2. GDB Debugging

Compile dengan debug symbols:
```cmake
target_compile_options(kernel.elf PRIVATE -g)
```

Jalankan QEMU dengan GDB server:
```bash
qemu-system-i386 -drive format=raw,file=os.img -s -S
```
- `-s`: Enable GDB server on port 1234
- `-S`: Freeze CPU at startup

Di terminal lain:
```bash
gdb kernel.elf
(gdb) target remote localhost:1234
(gdb) break _start
(gdb) continue
```

#### 3. Bochs Debugger

Bochs emulator memiliki built-in debugger yang powerful:
```bash
bochs -f bochsrc.txt
```

File `bochsrc.txt`:
```
floppya: 1_44=os.img, status=inserted
boot: a
```

### Migrasi ke Protected Mode

Untuk upgrade ke 32-bit protected mode:

1. **Setup GDT** (Global Descriptor Table):
```cpp
struct GDT_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));
```

2. **Enable PE bit di CR0**:
```asm
mov eax, cr0
or eax, 0x1
mov cr0, eax
```

3. **Far jump ke 32-bit code**:
```asm
jmp 0x08:protected_mode_start
```

---

## Troubleshooting

### Build Errors

#### Error: `undefined reference to '__stack_chk_fail'`

**Penyebab**: Stack protector tidak compatible di freestanding environment.

**Solusi**: Tambahkan flag `-fno-stack-protector` di CMakeLists.txt:
```cmake
set(FREESTANDING_FLAGS 
    ...
    -fno-stack-protector
)
```

#### Error: `_start already defined`

**Penyebab**: Multiple definition dari `_start` function.

**Solusi**: Pastikan hanya ada satu `_start` dengan attribute `section(".kernel_entry")`.

#### Error: `cannot find -lgcc`

**Penyebab**: GCC support library tidak ditemukan untuk 32-bit.

**Solusi**: Install gcc-multilib:
```bash
sudo apt install gcc-multilib g++-multilib
```

#### Error: NASM not found

**Solusi**:
```bash
sudo apt install nasm
```

### Runtime Errors

#### QEMU: "No bootable device"

**Penyebab**: Boot signature `0xAA55` tidak ada di byte 510-511.

**Solusi**: Periksa `src/main.asm`:
```asm
times 510 - ($ - $$) db 0
dw 0xAA55
```

Verifikasi dengan:
```bash
xxd boot.bin | tail -n 1
```
Seharusnya output: `000001f0: ... 55aa`

#### Black screen setelah boot

**Penyebab**: 
1. Bootloader gagal load kernel
2. Kernel crash sebelum VGA init
3. Jump ke alamat salah

**Debug**:
1. Tambahkan print statement di bootloader:
```asm
; Setelah INT 13h
lea si, [ds:L1]
call print
```

2. Periksa linker script `link.ld` - pastikan `.text` section di `0x9000`.

3. Jalankan dengan Bochs debugger untuk trace execution.

#### Keyboard tidak merespons

**Penyebab**: Interrupt disabled atau keyboard controller belum ready.

**Solusi**:
1. Pastikan `cli` dan `sti` balance:
```cpp
__asm__ __volatile__("sti");  // Enable interrupts
```

2. Periksa keyboard status port:
```cpp
while(!(IOCTL::in_byte<uint8_t>(0x64) & 0x01));
```

#### Characters muncul ganda

**Penyebab**: Tidak memfilter key release events (break codes).

**Solusi**: Filter scan codes > 0x80:
```cpp
if(character & 0x80) continue;  // Skip break codes
```

#### Shutdown tidak bekerja

**Penyebab**: Port `0x604` hanya bekerja di QEMU.

**Solusi**: Untuk hardware real, gunakan ACPI atau APM shutdown.

### Memory Issues

#### Stack overflow

**Gejala**: Random crashes, corruption.

**Solusi**: 
1. Increase stack size di bootloader:
```asm
mov sp, 0x9000  ; Atau nilai lebih tinggi
```

2. Reduce local variable usage - gunakan static atau dynamic allocation.

#### Kernel terlalu besar

**Gejala**: Boot gagal setelah menambah banyak code.

**Solusi**:
1. Increase sector count di bootloader:
```asm
DEFAULT_SECTOR_COUNT equ 50  ; Dari 25 ke 50
```

2. Implementasikan two-stage bootloader.

### Display Issues

#### Warna salah

**Penyebab**: Attribute byte format salah.

**Format yang benar**:
```
High nibble (4 bits): Background color
Low nibble (4 bits): Foreground color
```

Contoh:
```cpp
uint8_t color = (background << 4) | foreground;
```

#### Teks tidak muncul di posisi yang benar

**Penyebab**: Perhitungan index VGA buffer salah.

**Formula yang benar**:
```cpp
uint32_t index = row * VGA_DEFAULT_SCREEN_COLUMN + col;
vga_fb[index] = (color << 8) | character;
```

#### Cursor tidak terlihat

**Penyebab**: Cursor register tidak di-set.

**Solusi**: Call `set_cursor_screen()` setelah setiap output:
```cpp
kstd::VGA_framebuffer::set_cursor_screen(
    kstd::VGA_framebuffer::global_row_fb,
    kstd::VGA_framebuffer::global_col_fb
);
```

---

## Kontribusi

Kontribusi sangat diterima! Berikut adalah cara untuk berkontribusi:

### 1. Fork Repository

```bash
# Fork di GitHub, lalu clone
git clone https://github.com/your-username/x86-16-shell-os.git
cd x86-16-shell-os
```

### 2. Buat Branch Baru

```bash
git checkout -b feature/nama-fitur-baru
```

### 3. Lakukan Perubahan

Edit code, test, dan commit:
```bash
git add .
git commit -m "Add: deskripsi fitur baru"
```

### 4. Push dan Create Pull Request

```bash
git push origin feature/nama-fitur-baru
```

Lalu buat Pull Request di GitHub.

### Guidelines Kontribusi

- **Code Style**: Ikuti style yang sudah ada
  - Indentasi: 2 spaces
  - Naming: `snake_case` untuk fungsi, `PascalCase` untuk classes
  - Comment untuk logic yang complex
  
- **Testing**: Test code Anda di QEMU sebelum PR

- **Documentation**: Update README jika menambah fitur baru

- **Commit Messages**: 
  - Format: `[Type]: Description`
  - Type: `Add`, `Fix`, `Update`, `Remove`, `Refactor`
  - Contoh: `Add: speaker driver for PC beep`

### Ide Kontribusi

Beberapa ide fitur yang bisa dikembangkan:

1. **Filesystem sederhana** (FAT12 atau custom)
2. **Text editor** sederhana
3. **Calculator** dalam shell
4. **More commands**: `ls`, `cat`, `echo`, dll
5. **Protected mode** migration (32-bit)
6. **Multitasking** cooperative
7. **Graphics mode** (VGA 320x200 256 colors)
8. **Network driver** (NE2000)
9. **Mouse driver** (PS/2 mouse)
10. **Memory management** (malloc/free implementation)

---

## Lisensi

Proyek ini bersifat open-source untuk tujuan edukasi. Anda bebas untuk:
- Menggunakan kode untuk belajar
- Memodifikasi sesuai kebutuhan
- Mendistribusikan dengan atribusi
- Menggunakan di proyek pribadi atau komersial

**Disclaimer**: Proyek ini disediakan "AS IS" tanpa warranty. Penggunaan di hardware real adalah risiko Anda sendiri.

---

## Referensi & Resources

### Dokumentasi Resmi

- [Intel 64 and IA-32 Architectures Software Developer's Manual](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html)
- [OSDev Wiki](https://wiki.osdev.org/) - Encyclopedia untuk OS development
- [GCC Inline Assembly](https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html)
- [NASM Documentation](https://www.nasm.us/xdoc/2.15.05/html/nasmdoc0.html)

### Tutorials & Books

- **"Operating Systems: From 0 to 1"** by Tu, Do Hoang
- **"Writing a Simple Operating System from Scratch"** by Nick Blundell
- **"The little book about OS development"** by Erik Helin
- [OSDev.org Bare Bones Tutorial](https://wiki.osdev.org/Bare_Bones)
- [Brokenthorn OS Development Series](http://www.brokenthorn.com/Resources/OSDevIndex.html)

### Video Series

- [Writing an OS - YouTube](https://www.youtube.com/results?search_query=writing+an+operating+system+from+scratch)
- [nanobyte OS Development](https://www.youtube.com/c/nanobyte-dev)

### Tools & Utilities

- [QEMU Emulator](https://www.qemu.org/)
- [Bochs Emulator](http://bochs.sourceforge.net/)
- [GDB Debugger](https://www.gnu.org/software/gdb/)
- [Hex Editor (HxD)](https://mh-nexus.de/en/hxd/)

### Communities

- [OSDev Forum](https://forum.osdev.org/)
- [Reddit r/osdev](https://www.reddit.com/r/osdev/)
- [Stack Overflow - osdev tag](https://stackoverflow.com/questions/tagged/osdev)

---

## Catatan Pembelajaran

### Konsep Penting yang Dipelajari

Dengan menyelesaikan dan memahami proyek ini, Anda telah belajar:

**Low-level programming**
- Assembly x86 dan inline assembly
- Direct hardware manipulation
- Memory-mapped I/O

**Computer architecture**
- x86 real mode vs protected mode
- Segmentation dan paging
- Interrupt handling
- I/O ports dan DMA

**Operating system concepts**
- Boot process dan bootloader
- Kernel loading
- Driver development (keyboard, VGA)
- Command parsing dan execution

**Development practices**
- Cross-compilation
- Linker scripts
- Build systems (CMake)
- Debugging bare-metal code

---

## Kontak & Support

Jika Anda memiliki pertanyaan, bug reports, atau ide:

- **Issues**: Buat issue di GitHub repository
- **Discussions**: Gunakan GitHub Discussions untuk Q&A
- **Email**: [your-email@example.com] (jika ada)

---

## Acknowledgments

Terima kasih kepada:
- **OSDev.org community** untuk dokumentasi komprehensif
- **QEMU developers** untuk emulator yang powerful
- **GCC & NASM teams** untuk compiler/assembler yang excellent
- **Semua kontributor** yang telah membantu proyek ini

---

## Next Steps

Setelah memahami proyek ini, langkah selanjutnya:

1. **Expand ke Protected Mode** (32-bit)
2. **Implementasi filesystem** untuk persistent storage
3. **Tambah multitasking** dengan cooperative atau preemptive scheduling
4. **Pelajari long mode** (64-bit)
5. **Implementasi network stack** (TCP/IP)
6. **Explore modern OS concepts**: virtual memory, security, IPC

---

**Happy Operating System Development!**

*Last updated: January 2026*
*Version: 0.1 minor update*
