#include "platform.h"

char* platform_get_str(int *count){
    #if defined(PLAT_MSFT_64)
        *count = 5;
        return "WIN64";
    #elif defined(PLAT_MSFT)
        #if defined(ARCH_i386)
            *count = 5;
            return "WIN32";
        #elif defined(ARCH_ARM)
            *count = 6;
            return "WINARM";
        #elif defined(ARCH_UNK)
            *count = 6;
            return "WINUNK";
        #endif
    #elif defined(PLAT_MAC)
        #if defined(ARCH_X86_64)
            *count = 5;
            return "MAC64";
        #elif defined(ARCH_i386)
            *count = 5;
            return "MAC32";
        #elif defined(ARCH_ARM)
            *count = 6;
            return "MACARM";
        #elif defined(ARCH_UNK)
            *count = 6;
            return "MACUNK";
        #endif
    #elif defined(PLAT_IOS)
        *count = 3;
        return "IOS";
    #elif defined(PLAT_IOS_SIM)
        *count = 6;
        return "IOSSIM";
    #elif defined(PLAT_LINUX)
        #if defined(ARCH_X86_64)
            *count = 6;
            return "LINUX64";
        #elif defined(ARCH_i386)
            *count = 6;
            return "LINUX32";
        #elif defined(ARCH_ARM64)
            *count = 10;
            return "LINUXARM64";
        #elif defined(ARCH_ARM)
            *count = 10;
            return "LINUXARM32";
        #elif defined(ARCH_UNK)
            *count = 8;
            return "LINUXUNK";
        #endif
    #elif defined(PLAT_UNIX)
        #if defined(ARCH_X86_64)
            *count = 6;
            return "UNIX64";
        #elif defined(ARCH_i386)
            *count = 6;
            return "UNIX32";
        #elif defined(ARCH_ARM64)
            *count = 9;
            return "UNIXARM64";
        #elif defined(ARCH_ARM)
            *count = 9;
            return "UNIXARM32";
        #elif defined(ARCH_UNK)
            *count = 7;
            return "UNIXUNK";
        #endif
    #elif defined(PLAT_POSIX)
        #if defined(ARCH_X86_64)
            *count = 7;
            return "POSIX64";
        #elif defined(ARCH_i386)
            *count = 7;
            return "POSIX32";
        #elif defined(ARCH_ARM64)
            *count = 10;
            return "POSIXARM64";
        #elif defined(ARCH_ARM)
            *count = 10;
            return "POSIXARM32";
        #elif defined(ARCH_UNK)
            *count = 8;
            return "POSIXUNK";
        #endif
    #elif defined(PLAT_BSD)
        #if defined(ARCH_X86_64)
            *count = 5;
            return "BSD64";
        #elif defined(ARCH_i386)
            *count = 5;
            return "BSD32";
        #elif defined(ARCH_ARM64)
            *count = 8;
            return "BSDARM64";
        #elif defined(ARCH_ARM)
            *count = 8;
            return "BSDARM32";
        #elif defined(ARCH_UNK)
            *count = 5;
            return "BSDUNK";
        #endif
    #elif defined(PLAT_UNK)
        #if defined(ARCH_X86_64)
            *count = 5;
            return "UNK64";
        #elif defined(ARCH_i386)
            *count = 5;
            return "UNK32";
        #elif defined(ARCH_ARM64)
            *count = 8;
            return "UNKARM64";
        #elif defined(ARCH_ARM)
            *count = 8;
            return "UNKARM32";
        #elif defined(ARCH_UNK)
            *count = 6;
            return "UNKUNK";
        #endif
#endif
}
char* arch_get_str(int *count){
    #if defined(ARCH_X86_64)
        *count = 6;
        return "X86_64";
    #elif defined(ARCH_i386)
        *count = 4;
        return "i386";
    #elif defined(ARCH_ARM64)
        *count = 5;
        return "ARM64";
    #elif defined(ARCH_ARM)
        *count = 3;
        return "ARM";
    #elif defined(ARCH_UNK)
        *count = 3;
        return "UNK";
    #endif
}
