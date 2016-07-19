cmd_/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/fc8080_bb.o := arm-cortex_a9-linux-gnueabi-gcc -Wp,-MD,/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/.fc8080_bb.o.d  -nostdinc -isystem /opt/crosstools/arm-cortex_a9-eabi-4.7-eglibc-2.18/lib/gcc/arm-cortex_a9-linux-gnueabi/4.7.4/include -I/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include -Iarch/arm/include/generated -Iinclude  -include /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-s5p4418/include -Iarch/arm/plat-s5p4418/include -I/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/plat-s5p4418/common -I/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/plat-s5p4418/iA1/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -O2 -marm -fno-dwarf2-cfi-asm -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -I/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/mach-s5p4418/prototype/base -I/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/mach-s5p4418/prototype/module -I/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/mach-s5p4418/prototype/module -D__LINUX__ -D__PRINTK__ -DNX_RELEASE -mtune=cortex-a9 -mfpu=vfp -mfloat-abi=softfp -I/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/plat-s5p4418/common -I/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/plat-s5p4418/iA1/include -Wframe-larger-than=1024 -fno-stack-protector -Wno-unused-but-set-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -DCC_HAVE_ASM_GOTO -I./include  -DMODULE  -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(fc8080_bb)"  -D"KBUILD_MODNAME=KBUILD_STR(fc8080_tpeg)" -c -o /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/fc8080_bb.o /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/fc8080_bb.c

source_/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/fc8080_bb.o := /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/fc8080_bb.c

deps_/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/fc8080_bb.o := \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/../include/fc8080.h \
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
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/../include/fci_types.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/../include/fci_ringbuffer.h \
  include/linux/spinlock.h \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/generic/lockbreak.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
  include/linux/typecheck.h \
  include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/preempt/count.h) \
    $(wildcard include/config/preempt/notifiers.h) \
  include/linux/thread_info.h \
    $(wildcard include/config/compat.h) \
  include/linux/bitops.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/bitops.h \
  include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/irqflags.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/ptrace.h \
    $(wildcard include/config/cpu/endian/be8.h) \
    $(wildcard include/config/arm/thumb.h) \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/hwcap.h \
  include/asm-generic/bitops/non-atomic.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/arch_hweight.h \
  include/asm-generic/bitops/const_hweight.h \
  include/asm-generic/bitops/lock.h \
  include/asm-generic/bitops/le.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/byteorder.h \
  include/linux/byteorder/little_endian.h \
  include/linux/swab.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/swab.h \
  include/linux/byteorder/generic.h \
  include/asm-generic/bitops/ext2-atomic-setbit.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/thread_info.h \
    $(wildcard include/config/arm/thumbee.h) \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/fpstate.h \
    $(wildcard include/config/vfpv3.h) \
    $(wildcard include/config/iwmmxt.h) \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/domain.h \
    $(wildcard include/config/io/36.h) \
    $(wildcard include/config/cpu/use/domains.h) \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/barrier.h \
    $(wildcard include/config/cpu/32v6k.h) \
    $(wildcard include/config/cpu/xsc3.h) \
    $(wildcard include/config/cpu/fa526.h) \
    $(wildcard include/config/arch/has/barriers.h) \
    $(wildcard include/config/arm/dma/mem/bufferable.h) \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/outercache.h \
    $(wildcard include/config/outer/cache/sync.h) \
    $(wildcard include/config/outer/cache.h) \
  include/linux/linkage.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/linkage.h \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/atomic/sleep.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/ring/buffer.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/compaction.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
  include/linux/sysinfo.h \
  /opt/crosstools/arm-cortex_a9-eabi-4.7-eglibc-2.18/lib/gcc/arm-cortex_a9-linux-gnueabi/4.7.4/include/stdarg.h \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/linux/printk.h \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
  include/linux/init.h \
    $(wildcard include/config/deferred/init/call.h) \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/hotplug.h) \
  include/linux/dynamic_debug.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/div64.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/compiler.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/thumb2/kernel.h) \
    $(wildcard include/config/debug/bugverbose.h) \
    $(wildcard include/config/arm/lpae.h) \
  include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
  include/linux/stringify.h \
  include/linux/bottom_half.h \
  include/linux/spinlock_types.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/spinlock_types.h \
  include/linux/lockdep.h \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/lock/stat.h) \
    $(wildcard include/config/prove/rcu.h) \
  include/linux/rwlock_types.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/spinlock.h \
    $(wildcard include/config/arch/s5p6818.h) \
    $(wildcard include/config/arch/s5p6818/rev.h) \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/processor.h \
    $(wildcard include/config/have/hw/breakpoint.h) \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/arm/errata/754327.h) \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/hw_breakpoint.h \
  include/linux/rwlock.h \
  include/linux/spinlock_api_smp.h \
    $(wildcard include/config/inline/spin/lock.h) \
    $(wildcard include/config/inline/spin/lock/bh.h) \
    $(wildcard include/config/inline/spin/lock/irq.h) \
    $(wildcard include/config/inline/spin/lock/irqsave.h) \
    $(wildcard include/config/inline/spin/trylock.h) \
    $(wildcard include/config/inline/spin/trylock/bh.h) \
    $(wildcard include/config/uninline/spin/unlock.h) \
    $(wildcard include/config/inline/spin/unlock/bh.h) \
    $(wildcard include/config/inline/spin/unlock/irq.h) \
    $(wildcard include/config/inline/spin/unlock/irqrestore.h) \
  include/linux/rwlock_api_smp.h \
    $(wildcard include/config/inline/read/lock.h) \
    $(wildcard include/config/inline/write/lock.h) \
    $(wildcard include/config/inline/read/lock/bh.h) \
    $(wildcard include/config/inline/write/lock/bh.h) \
    $(wildcard include/config/inline/read/lock/irq.h) \
    $(wildcard include/config/inline/write/lock/irq.h) \
    $(wildcard include/config/inline/read/lock/irqsave.h) \
    $(wildcard include/config/inline/write/lock/irqsave.h) \
    $(wildcard include/config/inline/read/trylock.h) \
    $(wildcard include/config/inline/write/trylock.h) \
    $(wildcard include/config/inline/read/unlock.h) \
    $(wildcard include/config/inline/write/unlock.h) \
    $(wildcard include/config/inline/read/unlock/bh.h) \
    $(wildcard include/config/inline/write/unlock/bh.h) \
    $(wildcard include/config/inline/read/unlock/irq.h) \
    $(wildcard include/config/inline/write/unlock/irq.h) \
    $(wildcard include/config/inline/read/unlock/irqrestore.h) \
    $(wildcard include/config/inline/write/unlock/irqrestore.h) \
  include/linux/atomic.h \
    $(wildcard include/config/arch/has/atomic/or.h) \
    $(wildcard include/config/generic/atomic64.h) \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/atomic.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/cmpxchg.h \
    $(wildcard include/config/cpu/sa1100.h) \
    $(wildcard include/config/cpu/sa110.h) \
    $(wildcard include/config/cpu/v6.h) \
  include/asm-generic/cmpxchg-local.h \
  include/asm-generic/atomic-long.h \
  include/linux/wait.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/kernel/kernel-3.4.39/arch/arm/include/asm/current.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/../include/fci_types.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/../include/fci_oal.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/../include/fci_hal.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/../include/fci_tun.h \
  /home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/../include/fc8080_regs.h \

/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/fc8080_bb.o: $(deps_/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/fc8080_bb.o)

$(deps_/home/appsw4/work/Customers/IA-INC/iA1/linux-IAHD/platform/s5p4418/modules/fc8080_tpeg/src/fc8080_bb.o):
