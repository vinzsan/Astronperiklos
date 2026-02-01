#include <cstdint>

#include "../include/kstd.hpp"
#include "../lib/memory.hpp"
#include "../include/non_acpi.hpp"
#include "../include/irq.hpp"

#define DEFAULT_CHAR_BUFFER_READ_SIZE 1024 * 4

__asm__(".code16gcc");

constexpr uint8_t SET_DEFAULT_COLOR_ = 0x0F;

enum class KeyboardKey {
  KEY_SHIFT,
  KEY_NON_SHIFT
};

enum class CMD_command_list {
  CMD_START,
  CMD_VM_SHUTDOWN,
  CMD_HELP,
  CMD_CLEAR_SCREEN,
  CMD_VERSION,
};

struct CMD_metadata_args {
  void *args1;
  void *args2;
};

struct CMD_list_func {
  const char *cmd_str;
  CMD_command_list cmd_commd_list;
  void(*handler)(CMD_metadata_args *args);
};

/*
  NOTE : function CMD handler
*/

inline void help_handler(CMD_metadata_args *a){
  kstd::VGA_framebuffer::write_screen(
    "Simple x86_16 shell (not an OS dawg)\n\n \
    -help (to show ts message)\n \
    \n"
  );
}

inline void shutdown_handler(CMD_metadata_args *a){
  N_ACPI::shutdown_vm();
}

inline void clear_handler(CMD_metadata_args *a){
  auto *s = (uint32_t *)a->args1;
  *s = 0;
  kstd::VGA_framebuffer::clear_screen();
}

inline void version_handler(CMD_metadata_args *a) {
  kstd::VGA_framebuffer::write_screen("version 0.1 minor update\n");
}

inline void start_handler(CMD_metadata_args *a){
  constexpr int HEIGHT_AWAL = 3;
  int skor_buffer = 0;
  kstd::VGA_framebuffer::clear_screen();
  kstd::VGA_framebuffer::write_screen("Shell game (preproject)\n",0x5F);
  for(int x = 0;x < VGA_DEFAULT_SCREEN_COLUMN;x++){
    kstd::VGA_framebuffer::vga_fb[HEIGHT_AWAL * VGA_DEFAULT_SCREEN_COLUMN + x] = ((uint8_t)0xAF << 8) | ' ';
    for(int y = HEIGHT_AWAL;y < VGA_DEFAULT_SCREEN_ROW;y++){
      kstd::VGA_framebuffer::vga_fb[y * VGA_DEFAULT_SCREEN_COLUMN + 0] = ((uint8_t)0xAF << 8) | ' ';
    }
  }

  for(int x = 0;x < VGA_DEFAULT_SCREEN_COLUMN;x++){
    kstd::VGA_framebuffer::vga_fb[(VGA_DEFAULT_SCREEN_ROW - 1 )* VGA_DEFAULT_SCREEN_COLUMN + x] = ((uint8_t)0xAF << 8) | ' ';
    for(int y = HEIGHT_AWAL;y < VGA_DEFAULT_SCREEN_ROW;y++){
      kstd::VGA_framebuffer::vga_fb[y * VGA_DEFAULT_SCREEN_COLUMN + (VGA_DEFAULT_SCREEN_COLUMN - 1)] = ((uint8_t)0xAF << 8) | ' ';
    }
  }
  
  kstd::VGA_framebuffer::vga_fb[VGA_DEFAULT_SCREEN_ROW * VGA_DEFAULT_SCREEN_COLUMN / 2] = ((uint8_t)0x1F << 8) | ' ';
  kstd::VGA_framebuffer::global_col_fb = 1;
  kstd::VGA_framebuffer::global_row_fb = HEIGHT_AWAL + 1;
  kstd::VGA_framebuffer::write_screen("Skor kamu adalah : ");
  kstd::VGA_framebuffer::write_screen(kstd::str::int2str(skor_buffer));
  kstd::VGA_framebuffer::write_screen("\n");
  PIT_pack::sleep_iter(10000);
}

CMD_metadata_args args;

static CMD_list_func cmd_list_f[] = {
  {"help",CMD_command_list::CMD_HELP,help_handler},
  {"shutdown",CMD_command_list::CMD_VM_SHUTDOWN,shutdown_handler},
  {"clear",CMD_command_list::CMD_CLEAR_SCREEN,clear_handler},
  {"version",CMD_command_list::CMD_VERSION,version_handler},
  {"start",CMD_command_list::CMD_START,start_handler}
};

CMD_list_func *cmd_search_command(char *__input){
  for(auto &i : cmd_list_f){
    if(kstd::str::strcompare(__input, i.cmd_str)){
      return &i;
    }
  }
  return nullptr;
}

extern "C" __attribute__((section(".kernel_entry")))
void _start(void){
  __asm__ __volatile__(
    "cli\n\t"
    "xor %%ax,%%ax\n\t"
    "mov %%ax,%%ds\n\t"
    "mov %%ax,%%es\n\t"
    "mov %%ax,%%ss\n\t"
    "mov $0x9000,%%sp\n\t"
    "cld\n\t"
    :::"ax"
  );
  
  PAE::enable_nxe();
  PIT_pack::pit_setup_init(1132);
  kstd::VGA_framebuffer::clear_screen(SET_DEFAULT_COLOR_);

  {
    using namespace kstd::VGA_framebuffer;
    write_screen("[INFO] A20 Gate : ",0x1F);
    write_screen(kstd::str::int2str(IOCTL::in_byte<uint8_t>(0x92)),0x4F);
    write_screen("\n");

    kstd::arch::a20_gate_set();
    
    write_screen("[INFO] A20 Gate After : ");
    write_screen(kstd::str::int2str(IOCTL::in_byte<uint8_t>(0x92)));
    write_screen("\n\n\n");
  }

  kstd::VGA_framebuffer::write_screen("Welcome to x86-16 shell\n");

  uint32_t char_length = 0;
  uint32_t line_current = 0;
  static char buffer_read[DEFAULT_CHAR_BUFFER_READ_SIZE] = {0};
  
  while(true){
    __builtin_memset(buffer_read,0,sizeof(buffer_read));
    kstd::VGA_framebuffer::write_screen("Shell [");
    kstd::VGA_framebuffer::write_screen(kstd::str::int2str(line_current),0x02);
    kstd::VGA_framebuffer::write_screen("] : >> ");
    KeyboardKey key_shift_state = KeyboardKey::KEY_NON_SHIFT;

    while(true){
      uint8_t character = kstd::KeyboardStatus::read_non_blocking();

      if(character == 0x2A || character == 0x36){
        key_shift_state = KeyboardKey::KEY_SHIFT;
        continue;
      }

      if(character == 0xAA || character == 0xB6){
        key_shift_state = KeyboardKey::KEY_NON_SHIFT;
        continue;
      }

      if(character & 0x80) continue;
      
      char sc;
      if(key_shift_state == KeyboardKey::KEY_SHIFT){
        sc = scancode_to_ascii_upper_case[character];
      } else {
        sc = scancode_to_ascii_lower_case[character];
      }
      
      if(sc == '\n'){
        kstd::VGA_framebuffer::global_col_fb = 0;
        kstd::VGA_framebuffer::global_row_fb++;
        if(kstd::VGA_framebuffer::global_row_fb >= VGA_DEFAULT_SCREEN_ROW){
          kstd::VGA_framebuffer::scroll_cursor_screen();
        }
        kstd::VGA_framebuffer::set_cursor_screen(kstd::VGA_framebuffer::global_row_fb, kstd::VGA_framebuffer::global_col_fb);
        break;
      }

      if(sc == '\b' && char_length != 0){
        if(char_length == 0) continue;

        char_length--;
        if(kstd::VGA_framebuffer::global_col_fb > 0){
          kstd::VGA_framebuffer::global_col_fb--;
          kstd::VGA_framebuffer::vga_fb[kstd::VGA_framebuffer::global_row_fb * 
            VGA_DEFAULT_SCREEN_COLUMN + 
            kstd::VGA_framebuffer::global_col_fb] = static_cast<uint8_t>(SET_DEFAULT_COLOR_) << 8 | ' ';

          kstd::VGA_framebuffer::set_cursor_screen(
            kstd::VGA_framebuffer::global_row_fb,
            kstd::VGA_framebuffer::global_col_fb
          );
        }
      }

      if(sc && sc != '\b'){
        buffer_read[char_length++] = sc;
        kstd::VGA_framebuffer::putchar_screen(sc,SET_DEFAULT_COLOR_);
      }
    }
    
    buffer_read[char_length] = '\0';

    CMD_list_func *func_res = cmd_search_command(buffer_read);
    args.args1 = &line_current;

    if(func_res){
      func_res->handler(&args);
    }
    else {
      buffer_read[char_length] = '\n';
      kstd::VGA_framebuffer::write_screen(buffer_read);
    }
    char_length = 0;
    line_current++;
  }
  
  __asm__ __volatile__("sti");
  while(true) __asm__ __volatile__("hlt");
  __builtin_unreachable();
}