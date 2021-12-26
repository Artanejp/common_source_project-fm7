#pragma once

// rgb color
#if !defined(_RGB555) && !defined(_RGB565) && !defined(_RGB888)
	#define _RGB888
#endif

#if defined(_RGB555) || defined(_RGB565)
	typedef uint16_t scrntype_t;
#elif defined(_RGB888)
	typedef uint32_t scrntype_t;
#endif
