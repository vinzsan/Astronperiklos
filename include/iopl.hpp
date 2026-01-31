#ifndef IOPL_HPP
#define IOPL_HPP

namespace IOCTL {
    template<typename T>
    inline void out_byte(unsigned short port,T data){
        __asm__ __volatile__(
            "out %1,%0"
            :
            :"Nd"(port),"a"(data)
            :
        );
    }

    template <typename T>
    inline T in_byte(unsigned short port){
        T result_data;
        __asm__ __volatile__(
            "in %1,%0"
            :"=a"(result_data)
            :"Nd"(port)
            :
        );
        return result_data;
    }
}

#endif