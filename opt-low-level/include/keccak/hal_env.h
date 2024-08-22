#ifndef QEMU_V8A_HAL_ENV_H
#define QEMU_V8A_HAL_ENV_H

#ifdef __linux__

#define SEP ;

#define ASM_LOAD(dst,symbol) 	\
  adrp dst, symbol ; add  dst, dst, :lo12:symbol;
  
#endif

#ifdef __APPLE__

#define SEP %%

#define ASM_LOAD(dst,symbol) 	\
  adrp dst, symbol@PAGE  %% add dst, dst, symbol@PAGEOFF
  
#endif

#endif /* QEMU_V8A_HAL_ENV_H */