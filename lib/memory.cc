#include <cstdint>

extern "C"{
    
void *memset(void *src,int i,uint32_t n){
    uint8_t *source = (uint8_t *)src;
    uint8_t set = (uint8_t)i;
    for(uint32_t i = 0;i < n;i++){
        source[i] = set;
    }
    return source;
}

void *memcpy(void *__dest,void *__src,uint32_t n){
    uint8_t *__d = (uint8_t *)__dest;
    uint8_t *__s = (uint8_t *)__src;
    for(uint32_t i = 0;i < n;i++){
        __d[i] = __s[i] ? __s[i] : 0;
    }
    return __d;
}

}