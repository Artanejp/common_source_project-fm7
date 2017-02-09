/****************************************************************************
*             real mode i286 emulator v1.4 by Fabrice Frances               *
*               (initial work based on David Hedley's pcemu)                *
****************************************************************************/

// file will be included in all cpu variants
// timing value should move to separate array

//void IX86_OPS::PREFIX186(_pusha)();
//void IX86_OPS::PREFIX186(_popa)();
//void IX86_OPS::PREFIX186(_bound)();
//void IX86_OPS::PREFIX186(_push_d16)();
//void IX86_OPS::PREFIX186(_imul_d16)();
//void IX86_OPS::PREFIX186(_push_d8)();
//void IX86_OPS::PREFIX186(_imul_d8)();
//void IX86_OPS::PREFIX186(_rotshft_bd8)();
//void IX86_OPS::PREFIX186(_rotshft_wd8)();
//void IX86_OPS::PREFIX186(_enter)();
//void IX86_OPS::PREFIX186(_leave)();
//void IX86_OPS::PREFIX186(_insb)();
//void IX86_OPS::PREFIX186(_insw)();
//void IX86_OPS::PREFIX186(_outsb)();
//void IX86_OPS::PREFIX186(_outsw)();

/* changed instructions */
//void IX86_OPS::PREFIX(_pop_ss)();
//void IX86_OPS::PREFIX(_es)();
//void IX86_OPS::PREFIX(_cs)();
//void IX86_OPS::PREFIX(_ss)();
//void IX86_OPS::PREFIX(_ds)();
//void IX86_OPS::PREFIX(_mov_sregw)();
//void IX86_OPS::PREFIX186(_repne)();
//void IX86_OPS::PREFIX186(_repe)();
//void IX86_OPS::PREFIX186(_sti)();
//void IX86_OPS::PREFIX186(_rotshft_bcl)();
//void IX86_OPS::PREFIX186(_rotshft_wcl)();
