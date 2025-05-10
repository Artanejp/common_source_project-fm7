/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2025.05.10 -

	[FM-Towns CD-ROM based on SCSI CDROM; Definitions of CD-ROM Sector / SUBQ structures.]
	History:
	  2025-05-10 K.Ohta : Split from vm/fmtowns/cdrom.h .
*/
#pragma once

#include <cstdint>
/*!
 * @brief Belows are CD-ROM sector structuer.
 * @note See https://en.wikipedia.org/wiki/CD-ROM#Sector_structure .
 */

#pragma pack(push, 1)
/*!
 * @brief Structure of CD-ROM header. (by "RED BOOK").
 */
typedef struct {
	uint8_t sync[12];
	uint8_t addr_m;
	uint8_t addr_s;
	uint8_t addr_f;
	uint8_t sector_type; //! 1 = MODE1, 2=MODE2
} cd_data_head_t;
#pragma pack(pop)

#pragma pack(push, 1)
/*!
 * @brief Sector structure of CDROM MODE1/DATA.
 *
 * @note ToDo: Still not implement crc32 and ecc.
 * @note 20201116 K.O
 */
typedef struct {
	cd_data_head_t header;
	uint8_t data[2048];
	uint8_t crc32[4]; //! CRC32 checksum.
	uint8_t reserved[8];
	uint8_t ecc[276]; //! ERROR CORRECTIOM DATA; by read solomon code.
} cd_data_mode1_t;
#pragma pack(pop)

#pragma pack(push, 1)
/*!
 * @brief Sector structure of CDROM MODE2/DATA.
 */
typedef struct {
	cd_data_head_t header;
	uint8_t data[2336];
} cd_data_mode2_t;
#pragma pack(pop)

/*!
 * @brief Sector structure of CD AUDIO.
 */
#pragma pack(push, 1)
typedef struct {
	uint8_t data[2352];
} cd_audio_sector_t;
#pragma pack(pop)

#pragma pack(push, 1)
/*!
 * @brief Sector ISO/2048 virtual image.
 *
 * @note ToDo: Add fake header and crc and ecc.
 * @note 20201116 K.O
 */
typedef struct {
	uint8_t data[2048];
} cd_data_iso_t;
#pragma pack(pop)

#pragma pack(push, 1)
/*!
 * @brief Buffer definition of CD-ROM sector buffer (excepts ISO/2048).
 *
 * @note 20201116 K.O
 */
typedef union cdimage_buffer_s {
	uint8_t rawdata[2352]; //!< @note OK?
	cd_data_mode1_t mode1;
	cd_data_mode2_t mode2;
	cd_audio_sector_t audio;
} cdimage_buffer_t;
#pragma pack(pop)


#pragma pack(push, 1)
/*!
 * @brief Union definition of SUBQ bitslice.
 *
 * @note 20201116 K.O
 */
typedef union SUBC_u {
	struct {
		uint8_t P:1;
		uint8_t Q:1;
		uint8_t R:1;
		uint8_t S:1;
		uint8_t T:1;
		uint8_t U:1;
		uint8_t V:1;
		uint8_t W:1;
	} bit;
	uint8_t byte;
} SUBC_t;
#pragma pack(pop)

