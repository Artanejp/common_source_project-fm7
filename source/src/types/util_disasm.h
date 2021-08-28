#pragma once

// for disassedmbler
uint32_t DLL_PREFIX get_relative_address_8bit(uint32_t base, uint32_t mask, int8_t offset);
uint32_t DLL_PREFIX get_relative_address_16bit(uint32_t base, uint32_t mask, int16_t offset);
uint32_t DLL_PREFIX get_relative_address_32bit(uint32_t base, uint32_t mask, int32_t offset);

// symbol
typedef struct symbol_s {
	uint32_t addr;
	_TCHAR *name;
	struct symbol_s *next_symbol;
} symbol_t;

const _TCHAR* DLL_PREFIX get_symbol(symbol_t *first_symbol, uint32_t addr);
const _TCHAR* DLL_PREFIX get_value_or_symbol(symbol_t *first_symbol, const _TCHAR *format, uint32_t addr);
const _TCHAR* DLL_PREFIX get_value_and_symbol(symbol_t *first_symbol, const _TCHAR *format, uint32_t addr);
