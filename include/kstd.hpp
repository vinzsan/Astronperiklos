#ifndef STRING_HPP
#define STRING_HPP

#include <cstdint>
#include "./iopl.hpp"

#define VGA_FRAME_BUFFER_ADDRESS 0xB8000
#define VGA_DEFAULT_SCREEN_ROW 25
#define VGA_DEFAULT_SCREEN_COLUMN 80

#define WRITE_ALERT 0x4F
#define WRITE_TFAULT 0x1F

static char scancode_to_ascii_lower_case[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0, 'a','s','d','f','g','h','j','k','l',';','\'','`',
    0, '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0,' '
};

static char scancode_to_ascii_upper_case[128] = {
    0,27,'!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,
    'A','S','D','F','G','H','J','K','L',':','"','~',0,'|',
    'Z','X','C','V','B','N','M','<','>','?',0,'*',0,' '
};

namespace kstd {
    namespace str {
        static inline bool strcompare(char *str,const char *cmp){
            uint32_t i = 0;
            while(str[i] != '\0' && cmp[i] != '\0') {
                if(str[i] != cmp[i]){
                    return false;
                }
                i++;
            }
            return str[i] == '\0' && cmp[i] == '\0';
        }
        
        inline char *int2str(int number){
            static char static_str2int[sizeof(uint32_t)];
            char *char_ptr = static_str2int + sizeof(static_str2int);
            *--char_ptr = '\0';

            uint32_t number_buffer;
            if(number < 0){
                number_buffer = static_cast<uint32_t>(-number);
            } else {
                number_buffer = static_cast<uint32_t>(number);
            }
            
            do {
                *--char_ptr = '0' + (number_buffer % 10);
                number_buffer /= 10;
            } while(number_buffer);
            if(number < 0) *--char_ptr = '-';
            return char_ptr;
        }

        inline int str2int(const char *str){
            return 0;
        }
        
        inline void *memcopy(char *dst,char *src,int n) {
            uint8_t *source = (uint8_t *)src;
            uint8_t *dest = (uint8_t *)dest;

            for(int i = 0;i < n;i++){
                dest[i] = source[i];
                if(source[i] == 0) break;
            }
            return dest;
        }
    }
    
    namespace KeyboardStatus {
        constexpr uint16_t DATA_PORT = 0x60;
        constexpr uint16_t STATUS_PORT = 0x64;
        
        extern "C" inline uint8_t read_non_blocking(){
            while(!(IOCTL::in_byte<uint16_t>(STATUS_PORT) & 0x01));
            return IOCTL::in_byte<uint16_t>(DATA_PORT);
        }
    }

    namespace VGA_framebuffer {
        inline int global_row_fb = 0;
        inline int global_col_fb = 0;
        static inline uint16_t *vga_fb = reinterpret_cast<uint16_t*>(VGA_FRAME_BUFFER_ADDRESS);

        extern "C" inline void set_cursor_screen(int row,int col){
          uint32_t position_cursor = row * VGA_DEFAULT_SCREEN_COLUMN + col;
        
          IOCTL::out_byte<uint16_t>(0x3D4,0x0F);
          IOCTL::out_byte<uint16_t>(0x3D5,static_cast<uint8_t>(position_cursor) & 0xFF);

          IOCTL::out_byte<uint16_t>(0x3D4,0x0E);
          IOCTL::out_byte<uint16_t>(0x3D5,static_cast<uint8_t>(position_cursor >> 8) & 0xFF);
        }
    
        extern "C" inline void scroll_cursor_screen(){
          for(uint32_t i = 0;i < VGA_DEFAULT_SCREEN_COLUMN * (VGA_DEFAULT_SCREEN_ROW - 1);i++){
            vga_fb[i] = vga_fb[i + VGA_DEFAULT_SCREEN_COLUMN];
          }
      
          uint32_t last_row = VGA_DEFAULT_SCREEN_COLUMN * (VGA_DEFAULT_SCREEN_ROW - 1);

          for(uint32_t i = 0;i < VGA_DEFAULT_SCREEN_COLUMN;i++){
            vga_fb[last_row + i] = static_cast<uint8_t>(0x0F) << 8 | ' ';
          }
      
          global_row_fb = VGA_DEFAULT_SCREEN_ROW - 1;
        }
    
        extern "C" inline void write_screen(const char *str,uint8_t color = 0x0F){
          for(uint32_t i = 0;str[i] != '\0';i++){
            if(str[i] == '\n'){
              global_row_fb++;
              global_col_fb = 0;
              if(global_row_fb >= VGA_DEFAULT_SCREEN_ROW){
                scroll_cursor_screen();
              }
              continue;
            }
        
            uint32_t index_vga_ptr_ds = global_row_fb * VGA_DEFAULT_SCREEN_COLUMN + global_col_fb;
            vga_fb[index_vga_ptr_ds] = ((static_cast<uint8_t>(color) << 8) | str[i]);
            global_col_fb++;
        
            if(global_col_fb >= VGA_DEFAULT_SCREEN_COLUMN){
              global_row_fb++;
              global_col_fb = 0;
              if(global_row_fb >= VGA_DEFAULT_SCREEN_ROW){
                scroll_cursor_screen();
              }
            }
            set_cursor_screen(global_row_fb, global_col_fb);
          }
        }

        extern "C" inline void putchar_screen(char c,uint8_t color = 0x0F){
          if(c == '\0') return;
          uint32_t index_cursor = global_row_fb * VGA_DEFAULT_SCREEN_COLUMN + global_col_fb;
          vga_fb[index_cursor] = static_cast<uint8_t>(color) << 8 | c;
        
          global_col_fb++;
          if(global_col_fb >= VGA_DEFAULT_SCREEN_COLUMN){
            global_col_fb = 0;
            global_row_fb++;
            if(global_row_fb >= VGA_DEFAULT_SCREEN_ROW){
              scroll_cursor_screen();
            }
          }
          set_cursor_screen(global_row_fb, global_col_fb);
        }
    
        extern "C" inline void clear_screen(uint8_t color = 0x0F){
          for(uint32_t i = 0;i < VGA_DEFAULT_SCREEN_ROW * VGA_DEFAULT_SCREEN_COLUMN;i++){
            vga_fb[i] = ((uint8_t)color << 8 | ' ');
          }
      
          global_row_fb = global_col_fb = 0;
          set_cursor_screen(0, 0);
        }
    }
    
    namespace arch {
        constexpr uint8_t ACTIVE_GATE = 0x02;
        constexpr uint8_t A20_BIT = (1 << 1);
        static uint16_t backup_status = 0;
        extern "C" inline void a20_gate_set(bool set = true){
            auto status = IOCTL::in_byte<uint8_t>(0x92);
            status &= ~A20_BIT;
            if(set){
                status |= A20_BIT;
            } else {
                status &= ~A20_BIT;
            }
            IOCTL::out_byte<uint8_t>(0x92, status);
        }
    }
}

#endif