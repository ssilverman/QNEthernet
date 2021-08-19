// Provide system function implementations.
// (c) 2021 Shawn Silverman

#include <stdint.h>
#include <lwip/arch.h>

extern volatile uint32_t systick_millis_count;

u32_t sys_now(void) {
  return systick_millis_count;
}

#if SYS_LIGHTWEIGHT_PROT
sys_prot_t sys_arch_protect(void) {
  return 0;
}

void sys_arch_unprotect(sys_prot_t pval) {
}
#endif  // SYS_LIGHTWEIGHT_PROT
