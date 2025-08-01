#ifndef _H_INCLUDE_ASSET

#include "macros.h"

#define ASTRINGIFY_(x) #x
#define ASTRINGIFY(x) ASTRINGIFY_(x)

#ifdef MODERN_COMPILER
#  define PUSHSECTION(SECTION) ".pushsection " SECTION "\n"
#  define POPSECTION ".popsection\n"
#else
#  define PUSHSECTION(SECTION) SECTION "\n"
#  define POPSECTION
#endif

#define _INCLUDE_IMG(FILENAME, SYMBOLNAME) \
    EXTERN_C unsigned char SYMBOLNAME[]; \
    __asm__( \
        ".globl " #SYMBOLNAME"\n" \
        PUSHSECTION(".data") \
        ".align 3\n" \
        ".type " #SYMBOLNAME", @object\n" \
        #SYMBOLNAME":\n" \
        ".incbin \"ver/"ASTRINGIFY(VERSION)"/build/" FILENAME ".bin\"\n" \
        POPSECTION \
    )

// two macros are needed for N() usage
#define INCLUDE_IMG(FILENAME, SYMBOLNAME) \
    _INCLUDE_IMG(FILENAME, SYMBOLNAME)

#define INCLUDE_PAL(FILENAME, SYMBOLNAME) \
    EXTERN_C unsigned short SYMBOLNAME[]; \
    __asm__( \
        ".globl " #SYMBOLNAME"\n" \
        PUSHSECTION(".data") \
        ".align 3\n" \
        ".type " #SYMBOLNAME", @object\n" \
        #SYMBOLNAME":\n" \
        ".incbin \"ver/"ASTRINGIFY(VERSION)"/build/" FILENAME ".bin\"\n" \
        POPSECTION \
    )

#define INCLUDE_RAW(FILENAME, SYMBOLNAME) \
    EXTERN_C unsigned char SYMBOLNAME[]; \
    __asm__( \
        ".globl " #SYMBOLNAME"\n" \
        PUSHSECTION(".data") \
        ".align 3\n" \
        ".type " #SYMBOLNAME", @object\n" \
        #SYMBOLNAME":\n" \
        ".incbin \"ver/"ASTRINGIFY(VERSION)"/build/assets/"ASTRINGIFY(VERSION)"/" FILENAME "\"\n" \
        POPSECTION \
    )


#endif // _H_INCLUDE_ASSET
