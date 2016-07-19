cmd_/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/fc8080_ppi.o := arm-cortex_a9-linux-gnueabi-gcc -Wp,-MD,/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/.fc8080_ppi.o.d  -nostdinc -isystem /opt/crosstools/arm-cortex_a9-eabi-4.7-eglibc-2.18/lib/gcc/arm-cortex_a9-linux-gnueabi/4.7.4/include -I/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include -Iarch/arm/include/generated -Iinclude  -include /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-s5p4418/include -Iarch/arm/plat-s5p4418/include -I/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/plat-s5p4418/common -I/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/plat-s5p4418/iA1/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -O2 -marm -fno-dwarf2-cfi-asm -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -I/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/mach-s5p4418/prototype/base -I/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/mach-s5p4418/prototype/module -I/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/mach-s5p4418/prototype/module -D__LINUX__ -D__PRINTK__ -DNX_RELEASE -mtune=cortex-a9 -mfpu=vfp -mfloat-abi=softfp -I/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/plat-s5p4418/common -I/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/plat-s5p4418/iA1/include -Wframe-larger-than=1024 -fno-stack-protector -Wno-unused-but-set-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -DCC_HAVE_ASM_GOTO -I./include  -DMODULE  -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(fc8080_ppi)"  -D"KBUILD_MODNAME=KBUILD_STR(fc8080_tpeg)" -c -o /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/fc8080_ppi.o /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/fc8080_ppi.c

source_/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/fc8080_ppi.o := /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/fc8080_ppi.c

deps_/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/fc8080_ppi.o := \
  include/linux/mutex.h \
    $(wildcard include/config/debug/mutexes.h) \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
    $(wildcard include/config/have/arch/mutex/cpu/relax.h) \
  include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/arch/dma/addr/t/64bit.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  arch/arm/include/generated/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/linux/posix_types.h \
  include/linux/stddef.h \
  include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/posix_types.h \
  include/asm-generic/posix_types.h \
  include/linux/poison.h \
    $(wildcard include/config/illegal/pointer/value.h) \
  include/linux/const.h \
  include/linux/spinlock_types.h \
    $(wildcard include/config/generic/lockbreak.h) \
    $(wildcard include/config/debug/spinlock.h) \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/spinlock_types.h \
  include/linux/lockdep.h \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/lock/stat.h) \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/prove/rcu.h) \
  include/linux/rwlock_types.h \
  include/linux/linkage.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/linkage.h \
  include/linux/atomic.h \
    $(wildcard include/config/arch/has/atomic/or.h) \
    $(wildcard include/config/generic/atomic64.h) \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/atomic.h \
  include/linux/irqflags.h \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
  include/linux/typecheck.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/irqflags.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/ptrace.h \
    $(wildcard include/config/cpu/endian/be8.h) \
    $(wildcard include/config/arm/thumb.h) \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/hwcap.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/barrier.h \
    $(wildcard include/config/cpu/32v6k.h) \
    $(wildcard include/config/cpu/xsc3.h) \
    $(wildcard include/config/cpu/fa526.h) \
    $(wildcard include/config/arch/has/barriers.h) \
    $(wildcard include/config/arm/dma/mem/bufferable.h) \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/outercache.h \
    $(wildcard include/config/outer/cache/sync.h) \
    $(wildcard include/config/outer/cache.h) \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/cmpxchg.h \
    $(wildcard include/config/cpu/sa1100.h) \
    $(wildcard include/config/cpu/sa110.h) \
    $(wildcard include/config/cpu/v6.h) \
  include/asm-generic/cmpxchg-local.h \
  include/asm-generic/atomic-long.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/../include/fci_types.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/../include/fc8080_regs.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/../include/fci_oal.h \

/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/fc8080_ppi.o: $(deps_/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/fc8080_ppi.o)

$(deps_/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/fc8080_ppi.o):
