#!/bin/bash
# ToDo: Determine SIMD TYPES
case ${SIMD_TYPE} in
	sse2 | SSE2 )
		BASICOPTS+=(-msse)
		BASICOPTS+=(-msse2)
		;;
	sse3 | SSE3 )
		BASICOPTS+=(-msse)
		BASICOPTS+=(-msse2)
		BASICOPTS+=(-msse3)
		;;
	sse4 | SSE4 )
		BASICOPTS+=(-msse)
		BASICOPTS+=(-msse2)
		BASICOPTS+=(-msse3)
		BASICOPTS+=(-msse4)
		;;
	avx | AVX )
		BASICOPTS+=(-msse)
		BASICOPTS+=(-msse2)
		BASICOPTS+=(-msse3)
		BASICOPTS+=(-msse4)
		BASICOPTS+=(-mavx)
		;;
	avx2 | AVX2 )
		BASICOPTS+=(-msse)
		BASICOPTS+=(-msse2)
		BASICOPTS+=(-msse3)
		BASICOPTS+=(-msse4)
		BASICOPTS+=(-mavx)
		BASICOPTS+=(-mavx2)
		;;
	* )
		;;
esac
