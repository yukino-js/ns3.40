
// NOLINTBEGIN
// clang-format off


#ifndef __VALGRIND_H
#define __VALGRIND_H



#define __VALGRIND_MAJOR__    3
#define __VALGRIND_MINOR__    9


#include <stdarg.h>


#undef PLAT_x86_darwin
#undef PLAT_amd64_darwin
#undef PLAT_x86_win32
#undef PLAT_amd64_win64
#undef PLAT_x86_linux
#undef PLAT_amd64_linux
#undef PLAT_ppc32_linux
#undef PLAT_ppc64_linux
#undef PLAT_arm_linux
#undef PLAT_arm64_linux
#undef PLAT_s390x_linux
#undef PLAT_mips32_linux
#undef PLAT_mips64_linux


#if defined(__APPLE__) && defined(__i386__)
#  define PLAT_x86_darwin 1
#elif defined(__APPLE__) && defined(__x86_64__)
#  define PLAT_amd64_darwin 1
#elif defined(__MINGW32__) || defined(__CYGWIN32__) \
      || (defined(_WIN32) && defined(_M_IX86))
#  define PLAT_x86_win32 1
#elif defined(__MINGW64__) || (defined(_WIN64) && defined(_M_X64))
#  define PLAT_amd64_win64 1
#elif defined(__linux__) && defined(__i386__)
#  define PLAT_x86_linux 1
#elif defined(__linux__) && defined(__x86_64__)
#  define PLAT_amd64_linux 1
#elif defined(__linux__) && defined(__powerpc__) && !defined(__powerpc64__)
#  define PLAT_ppc32_linux 1
#elif defined(__linux__) && defined(__powerpc__) && defined(__powerpc64__)
#  define PLAT_ppc64_linux 1
#elif defined(__linux__) && defined(__arm__) && !defined(__aarch64__)
#  define PLAT_arm_linux 1
#elif defined(__linux__) && defined(__aarch64__) && !defined(__arm__)
#  define PLAT_arm64_linux 1
#elif defined(__linux__) && defined(__s390__) && defined(__s390x__)
#  define PLAT_s390x_linux 1
#elif defined(__linux__) && defined(__mips__) && (__mips==64)
#  define PLAT_mips64_linux 1
#elif defined(__linux__) && defined(__mips__) && (__mips!=64)
#  define PLAT_mips32_linux 1
#else
#  if !defined(NVALGRIND)
#    define NVALGRIND 1
#  endif
#endif




#define VALGRIND_DO_CLIENT_REQUEST(_zzq_rlval, _zzq_default,            \
                                   _zzq_request, _zzq_arg1, _zzq_arg2,  \
                                   _zzq_arg3, _zzq_arg4, _zzq_arg5)     \
  do { (_zzq_rlval) = VALGRIND_DO_CLIENT_REQUEST_EXPR((_zzq_default),   \
                        (_zzq_request), (_zzq_arg1), (_zzq_arg2),       \
                        (_zzq_arg3), (_zzq_arg4), (_zzq_arg5)); } while (0)

#define VALGRIND_DO_CLIENT_REQUEST_STMT(_zzq_request, _zzq_arg1,        \
                           _zzq_arg2,  _zzq_arg3, _zzq_arg4, _zzq_arg5) \
  do { (void) VALGRIND_DO_CLIENT_REQUEST_EXPR(0,                        \
                    (_zzq_request), (_zzq_arg1), (_zzq_arg2),           \
                    (_zzq_arg3), (_zzq_arg4), (_zzq_arg5)); } while (0)

#if defined(NVALGRIND)

#define VALGRIND_DO_CLIENT_REQUEST_EXPR(                          \
        _zzq_default, _zzq_request,                               \
        _zzq_arg1, _zzq_arg2, _zzq_arg3, _zzq_arg4, _zzq_arg5)    \
      (_zzq_default)

#else



#if defined(PLAT_x86_linux)  ||  defined(PLAT_x86_darwin)  \
    ||  (defined(PLAT_x86_win32) && defined(__GNUC__))

typedef
   struct {
      unsigned int nraddr;
   }
   OrigFn;

#define __SPECIAL_INSTRUCTION_PREAMBLE                            \
                     "roll $3,  %%edi ; roll $13, %%edi\n\t"      \
                     "roll $29, %%edi ; roll $19, %%edi\n\t"

#define VALGRIND_DO_CLIENT_REQUEST_EXPR(                          \
        _zzq_default, _zzq_request,                               \
        _zzq_arg1, _zzq_arg2, _zzq_arg3, _zzq_arg4, _zzq_arg5)    \
  __extension__                                                   \
  ({volatile unsigned int _zzq_args[6];                           \
    volatile unsigned int _zzq_result;                            \
    _zzq_args[0] = (unsigned int)(_zzq_request);                  \
    _zzq_args[1] = (unsigned int)(_zzq_arg1);                     \
    _zzq_args[2] = (unsigned int)(_zzq_arg2);                     \
    _zzq_args[3] = (unsigned int)(_zzq_arg3);                     \
    _zzq_args[4] = (unsigned int)(_zzq_arg4);                     \
    _zzq_args[5] = (unsigned int)(_zzq_arg5);                     \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE               \
         \
                     "xchgl %%ebx,%%ebx"                          \
                     : "=d" (_zzq_result)                         \
                     : "a" (&_zzq_args[0]), "0" (_zzq_default)    \
                     : "cc", "memory"                             \
                    );                                            \
    _zzq_result;                                                  \
  })

#define VALGRIND_GET_NR_CONTEXT(_zzq_rlval)                       \
  { volatile OrigFn* _zzq_orig = &(_zzq_rlval);                   \
    volatile unsigned int __addr;                                 \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE               \
                    \
                     "xchgl %%ecx,%%ecx"                          \
                     : "=a" (__addr)                              \
                     :                                            \
                     : "cc", "memory"                             \
                    );                                            \
    _zzq_orig->nraddr = __addr;                                   \
  }

#define VALGRIND_CALL_NOREDIR_EAX                                 \
                     __SPECIAL_INSTRUCTION_PREAMBLE               \
                     \
                     "xchgl %%edx,%%edx\n\t"

#define VALGRIND_VEX_INJECT_IR()                                 \
 do {                                                            \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE              \
                     "xchgl %%edi,%%edi\n\t"                     \
                     : : : "cc", "memory"                        \
                    );                                           \
 } while (0)

#endif


#if defined(PLAT_x86_win32) && !defined(__GNUC__)

typedef
   struct {
      unsigned int nraddr;
   }
   OrigFn;

#if defined(_MSC_VER)

#define __SPECIAL_INSTRUCTION_PREAMBLE                            \
                     __asm rol edi, 3  __asm rol edi, 13          \
                     __asm rol edi, 29 __asm rol edi, 19

#define VALGRIND_DO_CLIENT_REQUEST_EXPR(                          \
        _zzq_default, _zzq_request,                               \
        _zzq_arg1, _zzq_arg2, _zzq_arg3, _zzq_arg4, _zzq_arg5)    \
    valgrind_do_client_request_expr((uintptr_t)(_zzq_default),    \
        (uintptr_t)(_zzq_request), (uintptr_t)(_zzq_arg1),        \
        (uintptr_t)(_zzq_arg2), (uintptr_t)(_zzq_arg3),           \
        (uintptr_t)(_zzq_arg4), (uintptr_t)(_zzq_arg5))

static __inline uintptr_t
valgrind_do_client_request_expr(uintptr_t _zzq_default, uintptr_t _zzq_request,
                                uintptr_t _zzq_arg1, uintptr_t _zzq_arg2,
                                uintptr_t _zzq_arg3, uintptr_t _zzq_arg4,
                                uintptr_t _zzq_arg5)
{
    volatile uintptr_t _zzq_args[6];
    volatile unsigned int _zzq_result;
    _zzq_args[0] = (uintptr_t)(_zzq_request);
    _zzq_args[1] = (uintptr_t)(_zzq_arg1);
    _zzq_args[2] = (uintptr_t)(_zzq_arg2);
    _zzq_args[3] = (uintptr_t)(_zzq_arg3);
    _zzq_args[4] = (uintptr_t)(_zzq_arg4);
    _zzq_args[5] = (uintptr_t)(_zzq_arg5);
    __asm { __asm lea eax, _zzq_args __asm mov edx, _zzq_default
            __SPECIAL_INSTRUCTION_PREAMBLE
            __asm xchg ebx,ebx
            __asm mov _zzq_result, edx
    }
    return _zzq_result;
}

#define VALGRIND_GET_NR_CONTEXT(_zzq_rlval)                       \
  { volatile OrigFn* _zzq_orig = &(_zzq_rlval);                   \
    volatile unsigned int __addr;                                 \
    __asm { __SPECIAL_INSTRUCTION_PREAMBLE                        \
                             \
            __asm xchg ecx,ecx                                    \
            __asm mov __addr, eax                                 \
    }                                                             \
    _zzq_orig->nraddr = __addr;                                   \
  }

#define VALGRIND_CALL_NOREDIR_EAX ERROR

#define VALGRIND_VEX_INJECT_IR()                                 \
 do {                                                            \
    __asm { __SPECIAL_INSTRUCTION_PREAMBLE                       \
            __asm xchg edi,edi                                   \
    }                                                            \
 } while (0)

#else
#error Unsupported compiler.
#endif

#endif


#if defined(PLAT_amd64_linux)  ||  defined(PLAT_amd64_darwin)

typedef
   struct {
      unsigned long long int nraddr;
   }
   OrigFn;

#define __SPECIAL_INSTRUCTION_PREAMBLE                            \
                     "rolq $3,  %%rdi ; rolq $13, %%rdi\n\t"      \
                     "rolq $61, %%rdi ; rolq $51, %%rdi\n\t"

#define VALGRIND_DO_CLIENT_REQUEST_EXPR(                          \
        _zzq_default, _zzq_request,                               \
        _zzq_arg1, _zzq_arg2, _zzq_arg3, _zzq_arg4, _zzq_arg5)    \
    __extension__                                                 \
    ({ volatile unsigned long long int _zzq_args[6];              \
    volatile unsigned long long int _zzq_result;                  \
    _zzq_args[0] = (unsigned long long int)(_zzq_request);        \
    _zzq_args[1] = (unsigned long long int)(_zzq_arg1);           \
    _zzq_args[2] = (unsigned long long int)(_zzq_arg2);           \
    _zzq_args[3] = (unsigned long long int)(_zzq_arg3);           \
    _zzq_args[4] = (unsigned long long int)(_zzq_arg4);           \
    _zzq_args[5] = (unsigned long long int)(_zzq_arg5);           \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE               \
         \
                     "xchgq %%rbx,%%rbx"                          \
                     : "=d" (_zzq_result)                         \
                     : "a" (&_zzq_args[0]), "0" (_zzq_default)    \
                     : "cc", "memory"                             \
                    );                                            \
    _zzq_result;                                                  \
    })

#define VALGRIND_GET_NR_CONTEXT(_zzq_rlval)                       \
  { volatile OrigFn* _zzq_orig = &(_zzq_rlval);                   \
    volatile unsigned long long int __addr;                       \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE               \
                    \
                     "xchgq %%rcx,%%rcx"                          \
                     : "=a" (__addr)                              \
                     :                                            \
                     : "cc", "memory"                             \
                    );                                            \
    _zzq_orig->nraddr = __addr;                                   \
  }

#define VALGRIND_CALL_NOREDIR_RAX                                 \
                     __SPECIAL_INSTRUCTION_PREAMBLE               \
                     \
                     "xchgq %%rdx,%%rdx\n\t"

#define VALGRIND_VEX_INJECT_IR()                                 \
 do {                                                            \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE              \
                     "xchgq %%rdi,%%rdi\n\t"                     \
                     : : : "cc", "memory"                        \
                    );                                           \
 } while (0)

#endif


#if defined(PLAT_ppc32_linux)

typedef
   struct {
      unsigned int nraddr;
   }
   OrigFn;

#define __SPECIAL_INSTRUCTION_PREAMBLE                            \
                    "rlwinm 0,0,3,0,31  ; rlwinm 0,0,13,0,31\n\t" \
                    "rlwinm 0,0,29,0,31 ; rlwinm 0,0,19,0,31\n\t"

#define VALGRIND_DO_CLIENT_REQUEST_EXPR(                          \
        _zzq_default, _zzq_request,                               \
        _zzq_arg1, _zzq_arg2, _zzq_arg3, _zzq_arg4, _zzq_arg5)    \
                                                                  \
    __extension__                                                 \
  ({         unsigned int  _zzq_args[6];                          \
             unsigned int  _zzq_result;                           \
             unsigned int* _zzq_ptr;                              \
    _zzq_args[0] = (unsigned int)(_zzq_request);                  \
    _zzq_args[1] = (unsigned int)(_zzq_arg1);                     \
    _zzq_args[2] = (unsigned int)(_zzq_arg2);                     \
    _zzq_args[3] = (unsigned int)(_zzq_arg3);                     \
    _zzq_args[4] = (unsigned int)(_zzq_arg4);                     \
    _zzq_args[5] = (unsigned int)(_zzq_arg5);                     \
    _zzq_ptr = _zzq_args;                                         \
    __asm__ volatile("mr 3,%1\n\t"                    \
                     "mr 4,%2\n\t"                        \
                     __SPECIAL_INSTRUCTION_PREAMBLE               \
           \
                     "or 1,1,1\n\t"                               \
                     "mr %0,3"                     \
                     : "=b" (_zzq_result)                         \
                     : "b" (_zzq_default), "b" (_zzq_ptr)         \
                     : "cc", "memory", "r3", "r4");               \
    _zzq_result;                                                  \
    })

#define VALGRIND_GET_NR_CONTEXT(_zzq_rlval)                       \
  { volatile OrigFn* _zzq_orig = &(_zzq_rlval);                   \
    unsigned int __addr;                                          \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE               \
                     \
                     "or 2,2,2\n\t"                               \
                     "mr %0,3"                                    \
                     : "=b" (__addr)                              \
                     :                                            \
                     : "cc", "memory", "r3"                       \
                    );                                            \
    _zzq_orig->nraddr = __addr;                                   \
  }

#define VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                   \
                     __SPECIAL_INSTRUCTION_PREAMBLE               \
       \
                     "or 3,3,3\n\t"

#define VALGRIND_VEX_INJECT_IR()                                 \
 do {                                                            \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE              \
                     "or 5,5,5\n\t"                              \
                    );                                           \
 } while (0)

#endif


#if defined(PLAT_ppc64_linux)

typedef
   struct {
      unsigned long long int nraddr;
      unsigned long long int r2;
   }
   OrigFn;

#define __SPECIAL_INSTRUCTION_PREAMBLE                            \
                     "rotldi 0,0,3  ; rotldi 0,0,13\n\t"          \
                     "rotldi 0,0,61 ; rotldi 0,0,51\n\t"

#define VALGRIND_DO_CLIENT_REQUEST_EXPR(                          \
        _zzq_default, _zzq_request,                               \
        _zzq_arg1, _zzq_arg2, _zzq_arg3, _zzq_arg4, _zzq_arg5)    \
                                                                  \
  __extension__                                                   \
  ({         unsigned long long int  _zzq_args[6];                \
             unsigned long long int  _zzq_result;                 \
             unsigned long long int* _zzq_ptr;                    \
    _zzq_args[0] = (unsigned long long int)(_zzq_request);        \
    _zzq_args[1] = (unsigned long long int)(_zzq_arg1);           \
    _zzq_args[2] = (unsigned long long int)(_zzq_arg2);           \
    _zzq_args[3] = (unsigned long long int)(_zzq_arg3);           \
    _zzq_args[4] = (unsigned long long int)(_zzq_arg4);           \
    _zzq_args[5] = (unsigned long long int)(_zzq_arg5);           \
    _zzq_ptr = _zzq_args;                                         \
    __asm__ volatile("mr 3,%1\n\t"                    \
                     "mr 4,%2\n\t"                        \
                     __SPECIAL_INSTRUCTION_PREAMBLE               \
           \
                     "or 1,1,1\n\t"                               \
                     "mr %0,3"                     \
                     : "=b" (_zzq_result)                         \
                     : "b" (_zzq_default), "b" (_zzq_ptr)         \
                     : "cc", "memory", "r3", "r4");               \
    _zzq_result;                                                  \
  })

#define VALGRIND_GET_NR_CONTEXT(_zzq_rlval)                       \
  { volatile OrigFn* _zzq_orig = &(_zzq_rlval);                   \
    unsigned long long int __addr;                                \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE               \
                     \
                     "or 2,2,2\n\t"                               \
                     "mr %0,3"                                    \
                     : "=b" (__addr)                              \
                     :                                            \
                     : "cc", "memory", "r3"                       \
                    );                                            \
    _zzq_orig->nraddr = __addr;                                   \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE               \
                \
                     "or 4,4,4\n\t"                               \
                     "mr %0,3"                                    \
                     : "=b" (__addr)                              \
                     :                                            \
                     : "cc", "memory", "r3"                       \
                    );                                            \
    _zzq_orig->r2 = __addr;                                       \
  }

#define VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                   \
                     __SPECIAL_INSTRUCTION_PREAMBLE               \
       \
                     "or 3,3,3\n\t"

#define VALGRIND_VEX_INJECT_IR()                                 \
 do {                                                            \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE              \
                     "or 5,5,5\n\t"                              \
                    );                                           \
 } while (0)

#endif


#if defined(PLAT_arm_linux)

typedef
   struct {
      unsigned int nraddr;
   }
   OrigFn;

#define __SPECIAL_INSTRUCTION_PREAMBLE                            \
            "mov r12, r12, ror #3  ; mov r12, r12, ror #13 \n\t"  \
            "mov r12, r12, ror #29 ; mov r12, r12, ror #19 \n\t"

#define VALGRIND_DO_CLIENT_REQUEST_EXPR(                          \
        _zzq_default, _zzq_request,                               \
        _zzq_arg1, _zzq_arg2, _zzq_arg3, _zzq_arg4, _zzq_arg5)    \
                                                                  \
  __extension__                                                   \
  ({volatile unsigned int  _zzq_args[6];                          \
    volatile unsigned int  _zzq_result;                           \
    _zzq_args[0] = (unsigned int)(_zzq_request);                  \
    _zzq_args[1] = (unsigned int)(_zzq_arg1);                     \
    _zzq_args[2] = (unsigned int)(_zzq_arg2);                     \
    _zzq_args[3] = (unsigned int)(_zzq_arg3);                     \
    _zzq_args[4] = (unsigned int)(_zzq_arg4);                     \
    _zzq_args[5] = (unsigned int)(_zzq_arg5);                     \
    __asm__ volatile("mov r3, %1\n\t"                 \
                     "mov r4, %2\n\t"                     \
                     __SPECIAL_INSTRUCTION_PREAMBLE               \
             \
                     "orr r10, r10, r10\n\t"                      \
                     "mov %0, r3"                  \
                     : "=r" (_zzq_result)                         \
                     : "r" (_zzq_default), "r" (&_zzq_args[0])    \
                     : "cc","memory", "r3", "r4");                \
    _zzq_result;                                                  \
  })

#define VALGRIND_GET_NR_CONTEXT(_zzq_rlval)                       \
  { volatile OrigFn* _zzq_orig = &(_zzq_rlval);                   \
    unsigned int __addr;                                          \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE               \
                      \
                     "orr r11, r11, r11\n\t"                      \
                     "mov %0, r3"                                 \
                     : "=r" (__addr)                              \
                     :                                            \
                     : "cc", "memory", "r3"                       \
                    );                                            \
    _zzq_orig->nraddr = __addr;                                   \
  }

#define VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R4                    \
                     __SPECIAL_INSTRUCTION_PREAMBLE               \
        \
                     "orr r12, r12, r12\n\t"

#define VALGRIND_VEX_INJECT_IR()                                 \
 do {                                                            \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE              \
                     "orr r9, r9, r9\n\t"                        \
                     : : : "cc", "memory"                        \
                    );                                           \
 } while (0)

#endif


#if defined(PLAT_arm64_linux)

typedef
   struct {
      unsigned long long int nraddr;
   }
   OrigFn;

#define __SPECIAL_INSTRUCTION_PREAMBLE                            \
            "ror x12, x12, #3  ;  ror x12, x12, #13 \n\t"         \
            "ror x12, x12, #51 ;  ror x12, x12, #61 \n\t"

#define VALGRIND_DO_CLIENT_REQUEST_EXPR(                          \
        _zzq_default, _zzq_request,                               \
        _zzq_arg1, _zzq_arg2, _zzq_arg3, _zzq_arg4, _zzq_arg5)    \
                                                                  \
  __extension__                                                   \
  ({volatile unsigned long long int  _zzq_args[6];                \
    volatile unsigned long long int  _zzq_result;                 \
    _zzq_args[0] = (unsigned long long int)(_zzq_request);        \
    _zzq_args[1] = (unsigned long long int)(_zzq_arg1);           \
    _zzq_args[2] = (unsigned long long int)(_zzq_arg2);           \
    _zzq_args[3] = (unsigned long long int)(_zzq_arg3);           \
    _zzq_args[4] = (unsigned long long int)(_zzq_arg4);           \
    _zzq_args[5] = (unsigned long long int)(_zzq_arg5);           \
    __asm__ volatile("mov x3, %1\n\t"                 \
                     "mov x4, %2\n\t"                     \
                     __SPECIAL_INSTRUCTION_PREAMBLE               \
             \
                     "orr x10, x10, x10\n\t"                      \
                     "mov %0, x3"                  \
                     : "=r" (_zzq_result)                         \
                     : "r" ((unsigned long long int)(_zzq_default)), "r" (&_zzq_args[0])    \
                     : "cc","memory", "x3", "x4");                \
    _zzq_result;                                                  \
  })

#define VALGRIND_GET_NR_CONTEXT(_zzq_rlval)                       \
  { volatile OrigFn* _zzq_orig = &(_zzq_rlval);                   \
    unsigned long long int __addr;                                \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE               \
                      \
                     "orr x11, x11, x11\n\t"                      \
                     "mov %0, x3"                                 \
                     : "=r" (__addr)                              \
                     :                                            \
                     : "cc", "memory", "x3"                       \
                    );                                            \
    _zzq_orig->nraddr = __addr;                                   \
  }

#define VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_X8                    \
                     __SPECIAL_INSTRUCTION_PREAMBLE               \
          \
                     "orr x12, x12, x12\n\t"

#define VALGRIND_VEX_INJECT_IR()                                 \
 do {                                                            \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE              \
                     "orr x9, x9, x9\n\t"                        \
                     : : : "cc", "memory"                        \
                    );                                           \
 } while (0)

#endif


#if defined(PLAT_s390x_linux)

typedef
  struct {
     unsigned long long int nraddr;
  }
  OrigFn;

#define __SPECIAL_INSTRUCTION_PREAMBLE                           \
                     "lr 15,15\n\t"                              \
                     "lr 1,1\n\t"                                \
                     "lr 2,2\n\t"                                \
                     "lr 3,3\n\t"

#define __CLIENT_REQUEST_CODE "lr 2,2\n\t"
#define __GET_NR_CONTEXT_CODE "lr 3,3\n\t"
#define __CALL_NO_REDIR_CODE  "lr 4,4\n\t"
#define __VEX_INJECT_IR_CODE  "lr 5,5\n\t"

#define VALGRIND_DO_CLIENT_REQUEST_EXPR(                         \
       _zzq_default, _zzq_request,                               \
       _zzq_arg1, _zzq_arg2, _zzq_arg3, _zzq_arg4, _zzq_arg5)    \
  __extension__                                                  \
 ({volatile unsigned long long int _zzq_args[6];                 \
   volatile unsigned long long int _zzq_result;                  \
   _zzq_args[0] = (unsigned long long int)(_zzq_request);        \
   _zzq_args[1] = (unsigned long long int)(_zzq_arg1);           \
   _zzq_args[2] = (unsigned long long int)(_zzq_arg2);           \
   _zzq_args[3] = (unsigned long long int)(_zzq_arg3);           \
   _zzq_args[4] = (unsigned long long int)(_zzq_arg4);           \
   _zzq_args[5] = (unsigned long long int)(_zzq_arg5);           \
   __asm__ volatile(                              \
                    "lgr 2,%1\n\t"                               \
                           \
                    "lgr 3,%2\n\t"                               \
                    __SPECIAL_INSTRUCTION_PREAMBLE               \
                    __CLIENT_REQUEST_CODE                        \
                           \
                    "lgr %0, 3\n\t"                              \
                    : "=d" (_zzq_result)                         \
                    : "a" (&_zzq_args[0]), "0" (_zzq_default)    \
                    : "cc", "2", "3", "memory"                   \
                   );                                            \
   _zzq_result;                                                  \
 })

#define VALGRIND_GET_NR_CONTEXT(_zzq_rlval)                      \
 { volatile OrigFn* _zzq_orig = &(_zzq_rlval);                   \
   volatile unsigned long long int __addr;                       \
   __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE               \
                    __GET_NR_CONTEXT_CODE                        \
                    "lgr %0, 3\n\t"                              \
                    : "=a" (__addr)                              \
                    :                                            \
                    : "cc", "3", "memory"                        \
                   );                                            \
   _zzq_orig->nraddr = __addr;                                   \
 }

#define VALGRIND_CALL_NOREDIR_R1                                 \
                    __SPECIAL_INSTRUCTION_PREAMBLE               \
                    __CALL_NO_REDIR_CODE

#define VALGRIND_VEX_INJECT_IR()                                 \
 do {                                                            \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE              \
                     __VEX_INJECT_IR_CODE);                      \
 } while (0)

#endif


#if defined(PLAT_mips32_linux)

typedef
   struct {
      unsigned int nraddr;
   }
   OrigFn;

#define __SPECIAL_INSTRUCTION_PREAMBLE          \
                     "srl $0, $0, 13\n\t"       \
                     "srl $0, $0, 29\n\t"       \
                     "srl $0, $0, 3\n\t"        \
                     "srl $0, $0, 19\n\t"

#define VALGRIND_DO_CLIENT_REQUEST_EXPR(                          \
       _zzq_default, _zzq_request,                                \
       _zzq_arg1, _zzq_arg2, _zzq_arg3, _zzq_arg4, _zzq_arg5)     \
  __extension__                                                   \
  ({ volatile unsigned int _zzq_args[6];                          \
    volatile unsigned int _zzq_result;                            \
    _zzq_args[0] = (unsigned int)(_zzq_request);                  \
    _zzq_args[1] = (unsigned int)(_zzq_arg1);                     \
    _zzq_args[2] = (unsigned int)(_zzq_arg2);                     \
    _zzq_args[3] = (unsigned int)(_zzq_arg3);                     \
    _zzq_args[4] = (unsigned int)(_zzq_arg4);                     \
    _zzq_args[5] = (unsigned int)(_zzq_arg5);                     \
        __asm__ volatile("move $11, %1\n\t"           \
                     "move $12, %2\n\t"                   \
                     __SPECIAL_INSTRUCTION_PREAMBLE               \
             \
                     "or $13, $13, $13\n\t"                       \
                     "move %0, $11\n\t"            \
                     : "=r" (_zzq_result)                         \
                     : "r" (_zzq_default), "r" (&_zzq_args[0])    \
                     : "$11", "$12");                             \
    _zzq_result;                                                  \
  })

#define VALGRIND_GET_NR_CONTEXT(_zzq_rlval)                       \
  { volatile OrigFn* _zzq_orig = &(_zzq_rlval);                   \
    volatile unsigned int __addr;                                 \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE               \
                     \
                     "or $14, $14, $14\n\t"                       \
                     "move %0, $11"                \
                     : "=r" (__addr)                              \
                     :                                            \
                     : "$11"                                      \
                    );                                            \
    _zzq_orig->nraddr = __addr;                                   \
  }

#define VALGRIND_CALL_NOREDIR_T9                                 \
                     __SPECIAL_INSTRUCTION_PREAMBLE              \
                     \
                     "or $15, $15, $15\n\t"

#define VALGRIND_VEX_INJECT_IR()                                 \
 do {                                                            \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE              \
                     "or $11, $11, $11\n\t"                      \
                    );                                           \
 } while (0)


#endif


#if defined(PLAT_mips64_linux)

typedef
   struct {
      unsigned long long nraddr;
   }
   OrigFn;

#define __SPECIAL_INSTRUCTION_PREAMBLE                              \
                     "dsll $0,$0, 3 ; dsll $0,$0,13\n\t"            \
                     "dsll $0,$0,29 ; dsll $0,$0,19\n\t"

#define VALGRIND_DO_CLIENT_REQUEST_EXPR(                            \
       _zzq_default, _zzq_request,                                  \
       _zzq_arg1, _zzq_arg2, _zzq_arg3, _zzq_arg4, _zzq_arg5)       \
  __extension__                                                     \
  ({ volatile unsigned long long int _zzq_args[6];                  \
    volatile unsigned long long int _zzq_result;                    \
    _zzq_args[0] = (unsigned long long int)(_zzq_request);          \
    _zzq_args[1] = (unsigned long long int)(_zzq_arg1);             \
    _zzq_args[2] = (unsigned long long int)(_zzq_arg2);             \
    _zzq_args[3] = (unsigned long long int)(_zzq_arg3);             \
    _zzq_args[4] = (unsigned long long int)(_zzq_arg4);             \
    _zzq_args[5] = (unsigned long long int)(_zzq_arg5);             \
        __asm__ volatile("move $11, %1\n\t"             \
                         "move $12, %2\n\t"                 \
                         __SPECIAL_INSTRUCTION_PREAMBLE             \
         \
                         "or $13, $13, $13\n\t"                     \
                         "move %0, $11\n\t"          \
                         : "=r" (_zzq_result)                       \
                         : "r" (_zzq_default), "r" (&_zzq_args[0])  \
                         : "$11", "$12");                           \
    _zzq_result;                                                    \
  })

#define VALGRIND_GET_NR_CONTEXT(_zzq_rlval)                         \
  { volatile OrigFn* _zzq_orig = &(_zzq_rlval);                     \
    volatile unsigned long long int __addr;                         \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE                 \
                       \
                     "or $14, $14, $14\n\t"                         \
                     "move %0, $11"                  \
                     : "=r" (__addr)                                \
                     :                                              \
                     : "$11");                                      \
    _zzq_orig->nraddr = __addr;                                     \
  }

#define VALGRIND_CALL_NOREDIR_T9                                    \
                     __SPECIAL_INSTRUCTION_PREAMBLE                 \
                         \
                     "or $15, $15, $15\n\t"

#define VALGRIND_VEX_INJECT_IR()                                    \
 do {                                                               \
    __asm__ volatile(__SPECIAL_INSTRUCTION_PREAMBLE                 \
                     "or $11, $11, $11\n\t"                         \
                    );                                              \
 } while (0)

#endif


#endif





#define VG_CONCAT4(_aa,_bb,_cc,_dd) _aa##_bb##_cc##_dd

#define I_WRAP_SONAME_FNNAME_ZU(soname,fnname)                    \
   VG_CONCAT4(_vgw00000ZU_,soname,_,fnname)

#define I_WRAP_SONAME_FNNAME_ZZ(soname,fnname)                    \
   VG_CONCAT4(_vgw00000ZZ_,soname,_,fnname)

#define VALGRIND_GET_ORIG_FN(_lval)  VALGRIND_GET_NR_CONTEXT(_lval)


#define I_REPLACE_SONAME_FNNAME_ZU(soname,fnname)                 \
   VG_CONCAT4(_vgr00000ZU_,soname,_,fnname)

#define I_REPLACE_SONAME_FNNAME_ZZ(soname,fnname)                 \
   VG_CONCAT4(_vgr00000ZZ_,soname,_,fnname)


#define CALL_FN_v_v(fnptr)                                        \
   do { volatile unsigned long _junk;                             \
        CALL_FN_W_v(_junk,fnptr); } while (0)

#define CALL_FN_v_W(fnptr, arg1)                                  \
   do { volatile unsigned long _junk;                             \
        CALL_FN_W_W(_junk,fnptr,arg1); } while (0)

#define CALL_FN_v_WW(fnptr, arg1,arg2)                            \
   do { volatile unsigned long _junk;                             \
        CALL_FN_W_WW(_junk,fnptr,arg1,arg2); } while (0)

#define CALL_FN_v_WWW(fnptr, arg1,arg2,arg3)                      \
   do { volatile unsigned long _junk;                             \
        CALL_FN_W_WWW(_junk,fnptr,arg1,arg2,arg3); } while (0)

#define CALL_FN_v_WWWW(fnptr, arg1,arg2,arg3,arg4)                \
   do { volatile unsigned long _junk;                             \
        CALL_FN_W_WWWW(_junk,fnptr,arg1,arg2,arg3,arg4); } while (0)

#define CALL_FN_v_5W(fnptr, arg1,arg2,arg3,arg4,arg5)             \
   do { volatile unsigned long _junk;                             \
        CALL_FN_W_5W(_junk,fnptr,arg1,arg2,arg3,arg4,arg5); } while (0)

#define CALL_FN_v_6W(fnptr, arg1,arg2,arg3,arg4,arg5,arg6)        \
   do { volatile unsigned long _junk;                             \
        CALL_FN_W_6W(_junk,fnptr,arg1,arg2,arg3,arg4,arg5,arg6); } while (0)

#define CALL_FN_v_7W(fnptr, arg1,arg2,arg3,arg4,arg5,arg6,arg7)   \
   do { volatile unsigned long _junk;                             \
        CALL_FN_W_7W(_junk,fnptr,arg1,arg2,arg3,arg4,arg5,arg6,arg7); } while (0)


#if defined(PLAT_x86_linux)  ||  defined(PLAT_x86_darwin)

#define __CALLER_SAVED_REGS "ecx", "edx"


#define VALGRIND_ALIGN_STACK               \
      "movl %%esp,%%edi\n\t"               \
      "andl $0xfffffff0,%%esp\n\t"
#define VALGRIND_RESTORE_STACK             \
      "movl %%edi,%%esp\n\t"


#define CALL_FN_W_v(lval, orig)                                   \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[1];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "movl (%%eax), %%eax\n\t"            \
         VALGRIND_CALL_NOREDIR_EAX                                \
         VALGRIND_RESTORE_STACK                                   \
         :   "=a" (_res)                                  \
         :    "a" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "edi"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_W(lval, orig, arg1)                             \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[2];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "subl $12, %%esp\n\t"                                    \
         "pushl 4(%%eax)\n\t"                                     \
         "movl (%%eax), %%eax\n\t"            \
         VALGRIND_CALL_NOREDIR_EAX                                \
         VALGRIND_RESTORE_STACK                                   \
         :   "=a" (_res)                                  \
         :    "a" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "edi"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WW(lval, orig, arg1,arg2)                       \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[3];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "subl $8, %%esp\n\t"                                     \
         "pushl 8(%%eax)\n\t"                                     \
         "pushl 4(%%eax)\n\t"                                     \
         "movl (%%eax), %%eax\n\t"            \
         VALGRIND_CALL_NOREDIR_EAX                                \
         VALGRIND_RESTORE_STACK                                   \
         :   "=a" (_res)                                  \
         :    "a" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "edi"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WWW(lval, orig, arg1,arg2,arg3)                 \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[4];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "subl $4, %%esp\n\t"                                     \
         "pushl 12(%%eax)\n\t"                                    \
         "pushl 8(%%eax)\n\t"                                     \
         "pushl 4(%%eax)\n\t"                                     \
         "movl (%%eax), %%eax\n\t"            \
         VALGRIND_CALL_NOREDIR_EAX                                \
         VALGRIND_RESTORE_STACK                                   \
         :   "=a" (_res)                                  \
         :    "a" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "edi"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WWWW(lval, orig, arg1,arg2,arg3,arg4)           \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[5];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "pushl 16(%%eax)\n\t"                                    \
         "pushl 12(%%eax)\n\t"                                    \
         "pushl 8(%%eax)\n\t"                                     \
         "pushl 4(%%eax)\n\t"                                     \
         "movl (%%eax), %%eax\n\t"            \
         VALGRIND_CALL_NOREDIR_EAX                                \
         VALGRIND_RESTORE_STACK                                   \
         :   "=a" (_res)                                  \
         :    "a" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "edi"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_5W(lval, orig, arg1,arg2,arg3,arg4,arg5)        \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[6];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "subl $12, %%esp\n\t"                                    \
         "pushl 20(%%eax)\n\t"                                    \
         "pushl 16(%%eax)\n\t"                                    \
         "pushl 12(%%eax)\n\t"                                    \
         "pushl 8(%%eax)\n\t"                                     \
         "pushl 4(%%eax)\n\t"                                     \
         "movl (%%eax), %%eax\n\t"            \
         VALGRIND_CALL_NOREDIR_EAX                                \
         VALGRIND_RESTORE_STACK                                   \
         :   "=a" (_res)                                  \
         :    "a" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "edi"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_6W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6)   \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[7];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "subl $8, %%esp\n\t"                                     \
         "pushl 24(%%eax)\n\t"                                    \
         "pushl 20(%%eax)\n\t"                                    \
         "pushl 16(%%eax)\n\t"                                    \
         "pushl 12(%%eax)\n\t"                                    \
         "pushl 8(%%eax)\n\t"                                     \
         "pushl 4(%%eax)\n\t"                                     \
         "movl (%%eax), %%eax\n\t"            \
         VALGRIND_CALL_NOREDIR_EAX                                \
         VALGRIND_RESTORE_STACK                                   \
         :   "=a" (_res)                                  \
         :    "a" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "edi"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_7W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,   \
                                 arg7)                            \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[8];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "subl $4, %%esp\n\t"                                     \
         "pushl 28(%%eax)\n\t"                                    \
         "pushl 24(%%eax)\n\t"                                    \
         "pushl 20(%%eax)\n\t"                                    \
         "pushl 16(%%eax)\n\t"                                    \
         "pushl 12(%%eax)\n\t"                                    \
         "pushl 8(%%eax)\n\t"                                     \
         "pushl 4(%%eax)\n\t"                                     \
         "movl (%%eax), %%eax\n\t"            \
         VALGRIND_CALL_NOREDIR_EAX                                \
         VALGRIND_RESTORE_STACK                                   \
         :   "=a" (_res)                                  \
         :    "a" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "edi"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_8W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,   \
                                 arg7,arg8)                       \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[9];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "pushl 32(%%eax)\n\t"                                    \
         "pushl 28(%%eax)\n\t"                                    \
         "pushl 24(%%eax)\n\t"                                    \
         "pushl 20(%%eax)\n\t"                                    \
         "pushl 16(%%eax)\n\t"                                    \
         "pushl 12(%%eax)\n\t"                                    \
         "pushl 8(%%eax)\n\t"                                     \
         "pushl 4(%%eax)\n\t"                                     \
         "movl (%%eax), %%eax\n\t"            \
         VALGRIND_CALL_NOREDIR_EAX                                \
         VALGRIND_RESTORE_STACK                                   \
         :   "=a" (_res)                                  \
         :    "a" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "edi"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_9W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,   \
                                 arg7,arg8,arg9)                  \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[10];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      _argvec[9] = (unsigned long)(arg9);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "subl $12, %%esp\n\t"                                    \
         "pushl 36(%%eax)\n\t"                                    \
         "pushl 32(%%eax)\n\t"                                    \
         "pushl 28(%%eax)\n\t"                                    \
         "pushl 24(%%eax)\n\t"                                    \
         "pushl 20(%%eax)\n\t"                                    \
         "pushl 16(%%eax)\n\t"                                    \
         "pushl 12(%%eax)\n\t"                                    \
         "pushl 8(%%eax)\n\t"                                     \
         "pushl 4(%%eax)\n\t"                                     \
         "movl (%%eax), %%eax\n\t"            \
         VALGRIND_CALL_NOREDIR_EAX                                \
         VALGRIND_RESTORE_STACK                                   \
         :   "=a" (_res)                                  \
         :    "a" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "edi"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_10W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,  \
                                  arg7,arg8,arg9,arg10)           \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[11];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      _argvec[9] = (unsigned long)(arg9);                         \
      _argvec[10] = (unsigned long)(arg10);                       \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "subl $8, %%esp\n\t"                                     \
         "pushl 40(%%eax)\n\t"                                    \
         "pushl 36(%%eax)\n\t"                                    \
         "pushl 32(%%eax)\n\t"                                    \
         "pushl 28(%%eax)\n\t"                                    \
         "pushl 24(%%eax)\n\t"                                    \
         "pushl 20(%%eax)\n\t"                                    \
         "pushl 16(%%eax)\n\t"                                    \
         "pushl 12(%%eax)\n\t"                                    \
         "pushl 8(%%eax)\n\t"                                     \
         "pushl 4(%%eax)\n\t"                                     \
         "movl (%%eax), %%eax\n\t"            \
         VALGRIND_CALL_NOREDIR_EAX                                \
         VALGRIND_RESTORE_STACK                                   \
         :   "=a" (_res)                                  \
         :    "a" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "edi"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_11W(lval, orig, arg1,arg2,arg3,arg4,arg5,       \
                                  arg6,arg7,arg8,arg9,arg10,      \
                                  arg11)                          \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[12];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      _argvec[9] = (unsigned long)(arg9);                         \
      _argvec[10] = (unsigned long)(arg10);                       \
      _argvec[11] = (unsigned long)(arg11);                       \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "subl $4, %%esp\n\t"                                     \
         "pushl 44(%%eax)\n\t"                                    \
         "pushl 40(%%eax)\n\t"                                    \
         "pushl 36(%%eax)\n\t"                                    \
         "pushl 32(%%eax)\n\t"                                    \
         "pushl 28(%%eax)\n\t"                                    \
         "pushl 24(%%eax)\n\t"                                    \
         "pushl 20(%%eax)\n\t"                                    \
         "pushl 16(%%eax)\n\t"                                    \
         "pushl 12(%%eax)\n\t"                                    \
         "pushl 8(%%eax)\n\t"                                     \
         "pushl 4(%%eax)\n\t"                                     \
         "movl (%%eax), %%eax\n\t"            \
         VALGRIND_CALL_NOREDIR_EAX                                \
         VALGRIND_RESTORE_STACK                                   \
         :   "=a" (_res)                                  \
         :    "a" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "edi"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_12W(lval, orig, arg1,arg2,arg3,arg4,arg5,       \
                                  arg6,arg7,arg8,arg9,arg10,      \
                                  arg11,arg12)                    \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[13];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      _argvec[9] = (unsigned long)(arg9);                         \
      _argvec[10] = (unsigned long)(arg10);                       \
      _argvec[11] = (unsigned long)(arg11);                       \
      _argvec[12] = (unsigned long)(arg12);                       \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "pushl 48(%%eax)\n\t"                                    \
         "pushl 44(%%eax)\n\t"                                    \
         "pushl 40(%%eax)\n\t"                                    \
         "pushl 36(%%eax)\n\t"                                    \
         "pushl 32(%%eax)\n\t"                                    \
         "pushl 28(%%eax)\n\t"                                    \
         "pushl 24(%%eax)\n\t"                                    \
         "pushl 20(%%eax)\n\t"                                    \
         "pushl 16(%%eax)\n\t"                                    \
         "pushl 12(%%eax)\n\t"                                    \
         "pushl 8(%%eax)\n\t"                                     \
         "pushl 4(%%eax)\n\t"                                     \
         "movl (%%eax), %%eax\n\t"            \
         VALGRIND_CALL_NOREDIR_EAX                                \
         VALGRIND_RESTORE_STACK                                   \
         :   "=a" (_res)                                  \
         :    "a" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "edi"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#endif


#if defined(PLAT_amd64_linux)  ||  defined(PLAT_amd64_darwin)


#define __CALLER_SAVED_REGS "rcx", "rdx", "rsi",       \
                            "rdi", "r8", "r9", "r10", "r11"

#if defined(__GNUC__) && defined(__GCC_HAVE_DWARF2_CFI_ASM)
#  define __FRAME_POINTER                                         \
      ,"r"(__builtin_dwarf_cfa())
#  define VALGRIND_CFI_PROLOGUE                                   \
      "movq %%rbp, %%r15\n\t"                                     \
      "movq %2, %%rbp\n\t"                                        \
      ".cfi_remember_state\n\t"                                   \
      ".cfi_def_cfa rbp, 0\n\t"
#  define VALGRIND_CFI_EPILOGUE                                   \
      "movq %%r15, %%rbp\n\t"                                     \
      ".cfi_restore_state\n\t"
#else
#  define __FRAME_POINTER
#  define VALGRIND_CFI_PROLOGUE
#  define VALGRIND_CFI_EPILOGUE
#endif


#define VALGRIND_ALIGN_STACK               \
      "movq %%rsp,%%r14\n\t"               \
      "andq $0xfffffffffffffff0,%%rsp\n\t"
#define VALGRIND_RESTORE_STACK             \
      "movq %%r14,%%rsp\n\t"



#define CALL_FN_W_v(lval, orig)                                        \
   do {                                                                \
      volatile OrigFn        _orig = (orig);                           \
      volatile unsigned long _argvec[1];                               \
      volatile unsigned long _res;                                     \
      _argvec[0] = (unsigned long)_orig.nraddr;                        \
      __asm__ volatile(                                                \
         VALGRIND_CFI_PROLOGUE                                         \
         VALGRIND_ALIGN_STACK                                          \
         "subq $128,%%rsp\n\t"                                         \
         "movq (%%rax), %%rax\n\t"                 \
         VALGRIND_CALL_NOREDIR_RAX                                     \
         VALGRIND_RESTORE_STACK                                        \
         VALGRIND_CFI_EPILOGUE                                         \
         :   "=a" (_res)                                       \
         :    "a" (&_argvec[0]) __FRAME_POINTER                 \
         : "cc", "memory", __CALLER_SAVED_REGS, "r14", "r15" \
      );                                                               \
      lval = (__typeof__(lval)) _res;                                  \
   } while (0)

#define CALL_FN_W_W(lval, orig, arg1)                                  \
   do {                                                                \
      volatile OrigFn        _orig = (orig);                           \
      volatile unsigned long _argvec[2];                               \
      volatile unsigned long _res;                                     \
      _argvec[0] = (unsigned long)_orig.nraddr;                        \
      _argvec[1] = (unsigned long)(arg1);                              \
      __asm__ volatile(                                                \
         VALGRIND_CFI_PROLOGUE                                         \
         VALGRIND_ALIGN_STACK                                          \
         "subq $128,%%rsp\n\t"                                         \
         "movq 8(%%rax), %%rdi\n\t"                                    \
         "movq (%%rax), %%rax\n\t"                 \
         VALGRIND_CALL_NOREDIR_RAX                                     \
         VALGRIND_RESTORE_STACK                                        \
         VALGRIND_CFI_EPILOGUE                                         \
         :   "=a" (_res)                                       \
         :    "a" (&_argvec[0]) __FRAME_POINTER                 \
         : "cc", "memory", __CALLER_SAVED_REGS, "r14", "r15" \
      );                                                               \
      lval = (__typeof__(lval)) _res;                                  \
   } while (0)

#define CALL_FN_W_WW(lval, orig, arg1,arg2)                            \
   do {                                                                \
      volatile OrigFn        _orig = (orig);                           \
      volatile unsigned long _argvec[3];                               \
      volatile unsigned long _res;                                     \
      _argvec[0] = (unsigned long)_orig.nraddr;                        \
      _argvec[1] = (unsigned long)(arg1);                              \
      _argvec[2] = (unsigned long)(arg2);                              \
      __asm__ volatile(                                                \
         VALGRIND_CFI_PROLOGUE                                         \
         VALGRIND_ALIGN_STACK                                          \
         "subq $128,%%rsp\n\t"                                         \
         "movq 16(%%rax), %%rsi\n\t"                                   \
         "movq 8(%%rax), %%rdi\n\t"                                    \
         "movq (%%rax), %%rax\n\t"                 \
         VALGRIND_CALL_NOREDIR_RAX                                     \
         VALGRIND_RESTORE_STACK                                        \
         VALGRIND_CFI_EPILOGUE                                         \
         :   "=a" (_res)                                       \
         :    "a" (&_argvec[0]) __FRAME_POINTER                 \
         : "cc", "memory", __CALLER_SAVED_REGS, "r14", "r15" \
      );                                                               \
      lval = (__typeof__(lval)) _res;                                  \
   } while (0)

#define CALL_FN_W_WWW(lval, orig, arg1,arg2,arg3)                      \
   do {                                                                \
      volatile OrigFn        _orig = (orig);                           \
      volatile unsigned long _argvec[4];                               \
      volatile unsigned long _res;                                     \
      _argvec[0] = (unsigned long)_orig.nraddr;                        \
      _argvec[1] = (unsigned long)(arg1);                              \
      _argvec[2] = (unsigned long)(arg2);                              \
      _argvec[3] = (unsigned long)(arg3);                              \
      __asm__ volatile(                                                \
         VALGRIND_CFI_PROLOGUE                                         \
         VALGRIND_ALIGN_STACK                                          \
         "subq $128,%%rsp\n\t"                                         \
         "movq 24(%%rax), %%rdx\n\t"                                   \
         "movq 16(%%rax), %%rsi\n\t"                                   \
         "movq 8(%%rax), %%rdi\n\t"                                    \
         "movq (%%rax), %%rax\n\t"                 \
         VALGRIND_CALL_NOREDIR_RAX                                     \
         VALGRIND_RESTORE_STACK                                        \
         VALGRIND_CFI_EPILOGUE                                         \
         :   "=a" (_res)                                       \
         :    "a" (&_argvec[0]) __FRAME_POINTER                 \
         : "cc", "memory", __CALLER_SAVED_REGS, "r14", "r15" \
      );                                                               \
      lval = (__typeof__(lval)) _res;                                  \
   } while (0)

#define CALL_FN_W_WWWW(lval, orig, arg1,arg2,arg3,arg4)                \
   do {                                                                \
      volatile OrigFn        _orig = (orig);                           \
      volatile unsigned long _argvec[5];                               \
      volatile unsigned long _res;                                     \
      _argvec[0] = (unsigned long)_orig.nraddr;                        \
      _argvec[1] = (unsigned long)(arg1);                              \
      _argvec[2] = (unsigned long)(arg2);                              \
      _argvec[3] = (unsigned long)(arg3);                              \
      _argvec[4] = (unsigned long)(arg4);                              \
      __asm__ volatile(                                                \
         VALGRIND_CFI_PROLOGUE                                         \
         VALGRIND_ALIGN_STACK                                          \
         "subq $128,%%rsp\n\t"                                         \
         "movq 32(%%rax), %%rcx\n\t"                                   \
         "movq 24(%%rax), %%rdx\n\t"                                   \
         "movq 16(%%rax), %%rsi\n\t"                                   \
         "movq 8(%%rax), %%rdi\n\t"                                    \
         "movq (%%rax), %%rax\n\t"                 \
         VALGRIND_CALL_NOREDIR_RAX                                     \
         VALGRIND_RESTORE_STACK                                        \
         VALGRIND_CFI_EPILOGUE                                         \
         :   "=a" (_res)                                       \
         :    "a" (&_argvec[0]) __FRAME_POINTER                 \
         : "cc", "memory", __CALLER_SAVED_REGS, "r14", "r15" \
      );                                                               \
      lval = (__typeof__(lval)) _res;                                  \
   } while (0)

#define CALL_FN_W_5W(lval, orig, arg1,arg2,arg3,arg4,arg5)             \
   do {                                                                \
      volatile OrigFn        _orig = (orig);                           \
      volatile unsigned long _argvec[6];                               \
      volatile unsigned long _res;                                     \
      _argvec[0] = (unsigned long)_orig.nraddr;                        \
      _argvec[1] = (unsigned long)(arg1);                              \
      _argvec[2] = (unsigned long)(arg2);                              \
      _argvec[3] = (unsigned long)(arg3);                              \
      _argvec[4] = (unsigned long)(arg4);                              \
      _argvec[5] = (unsigned long)(arg5);                              \
      __asm__ volatile(                                                \
         VALGRIND_CFI_PROLOGUE                                         \
         VALGRIND_ALIGN_STACK                                          \
         "subq $128,%%rsp\n\t"                                         \
         "movq 40(%%rax), %%r8\n\t"                                    \
         "movq 32(%%rax), %%rcx\n\t"                                   \
         "movq 24(%%rax), %%rdx\n\t"                                   \
         "movq 16(%%rax), %%rsi\n\t"                                   \
         "movq 8(%%rax), %%rdi\n\t"                                    \
         "movq (%%rax), %%rax\n\t"                 \
         VALGRIND_CALL_NOREDIR_RAX                                     \
         VALGRIND_RESTORE_STACK                                        \
         VALGRIND_CFI_EPILOGUE                                         \
         :   "=a" (_res)                                       \
         :    "a" (&_argvec[0]) __FRAME_POINTER                 \
         : "cc", "memory", __CALLER_SAVED_REGS, "r14", "r15" \
      );                                                               \
      lval = (__typeof__(lval)) _res;                                  \
   } while (0)

#define CALL_FN_W_6W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6)        \
   do {                                                                \
      volatile OrigFn        _orig = (orig);                           \
      volatile unsigned long _argvec[7];                               \
      volatile unsigned long _res;                                     \
      _argvec[0] = (unsigned long)_orig.nraddr;                        \
      _argvec[1] = (unsigned long)(arg1);                              \
      _argvec[2] = (unsigned long)(arg2);                              \
      _argvec[3] = (unsigned long)(arg3);                              \
      _argvec[4] = (unsigned long)(arg4);                              \
      _argvec[5] = (unsigned long)(arg5);                              \
      _argvec[6] = (unsigned long)(arg6);                              \
      __asm__ volatile(                                                \
         VALGRIND_CFI_PROLOGUE                                         \
         VALGRIND_ALIGN_STACK                                          \
         "subq $128,%%rsp\n\t"                                         \
         "movq 48(%%rax), %%r9\n\t"                                    \
         "movq 40(%%rax), %%r8\n\t"                                    \
         "movq 32(%%rax), %%rcx\n\t"                                   \
         "movq 24(%%rax), %%rdx\n\t"                                   \
         "movq 16(%%rax), %%rsi\n\t"                                   \
         "movq 8(%%rax), %%rdi\n\t"                                    \
         "movq (%%rax), %%rax\n\t"                 \
         VALGRIND_CALL_NOREDIR_RAX                                     \
         VALGRIND_RESTORE_STACK                                        \
         VALGRIND_CFI_EPILOGUE                                         \
         :   "=a" (_res)                                       \
         :    "a" (&_argvec[0]) __FRAME_POINTER                 \
         : "cc", "memory", __CALLER_SAVED_REGS, "r14", "r15" \
      );                                                               \
      lval = (__typeof__(lval)) _res;                                  \
   } while (0)

#define CALL_FN_W_7W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,        \
                                 arg7)                                 \
   do {                                                                \
      volatile OrigFn        _orig = (orig);                           \
      volatile unsigned long _argvec[8];                               \
      volatile unsigned long _res;                                     \
      _argvec[0] = (unsigned long)_orig.nraddr;                        \
      _argvec[1] = (unsigned long)(arg1);                              \
      _argvec[2] = (unsigned long)(arg2);                              \
      _argvec[3] = (unsigned long)(arg3);                              \
      _argvec[4] = (unsigned long)(arg4);                              \
      _argvec[5] = (unsigned long)(arg5);                              \
      _argvec[6] = (unsigned long)(arg6);                              \
      _argvec[7] = (unsigned long)(arg7);                              \
      __asm__ volatile(                                                \
         VALGRIND_CFI_PROLOGUE                                         \
         VALGRIND_ALIGN_STACK                                          \
         "subq $136,%%rsp\n\t"                                         \
         "pushq 56(%%rax)\n\t"                                         \
         "movq 48(%%rax), %%r9\n\t"                                    \
         "movq 40(%%rax), %%r8\n\t"                                    \
         "movq 32(%%rax), %%rcx\n\t"                                   \
         "movq 24(%%rax), %%rdx\n\t"                                   \
         "movq 16(%%rax), %%rsi\n\t"                                   \
         "movq 8(%%rax), %%rdi\n\t"                                    \
         "movq (%%rax), %%rax\n\t"                 \
         VALGRIND_CALL_NOREDIR_RAX                                     \
         VALGRIND_RESTORE_STACK                                        \
         VALGRIND_CFI_EPILOGUE                                         \
         :   "=a" (_res)                                       \
         :    "a" (&_argvec[0]) __FRAME_POINTER                 \
         : "cc", "memory", __CALLER_SAVED_REGS, "r14", "r15" \
      );                                                               \
      lval = (__typeof__(lval)) _res;                                  \
   } while (0)

#define CALL_FN_W_8W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,        \
                                 arg7,arg8)                            \
   do {                                                                \
      volatile OrigFn        _orig = (orig);                           \
      volatile unsigned long _argvec[9];                               \
      volatile unsigned long _res;                                     \
      _argvec[0] = (unsigned long)_orig.nraddr;                        \
      _argvec[1] = (unsigned long)(arg1);                              \
      _argvec[2] = (unsigned long)(arg2);                              \
      _argvec[3] = (unsigned long)(arg3);                              \
      _argvec[4] = (unsigned long)(arg4);                              \
      _argvec[5] = (unsigned long)(arg5);                              \
      _argvec[6] = (unsigned long)(arg6);                              \
      _argvec[7] = (unsigned long)(arg7);                              \
      _argvec[8] = (unsigned long)(arg8);                              \
      __asm__ volatile(                                                \
         VALGRIND_CFI_PROLOGUE                                         \
         VALGRIND_ALIGN_STACK                                          \
         "subq $128,%%rsp\n\t"                                         \
         "pushq 64(%%rax)\n\t"                                         \
         "pushq 56(%%rax)\n\t"                                         \
         "movq 48(%%rax), %%r9\n\t"                                    \
         "movq 40(%%rax), %%r8\n\t"                                    \
         "movq 32(%%rax), %%rcx\n\t"                                   \
         "movq 24(%%rax), %%rdx\n\t"                                   \
         "movq 16(%%rax), %%rsi\n\t"                                   \
         "movq 8(%%rax), %%rdi\n\t"                                    \
         "movq (%%rax), %%rax\n\t"                 \
         VALGRIND_CALL_NOREDIR_RAX                                     \
         VALGRIND_RESTORE_STACK                                        \
         VALGRIND_CFI_EPILOGUE                                         \
         :   "=a" (_res)                                       \
         :    "a" (&_argvec[0]) __FRAME_POINTER                 \
         : "cc", "memory", __CALLER_SAVED_REGS, "r14", "r15" \
      );                                                               \
      lval = (__typeof__(lval)) _res;                                  \
   } while (0)

#define CALL_FN_W_9W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,        \
                                 arg7,arg8,arg9)                       \
   do {                                                                \
      volatile OrigFn        _orig = (orig);                           \
      volatile unsigned long _argvec[10];                              \
      volatile unsigned long _res;                                     \
      _argvec[0] = (unsigned long)_orig.nraddr;                        \
      _argvec[1] = (unsigned long)(arg1);                              \
      _argvec[2] = (unsigned long)(arg2);                              \
      _argvec[3] = (unsigned long)(arg3);                              \
      _argvec[4] = (unsigned long)(arg4);                              \
      _argvec[5] = (unsigned long)(arg5);                              \
      _argvec[6] = (unsigned long)(arg6);                              \
      _argvec[7] = (unsigned long)(arg7);                              \
      _argvec[8] = (unsigned long)(arg8);                              \
      _argvec[9] = (unsigned long)(arg9);                              \
      __asm__ volatile(                                                \
         VALGRIND_CFI_PROLOGUE                                         \
         VALGRIND_ALIGN_STACK                                          \
         "subq $136,%%rsp\n\t"                                         \
         "pushq 72(%%rax)\n\t"                                         \
         "pushq 64(%%rax)\n\t"                                         \
         "pushq 56(%%rax)\n\t"                                         \
         "movq 48(%%rax), %%r9\n\t"                                    \
         "movq 40(%%rax), %%r8\n\t"                                    \
         "movq 32(%%rax), %%rcx\n\t"                                   \
         "movq 24(%%rax), %%rdx\n\t"                                   \
         "movq 16(%%rax), %%rsi\n\t"                                   \
         "movq 8(%%rax), %%rdi\n\t"                                    \
         "movq (%%rax), %%rax\n\t"                 \
         VALGRIND_CALL_NOREDIR_RAX                                     \
         VALGRIND_RESTORE_STACK                                        \
         VALGRIND_CFI_EPILOGUE                                         \
         :   "=a" (_res)                                       \
         :    "a" (&_argvec[0]) __FRAME_POINTER                 \
         : "cc", "memory", __CALLER_SAVED_REGS, "r14", "r15" \
      );                                                               \
      lval = (__typeof__(lval)) _res;                                  \
   } while (0)

#define CALL_FN_W_10W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,       \
                                  arg7,arg8,arg9,arg10)                \
   do {                                                                \
      volatile OrigFn        _orig = (orig);                           \
      volatile unsigned long _argvec[11];                              \
      volatile unsigned long _res;                                     \
      _argvec[0] = (unsigned long)_orig.nraddr;                        \
      _argvec[1] = (unsigned long)(arg1);                              \
      _argvec[2] = (unsigned long)(arg2);                              \
      _argvec[3] = (unsigned long)(arg3);                              \
      _argvec[4] = (unsigned long)(arg4);                              \
      _argvec[5] = (unsigned long)(arg5);                              \
      _argvec[6] = (unsigned long)(arg6);                              \
      _argvec[7] = (unsigned long)(arg7);                              \
      _argvec[8] = (unsigned long)(arg8);                              \
      _argvec[9] = (unsigned long)(arg9);                              \
      _argvec[10] = (unsigned long)(arg10);                            \
      __asm__ volatile(                                                \
         VALGRIND_CFI_PROLOGUE                                         \
         VALGRIND_ALIGN_STACK                                          \
         "subq $128,%%rsp\n\t"                                         \
         "pushq 80(%%rax)\n\t"                                         \
         "pushq 72(%%rax)\n\t"                                         \
         "pushq 64(%%rax)\n\t"                                         \
         "pushq 56(%%rax)\n\t"                                         \
         "movq 48(%%rax), %%r9\n\t"                                    \
         "movq 40(%%rax), %%r8\n\t"                                    \
         "movq 32(%%rax), %%rcx\n\t"                                   \
         "movq 24(%%rax), %%rdx\n\t"                                   \
         "movq 16(%%rax), %%rsi\n\t"                                   \
         "movq 8(%%rax), %%rdi\n\t"                                    \
         "movq (%%rax), %%rax\n\t"                 \
         VALGRIND_CALL_NOREDIR_RAX                                     \
         VALGRIND_RESTORE_STACK                                        \
         VALGRIND_CFI_EPILOGUE                                         \
         :   "=a" (_res)                                       \
         :    "a" (&_argvec[0]) __FRAME_POINTER                 \
         : "cc", "memory", __CALLER_SAVED_REGS, "r14", "r15" \
      );                                                               \
      lval = (__typeof__(lval)) _res;                                  \
   } while (0)

#define CALL_FN_W_11W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,       \
                                  arg7,arg8,arg9,arg10,arg11)          \
   do {                                                                \
      volatile OrigFn        _orig = (orig);                           \
      volatile unsigned long _argvec[12];                              \
      volatile unsigned long _res;                                     \
      _argvec[0] = (unsigned long)_orig.nraddr;                        \
      _argvec[1] = (unsigned long)(arg1);                              \
      _argvec[2] = (unsigned long)(arg2);                              \
      _argvec[3] = (unsigned long)(arg3);                              \
      _argvec[4] = (unsigned long)(arg4);                              \
      _argvec[5] = (unsigned long)(arg5);                              \
      _argvec[6] = (unsigned long)(arg6);                              \
      _argvec[7] = (unsigned long)(arg7);                              \
      _argvec[8] = (unsigned long)(arg8);                              \
      _argvec[9] = (unsigned long)(arg9);                              \
      _argvec[10] = (unsigned long)(arg10);                            \
      _argvec[11] = (unsigned long)(arg11);                            \
      __asm__ volatile(                                                \
         VALGRIND_CFI_PROLOGUE                                         \
         VALGRIND_ALIGN_STACK                                          \
         "subq $136,%%rsp\n\t"                                         \
         "pushq 88(%%rax)\n\t"                                         \
         "pushq 80(%%rax)\n\t"                                         \
         "pushq 72(%%rax)\n\t"                                         \
         "pushq 64(%%rax)\n\t"                                         \
         "pushq 56(%%rax)\n\t"                                         \
         "movq 48(%%rax), %%r9\n\t"                                    \
         "movq 40(%%rax), %%r8\n\t"                                    \
         "movq 32(%%rax), %%rcx\n\t"                                   \
         "movq 24(%%rax), %%rdx\n\t"                                   \
         "movq 16(%%rax), %%rsi\n\t"                                   \
         "movq 8(%%rax), %%rdi\n\t"                                    \
         "movq (%%rax), %%rax\n\t"                 \
         VALGRIND_CALL_NOREDIR_RAX                                     \
         VALGRIND_RESTORE_STACK                                        \
         VALGRIND_CFI_EPILOGUE                                         \
         :   "=a" (_res)                                       \
         :    "a" (&_argvec[0]) __FRAME_POINTER                 \
         : "cc", "memory", __CALLER_SAVED_REGS, "r14", "r15" \
      );                                                               \
      lval = (__typeof__(lval)) _res;                                  \
   } while (0)

#define CALL_FN_W_12W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,       \
                                arg7,arg8,arg9,arg10,arg11,arg12)      \
   do {                                                                \
      volatile OrigFn        _orig = (orig);                           \
      volatile unsigned long _argvec[13];                              \
      volatile unsigned long _res;                                     \
      _argvec[0] = (unsigned long)_orig.nraddr;                        \
      _argvec[1] = (unsigned long)(arg1);                              \
      _argvec[2] = (unsigned long)(arg2);                              \
      _argvec[3] = (unsigned long)(arg3);                              \
      _argvec[4] = (unsigned long)(arg4);                              \
      _argvec[5] = (unsigned long)(arg5);                              \
      _argvec[6] = (unsigned long)(arg6);                              \
      _argvec[7] = (unsigned long)(arg7);                              \
      _argvec[8] = (unsigned long)(arg8);                              \
      _argvec[9] = (unsigned long)(arg9);                              \
      _argvec[10] = (unsigned long)(arg10);                            \
      _argvec[11] = (unsigned long)(arg11);                            \
      _argvec[12] = (unsigned long)(arg12);                            \
      __asm__ volatile(                                                \
         VALGRIND_CFI_PROLOGUE                                         \
         VALGRIND_ALIGN_STACK                                          \
         "subq $128,%%rsp\n\t"                                         \
         "pushq 96(%%rax)\n\t"                                         \
         "pushq 88(%%rax)\n\t"                                         \
         "pushq 80(%%rax)\n\t"                                         \
         "pushq 72(%%rax)\n\t"                                         \
         "pushq 64(%%rax)\n\t"                                         \
         "pushq 56(%%rax)\n\t"                                         \
         "movq 48(%%rax), %%r9\n\t"                                    \
         "movq 40(%%rax), %%r8\n\t"                                    \
         "movq 32(%%rax), %%rcx\n\t"                                   \
         "movq 24(%%rax), %%rdx\n\t"                                   \
         "movq 16(%%rax), %%rsi\n\t"                                   \
         "movq 8(%%rax), %%rdi\n\t"                                    \
         "movq (%%rax), %%rax\n\t"                 \
         VALGRIND_CALL_NOREDIR_RAX                                     \
         VALGRIND_RESTORE_STACK                                        \
         VALGRIND_CFI_EPILOGUE                                         \
         :   "=a" (_res)                                       \
         :    "a" (&_argvec[0]) __FRAME_POINTER                 \
         : "cc", "memory", __CALLER_SAVED_REGS, "r14", "r15" \
      );                                                               \
      lval = (__typeof__(lval)) _res;                                  \
   } while (0)

#endif


#if defined(PLAT_ppc32_linux)



#define __CALLER_SAVED_REGS                                       \
   "lr", "ctr", "xer",                                            \
   "cr0", "cr1", "cr2", "cr3", "cr4", "cr5", "cr6", "cr7",        \
   "r0", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",   \
   "r11", "r12", "r13"


#define VALGRIND_ALIGN_STACK               \
      "mr 28,1\n\t"                        \
      "rlwinm 1,1,0,0,27\n\t"
#define VALGRIND_RESTORE_STACK             \
      "mr 1,28\n\t"


#define CALL_FN_W_v(lval, orig)                                   \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[1];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "lwz 11,0(11)\n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         VALGRIND_RESTORE_STACK                                   \
         "mr %0,3"                                                \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_W(lval, orig, arg1)                             \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[2];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)arg1;                           \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "lwz 3,4(11)\n\t"                       \
         "lwz 11,0(11)\n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         VALGRIND_RESTORE_STACK                                   \
         "mr %0,3"                                                \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WW(lval, orig, arg1,arg2)                       \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[3];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)arg1;                           \
      _argvec[2] = (unsigned long)arg2;                           \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "lwz 3,4(11)\n\t"                       \
         "lwz 4,8(11)\n\t"                                        \
         "lwz 11,0(11)\n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         VALGRIND_RESTORE_STACK                                   \
         "mr %0,3"                                                \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WWW(lval, orig, arg1,arg2,arg3)                 \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[4];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)arg1;                           \
      _argvec[2] = (unsigned long)arg2;                           \
      _argvec[3] = (unsigned long)arg3;                           \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "lwz 3,4(11)\n\t"                       \
         "lwz 4,8(11)\n\t"                                        \
         "lwz 5,12(11)\n\t"                                       \
         "lwz 11,0(11)\n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         VALGRIND_RESTORE_STACK                                   \
         "mr %0,3"                                                \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WWWW(lval, orig, arg1,arg2,arg3,arg4)           \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[5];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)arg1;                           \
      _argvec[2] = (unsigned long)arg2;                           \
      _argvec[3] = (unsigned long)arg3;                           \
      _argvec[4] = (unsigned long)arg4;                           \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "lwz 3,4(11)\n\t"                       \
         "lwz 4,8(11)\n\t"                                        \
         "lwz 5,12(11)\n\t"                                       \
         "lwz 6,16(11)\n\t"                       \
         "lwz 11,0(11)\n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         VALGRIND_RESTORE_STACK                                   \
         "mr %0,3"                                                \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_5W(lval, orig, arg1,arg2,arg3,arg4,arg5)        \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[6];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)arg1;                           \
      _argvec[2] = (unsigned long)arg2;                           \
      _argvec[3] = (unsigned long)arg3;                           \
      _argvec[4] = (unsigned long)arg4;                           \
      _argvec[5] = (unsigned long)arg5;                           \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "lwz 3,4(11)\n\t"                       \
         "lwz 4,8(11)\n\t"                                        \
         "lwz 5,12(11)\n\t"                                       \
         "lwz 6,16(11)\n\t"                       \
         "lwz 7,20(11)\n\t"                                       \
         "lwz 11,0(11)\n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         VALGRIND_RESTORE_STACK                                   \
         "mr %0,3"                                                \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_6W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6)   \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[7];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)arg1;                           \
      _argvec[2] = (unsigned long)arg2;                           \
      _argvec[3] = (unsigned long)arg3;                           \
      _argvec[4] = (unsigned long)arg4;                           \
      _argvec[5] = (unsigned long)arg5;                           \
      _argvec[6] = (unsigned long)arg6;                           \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "lwz 3,4(11)\n\t"                       \
         "lwz 4,8(11)\n\t"                                        \
         "lwz 5,12(11)\n\t"                                       \
         "lwz 6,16(11)\n\t"                       \
         "lwz 7,20(11)\n\t"                                       \
         "lwz 8,24(11)\n\t"                                       \
         "lwz 11,0(11)\n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         VALGRIND_RESTORE_STACK                                   \
         "mr %0,3"                                                \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_7W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,   \
                                 arg7)                            \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[8];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)arg1;                           \
      _argvec[2] = (unsigned long)arg2;                           \
      _argvec[3] = (unsigned long)arg3;                           \
      _argvec[4] = (unsigned long)arg4;                           \
      _argvec[5] = (unsigned long)arg5;                           \
      _argvec[6] = (unsigned long)arg6;                           \
      _argvec[7] = (unsigned long)arg7;                           \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "lwz 3,4(11)\n\t"                       \
         "lwz 4,8(11)\n\t"                                        \
         "lwz 5,12(11)\n\t"                                       \
         "lwz 6,16(11)\n\t"                       \
         "lwz 7,20(11)\n\t"                                       \
         "lwz 8,24(11)\n\t"                                       \
         "lwz 9,28(11)\n\t"                                       \
         "lwz 11,0(11)\n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         VALGRIND_RESTORE_STACK                                   \
         "mr %0,3"                                                \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_8W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,   \
                                 arg7,arg8)                       \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[9];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)arg1;                           \
      _argvec[2] = (unsigned long)arg2;                           \
      _argvec[3] = (unsigned long)arg3;                           \
      _argvec[4] = (unsigned long)arg4;                           \
      _argvec[5] = (unsigned long)arg5;                           \
      _argvec[6] = (unsigned long)arg6;                           \
      _argvec[7] = (unsigned long)arg7;                           \
      _argvec[8] = (unsigned long)arg8;                           \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "lwz 3,4(11)\n\t"                       \
         "lwz 4,8(11)\n\t"                                        \
         "lwz 5,12(11)\n\t"                                       \
         "lwz 6,16(11)\n\t"                       \
         "lwz 7,20(11)\n\t"                                       \
         "lwz 8,24(11)\n\t"                                       \
         "lwz 9,28(11)\n\t"                                       \
         "lwz 10,32(11)\n\t"                      \
         "lwz 11,0(11)\n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         VALGRIND_RESTORE_STACK                                   \
         "mr %0,3"                                                \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_9W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,   \
                                 arg7,arg8,arg9)                  \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[10];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)arg1;                           \
      _argvec[2] = (unsigned long)arg2;                           \
      _argvec[3] = (unsigned long)arg3;                           \
      _argvec[4] = (unsigned long)arg4;                           \
      _argvec[5] = (unsigned long)arg5;                           \
      _argvec[6] = (unsigned long)arg6;                           \
      _argvec[7] = (unsigned long)arg7;                           \
      _argvec[8] = (unsigned long)arg8;                           \
      _argvec[9] = (unsigned long)arg9;                           \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "addi 1,1,-16\n\t"                                       \
                                               \
         "lwz 3,36(11)\n\t"                                       \
         "stw 3,8(1)\n\t"                                         \
                                            \
         "lwz 3,4(11)\n\t"                       \
         "lwz 4,8(11)\n\t"                                        \
         "lwz 5,12(11)\n\t"                                       \
         "lwz 6,16(11)\n\t"                       \
         "lwz 7,20(11)\n\t"                                       \
         "lwz 8,24(11)\n\t"                                       \
         "lwz 9,28(11)\n\t"                                       \
         "lwz 10,32(11)\n\t"                      \
         "lwz 11,0(11)\n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         VALGRIND_RESTORE_STACK                                   \
         "mr %0,3"                                                \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_10W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,  \
                                  arg7,arg8,arg9,arg10)           \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[11];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)arg1;                           \
      _argvec[2] = (unsigned long)arg2;                           \
      _argvec[3] = (unsigned long)arg3;                           \
      _argvec[4] = (unsigned long)arg4;                           \
      _argvec[5] = (unsigned long)arg5;                           \
      _argvec[6] = (unsigned long)arg6;                           \
      _argvec[7] = (unsigned long)arg7;                           \
      _argvec[8] = (unsigned long)arg8;                           \
      _argvec[9] = (unsigned long)arg9;                           \
      _argvec[10] = (unsigned long)arg10;                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "addi 1,1,-16\n\t"                                       \
                                              \
         "lwz 3,40(11)\n\t"                                       \
         "stw 3,12(1)\n\t"                                        \
                                               \
         "lwz 3,36(11)\n\t"                                       \
         "stw 3,8(1)\n\t"                                         \
                                            \
         "lwz 3,4(11)\n\t"                       \
         "lwz 4,8(11)\n\t"                                        \
         "lwz 5,12(11)\n\t"                                       \
         "lwz 6,16(11)\n\t"                       \
         "lwz 7,20(11)\n\t"                                       \
         "lwz 8,24(11)\n\t"                                       \
         "lwz 9,28(11)\n\t"                                       \
         "lwz 10,32(11)\n\t"                      \
         "lwz 11,0(11)\n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         VALGRIND_RESTORE_STACK                                   \
         "mr %0,3"                                                \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_11W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,  \
                                  arg7,arg8,arg9,arg10,arg11)     \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[12];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)arg1;                           \
      _argvec[2] = (unsigned long)arg2;                           \
      _argvec[3] = (unsigned long)arg3;                           \
      _argvec[4] = (unsigned long)arg4;                           \
      _argvec[5] = (unsigned long)arg5;                           \
      _argvec[6] = (unsigned long)arg6;                           \
      _argvec[7] = (unsigned long)arg7;                           \
      _argvec[8] = (unsigned long)arg8;                           \
      _argvec[9] = (unsigned long)arg9;                           \
      _argvec[10] = (unsigned long)arg10;                         \
      _argvec[11] = (unsigned long)arg11;                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "addi 1,1,-32\n\t"                                       \
                                              \
         "lwz 3,44(11)\n\t"                                       \
         "stw 3,16(1)\n\t"                                        \
                                              \
         "lwz 3,40(11)\n\t"                                       \
         "stw 3,12(1)\n\t"                                        \
                                               \
         "lwz 3,36(11)\n\t"                                       \
         "stw 3,8(1)\n\t"                                         \
                                            \
         "lwz 3,4(11)\n\t"                       \
         "lwz 4,8(11)\n\t"                                        \
         "lwz 5,12(11)\n\t"                                       \
         "lwz 6,16(11)\n\t"                       \
         "lwz 7,20(11)\n\t"                                       \
         "lwz 8,24(11)\n\t"                                       \
         "lwz 9,28(11)\n\t"                                       \
         "lwz 10,32(11)\n\t"                      \
         "lwz 11,0(11)\n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         VALGRIND_RESTORE_STACK                                   \
         "mr %0,3"                                                \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_12W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,  \
                                arg7,arg8,arg9,arg10,arg11,arg12) \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[13];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)arg1;                           \
      _argvec[2] = (unsigned long)arg2;                           \
      _argvec[3] = (unsigned long)arg3;                           \
      _argvec[4] = (unsigned long)arg4;                           \
      _argvec[5] = (unsigned long)arg5;                           \
      _argvec[6] = (unsigned long)arg6;                           \
      _argvec[7] = (unsigned long)arg7;                           \
      _argvec[8] = (unsigned long)arg8;                           \
      _argvec[9] = (unsigned long)arg9;                           \
      _argvec[10] = (unsigned long)arg10;                         \
      _argvec[11] = (unsigned long)arg11;                         \
      _argvec[12] = (unsigned long)arg12;                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "addi 1,1,-32\n\t"                                       \
                                              \
         "lwz 3,48(11)\n\t"                                       \
         "stw 3,20(1)\n\t"                                        \
                                              \
         "lwz 3,44(11)\n\t"                                       \
         "stw 3,16(1)\n\t"                                        \
                                              \
         "lwz 3,40(11)\n\t"                                       \
         "stw 3,12(1)\n\t"                                        \
                                               \
         "lwz 3,36(11)\n\t"                                       \
         "stw 3,8(1)\n\t"                                         \
                                            \
         "lwz 3,4(11)\n\t"                       \
         "lwz 4,8(11)\n\t"                                        \
         "lwz 5,12(11)\n\t"                                       \
         "lwz 6,16(11)\n\t"                       \
         "lwz 7,20(11)\n\t"                                       \
         "lwz 8,24(11)\n\t"                                       \
         "lwz 9,28(11)\n\t"                                       \
         "lwz 10,32(11)\n\t"                      \
         "lwz 11,0(11)\n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         VALGRIND_RESTORE_STACK                                   \
         "mr %0,3"                                                \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#endif


#if defined(PLAT_ppc64_linux)


#define __CALLER_SAVED_REGS                                       \
   "lr", "ctr", "xer",                                            \
   "cr0", "cr1", "cr2", "cr3", "cr4", "cr5", "cr6", "cr7",        \
   "r0", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",   \
   "r11", "r12", "r13"


#define VALGRIND_ALIGN_STACK               \
      "mr 28,1\n\t"                        \
      "rldicr 1,1,0,59\n\t"
#define VALGRIND_RESTORE_STACK             \
      "mr 1,28\n\t"


#define CALL_FN_W_v(lval, orig)                                   \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[3+0];                        \
      volatile unsigned long _res;                                \
           \
      _argvec[1] = (unsigned long)_orig.r2;                       \
      _argvec[2] = (unsigned long)_orig.nraddr;                   \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "std 2,-16(11)\n\t"                   \
         "ld   2,-8(11)\n\t"           \
         "ld  11, 0(11)\n\t"                   \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         "mr 11,%1\n\t"                                           \
         "mr %0,3\n\t"                                            \
         "ld 2,-16(11)\n\t"                  \
         VALGRIND_RESTORE_STACK                                   \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[2])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_W(lval, orig, arg1)                             \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[3+1];                        \
      volatile unsigned long _res;                                \
           \
      _argvec[1]   = (unsigned long)_orig.r2;                     \
      _argvec[2]   = (unsigned long)_orig.nraddr;                 \
      _argvec[2+1] = (unsigned long)arg1;                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "std 2,-16(11)\n\t"                   \
         "ld   2,-8(11)\n\t"           \
         "ld   3, 8(11)\n\t"                      \
         "ld  11, 0(11)\n\t"                   \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         "mr 11,%1\n\t"                                           \
         "mr %0,3\n\t"                                            \
         "ld 2,-16(11)\n\t"                  \
         VALGRIND_RESTORE_STACK                                   \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[2])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WW(lval, orig, arg1,arg2)                       \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[3+2];                        \
      volatile unsigned long _res;                                \
           \
      _argvec[1]   = (unsigned long)_orig.r2;                     \
      _argvec[2]   = (unsigned long)_orig.nraddr;                 \
      _argvec[2+1] = (unsigned long)arg1;                         \
      _argvec[2+2] = (unsigned long)arg2;                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "std 2,-16(11)\n\t"                   \
         "ld   2,-8(11)\n\t"           \
         "ld   3, 8(11)\n\t"                      \
         "ld   4, 16(11)\n\t"                      \
         "ld  11, 0(11)\n\t"                   \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         "mr 11,%1\n\t"                                           \
         "mr %0,3\n\t"                                            \
         "ld 2,-16(11)\n\t"                  \
         VALGRIND_RESTORE_STACK                                   \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[2])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WWW(lval, orig, arg1,arg2,arg3)                 \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[3+3];                        \
      volatile unsigned long _res;                                \
           \
      _argvec[1]   = (unsigned long)_orig.r2;                     \
      _argvec[2]   = (unsigned long)_orig.nraddr;                 \
      _argvec[2+1] = (unsigned long)arg1;                         \
      _argvec[2+2] = (unsigned long)arg2;                         \
      _argvec[2+3] = (unsigned long)arg3;                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "std 2,-16(11)\n\t"                   \
         "ld   2,-8(11)\n\t"           \
         "ld   3, 8(11)\n\t"                      \
         "ld   4, 16(11)\n\t"                      \
         "ld   5, 24(11)\n\t"                      \
         "ld  11, 0(11)\n\t"                   \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         "mr 11,%1\n\t"                                           \
         "mr %0,3\n\t"                                            \
         "ld 2,-16(11)\n\t"                  \
         VALGRIND_RESTORE_STACK                                   \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[2])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WWWW(lval, orig, arg1,arg2,arg3,arg4)           \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[3+4];                        \
      volatile unsigned long _res;                                \
           \
      _argvec[1]   = (unsigned long)_orig.r2;                     \
      _argvec[2]   = (unsigned long)_orig.nraddr;                 \
      _argvec[2+1] = (unsigned long)arg1;                         \
      _argvec[2+2] = (unsigned long)arg2;                         \
      _argvec[2+3] = (unsigned long)arg3;                         \
      _argvec[2+4] = (unsigned long)arg4;                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "std 2,-16(11)\n\t"                   \
         "ld   2,-8(11)\n\t"           \
         "ld   3, 8(11)\n\t"                      \
         "ld   4, 16(11)\n\t"                      \
         "ld   5, 24(11)\n\t"                      \
         "ld   6, 32(11)\n\t"                      \
         "ld  11, 0(11)\n\t"                   \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         "mr 11,%1\n\t"                                           \
         "mr %0,3\n\t"                                            \
         "ld 2,-16(11)\n\t"                  \
         VALGRIND_RESTORE_STACK                                   \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[2])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_5W(lval, orig, arg1,arg2,arg3,arg4,arg5)        \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[3+5];                        \
      volatile unsigned long _res;                                \
           \
      _argvec[1]   = (unsigned long)_orig.r2;                     \
      _argvec[2]   = (unsigned long)_orig.nraddr;                 \
      _argvec[2+1] = (unsigned long)arg1;                         \
      _argvec[2+2] = (unsigned long)arg2;                         \
      _argvec[2+3] = (unsigned long)arg3;                         \
      _argvec[2+4] = (unsigned long)arg4;                         \
      _argvec[2+5] = (unsigned long)arg5;                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "std 2,-16(11)\n\t"                   \
         "ld   2,-8(11)\n\t"           \
         "ld   3, 8(11)\n\t"                      \
         "ld   4, 16(11)\n\t"                      \
         "ld   5, 24(11)\n\t"                      \
         "ld   6, 32(11)\n\t"                      \
         "ld   7, 40(11)\n\t"                      \
         "ld  11, 0(11)\n\t"                   \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         "mr 11,%1\n\t"                                           \
         "mr %0,3\n\t"                                            \
         "ld 2,-16(11)\n\t"                  \
         VALGRIND_RESTORE_STACK                                   \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[2])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_6W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6)   \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[3+6];                        \
      volatile unsigned long _res;                                \
           \
      _argvec[1]   = (unsigned long)_orig.r2;                     \
      _argvec[2]   = (unsigned long)_orig.nraddr;                 \
      _argvec[2+1] = (unsigned long)arg1;                         \
      _argvec[2+2] = (unsigned long)arg2;                         \
      _argvec[2+3] = (unsigned long)arg3;                         \
      _argvec[2+4] = (unsigned long)arg4;                         \
      _argvec[2+5] = (unsigned long)arg5;                         \
      _argvec[2+6] = (unsigned long)arg6;                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "std 2,-16(11)\n\t"                   \
         "ld   2,-8(11)\n\t"           \
         "ld   3, 8(11)\n\t"                      \
         "ld   4, 16(11)\n\t"                      \
         "ld   5, 24(11)\n\t"                      \
         "ld   6, 32(11)\n\t"                      \
         "ld   7, 40(11)\n\t"                      \
         "ld   8, 48(11)\n\t"                      \
         "ld  11, 0(11)\n\t"                   \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         "mr 11,%1\n\t"                                           \
         "mr %0,3\n\t"                                            \
         "ld 2,-16(11)\n\t"                  \
         VALGRIND_RESTORE_STACK                                   \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[2])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_7W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,   \
                                 arg7)                            \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[3+7];                        \
      volatile unsigned long _res;                                \
           \
      _argvec[1]   = (unsigned long)_orig.r2;                     \
      _argvec[2]   = (unsigned long)_orig.nraddr;                 \
      _argvec[2+1] = (unsigned long)arg1;                         \
      _argvec[2+2] = (unsigned long)arg2;                         \
      _argvec[2+3] = (unsigned long)arg3;                         \
      _argvec[2+4] = (unsigned long)arg4;                         \
      _argvec[2+5] = (unsigned long)arg5;                         \
      _argvec[2+6] = (unsigned long)arg6;                         \
      _argvec[2+7] = (unsigned long)arg7;                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "std 2,-16(11)\n\t"                   \
         "ld   2,-8(11)\n\t"           \
         "ld   3, 8(11)\n\t"                      \
         "ld   4, 16(11)\n\t"                      \
         "ld   5, 24(11)\n\t"                      \
         "ld   6, 32(11)\n\t"                      \
         "ld   7, 40(11)\n\t"                      \
         "ld   8, 48(11)\n\t"                      \
         "ld   9, 56(11)\n\t"                      \
         "ld  11, 0(11)\n\t"                   \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         "mr 11,%1\n\t"                                           \
         "mr %0,3\n\t"                                            \
         "ld 2,-16(11)\n\t"                  \
         VALGRIND_RESTORE_STACK                                   \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[2])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_8W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,   \
                                 arg7,arg8)                       \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[3+8];                        \
      volatile unsigned long _res;                                \
           \
      _argvec[1]   = (unsigned long)_orig.r2;                     \
      _argvec[2]   = (unsigned long)_orig.nraddr;                 \
      _argvec[2+1] = (unsigned long)arg1;                         \
      _argvec[2+2] = (unsigned long)arg2;                         \
      _argvec[2+3] = (unsigned long)arg3;                         \
      _argvec[2+4] = (unsigned long)arg4;                         \
      _argvec[2+5] = (unsigned long)arg5;                         \
      _argvec[2+6] = (unsigned long)arg6;                         \
      _argvec[2+7] = (unsigned long)arg7;                         \
      _argvec[2+8] = (unsigned long)arg8;                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "std 2,-16(11)\n\t"                   \
         "ld   2,-8(11)\n\t"           \
         "ld   3, 8(11)\n\t"                      \
         "ld   4, 16(11)\n\t"                      \
         "ld   5, 24(11)\n\t"                      \
         "ld   6, 32(11)\n\t"                      \
         "ld   7, 40(11)\n\t"                      \
         "ld   8, 48(11)\n\t"                      \
         "ld   9, 56(11)\n\t"                      \
         "ld  10, 64(11)\n\t"                     \
         "ld  11, 0(11)\n\t"                   \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         "mr 11,%1\n\t"                                           \
         "mr %0,3\n\t"                                            \
         "ld 2,-16(11)\n\t"                  \
         VALGRIND_RESTORE_STACK                                   \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[2])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_9W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,   \
                                 arg7,arg8,arg9)                  \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[3+9];                        \
      volatile unsigned long _res;                                \
           \
      _argvec[1]   = (unsigned long)_orig.r2;                     \
      _argvec[2]   = (unsigned long)_orig.nraddr;                 \
      _argvec[2+1] = (unsigned long)arg1;                         \
      _argvec[2+2] = (unsigned long)arg2;                         \
      _argvec[2+3] = (unsigned long)arg3;                         \
      _argvec[2+4] = (unsigned long)arg4;                         \
      _argvec[2+5] = (unsigned long)arg5;                         \
      _argvec[2+6] = (unsigned long)arg6;                         \
      _argvec[2+7] = (unsigned long)arg7;                         \
      _argvec[2+8] = (unsigned long)arg8;                         \
      _argvec[2+9] = (unsigned long)arg9;                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "std 2,-16(11)\n\t"                   \
         "ld   2,-8(11)\n\t"           \
         "addi 1,1,-128\n\t"            \
                                               \
         "ld  3,72(11)\n\t"                                       \
         "std 3,112(1)\n\t"                                       \
                                            \
         "ld   3, 8(11)\n\t"                      \
         "ld   4, 16(11)\n\t"                      \
         "ld   5, 24(11)\n\t"                      \
         "ld   6, 32(11)\n\t"                      \
         "ld   7, 40(11)\n\t"                      \
         "ld   8, 48(11)\n\t"                      \
         "ld   9, 56(11)\n\t"                      \
         "ld  10, 64(11)\n\t"                     \
         "ld  11, 0(11)\n\t"                   \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         "mr 11,%1\n\t"                                           \
         "mr %0,3\n\t"                                            \
         "ld 2,-16(11)\n\t"                  \
         VALGRIND_RESTORE_STACK                                   \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[2])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_10W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,  \
                                  arg7,arg8,arg9,arg10)           \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[3+10];                       \
      volatile unsigned long _res;                                \
           \
      _argvec[1]   = (unsigned long)_orig.r2;                     \
      _argvec[2]   = (unsigned long)_orig.nraddr;                 \
      _argvec[2+1] = (unsigned long)arg1;                         \
      _argvec[2+2] = (unsigned long)arg2;                         \
      _argvec[2+3] = (unsigned long)arg3;                         \
      _argvec[2+4] = (unsigned long)arg4;                         \
      _argvec[2+5] = (unsigned long)arg5;                         \
      _argvec[2+6] = (unsigned long)arg6;                         \
      _argvec[2+7] = (unsigned long)arg7;                         \
      _argvec[2+8] = (unsigned long)arg8;                         \
      _argvec[2+9] = (unsigned long)arg9;                         \
      _argvec[2+10] = (unsigned long)arg10;                       \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "std 2,-16(11)\n\t"                   \
         "ld   2,-8(11)\n\t"           \
         "addi 1,1,-128\n\t"            \
                                              \
         "ld  3,80(11)\n\t"                                       \
         "std 3,120(1)\n\t"                                       \
                                               \
         "ld  3,72(11)\n\t"                                       \
         "std 3,112(1)\n\t"                                       \
                                            \
         "ld   3, 8(11)\n\t"                      \
         "ld   4, 16(11)\n\t"                      \
         "ld   5, 24(11)\n\t"                      \
         "ld   6, 32(11)\n\t"                      \
         "ld   7, 40(11)\n\t"                      \
         "ld   8, 48(11)\n\t"                      \
         "ld   9, 56(11)\n\t"                      \
         "ld  10, 64(11)\n\t"                     \
         "ld  11, 0(11)\n\t"                   \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         "mr 11,%1\n\t"                                           \
         "mr %0,3\n\t"                                            \
         "ld 2,-16(11)\n\t"                  \
         VALGRIND_RESTORE_STACK                                   \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[2])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_11W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,  \
                                  arg7,arg8,arg9,arg10,arg11)     \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[3+11];                       \
      volatile unsigned long _res;                                \
           \
      _argvec[1]   = (unsigned long)_orig.r2;                     \
      _argvec[2]   = (unsigned long)_orig.nraddr;                 \
      _argvec[2+1] = (unsigned long)arg1;                         \
      _argvec[2+2] = (unsigned long)arg2;                         \
      _argvec[2+3] = (unsigned long)arg3;                         \
      _argvec[2+4] = (unsigned long)arg4;                         \
      _argvec[2+5] = (unsigned long)arg5;                         \
      _argvec[2+6] = (unsigned long)arg6;                         \
      _argvec[2+7] = (unsigned long)arg7;                         \
      _argvec[2+8] = (unsigned long)arg8;                         \
      _argvec[2+9] = (unsigned long)arg9;                         \
      _argvec[2+10] = (unsigned long)arg10;                       \
      _argvec[2+11] = (unsigned long)arg11;                       \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "std 2,-16(11)\n\t"                   \
         "ld   2,-8(11)\n\t"           \
         "addi 1,1,-144\n\t"            \
                                              \
         "ld  3,88(11)\n\t"                                       \
         "std 3,128(1)\n\t"                                       \
                                              \
         "ld  3,80(11)\n\t"                                       \
         "std 3,120(1)\n\t"                                       \
                                               \
         "ld  3,72(11)\n\t"                                       \
         "std 3,112(1)\n\t"                                       \
                                            \
         "ld   3, 8(11)\n\t"                      \
         "ld   4, 16(11)\n\t"                      \
         "ld   5, 24(11)\n\t"                      \
         "ld   6, 32(11)\n\t"                      \
         "ld   7, 40(11)\n\t"                      \
         "ld   8, 48(11)\n\t"                      \
         "ld   9, 56(11)\n\t"                      \
         "ld  10, 64(11)\n\t"                     \
         "ld  11, 0(11)\n\t"                   \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         "mr 11,%1\n\t"                                           \
         "mr %0,3\n\t"                                            \
         "ld 2,-16(11)\n\t"                  \
         VALGRIND_RESTORE_STACK                                   \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[2])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_12W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,  \
                                arg7,arg8,arg9,arg10,arg11,arg12) \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[3+12];                       \
      volatile unsigned long _res;                                \
           \
      _argvec[1]   = (unsigned long)_orig.r2;                     \
      _argvec[2]   = (unsigned long)_orig.nraddr;                 \
      _argvec[2+1] = (unsigned long)arg1;                         \
      _argvec[2+2] = (unsigned long)arg2;                         \
      _argvec[2+3] = (unsigned long)arg3;                         \
      _argvec[2+4] = (unsigned long)arg4;                         \
      _argvec[2+5] = (unsigned long)arg5;                         \
      _argvec[2+6] = (unsigned long)arg6;                         \
      _argvec[2+7] = (unsigned long)arg7;                         \
      _argvec[2+8] = (unsigned long)arg8;                         \
      _argvec[2+9] = (unsigned long)arg9;                         \
      _argvec[2+10] = (unsigned long)arg10;                       \
      _argvec[2+11] = (unsigned long)arg11;                       \
      _argvec[2+12] = (unsigned long)arg12;                       \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "mr 11,%1\n\t"                                           \
         "std 2,-16(11)\n\t"                   \
         "ld   2,-8(11)\n\t"           \
         "addi 1,1,-144\n\t"            \
                                              \
         "ld  3,96(11)\n\t"                                       \
         "std 3,136(1)\n\t"                                       \
                                              \
         "ld  3,88(11)\n\t"                                       \
         "std 3,128(1)\n\t"                                       \
                                              \
         "ld  3,80(11)\n\t"                                       \
         "std 3,120(1)\n\t"                                       \
                                               \
         "ld  3,72(11)\n\t"                                       \
         "std 3,112(1)\n\t"                                       \
                                            \
         "ld   3, 8(11)\n\t"                      \
         "ld   4, 16(11)\n\t"                      \
         "ld   5, 24(11)\n\t"                      \
         "ld   6, 32(11)\n\t"                      \
         "ld   7, 40(11)\n\t"                      \
         "ld   8, 48(11)\n\t"                      \
         "ld   9, 56(11)\n\t"                      \
         "ld  10, 64(11)\n\t"                     \
         "ld  11, 0(11)\n\t"                   \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R11                  \
         "mr 11,%1\n\t"                                           \
         "mr %0,3\n\t"                                            \
         "ld 2,-16(11)\n\t"                  \
         VALGRIND_RESTORE_STACK                                   \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[2])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r28"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#endif


#if defined(PLAT_arm_linux)

#define __CALLER_SAVED_REGS "r0", "r1", "r2", "r3","r4","r14"


#define VALGRIND_ALIGN_STACK               \
      "mov r10, sp\n\t"                    \
      "mov r4,  sp\n\t"                    \
      "bic r4,  r4, #7\n\t"                \
      "mov sp,  r4\n\t"
#define VALGRIND_RESTORE_STACK             \
      "mov sp,  r10\n\t"


#define CALL_FN_W_v(lval, orig)                                   \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[1];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "ldr r4, [%1] \n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R4                   \
         VALGRIND_RESTORE_STACK                                   \
         "mov %0, r0\n"                                           \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r10"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_W(lval, orig, arg1)                             \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[2];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "ldr r0, [%1, #4] \n\t"                                  \
         "ldr r4, [%1] \n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R4                   \
         VALGRIND_RESTORE_STACK                                   \
         "mov %0, r0\n"                                           \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r10"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WW(lval, orig, arg1,arg2)                       \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[3];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "ldr r0, [%1, #4] \n\t"                                  \
         "ldr r1, [%1, #8] \n\t"                                  \
         "ldr r4, [%1] \n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R4                   \
         VALGRIND_RESTORE_STACK                                   \
         "mov %0, r0\n"                                           \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r10"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WWW(lval, orig, arg1,arg2,arg3)                 \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[4];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "ldr r0, [%1, #4] \n\t"                                  \
         "ldr r1, [%1, #8] \n\t"                                  \
         "ldr r2, [%1, #12] \n\t"                                 \
         "ldr r4, [%1] \n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R4                   \
         VALGRIND_RESTORE_STACK                                   \
         "mov %0, r0\n"                                           \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r10"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WWWW(lval, orig, arg1,arg2,arg3,arg4)           \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[5];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "ldr r0, [%1, #4] \n\t"                                  \
         "ldr r1, [%1, #8] \n\t"                                  \
         "ldr r2, [%1, #12] \n\t"                                 \
         "ldr r3, [%1, #16] \n\t"                                 \
         "ldr r4, [%1] \n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R4                   \
         VALGRIND_RESTORE_STACK                                   \
         "mov %0, r0"                                             \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r10"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_5W(lval, orig, arg1,arg2,arg3,arg4,arg5)        \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[6];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "sub sp, sp, #4 \n\t"                                    \
         "ldr r0, [%1, #20] \n\t"                                 \
         "push {r0} \n\t"                                         \
         "ldr r0, [%1, #4] \n\t"                                  \
         "ldr r1, [%1, #8] \n\t"                                  \
         "ldr r2, [%1, #12] \n\t"                                 \
         "ldr r3, [%1, #16] \n\t"                                 \
         "ldr r4, [%1] \n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R4                   \
         VALGRIND_RESTORE_STACK                                   \
         "mov %0, r0"                                             \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r10"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_6W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6)   \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[7];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "ldr r0, [%1, #20] \n\t"                                 \
         "ldr r1, [%1, #24] \n\t"                                 \
         "push {r0, r1} \n\t"                                     \
         "ldr r0, [%1, #4] \n\t"                                  \
         "ldr r1, [%1, #8] \n\t"                                  \
         "ldr r2, [%1, #12] \n\t"                                 \
         "ldr r3, [%1, #16] \n\t"                                 \
         "ldr r4, [%1] \n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R4                   \
         VALGRIND_RESTORE_STACK                                   \
         "mov %0, r0"                                             \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r10"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_7W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,   \
                                 arg7)                            \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[8];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "sub sp, sp, #4 \n\t"                                    \
         "ldr r0, [%1, #20] \n\t"                                 \
         "ldr r1, [%1, #24] \n\t"                                 \
         "ldr r2, [%1, #28] \n\t"                                 \
         "push {r0, r1, r2} \n\t"                                 \
         "ldr r0, [%1, #4] \n\t"                                  \
         "ldr r1, [%1, #8] \n\t"                                  \
         "ldr r2, [%1, #12] \n\t"                                 \
         "ldr r3, [%1, #16] \n\t"                                 \
         "ldr r4, [%1] \n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R4                   \
         VALGRIND_RESTORE_STACK                                   \
         "mov %0, r0"                                             \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r10"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_8W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,   \
                                 arg7,arg8)                       \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[9];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "ldr r0, [%1, #20] \n\t"                                 \
         "ldr r1, [%1, #24] \n\t"                                 \
         "ldr r2, [%1, #28] \n\t"                                 \
         "ldr r3, [%1, #32] \n\t"                                 \
         "push {r0, r1, r2, r3} \n\t"                             \
         "ldr r0, [%1, #4] \n\t"                                  \
         "ldr r1, [%1, #8] \n\t"                                  \
         "ldr r2, [%1, #12] \n\t"                                 \
         "ldr r3, [%1, #16] \n\t"                                 \
         "ldr r4, [%1] \n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R4                   \
         VALGRIND_RESTORE_STACK                                   \
         "mov %0, r0"                                             \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r10"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_9W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,   \
                                 arg7,arg8,arg9)                  \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[10];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      _argvec[9] = (unsigned long)(arg9);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "sub sp, sp, #4 \n\t"                                    \
         "ldr r0, [%1, #20] \n\t"                                 \
         "ldr r1, [%1, #24] \n\t"                                 \
         "ldr r2, [%1, #28] \n\t"                                 \
         "ldr r3, [%1, #32] \n\t"                                 \
         "ldr r4, [%1, #36] \n\t"                                 \
         "push {r0, r1, r2, r3, r4} \n\t"                         \
         "ldr r0, [%1, #4] \n\t"                                  \
         "ldr r1, [%1, #8] \n\t"                                  \
         "ldr r2, [%1, #12] \n\t"                                 \
         "ldr r3, [%1, #16] \n\t"                                 \
         "ldr r4, [%1] \n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R4                   \
         VALGRIND_RESTORE_STACK                                   \
         "mov %0, r0"                                             \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r10"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_10W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,  \
                                  arg7,arg8,arg9,arg10)           \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[11];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      _argvec[9] = (unsigned long)(arg9);                         \
      _argvec[10] = (unsigned long)(arg10);                       \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "ldr r0, [%1, #40] \n\t"                                 \
         "push {r0} \n\t"                                         \
         "ldr r0, [%1, #20] \n\t"                                 \
         "ldr r1, [%1, #24] \n\t"                                 \
         "ldr r2, [%1, #28] \n\t"                                 \
         "ldr r3, [%1, #32] \n\t"                                 \
         "ldr r4, [%1, #36] \n\t"                                 \
         "push {r0, r1, r2, r3, r4} \n\t"                         \
         "ldr r0, [%1, #4] \n\t"                                  \
         "ldr r1, [%1, #8] \n\t"                                  \
         "ldr r2, [%1, #12] \n\t"                                 \
         "ldr r3, [%1, #16] \n\t"                                 \
         "ldr r4, [%1] \n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R4                   \
         VALGRIND_RESTORE_STACK                                   \
         "mov %0, r0"                                             \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r10"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_11W(lval, orig, arg1,arg2,arg3,arg4,arg5,       \
                                  arg6,arg7,arg8,arg9,arg10,      \
                                  arg11)                          \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[12];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      _argvec[9] = (unsigned long)(arg9);                         \
      _argvec[10] = (unsigned long)(arg10);                       \
      _argvec[11] = (unsigned long)(arg11);                       \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "sub sp, sp, #4 \n\t"                                    \
         "ldr r0, [%1, #40] \n\t"                                 \
         "ldr r1, [%1, #44] \n\t"                                 \
         "push {r0, r1} \n\t"                                     \
         "ldr r0, [%1, #20] \n\t"                                 \
         "ldr r1, [%1, #24] \n\t"                                 \
         "ldr r2, [%1, #28] \n\t"                                 \
         "ldr r3, [%1, #32] \n\t"                                 \
         "ldr r4, [%1, #36] \n\t"                                 \
         "push {r0, r1, r2, r3, r4} \n\t"                         \
         "ldr r0, [%1, #4] \n\t"                                  \
         "ldr r1, [%1, #8] \n\t"                                  \
         "ldr r2, [%1, #12] \n\t"                                 \
         "ldr r3, [%1, #16] \n\t"                                 \
         "ldr r4, [%1] \n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R4                   \
         VALGRIND_RESTORE_STACK                                   \
         "mov %0, r0"                                             \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r10"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_12W(lval, orig, arg1,arg2,arg3,arg4,arg5,       \
                                  arg6,arg7,arg8,arg9,arg10,      \
                                  arg11,arg12)                    \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[13];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      _argvec[9] = (unsigned long)(arg9);                         \
      _argvec[10] = (unsigned long)(arg10);                       \
      _argvec[11] = (unsigned long)(arg11);                       \
      _argvec[12] = (unsigned long)(arg12);                       \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "ldr r0, [%1, #40] \n\t"                                 \
         "ldr r1, [%1, #44] \n\t"                                 \
         "ldr r2, [%1, #48] \n\t"                                 \
         "push {r0, r1, r2} \n\t"                                 \
         "ldr r0, [%1, #20] \n\t"                                 \
         "ldr r1, [%1, #24] \n\t"                                 \
         "ldr r2, [%1, #28] \n\t"                                 \
         "ldr r3, [%1, #32] \n\t"                                 \
         "ldr r4, [%1, #36] \n\t"                                 \
         "push {r0, r1, r2, r3, r4} \n\t"                         \
         "ldr r0, [%1, #4] \n\t"                                  \
         "ldr r1, [%1, #8] \n\t"                                  \
         "ldr r2, [%1, #12] \n\t"                                 \
         "ldr r3, [%1, #16] \n\t"                                 \
         "ldr r4, [%1] \n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_R4                   \
         VALGRIND_RESTORE_STACK                                   \
         "mov %0, r0"                                             \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "r10"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#endif


#if defined(PLAT_arm64_linux)

#define __CALLER_SAVED_REGS \
     "x0", "x1", "x2", "x3","x4", "x5", "x6", "x7", "x8", "x9",   \
     "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17",      \
     "x18", "x19", "x20", "x30",                                  \
     "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9",  \
     "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17",      \
     "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25",      \
     "v26", "v27", "v28", "v29", "v30", "v31"

#define VALGRIND_ALIGN_STACK               \
      "mov x21, sp\n\t"                    \
      "bic sp, x21, #15\n\t"
#define VALGRIND_RESTORE_STACK             \
      "mov sp,  x21\n\t"


#define CALL_FN_W_v(lval, orig)                                   \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[1];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "ldr x8, [%1] \n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_X8                   \
         VALGRIND_RESTORE_STACK                                   \
         "mov %0, x0\n"                                           \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "x21"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_W(lval, orig, arg1)                             \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[2];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "ldr x0, [%1, #8] \n\t"                                  \
         "ldr x8, [%1] \n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_X8                   \
         VALGRIND_RESTORE_STACK                                   \
         "mov %0, x0\n"                                           \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS, "x21"   \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WW(lval, orig, arg1,arg2)                       \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[3];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "ldr x0, [%1, #8] \n\t"                                  \
         "ldr x1, [%1, #16] \n\t"                                 \
         "ldr x8, [%1] \n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_X8                   \
         VALGRIND_RESTORE_STACK                                   \
         "mov %0, x0\n"                                           \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS     \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WWW(lval, orig, arg1,arg2,arg3)                 \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[4];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "ldr x0, [%1, #8] \n\t"                                  \
         "ldr x1, [%1, #16] \n\t"                                 \
         "ldr x2, [%1, #24] \n\t"                                 \
         "ldr x8, [%1] \n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_X8                   \
         VALGRIND_RESTORE_STACK                                   \
         "mov %0, x0\n"                                           \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS     \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WWWW(lval, orig, arg1,arg2,arg3,arg4)           \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[5];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      __asm__ volatile(                                           \
         VALGRIND_ALIGN_STACK                                     \
         "ldr x0, [%1, #8] \n\t"                                  \
         "ldr x1, [%1, #16] \n\t"                                 \
         "ldr x2, [%1, #24] \n\t"                                 \
         "ldr x3, [%1, #32] \n\t"                                 \
         "ldr x8, [%1] \n\t"                    \
         VALGRIND_BRANCH_AND_LINK_TO_NOREDIR_X8                   \
         VALGRIND_RESTORE_STACK                                   \
         "mov %0, x0"                                             \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "cc", "memory", __CALLER_SAVED_REGS     \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#endif


#if defined(PLAT_s390x_linux)

#if defined(__GNUC__) && defined(__GCC_HAVE_DWARF2_CFI_ASM)
#  define __FRAME_POINTER                                         \
      ,"d"(__builtin_dwarf_cfa())
#  define VALGRIND_CFI_PROLOGUE                                   \
      ".cfi_remember_state\n\t"                                   \
      "lgr 1,%1\n\t"          \
      "lgr 7,11\n\t"                                              \
      "lgr 11,%2\n\t"                                             \
      ".cfi_def_cfa r11, 0\n\t"
#  define VALGRIND_CFI_EPILOGUE                                   \
      "lgr 11, 7\n\t"                                             \
      ".cfi_restore_state\n\t"
#else
#  define __FRAME_POINTER
#  define VALGRIND_CFI_PROLOGUE                                   \
      "lgr 1,%1\n\t"
#  define VALGRIND_CFI_EPILOGUE
#endif


#define __CALLER_SAVED_REGS "0","1","2","3","4","5","14", \
                           "f0","f1","f2","f3","f4","f5","f6","f7"


#define CALL_FN_W_v(lval, orig)                                  \
   do {                                                          \
      volatile OrigFn        _orig = (orig);                     \
      volatile unsigned long  _argvec[1];                        \
      volatile unsigned long _res;                               \
      _argvec[0] = (unsigned long)_orig.nraddr;                  \
      __asm__ volatile(                                          \
         VALGRIND_CFI_PROLOGUE                                   \
         "aghi 15,-160\n\t"                                      \
         "lg 1, 0(1)\n\t"                      \
         VALGRIND_CALL_NOREDIR_R1                                \
         "lgr %0, 2\n\t"                                         \
         "aghi 15,160\n\t"                                       \
         VALGRIND_CFI_EPILOGUE                                   \
         :   "=d" (_res)                                 \
         :    "d" (&_argvec[0]) __FRAME_POINTER           \
         : "cc", "memory", __CALLER_SAVED_REGS,"7"     \
      );                                                         \
      lval = (__typeof__(lval)) _res;                            \
   } while (0)

#define CALL_FN_W_W(lval, orig, arg1)                            \
   do {                                                          \
      volatile OrigFn        _orig = (orig);                     \
      volatile unsigned long _argvec[2];                         \
      volatile unsigned long _res;                               \
      _argvec[0] = (unsigned long)_orig.nraddr;                  \
      _argvec[1] = (unsigned long)arg1;                          \
      __asm__ volatile(                                          \
         VALGRIND_CFI_PROLOGUE                                   \
         "aghi 15,-160\n\t"                                      \
         "lg 2, 8(1)\n\t"                                        \
         "lg 1, 0(1)\n\t"                                        \
         VALGRIND_CALL_NOREDIR_R1                                \
         "lgr %0, 2\n\t"                                         \
         "aghi 15,160\n\t"                                       \
         VALGRIND_CFI_EPILOGUE                                   \
         :   "=d" (_res)                                 \
         :    "a" (&_argvec[0]) __FRAME_POINTER           \
         : "cc", "memory", __CALLER_SAVED_REGS,"7"     \
      );                                                         \
      lval = (__typeof__(lval)) _res;                            \
   } while (0)

#define CALL_FN_W_WW(lval, orig, arg1, arg2)                     \
   do {                                                          \
      volatile OrigFn        _orig = (orig);                     \
      volatile unsigned long _argvec[3];                         \
      volatile unsigned long _res;                               \
      _argvec[0] = (unsigned long)_orig.nraddr;                  \
      _argvec[1] = (unsigned long)arg1;                          \
      _argvec[2] = (unsigned long)arg2;                          \
      __asm__ volatile(                                          \
         VALGRIND_CFI_PROLOGUE                                   \
         "aghi 15,-160\n\t"                                      \
         "lg 2, 8(1)\n\t"                                        \
         "lg 3,16(1)\n\t"                                        \
         "lg 1, 0(1)\n\t"                                        \
         VALGRIND_CALL_NOREDIR_R1                                \
         "lgr %0, 2\n\t"                                         \
         "aghi 15,160\n\t"                                       \
         VALGRIND_CFI_EPILOGUE                                   \
         :   "=d" (_res)                                 \
         :    "a" (&_argvec[0]) __FRAME_POINTER           \
         : "cc", "memory", __CALLER_SAVED_REGS,"7"     \
      );                                                         \
      lval = (__typeof__(lval)) _res;                            \
   } while (0)

#define CALL_FN_W_WWW(lval, orig, arg1, arg2, arg3)              \
   do {                                                          \
      volatile OrigFn        _orig = (orig);                     \
      volatile unsigned long _argvec[4];                         \
      volatile unsigned long _res;                               \
      _argvec[0] = (unsigned long)_orig.nraddr;                  \
      _argvec[1] = (unsigned long)arg1;                          \
      _argvec[2] = (unsigned long)arg2;                          \
      _argvec[3] = (unsigned long)arg3;                          \
      __asm__ volatile(                                          \
         VALGRIND_CFI_PROLOGUE                                   \
         "aghi 15,-160\n\t"                                      \
         "lg 2, 8(1)\n\t"                                        \
         "lg 3,16(1)\n\t"                                        \
         "lg 4,24(1)\n\t"                                        \
         "lg 1, 0(1)\n\t"                                        \
         VALGRIND_CALL_NOREDIR_R1                                \
         "lgr %0, 2\n\t"                                         \
         "aghi 15,160\n\t"                                       \
         VALGRIND_CFI_EPILOGUE                                   \
         :   "=d" (_res)                                 \
         :    "a" (&_argvec[0]) __FRAME_POINTER           \
         : "cc", "memory", __CALLER_SAVED_REGS,"7"     \
      );                                                         \
      lval = (__typeof__(lval)) _res;                            \
   } while (0)

#define CALL_FN_W_WWWW(lval, orig, arg1, arg2, arg3, arg4)       \
   do {                                                          \
      volatile OrigFn        _orig = (orig);                     \
      volatile unsigned long _argvec[5];                         \
      volatile unsigned long _res;                               \
      _argvec[0] = (unsigned long)_orig.nraddr;                  \
      _argvec[1] = (unsigned long)arg1;                          \
      _argvec[2] = (unsigned long)arg2;                          \
      _argvec[3] = (unsigned long)arg3;                          \
      _argvec[4] = (unsigned long)arg4;                          \
      __asm__ volatile(                                          \
         VALGRIND_CFI_PROLOGUE                                   \
         "aghi 15,-160\n\t"                                      \
         "lg 2, 8(1)\n\t"                                        \
         "lg 3,16(1)\n\t"                                        \
         "lg 4,24(1)\n\t"                                        \
         "lg 5,32(1)\n\t"                                        \
         "lg 1, 0(1)\n\t"                                        \
         VALGRIND_CALL_NOREDIR_R1                                \
         "lgr %0, 2\n\t"                                         \
         "aghi 15,160\n\t"                                       \
         VALGRIND_CFI_EPILOGUE                                   \
         :   "=d" (_res)                                 \
         :    "a" (&_argvec[0]) __FRAME_POINTER           \
         : "cc", "memory", __CALLER_SAVED_REGS,"7"     \
      );                                                         \
      lval = (__typeof__(lval)) _res;                            \
   } while (0)

#define CALL_FN_W_5W(lval, orig, arg1, arg2, arg3, arg4, arg5)   \
   do {                                                          \
      volatile OrigFn        _orig = (orig);                     \
      volatile unsigned long _argvec[6];                         \
      volatile unsigned long _res;                               \
      _argvec[0] = (unsigned long)_orig.nraddr;                  \
      _argvec[1] = (unsigned long)arg1;                          \
      _argvec[2] = (unsigned long)arg2;                          \
      _argvec[3] = (unsigned long)arg3;                          \
      _argvec[4] = (unsigned long)arg4;                          \
      _argvec[5] = (unsigned long)arg5;                          \
      __asm__ volatile(                                          \
         VALGRIND_CFI_PROLOGUE                                   \
         "aghi 15,-160\n\t"                                      \
         "lg 2, 8(1)\n\t"                                        \
         "lg 3,16(1)\n\t"                                        \
         "lg 4,24(1)\n\t"                                        \
         "lg 5,32(1)\n\t"                                        \
         "lg 6,40(1)\n\t"                                        \
         "lg 1, 0(1)\n\t"                                        \
         VALGRIND_CALL_NOREDIR_R1                                \
         "lgr %0, 2\n\t"                                         \
         "aghi 15,160\n\t"                                       \
         VALGRIND_CFI_EPILOGUE                                   \
         :   "=d" (_res)                                 \
         :    "a" (&_argvec[0]) __FRAME_POINTER           \
         : "cc", "memory", __CALLER_SAVED_REGS,"6","7" \
      );                                                         \
      lval = (__typeof__(lval)) _res;                            \
   } while (0)

#define CALL_FN_W_6W(lval, orig, arg1, arg2, arg3, arg4, arg5,   \
                     arg6)                                       \
   do {                                                          \
      volatile OrigFn        _orig = (orig);                     \
      volatile unsigned long _argvec[7];                         \
      volatile unsigned long _res;                               \
      _argvec[0] = (unsigned long)_orig.nraddr;                  \
      _argvec[1] = (unsigned long)arg1;                          \
      _argvec[2] = (unsigned long)arg2;                          \
      _argvec[3] = (unsigned long)arg3;                          \
      _argvec[4] = (unsigned long)arg4;                          \
      _argvec[5] = (unsigned long)arg5;                          \
      _argvec[6] = (unsigned long)arg6;                          \
      __asm__ volatile(                                          \
         VALGRIND_CFI_PROLOGUE                                   \
         "aghi 15,-168\n\t"                                      \
         "lg 2, 8(1)\n\t"                                        \
         "lg 3,16(1)\n\t"                                        \
         "lg 4,24(1)\n\t"                                        \
         "lg 5,32(1)\n\t"                                        \
         "lg 6,40(1)\n\t"                                        \
         "mvc 160(8,15), 48(1)\n\t"                              \
         "lg 1, 0(1)\n\t"                                        \
         VALGRIND_CALL_NOREDIR_R1                                \
         "lgr %0, 2\n\t"                                         \
         "aghi 15,168\n\t"                                       \
         VALGRIND_CFI_EPILOGUE                                   \
         :   "=d" (_res)                                 \
         :    "a" (&_argvec[0]) __FRAME_POINTER           \
         : "cc", "memory", __CALLER_SAVED_REGS,"6","7" \
      );                                                         \
      lval = (__typeof__(lval)) _res;                            \
   } while (0)

#define CALL_FN_W_7W(lval, orig, arg1, arg2, arg3, arg4, arg5,   \
                     arg6, arg7)                                 \
   do {                                                          \
      volatile OrigFn        _orig = (orig);                     \
      volatile unsigned long _argvec[8];                         \
      volatile unsigned long _res;                               \
      _argvec[0] = (unsigned long)_orig.nraddr;                  \
      _argvec[1] = (unsigned long)arg1;                          \
      _argvec[2] = (unsigned long)arg2;                          \
      _argvec[3] = (unsigned long)arg3;                          \
      _argvec[4] = (unsigned long)arg4;                          \
      _argvec[5] = (unsigned long)arg5;                          \
      _argvec[6] = (unsigned long)arg6;                          \
      _argvec[7] = (unsigned long)arg7;                          \
      __asm__ volatile(                                          \
         VALGRIND_CFI_PROLOGUE                                   \
         "aghi 15,-176\n\t"                                      \
         "lg 2, 8(1)\n\t"                                        \
         "lg 3,16(1)\n\t"                                        \
         "lg 4,24(1)\n\t"                                        \
         "lg 5,32(1)\n\t"                                        \
         "lg 6,40(1)\n\t"                                        \
         "mvc 160(8,15), 48(1)\n\t"                              \
         "mvc 168(8,15), 56(1)\n\t"                              \
         "lg 1, 0(1)\n\t"                                        \
         VALGRIND_CALL_NOREDIR_R1                                \
         "lgr %0, 2\n\t"                                         \
         "aghi 15,176\n\t"                                       \
         VALGRIND_CFI_EPILOGUE                                   \
         :   "=d" (_res)                                 \
         :    "a" (&_argvec[0]) __FRAME_POINTER           \
         : "cc", "memory", __CALLER_SAVED_REGS,"6","7" \
      );                                                         \
      lval = (__typeof__(lval)) _res;                            \
   } while (0)

#define CALL_FN_W_8W(lval, orig, arg1, arg2, arg3, arg4, arg5,   \
                     arg6, arg7 ,arg8)                           \
   do {                                                          \
      volatile OrigFn        _orig = (orig);                     \
      volatile unsigned long _argvec[9];                         \
      volatile unsigned long _res;                               \
      _argvec[0] = (unsigned long)_orig.nraddr;                  \
      _argvec[1] = (unsigned long)arg1;                          \
      _argvec[2] = (unsigned long)arg2;                          \
      _argvec[3] = (unsigned long)arg3;                          \
      _argvec[4] = (unsigned long)arg4;                          \
      _argvec[5] = (unsigned long)arg5;                          \
      _argvec[6] = (unsigned long)arg6;                          \
      _argvec[7] = (unsigned long)arg7;                          \
      _argvec[8] = (unsigned long)arg8;                          \
      __asm__ volatile(                                          \
         VALGRIND_CFI_PROLOGUE                                   \
         "aghi 15,-184\n\t"                                      \
         "lg 2, 8(1)\n\t"                                        \
         "lg 3,16(1)\n\t"                                        \
         "lg 4,24(1)\n\t"                                        \
         "lg 5,32(1)\n\t"                                        \
         "lg 6,40(1)\n\t"                                        \
         "mvc 160(8,15), 48(1)\n\t"                              \
         "mvc 168(8,15), 56(1)\n\t"                              \
         "mvc 176(8,15), 64(1)\n\t"                              \
         "lg 1, 0(1)\n\t"                                        \
         VALGRIND_CALL_NOREDIR_R1                                \
         "lgr %0, 2\n\t"                                         \
         "aghi 15,184\n\t"                                       \
         VALGRIND_CFI_EPILOGUE                                   \
         :   "=d" (_res)                                 \
         :    "a" (&_argvec[0]) __FRAME_POINTER           \
         : "cc", "memory", __CALLER_SAVED_REGS,"6","7" \
      );                                                         \
      lval = (__typeof__(lval)) _res;                            \
   } while (0)

#define CALL_FN_W_9W(lval, orig, arg1, arg2, arg3, arg4, arg5,   \
                     arg6, arg7 ,arg8, arg9)                     \
   do {                                                          \
      volatile OrigFn        _orig = (orig);                     \
      volatile unsigned long _argvec[10];                        \
      volatile unsigned long _res;                               \
      _argvec[0] = (unsigned long)_orig.nraddr;                  \
      _argvec[1] = (unsigned long)arg1;                          \
      _argvec[2] = (unsigned long)arg2;                          \
      _argvec[3] = (unsigned long)arg3;                          \
      _argvec[4] = (unsigned long)arg4;                          \
      _argvec[5] = (unsigned long)arg5;                          \
      _argvec[6] = (unsigned long)arg6;                          \
      _argvec[7] = (unsigned long)arg7;                          \
      _argvec[8] = (unsigned long)arg8;                          \
      _argvec[9] = (unsigned long)arg9;                          \
      __asm__ volatile(                                          \
         VALGRIND_CFI_PROLOGUE                                   \
         "aghi 15,-192\n\t"                                      \
         "lg 2, 8(1)\n\t"                                        \
         "lg 3,16(1)\n\t"                                        \
         "lg 4,24(1)\n\t"                                        \
         "lg 5,32(1)\n\t"                                        \
         "lg 6,40(1)\n\t"                                        \
         "mvc 160(8,15), 48(1)\n\t"                              \
         "mvc 168(8,15), 56(1)\n\t"                              \
         "mvc 176(8,15), 64(1)\n\t"                              \
         "mvc 184(8,15), 72(1)\n\t"                              \
         "lg 1, 0(1)\n\t"                                        \
         VALGRIND_CALL_NOREDIR_R1                                \
         "lgr %0, 2\n\t"                                         \
         "aghi 15,192\n\t"                                       \
         VALGRIND_CFI_EPILOGUE                                   \
         :   "=d" (_res)                                 \
         :    "a" (&_argvec[0]) __FRAME_POINTER           \
         : "cc", "memory", __CALLER_SAVED_REGS,"6","7" \
      );                                                         \
      lval = (__typeof__(lval)) _res;                            \
   } while (0)

#define CALL_FN_W_10W(lval, orig, arg1, arg2, arg3, arg4, arg5,  \
                     arg6, arg7 ,arg8, arg9, arg10)              \
   do {                                                          \
      volatile OrigFn        _orig = (orig);                     \
      volatile unsigned long _argvec[11];                        \
      volatile unsigned long _res;                               \
      _argvec[0] = (unsigned long)_orig.nraddr;                  \
      _argvec[1] = (unsigned long)arg1;                          \
      _argvec[2] = (unsigned long)arg2;                          \
      _argvec[3] = (unsigned long)arg3;                          \
      _argvec[4] = (unsigned long)arg4;                          \
      _argvec[5] = (unsigned long)arg5;                          \
      _argvec[6] = (unsigned long)arg6;                          \
      _argvec[7] = (unsigned long)arg7;                          \
      _argvec[8] = (unsigned long)arg8;                          \
      _argvec[9] = (unsigned long)arg9;                          \
      _argvec[10] = (unsigned long)arg10;                        \
      __asm__ volatile(                                          \
         VALGRIND_CFI_PROLOGUE                                   \
         "aghi 15,-200\n\t"                                      \
         "lg 2, 8(1)\n\t"                                        \
         "lg 3,16(1)\n\t"                                        \
         "lg 4,24(1)\n\t"                                        \
         "lg 5,32(1)\n\t"                                        \
         "lg 6,40(1)\n\t"                                        \
         "mvc 160(8,15), 48(1)\n\t"                              \
         "mvc 168(8,15), 56(1)\n\t"                              \
         "mvc 176(8,15), 64(1)\n\t"                              \
         "mvc 184(8,15), 72(1)\n\t"                              \
         "mvc 192(8,15), 80(1)\n\t"                              \
         "lg 1, 0(1)\n\t"                                        \
         VALGRIND_CALL_NOREDIR_R1                                \
         "lgr %0, 2\n\t"                                         \
         "aghi 15,200\n\t"                                       \
         VALGRIND_CFI_EPILOGUE                                   \
         :   "=d" (_res)                                 \
         :    "a" (&_argvec[0]) __FRAME_POINTER           \
         : "cc", "memory", __CALLER_SAVED_REGS,"6","7" \
      );                                                         \
      lval = (__typeof__(lval)) _res;                            \
   } while (0)

#define CALL_FN_W_11W(lval, orig, arg1, arg2, arg3, arg4, arg5,  \
                     arg6, arg7 ,arg8, arg9, arg10, arg11)       \
   do {                                                          \
      volatile OrigFn        _orig = (orig);                     \
      volatile unsigned long _argvec[12];                        \
      volatile unsigned long _res;                               \
      _argvec[0] = (unsigned long)_orig.nraddr;                  \
      _argvec[1] = (unsigned long)arg1;                          \
      _argvec[2] = (unsigned long)arg2;                          \
      _argvec[3] = (unsigned long)arg3;                          \
      _argvec[4] = (unsigned long)arg4;                          \
      _argvec[5] = (unsigned long)arg5;                          \
      _argvec[6] = (unsigned long)arg6;                          \
      _argvec[7] = (unsigned long)arg7;                          \
      _argvec[8] = (unsigned long)arg8;                          \
      _argvec[9] = (unsigned long)arg9;                          \
      _argvec[10] = (unsigned long)arg10;                        \
      _argvec[11] = (unsigned long)arg11;                        \
      __asm__ volatile(                                          \
         VALGRIND_CFI_PROLOGUE                                   \
         "aghi 15,-208\n\t"                                      \
         "lg 2, 8(1)\n\t"                                        \
         "lg 3,16(1)\n\t"                                        \
         "lg 4,24(1)\n\t"                                        \
         "lg 5,32(1)\n\t"                                        \
         "lg 6,40(1)\n\t"                                        \
         "mvc 160(8,15), 48(1)\n\t"                              \
         "mvc 168(8,15), 56(1)\n\t"                              \
         "mvc 176(8,15), 64(1)\n\t"                              \
         "mvc 184(8,15), 72(1)\n\t"                              \
         "mvc 192(8,15), 80(1)\n\t"                              \
         "mvc 200(8,15), 88(1)\n\t"                              \
         "lg 1, 0(1)\n\t"                                        \
         VALGRIND_CALL_NOREDIR_R1                                \
         "lgr %0, 2\n\t"                                         \
         "aghi 15,208\n\t"                                       \
         VALGRIND_CFI_EPILOGUE                                   \
         :   "=d" (_res)                                 \
         :    "a" (&_argvec[0]) __FRAME_POINTER           \
         : "cc", "memory", __CALLER_SAVED_REGS,"6","7" \
      );                                                         \
      lval = (__typeof__(lval)) _res;                            \
   } while (0)

#define CALL_FN_W_12W(lval, orig, arg1, arg2, arg3, arg4, arg5,  \
                     arg6, arg7 ,arg8, arg9, arg10, arg11, arg12)\
   do {                                                          \
      volatile OrigFn        _orig = (orig);                     \
      volatile unsigned long _argvec[13];                        \
      volatile unsigned long _res;                               \
      _argvec[0] = (unsigned long)_orig.nraddr;                  \
      _argvec[1] = (unsigned long)arg1;                          \
      _argvec[2] = (unsigned long)arg2;                          \
      _argvec[3] = (unsigned long)arg3;                          \
      _argvec[4] = (unsigned long)arg4;                          \
      _argvec[5] = (unsigned long)arg5;                          \
      _argvec[6] = (unsigned long)arg6;                          \
      _argvec[7] = (unsigned long)arg7;                          \
      _argvec[8] = (unsigned long)arg8;                          \
      _argvec[9] = (unsigned long)arg9;                          \
      _argvec[10] = (unsigned long)arg10;                        \
      _argvec[11] = (unsigned long)arg11;                        \
      _argvec[12] = (unsigned long)arg12;                        \
      __asm__ volatile(                                          \
         VALGRIND_CFI_PROLOGUE                                   \
         "aghi 15,-216\n\t"                                      \
         "lg 2, 8(1)\n\t"                                        \
         "lg 3,16(1)\n\t"                                        \
         "lg 4,24(1)\n\t"                                        \
         "lg 5,32(1)\n\t"                                        \
         "lg 6,40(1)\n\t"                                        \
         "mvc 160(8,15), 48(1)\n\t"                              \
         "mvc 168(8,15), 56(1)\n\t"                              \
         "mvc 176(8,15), 64(1)\n\t"                              \
         "mvc 184(8,15), 72(1)\n\t"                              \
         "mvc 192(8,15), 80(1)\n\t"                              \
         "mvc 200(8,15), 88(1)\n\t"                              \
         "mvc 208(8,15), 96(1)\n\t"                              \
         "lg 1, 0(1)\n\t"                                        \
         VALGRIND_CALL_NOREDIR_R1                                \
         "lgr %0, 2\n\t"                                         \
         "aghi 15,216\n\t"                                       \
         VALGRIND_CFI_EPILOGUE                                   \
         :   "=d" (_res)                                 \
         :    "a" (&_argvec[0]) __FRAME_POINTER           \
         : "cc", "memory", __CALLER_SAVED_REGS,"6","7" \
      );                                                         \
      lval = (__typeof__(lval)) _res;                            \
   } while (0)


#endif


#if defined(PLAT_mips32_linux)

#define __CALLER_SAVED_REGS "$2", "$3", "$4", "$5", "$6",       \
"$7", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$24", \
"$25", "$31"


#define CALL_FN_W_v(lval, orig)                                   \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[1];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      __asm__ volatile(                                           \
         "subu $29, $29, 8 \n\t"                                  \
         "sw $28, 0($29) \n\t"                                    \
         "sw $31, 4($29) \n\t"                                    \
         "subu $29, $29, 16 \n\t"                                 \
         "lw $25, 0(%1) \n\t"                   \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "addu $29, $29, 16\n\t"                                  \
         "lw $28, 0($29) \n\t"                                    \
         "lw $31, 4($29) \n\t"                                    \
         "addu $29, $29, 8 \n\t"                                  \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_W(lval, orig, arg1)                             \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
     volatile unsigned long _argvec[2];                           \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      __asm__ volatile(                                           \
         "subu $29, $29, 8 \n\t"                                  \
         "sw $28, 0($29) \n\t"                                    \
         "sw $31, 4($29) \n\t"                                    \
         "subu $29, $29, 16 \n\t"                                 \
         "lw $4, 4(%1) \n\t"                          \
         "lw $25, 0(%1) \n\t"                   \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "addu $29, $29, 16 \n\t"                                 \
         "lw $28, 0($29) \n\t"                                    \
         "lw $31, 4($29) \n\t"                                    \
         "addu $29, $29, 8 \n\t"                                  \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "memory",  __CALLER_SAVED_REGS               \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WW(lval, orig, arg1,arg2)                       \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[3];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      __asm__ volatile(                                           \
         "subu $29, $29, 8 \n\t"                                  \
         "sw $28, 0($29) \n\t"                                    \
         "sw $31, 4($29) \n\t"                                    \
         "subu $29, $29, 16 \n\t"                                 \
         "lw $4, 4(%1) \n\t"                                      \
         "lw $5, 8(%1) \n\t"                                      \
         "lw $25, 0(%1) \n\t"                   \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "addu $29, $29, 16 \n\t"                                 \
         "lw $28, 0($29) \n\t"                                    \
         "lw $31, 4($29) \n\t"                                    \
         "addu $29, $29, 8 \n\t"                                  \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WWW(lval, orig, arg1,arg2,arg3)                 \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[4];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      __asm__ volatile(                                           \
         "subu $29, $29, 8 \n\t"                                  \
         "sw $28, 0($29) \n\t"                                    \
         "sw $31, 4($29) \n\t"                                    \
         "subu $29, $29, 16 \n\t"                                 \
         "lw $4, 4(%1) \n\t"                                      \
         "lw $5, 8(%1) \n\t"                                      \
         "lw $6, 12(%1) \n\t"                                     \
         "lw $25, 0(%1) \n\t"                   \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "addu $29, $29, 16 \n\t"                                 \
         "lw $28, 0($29) \n\t"                                    \
         "lw $31, 4($29) \n\t"                                    \
         "addu $29, $29, 8 \n\t"                                  \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WWWW(lval, orig, arg1,arg2,arg3,arg4)           \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[5];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      __asm__ volatile(                                           \
         "subu $29, $29, 8 \n\t"                                  \
         "sw $28, 0($29) \n\t"                                    \
         "sw $31, 4($29) \n\t"                                    \
         "subu $29, $29, 16 \n\t"                                 \
         "lw $4, 4(%1) \n\t"                                      \
         "lw $5, 8(%1) \n\t"                                      \
         "lw $6, 12(%1) \n\t"                                     \
         "lw $7, 16(%1) \n\t"                                     \
         "lw $25, 0(%1) \n\t"                   \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "addu $29, $29, 16 \n\t"                                 \
         "lw $28, 0($29) \n\t"                                    \
         "lw $31, 4($29) \n\t"                                    \
         "addu $29, $29, 8 \n\t"                                  \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_5W(lval, orig, arg1,arg2,arg3,arg4,arg5)        \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[6];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      __asm__ volatile(                                           \
         "subu $29, $29, 8 \n\t"                                  \
         "sw $28, 0($29) \n\t"                                    \
         "sw $31, 4($29) \n\t"                                    \
         "lw $4, 20(%1) \n\t"                                     \
         "subu $29, $29, 24\n\t"                                  \
         "sw $4, 16($29) \n\t"                                    \
         "lw $4, 4(%1) \n\t"                                      \
         "lw $5, 8(%1) \n\t"                                      \
         "lw $6, 12(%1) \n\t"                                     \
         "lw $7, 16(%1) \n\t"                                     \
         "lw $25, 0(%1) \n\t"                   \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "addu $29, $29, 24 \n\t"                                 \
         "lw $28, 0($29) \n\t"                                    \
         "lw $31, 4($29) \n\t"                                    \
         "addu $29, $29, 8 \n\t"                                  \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)
#define CALL_FN_W_6W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6)   \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[7];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      __asm__ volatile(                                           \
         "subu $29, $29, 8 \n\t"                                  \
         "sw $28, 0($29) \n\t"                                    \
         "sw $31, 4($29) \n\t"                                    \
         "lw $4, 20(%1) \n\t"                                     \
         "subu $29, $29, 32\n\t"                                  \
         "sw $4, 16($29) \n\t"                                    \
         "lw $4, 24(%1) \n\t"                                     \
         "nop\n\t"                                                \
         "sw $4, 20($29) \n\t"                                    \
         "lw $4, 4(%1) \n\t"                                      \
         "lw $5, 8(%1) \n\t"                                      \
         "lw $6, 12(%1) \n\t"                                     \
         "lw $7, 16(%1) \n\t"                                     \
         "lw $25, 0(%1) \n\t"                   \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "addu $29, $29, 32 \n\t"                                 \
         "lw $28, 0($29) \n\t"                                    \
         "lw $31, 4($29) \n\t"                                    \
         "addu $29, $29, 8 \n\t"                                  \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_7W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,   \
                                 arg7)                            \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[8];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      __asm__ volatile(                                           \
         "subu $29, $29, 8 \n\t"                                  \
         "sw $28, 0($29) \n\t"                                    \
         "sw $31, 4($29) \n\t"                                    \
         "lw $4, 20(%1) \n\t"                                     \
         "subu $29, $29, 32\n\t"                                  \
         "sw $4, 16($29) \n\t"                                    \
         "lw $4, 24(%1) \n\t"                                     \
         "sw $4, 20($29) \n\t"                                    \
         "lw $4, 28(%1) \n\t"                                     \
         "sw $4, 24($29) \n\t"                                    \
         "lw $4, 4(%1) \n\t"                                      \
         "lw $5, 8(%1) \n\t"                                      \
         "lw $6, 12(%1) \n\t"                                     \
         "lw $7, 16(%1) \n\t"                                     \
         "lw $25, 0(%1) \n\t"                   \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "addu $29, $29, 32 \n\t"                                 \
         "lw $28, 0($29) \n\t"                                    \
         "lw $31, 4($29) \n\t"                                    \
         "addu $29, $29, 8 \n\t"                                  \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_8W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,   \
                                 arg7,arg8)                       \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[9];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      __asm__ volatile(                                           \
         "subu $29, $29, 8 \n\t"                                  \
         "sw $28, 0($29) \n\t"                                    \
         "sw $31, 4($29) \n\t"                                    \
         "lw $4, 20(%1) \n\t"                                     \
         "subu $29, $29, 40\n\t"                                  \
         "sw $4, 16($29) \n\t"                                    \
         "lw $4, 24(%1) \n\t"                                     \
         "sw $4, 20($29) \n\t"                                    \
         "lw $4, 28(%1) \n\t"                                     \
         "sw $4, 24($29) \n\t"                                    \
         "lw $4, 32(%1) \n\t"                                     \
         "sw $4, 28($29) \n\t"                                    \
         "lw $4, 4(%1) \n\t"                                      \
         "lw $5, 8(%1) \n\t"                                      \
         "lw $6, 12(%1) \n\t"                                     \
         "lw $7, 16(%1) \n\t"                                     \
         "lw $25, 0(%1) \n\t"                   \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "addu $29, $29, 40 \n\t"                                 \
         "lw $28, 0($29) \n\t"                                    \
         "lw $31, 4($29) \n\t"                                    \
         "addu $29, $29, 8 \n\t"                                  \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_9W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,   \
                                 arg7,arg8,arg9)                  \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[10];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      _argvec[9] = (unsigned long)(arg9);                         \
      __asm__ volatile(                                           \
         "subu $29, $29, 8 \n\t"                                  \
         "sw $28, 0($29) \n\t"                                    \
         "sw $31, 4($29) \n\t"                                    \
         "lw $4, 20(%1) \n\t"                                     \
         "subu $29, $29, 40\n\t"                                  \
         "sw $4, 16($29) \n\t"                                    \
         "lw $4, 24(%1) \n\t"                                     \
         "sw $4, 20($29) \n\t"                                    \
         "lw $4, 28(%1) \n\t"                                     \
         "sw $4, 24($29) \n\t"                                    \
         "lw $4, 32(%1) \n\t"                                     \
         "sw $4, 28($29) \n\t"                                    \
         "lw $4, 36(%1) \n\t"                                     \
         "sw $4, 32($29) \n\t"                                    \
         "lw $4, 4(%1) \n\t"                                      \
         "lw $5, 8(%1) \n\t"                                      \
         "lw $6, 12(%1) \n\t"                                     \
         "lw $7, 16(%1) \n\t"                                     \
         "lw $25, 0(%1) \n\t"                   \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "addu $29, $29, 40 \n\t"                                 \
         "lw $28, 0($29) \n\t"                                    \
         "lw $31, 4($29) \n\t"                                    \
         "addu $29, $29, 8 \n\t"                                  \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_10W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,  \
                                  arg7,arg8,arg9,arg10)           \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[11];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      _argvec[9] = (unsigned long)(arg9);                         \
      _argvec[10] = (unsigned long)(arg10);                       \
      __asm__ volatile(                                           \
         "subu $29, $29, 8 \n\t"                                  \
         "sw $28, 0($29) \n\t"                                    \
         "sw $31, 4($29) \n\t"                                    \
         "lw $4, 20(%1) \n\t"                                     \
         "subu $29, $29, 48\n\t"                                  \
         "sw $4, 16($29) \n\t"                                    \
         "lw $4, 24(%1) \n\t"                                     \
         "sw $4, 20($29) \n\t"                                    \
         "lw $4, 28(%1) \n\t"                                     \
         "sw $4, 24($29) \n\t"                                    \
         "lw $4, 32(%1) \n\t"                                     \
         "sw $4, 28($29) \n\t"                                    \
         "lw $4, 36(%1) \n\t"                                     \
         "sw $4, 32($29) \n\t"                                    \
         "lw $4, 40(%1) \n\t"                                     \
         "sw $4, 36($29) \n\t"                                    \
         "lw $4, 4(%1) \n\t"                                      \
         "lw $5, 8(%1) \n\t"                                      \
         "lw $6, 12(%1) \n\t"                                     \
         "lw $7, 16(%1) \n\t"                                     \
         "lw $25, 0(%1) \n\t"                   \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "addu $29, $29, 48 \n\t"                                 \
         "lw $28, 0($29) \n\t"                                    \
         "lw $31, 4($29) \n\t"                                    \
         "addu $29, $29, 8 \n\t"                                  \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_11W(lval, orig, arg1,arg2,arg3,arg4,arg5,       \
                                  arg6,arg7,arg8,arg9,arg10,      \
                                  arg11)                          \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[12];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      _argvec[9] = (unsigned long)(arg9);                         \
      _argvec[10] = (unsigned long)(arg10);                       \
      _argvec[11] = (unsigned long)(arg11);                       \
      __asm__ volatile(                                           \
         "subu $29, $29, 8 \n\t"                                  \
         "sw $28, 0($29) \n\t"                                    \
         "sw $31, 4($29) \n\t"                                    \
         "lw $4, 20(%1) \n\t"                                     \
         "subu $29, $29, 48\n\t"                                  \
         "sw $4, 16($29) \n\t"                                    \
         "lw $4, 24(%1) \n\t"                                     \
         "sw $4, 20($29) \n\t"                                    \
         "lw $4, 28(%1) \n\t"                                     \
         "sw $4, 24($29) \n\t"                                    \
         "lw $4, 32(%1) \n\t"                                     \
         "sw $4, 28($29) \n\t"                                    \
         "lw $4, 36(%1) \n\t"                                     \
         "sw $4, 32($29) \n\t"                                    \
         "lw $4, 40(%1) \n\t"                                     \
         "sw $4, 36($29) \n\t"                                    \
         "lw $4, 44(%1) \n\t"                                     \
         "sw $4, 40($29) \n\t"                                    \
         "lw $4, 4(%1) \n\t"                                      \
         "lw $5, 8(%1) \n\t"                                      \
         "lw $6, 12(%1) \n\t"                                     \
         "lw $7, 16(%1) \n\t"                                     \
         "lw $25, 0(%1) \n\t"                   \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "addu $29, $29, 48 \n\t"                                 \
         "lw $28, 0($29) \n\t"                                    \
         "lw $31, 4($29) \n\t"                                    \
         "addu $29, $29, 8 \n\t"                                  \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_12W(lval, orig, arg1,arg2,arg3,arg4,arg5,       \
                                  arg6,arg7,arg8,arg9,arg10,      \
                                  arg11,arg12)                    \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[13];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      _argvec[9] = (unsigned long)(arg9);                         \
      _argvec[10] = (unsigned long)(arg10);                       \
      _argvec[11] = (unsigned long)(arg11);                       \
      _argvec[12] = (unsigned long)(arg12);                       \
      __asm__ volatile(                                           \
         "subu $29, $29, 8 \n\t"                                  \
         "sw $28, 0($29) \n\t"                                    \
         "sw $31, 4($29) \n\t"                                    \
         "lw $4, 20(%1) \n\t"                                     \
         "subu $29, $29, 56\n\t"                                  \
         "sw $4, 16($29) \n\t"                                    \
         "lw $4, 24(%1) \n\t"                                     \
         "sw $4, 20($29) \n\t"                                    \
         "lw $4, 28(%1) \n\t"                                     \
         "sw $4, 24($29) \n\t"                                    \
         "lw $4, 32(%1) \n\t"                                     \
         "sw $4, 28($29) \n\t"                                    \
         "lw $4, 36(%1) \n\t"                                     \
         "sw $4, 32($29) \n\t"                                    \
         "lw $4, 40(%1) \n\t"                                     \
         "sw $4, 36($29) \n\t"                                    \
         "lw $4, 44(%1) \n\t"                                     \
         "sw $4, 40($29) \n\t"                                    \
         "lw $4, 48(%1) \n\t"                                     \
         "sw $4, 44($29) \n\t"                                    \
         "lw $4, 4(%1) \n\t"                                      \
         "lw $5, 8(%1) \n\t"                                      \
         "lw $6, 12(%1) \n\t"                                     \
         "lw $7, 16(%1) \n\t"                                     \
         "lw $25, 0(%1) \n\t"                   \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "addu $29, $29, 56 \n\t"                                 \
         "lw $28, 0($29) \n\t"                                    \
         "lw $31, 4($29) \n\t"                                    \
         "addu $29, $29, 8 \n\t"                                  \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#endif


#if defined(PLAT_mips64_linux)

#define __CALLER_SAVED_REGS "$2", "$3", "$4", "$5", "$6",       \
"$7", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$24", \
"$25", "$31"


#define CALL_FN_W_v(lval, orig)                                   \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[1];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      __asm__ volatile(                                           \
         "ld $25, 0(%1)\n\t"                    \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "0" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_W(lval, orig, arg1)                             \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[2];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      __asm__ volatile(                                           \
         "ld $4, 8(%1)\n\t"                           \
         "ld $25, 0(%1)\n\t"                    \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WW(lval, orig, arg1,arg2)                       \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[3];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      __asm__ volatile(                                           \
         "ld $4, 8(%1)\n\t"                                       \
         "ld $5, 16(%1)\n\t"                                      \
         "ld $25, 0(%1)\n\t"                    \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WWW(lval, orig, arg1,arg2,arg3)                 \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[4];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      __asm__ volatile(                                           \
         "ld $4, 8(%1)\n\t"                                       \
         "ld $5, 16(%1)\n\t"                                      \
         "ld $6, 24(%1)\n\t"                                      \
         "ld $25, 0(%1)\n\t"                    \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_WWWW(lval, orig, arg1,arg2,arg3,arg4)           \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[5];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      __asm__ volatile(                                           \
         "ld $4, 8(%1)\n\t"                                       \
         "ld $5, 16(%1)\n\t"                                      \
         "ld $6, 24(%1)\n\t"                                      \
         "ld $7, 32(%1)\n\t"                                      \
         "ld $25, 0(%1)\n\t"                    \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_5W(lval, orig, arg1,arg2,arg3,arg4,arg5)        \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[6];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      __asm__ volatile(                                           \
         "ld $4, 8(%1)\n\t"                                       \
         "ld $5, 16(%1)\n\t"                                      \
         "ld $6, 24(%1)\n\t"                                      \
         "ld $7, 32(%1)\n\t"                                      \
         "ld $8, 40(%1)\n\t"                                      \
         "ld $25, 0(%1)\n\t"                    \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_6W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6)   \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[7];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      __asm__ volatile(                                           \
         "ld $4, 8(%1)\n\t"                                       \
         "ld $5, 16(%1)\n\t"                                      \
         "ld $6, 24(%1)\n\t"                                      \
         "ld $7, 32(%1)\n\t"                                      \
         "ld $8, 40(%1)\n\t"                                      \
         "ld $9, 48(%1)\n\t"                                      \
         "ld $25, 0(%1)\n\t"                    \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_7W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,   \
                                 arg7)                            \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[8];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      __asm__ volatile(                                           \
         "ld $4, 8(%1)\n\t"                                       \
         "ld $5, 16(%1)\n\t"                                      \
         "ld $6, 24(%1)\n\t"                                      \
         "ld $7, 32(%1)\n\t"                                      \
         "ld $8, 40(%1)\n\t"                                      \
         "ld $9, 48(%1)\n\t"                                      \
         "ld $10, 56(%1)\n\t"                                     \
         "ld $25, 0(%1) \n\t"                   \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_8W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,   \
                                 arg7,arg8)                       \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[9];                          \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      __asm__ volatile(                                           \
         "ld $4, 8(%1)\n\t"                                       \
         "ld $5, 16(%1)\n\t"                                      \
         "ld $6, 24(%1)\n\t"                                      \
         "ld $7, 32(%1)\n\t"                                      \
         "ld $8, 40(%1)\n\t"                                      \
         "ld $9, 48(%1)\n\t"                                      \
         "ld $10, 56(%1)\n\t"                                     \
         "ld $11, 64(%1)\n\t"                                     \
         "ld $25, 0(%1) \n\t"                   \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_9W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,   \
                                 arg7,arg8,arg9)                  \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[10];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      _argvec[9] = (unsigned long)(arg9);                         \
      __asm__ volatile(                                           \
         "dsubu $29, $29, 8\n\t"                                  \
         "ld $4, 72(%1)\n\t"                                      \
         "sd $4, 0($29)\n\t"                                      \
         "ld $4, 8(%1)\n\t"                                       \
         "ld $5, 16(%1)\n\t"                                      \
         "ld $6, 24(%1)\n\t"                                      \
         "ld $7, 32(%1)\n\t"                                      \
         "ld $8, 40(%1)\n\t"                                      \
         "ld $9, 48(%1)\n\t"                                      \
         "ld $10, 56(%1)\n\t"                                     \
         "ld $11, 64(%1)\n\t"                                     \
         "ld $25, 0(%1)\n\t"                    \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "daddu $29, $29, 8\n\t"                                  \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_10W(lval, orig, arg1,arg2,arg3,arg4,arg5,arg6,  \
                                  arg7,arg8,arg9,arg10)           \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[11];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      _argvec[9] = (unsigned long)(arg9);                         \
      _argvec[10] = (unsigned long)(arg10);                       \
      __asm__ volatile(                                           \
         "dsubu $29, $29, 16\n\t"                                 \
         "ld $4, 72(%1)\n\t"                                      \
         "sd $4, 0($29)\n\t"                                      \
         "ld $4, 80(%1)\n\t"                                      \
         "sd $4, 8($29)\n\t"                                      \
         "ld $4, 8(%1)\n\t"                                       \
         "ld $5, 16(%1)\n\t"                                      \
         "ld $6, 24(%1)\n\t"                                      \
         "ld $7, 32(%1)\n\t"                                      \
         "ld $8, 40(%1)\n\t"                                      \
         "ld $9, 48(%1)\n\t"                                      \
         "ld $10, 56(%1)\n\t"                                     \
         "ld $11, 64(%1)\n\t"                                     \
         "ld $25, 0(%1)\n\t"                    \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "daddu $29, $29, 16\n\t"                                 \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_11W(lval, orig, arg1,arg2,arg3,arg4,arg5,       \
                                  arg6,arg7,arg8,arg9,arg10,      \
                                  arg11)                          \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[12];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      _argvec[9] = (unsigned long)(arg9);                         \
      _argvec[10] = (unsigned long)(arg10);                       \
      _argvec[11] = (unsigned long)(arg11);                       \
      __asm__ volatile(                                           \
         "dsubu $29, $29, 24\n\t"                                 \
         "ld $4, 72(%1)\n\t"                                      \
         "sd $4, 0($29)\n\t"                                      \
         "ld $4, 80(%1)\n\t"                                      \
         "sd $4, 8($29)\n\t"                                      \
         "ld $4, 88(%1)\n\t"                                      \
         "sd $4, 16($29)\n\t"                                     \
         "ld $4, 8(%1)\n\t"                                       \
         "ld $5, 16(%1)\n\t"                                      \
         "ld $6, 24(%1)\n\t"                                      \
         "ld $7, 32(%1)\n\t"                                      \
         "ld $8, 40(%1)\n\t"                                      \
         "ld $9, 48(%1)\n\t"                                      \
         "ld $10, 56(%1)\n\t"                                     \
         "ld $11, 64(%1)\n\t"                                     \
         "ld $25, 0(%1)\n\t"                    \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "daddu $29, $29, 24\n\t"                                 \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#define CALL_FN_W_12W(lval, orig, arg1,arg2,arg3,arg4,arg5,       \
                                  arg6,arg7,arg8,arg9,arg10,      \
                                  arg11,arg12)                    \
   do {                                                           \
      volatile OrigFn        _orig = (orig);                      \
      volatile unsigned long _argvec[13];                         \
      volatile unsigned long _res;                                \
      _argvec[0] = (unsigned long)_orig.nraddr;                   \
      _argvec[1] = (unsigned long)(arg1);                         \
      _argvec[2] = (unsigned long)(arg2);                         \
      _argvec[3] = (unsigned long)(arg3);                         \
      _argvec[4] = (unsigned long)(arg4);                         \
      _argvec[5] = (unsigned long)(arg5);                         \
      _argvec[6] = (unsigned long)(arg6);                         \
      _argvec[7] = (unsigned long)(arg7);                         \
      _argvec[8] = (unsigned long)(arg8);                         \
      _argvec[9] = (unsigned long)(arg9);                         \
      _argvec[10] = (unsigned long)(arg10);                       \
      _argvec[11] = (unsigned long)(arg11);                       \
      _argvec[12] = (unsigned long)(arg12);                       \
      __asm__ volatile(                                           \
         "dsubu $29, $29, 32\n\t"                                 \
         "ld $4, 72(%1)\n\t"                                      \
         "sd $4, 0($29)\n\t"                                      \
         "ld $4, 80(%1)\n\t"                                      \
         "sd $4, 8($29)\n\t"                                      \
         "ld $4, 88(%1)\n\t"                                      \
         "sd $4, 16($29)\n\t"                                     \
         "ld $4, 96(%1)\n\t"                                      \
         "sd $4, 24($29)\n\t"                                     \
         "ld $4, 8(%1)\n\t"                                       \
         "ld $5, 16(%1)\n\t"                                      \
         "ld $6, 24(%1)\n\t"                                      \
         "ld $7, 32(%1)\n\t"                                      \
         "ld $8, 40(%1)\n\t"                                      \
         "ld $9, 48(%1)\n\t"                                      \
         "ld $10, 56(%1)\n\t"                                     \
         "ld $11, 64(%1)\n\t"                                     \
         "ld $25, 0(%1)\n\t"                    \
         VALGRIND_CALL_NOREDIR_T9                                 \
         "daddu $29, $29, 32\n\t"                                 \
         "move %0, $2\n"                                          \
         :   "=r" (_res)                                  \
         :    "r" (&_argvec[0])                            \
         : "memory", __CALLER_SAVED_REGS                \
      );                                                          \
      lval = (__typeof__(lval)) _res;                             \
   } while (0)

#endif




#define VG_USERREQ_TOOL_BASE(a,b) \
   ((unsigned int)(((a)&0xff) << 24 | ((b)&0xff) << 16))
#define VG_IS_TOOL_USERREQ(a, b, v) \
   (VG_USERREQ_TOOL_BASE(a,b) == ((v) & 0xffff0000))

typedef
   enum { VG_USERREQ__RUNNING_ON_VALGRIND  = 0x1001,
          VG_USERREQ__DISCARD_TRANSLATIONS = 0x1002,

          VG_USERREQ__CLIENT_CALL0 = 0x1101,
          VG_USERREQ__CLIENT_CALL1 = 0x1102,
          VG_USERREQ__CLIENT_CALL2 = 0x1103,
          VG_USERREQ__CLIENT_CALL3 = 0x1104,

          VG_USERREQ__COUNT_ERRORS = 0x1201,

          VG_USERREQ__GDB_MONITOR_COMMAND = 0x1202,

          VG_USERREQ__MALLOCLIKE_BLOCK = 0x1301,
          VG_USERREQ__RESIZEINPLACE_BLOCK = 0x130b,
          VG_USERREQ__FREELIKE_BLOCK   = 0x1302,
          VG_USERREQ__CREATE_MEMPOOL   = 0x1303,
          VG_USERREQ__DESTROY_MEMPOOL  = 0x1304,
          VG_USERREQ__MEMPOOL_ALLOC    = 0x1305,
          VG_USERREQ__MEMPOOL_FREE     = 0x1306,
          VG_USERREQ__MEMPOOL_TRIM     = 0x1307,
          VG_USERREQ__MOVE_MEMPOOL     = 0x1308,
          VG_USERREQ__MEMPOOL_CHANGE   = 0x1309,
          VG_USERREQ__MEMPOOL_EXISTS   = 0x130a,

          VG_USERREQ__PRINTF           = 0x1401,
          VG_USERREQ__PRINTF_BACKTRACE = 0x1402,
          VG_USERREQ__PRINTF_VALIST_BY_REF = 0x1403,
          VG_USERREQ__PRINTF_BACKTRACE_VALIST_BY_REF = 0x1404,

          VG_USERREQ__STACK_REGISTER   = 0x1501,
          VG_USERREQ__STACK_DEREGISTER = 0x1502,
          VG_USERREQ__STACK_CHANGE     = 0x1503,

          VG_USERREQ__LOAD_PDB_DEBUGINFO = 0x1601,

          VG_USERREQ__MAP_IP_TO_SRCLOC = 0x1701,

          VG_USERREQ__CHANGE_ERR_DISABLEMENT = 0x1801,

          VG_USERREQ__VEX_INIT_FOR_IRI = 0x1901
   } Vg_ClientRequest;

#if !defined(__GNUC__)
#  define __extension__
#endif


#define RUNNING_ON_VALGRIND                                           \
    (unsigned)VALGRIND_DO_CLIENT_REQUEST_EXPR(0,         \
                                    VG_USERREQ__RUNNING_ON_VALGRIND,  \
                                    0, 0, 0, 0, 0)                    \


#define VALGRIND_DISCARD_TRANSLATIONS(_qzz_addr,_qzz_len)              \
    VALGRIND_DO_CLIENT_REQUEST_STMT(VG_USERREQ__DISCARD_TRANSLATIONS,  \
                                    _qzz_addr, _qzz_len, 0, 0, 0)



#if defined(__GNUC__) || defined(__INTEL_COMPILER) && !defined(_MSC_VER)
static int VALGRIND_PRINTF(const char *format, ...)
   __attribute__((format(__printf__, 1, 2), __unused__));
#endif
static int
#if defined(_MSC_VER)
__inline
#endif
VALGRIND_PRINTF(const char *format, ...)
{
#if defined(NVALGRIND)
   return 0;
#else
#if defined(_MSC_VER) || defined(__MINGW64__)
   uintptr_t _qzz_res;
#else
   unsigned long _qzz_res;
#endif
   va_list vargs;
   va_start(vargs, format);
#if defined(_MSC_VER) || defined(__MINGW64__)
   _qzz_res = VALGRIND_DO_CLIENT_REQUEST_EXPR(0,
                              VG_USERREQ__PRINTF_VALIST_BY_REF,
                              (uintptr_t)format,
                              (uintptr_t)&vargs,
                              0, 0, 0);
#else
   _qzz_res = VALGRIND_DO_CLIENT_REQUEST_EXPR(0,
                              VG_USERREQ__PRINTF_VALIST_BY_REF,
                              (unsigned long)format,
                              (unsigned long)&vargs,
                              0, 0, 0);
#endif
   va_end(vargs);
   return (int)_qzz_res;
#endif
}

#if defined(__GNUC__) || defined(__INTEL_COMPILER) && !defined(_MSC_VER)
static int VALGRIND_PRINTF_BACKTRACE(const char *format, ...)
   __attribute__((format(__printf__, 1, 2), __unused__));
#endif
static int
#if defined(_MSC_VER)
__inline
#endif
VALGRIND_PRINTF_BACKTRACE(const char *format, ...)
{
#if defined(NVALGRIND)
   return 0;
#else
#if defined(_MSC_VER) || defined(__MINGW64__)
   uintptr_t _qzz_res;
#else
   unsigned long _qzz_res;
#endif
   va_list vargs;
   va_start(vargs, format);
#if defined(_MSC_VER) || defined(__MINGW64__)
   _qzz_res = VALGRIND_DO_CLIENT_REQUEST_EXPR(0,
                              VG_USERREQ__PRINTF_BACKTRACE_VALIST_BY_REF,
                              (uintptr_t)format,
                              (uintptr_t)&vargs,
                              0, 0, 0);
#else
   _qzz_res = VALGRIND_DO_CLIENT_REQUEST_EXPR(0,
                              VG_USERREQ__PRINTF_BACKTRACE_VALIST_BY_REF,
                              (unsigned long)format,
                              (unsigned long)&vargs,
                              0, 0, 0);
#endif
   va_end(vargs);
   return (int)_qzz_res;
#endif
}


#define VALGRIND_NON_SIMD_CALL0(_qyy_fn)                          \
    VALGRIND_DO_CLIENT_REQUEST_EXPR(0,       \
                                    VG_USERREQ__CLIENT_CALL0,     \
                                    _qyy_fn,                      \
                                    0, 0, 0, 0)

#define VALGRIND_NON_SIMD_CALL1(_qyy_fn, _qyy_arg1)                    \
    VALGRIND_DO_CLIENT_REQUEST_EXPR(0,            \
                                    VG_USERREQ__CLIENT_CALL1,          \
                                    _qyy_fn,                           \
                                    _qyy_arg1, 0, 0, 0)

#define VALGRIND_NON_SIMD_CALL2(_qyy_fn, _qyy_arg1, _qyy_arg2)         \
    VALGRIND_DO_CLIENT_REQUEST_EXPR(0,            \
                                    VG_USERREQ__CLIENT_CALL2,          \
                                    _qyy_fn,                           \
                                    _qyy_arg1, _qyy_arg2, 0, 0)

#define VALGRIND_NON_SIMD_CALL3(_qyy_fn, _qyy_arg1, _qyy_arg2, _qyy_arg3) \
    VALGRIND_DO_CLIENT_REQUEST_EXPR(0,             \
                                    VG_USERREQ__CLIENT_CALL3,           \
                                    _qyy_fn,                            \
                                    _qyy_arg1, _qyy_arg2,               \
                                    _qyy_arg3, 0)


#define VALGRIND_COUNT_ERRORS                                     \
    (unsigned)VALGRIND_DO_CLIENT_REQUEST_EXPR(                    \
                               0,            \
                               VG_USERREQ__COUNT_ERRORS,          \
                               0, 0, 0, 0, 0)

#define VALGRIND_MALLOCLIKE_BLOCK(addr, sizeB, rzB, is_zeroed)          \
    VALGRIND_DO_CLIENT_REQUEST_STMT(VG_USERREQ__MALLOCLIKE_BLOCK,       \
                                    addr, sizeB, rzB, is_zeroed, 0)

#define VALGRIND_RESIZEINPLACE_BLOCK(addr, oldSizeB, newSizeB, rzB)     \
    VALGRIND_DO_CLIENT_REQUEST_STMT(VG_USERREQ__RESIZEINPLACE_BLOCK,    \
                                    addr, oldSizeB, newSizeB, rzB, 0)

#define VALGRIND_FREELIKE_BLOCK(addr, rzB)                              \
    VALGRIND_DO_CLIENT_REQUEST_STMT(VG_USERREQ__FREELIKE_BLOCK,         \
                                    addr, rzB, 0, 0, 0)

#define VALGRIND_CREATE_MEMPOOL(pool, rzB, is_zeroed)             \
    VALGRIND_DO_CLIENT_REQUEST_STMT(VG_USERREQ__CREATE_MEMPOOL,   \
                                    pool, rzB, is_zeroed, 0, 0)

#define VALGRIND_DESTROY_MEMPOOL(pool)                            \
    VALGRIND_DO_CLIENT_REQUEST_STMT(VG_USERREQ__DESTROY_MEMPOOL,  \
                                    pool, 0, 0, 0, 0)

#define VALGRIND_MEMPOOL_ALLOC(pool, addr, size)                  \
    VALGRIND_DO_CLIENT_REQUEST_STMT(VG_USERREQ__MEMPOOL_ALLOC,    \
                                    pool, addr, size, 0, 0)

#define VALGRIND_MEMPOOL_FREE(pool, addr)                         \
    VALGRIND_DO_CLIENT_REQUEST_STMT(VG_USERREQ__MEMPOOL_FREE,     \
                                    pool, addr, 0, 0, 0)

#define VALGRIND_MEMPOOL_TRIM(pool, addr, size)                   \
    VALGRIND_DO_CLIENT_REQUEST_STMT(VG_USERREQ__MEMPOOL_TRIM,     \
                                    pool, addr, size, 0, 0)

#define VALGRIND_MOVE_MEMPOOL(poolA, poolB)                       \
    VALGRIND_DO_CLIENT_REQUEST_STMT(VG_USERREQ__MOVE_MEMPOOL,     \
                                    poolA, poolB, 0, 0, 0)

#define VALGRIND_MEMPOOL_CHANGE(pool, addrA, addrB, size)         \
    VALGRIND_DO_CLIENT_REQUEST_STMT(VG_USERREQ__MEMPOOL_CHANGE,   \
                                    pool, addrA, addrB, size, 0)

#define VALGRIND_MEMPOOL_EXISTS(pool)                             \
    (unsigned)VALGRIND_DO_CLIENT_REQUEST_EXPR(0,                  \
                               VG_USERREQ__MEMPOOL_EXISTS,        \
                               pool, 0, 0, 0, 0)

#define VALGRIND_STACK_REGISTER(start, end)                       \
    (unsigned)VALGRIND_DO_CLIENT_REQUEST_EXPR(0,                  \
                               VG_USERREQ__STACK_REGISTER,        \
                               start, end, 0, 0, 0)

#define VALGRIND_STACK_DEREGISTER(id)                             \
    VALGRIND_DO_CLIENT_REQUEST_STMT(VG_USERREQ__STACK_DEREGISTER, \
                                    id, 0, 0, 0, 0)

#define VALGRIND_STACK_CHANGE(id, start, end)                     \
    VALGRIND_DO_CLIENT_REQUEST_STMT(VG_USERREQ__STACK_CHANGE,     \
                                    id, start, end, 0, 0)

#define VALGRIND_LOAD_PDB_DEBUGINFO(fd, ptr, total_size, delta)     \
    VALGRIND_DO_CLIENT_REQUEST_STMT(VG_USERREQ__LOAD_PDB_DEBUGINFO, \
                                    fd, ptr, total_size, delta, 0)

#define VALGRIND_MAP_IP_TO_SRCLOC(addr, buf64)                    \
    (unsigned)VALGRIND_DO_CLIENT_REQUEST_EXPR(0,                  \
                               VG_USERREQ__MAP_IP_TO_SRCLOC,      \
                               addr, buf64, 0, 0, 0)

#define VALGRIND_DISABLE_ERROR_REPORTING                                \
    VALGRIND_DO_CLIENT_REQUEST_STMT(VG_USERREQ__CHANGE_ERR_DISABLEMENT, \
                                    1, 0, 0, 0, 0)

#define VALGRIND_ENABLE_ERROR_REPORTING                                 \
    VALGRIND_DO_CLIENT_REQUEST_STMT(VG_USERREQ__CHANGE_ERR_DISABLEMENT, \
                                    -1, 0, 0, 0, 0)

#define VALGRIND_MONITOR_COMMAND(command)                               \
   VALGRIND_DO_CLIENT_REQUEST_EXPR(0, VG_USERREQ__GDB_MONITOR_COMMAND, \
                                   command, 0, 0, 0, 0)


#undef PLAT_x86_darwin
#undef PLAT_amd64_darwin
#undef PLAT_x86_win32
#undef PLAT_amd64_win64
#undef PLAT_x86_linux
#undef PLAT_amd64_linux
#undef PLAT_ppc32_linux
#undef PLAT_ppc64_linux
#undef PLAT_arm_linux
#undef PLAT_s390x_linux
#undef PLAT_mips32_linux
#undef PLAT_mips64_linux

#endif

// clang-format on
// NOLINTEND
