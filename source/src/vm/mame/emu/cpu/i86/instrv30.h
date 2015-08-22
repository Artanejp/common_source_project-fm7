static void PREFIXV30(_0fpre)(i8086_state *cpustate);
static void PREFIXV30(_repnc)(i8086_state *cpustate);
static void PREFIXV30(_repc)(i8086_state *cpustate);
static void PREFIXV30(_aad)(i8086_state *cpustate);
static void PREFIXV30(_setalc)(i8086_state *cpustate);
#if 0
static void PREFIXV30(_brks)(i8086_state *cpustate);
#endif

/* changed instructions */
static void PREFIX(_pop_ss)(i8086_state *cpustate);
static void PREFIX(_es)(i8086_state *cpustate);
static void PREFIX(_cs)(i8086_state *cpustate);
static void PREFIX(_ss)(i8086_state *cpustate);
static void PREFIX(_ds)(i8086_state *cpustate);
static void PREFIX(_mov_sregw)(i8086_state *cpustate);
static void PREFIXV30(_repne)(i8086_state *cpustate);
static void PREFIXV30(_repe)(i8086_state *cpustate);
static void PREFIXV30(_sti)(i8086_state *cpustate);
