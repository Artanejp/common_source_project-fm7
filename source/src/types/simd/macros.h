#pragma once


#define SIMD_MAKE_USE(name) SIMD_USE_##name
#define SIMD_MAKE_BACKUP(name) __SIMD_BACKUP_INSN_##name

#define SIMD_SET_INSN(name) {		 \
		#define USE_SIMD_##name 1	 \
	}

#define SIMD_CLEAR_INSN(name) {		 \
		#undef USE_SIMD_##name \
	}

#define SIMD_SET_BACKUP(name) { \
		#undef __SIMD_BACKUP_INSN_##name \
	}

#define SIMD_CLEAR_BACKUP(name) {		 \
		#undef __SIMD_BACKUP_INSN_##name \
	}

#define SIMD_BACKUP_INSN(name) {										\
		#if defined(SIMD_MAKE_USE(name)) && !defined(SIMD_MAKE_BACKUP(name)) \
		SIMD_SET_BACKUP(name)											\
		#endif															\
	}

#define SIMD_RESTORE_INSN(name) {										\
		#if !defined(SIMD_MAKE_USE(name)) && defined(SIMD_MAKE_BACKUP(name)) \
		SIMD_SET_INSN(name)												\
		#endif															\
	}


#define SIMD_PUSH_ARCH(name) {					\
		#if defined(SIMD_MAKE_USE(name))		\
		SIMD_BACKUP_INSN(name)					\
		#endif									\
	}

#define SIMD_POP_ARCH(name) {					\
		#if defined(SIMD_MAKE_BACKUP(name))		\
		SIMD_RESTORE_INSN(name)					\
		SIMD_CLEAR_BACKUP(name)					\
		#endif									\
	}

#define SIMD_PUSH_ALL_INSN { \
		SIMD_PUSH_ARCH(X86) \
		SIMD_PUSH_ARCH(ARM_NEON) \
		SIMD_PUSH_ARCH(ARM_SVE) \
		SIMD_PUSH_ARCH(MIPS) \
		SIMD_PUSH_ARCH(WASM) \
	}

#define SIMD_POP_ALL_INSN { \
		SIMD_PUSH_ARCH(X86) \
		SIMD_PUSH_ARCH(ARM_NEON) \
		SIMD_PUSH_ARCH(ARM_SVE) \
		SIMD_PUSH_ARCH(MIPS) \
		SIMD_PUSH_ARCH(WASM) \
	}

#define SIMD_CLEAR_ALL_INSN { \
		SIMD_CLEAR_INSN(X86) \
		SIMD_CLEAR_INSN(ARM_NEON) \
		SIMD_CLEAR_INSN(ARM_SVE) \
		SIMD_CLEAR_INSN(MIPS) \
		SIMD_CLEAR_INSN(WASM) \
	}

