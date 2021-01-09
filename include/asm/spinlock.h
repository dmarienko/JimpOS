#ifndef _SPINLOCK_H
#define _SPINLOCK_H

typedef struct _spinlock_t { uint32_t a; } spinlock_t;

#define __dummy_lock(lock) (*(spinlock_t*)(lock))
#define spin_lock_string  \
        "\n1:\t"          \
        "btsl $0,%0\n\t"  \
        "jnc 3f\n"        \
        "2:\t"            \
        "testb $1,%0\n\t" \
        "jne 2b\n\t"      \
        "jmp 1b\n"        \
        "3:\t"

#define spin_lock(lock)\
        __asm__ __volatile__(spin_lock_string :"=m" (__dummy_lock(lock)))

#define spin_unlock(lock)                               \
        __asm__ __volatile__("btrl $0,%0" :"=m" (__dummy_lock(lock)))

#endif
