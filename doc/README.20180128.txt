** UTF-8 **
This is test release for debugging FDC (MB8877).
This contains "Japanese Communication board"(日本語通信カード) emulation.
This also contains UART emulation.
Please check if you have ROM image.
Note:
 Some names of firmwares has changed.
 See fm7.txt and fm77av.txt .
Jan 28, 2018 K.Ohta <whatisthis.sowhat@gmail.com>

Updates:
commit 5e924c597da524a69d021541327c26dd0f178822
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Sun Jan 28 15:50:19 2018 +0900

    [VM][FM7][MB8877] TRY: Boot from FLEX for FM-7.
    [VM][MB8877] Fix not read status data at the endof SEEK and verifying.
    [VM][MB8877] Update structure of MB8877->fdc.
    [VM][DISK][FM7] Add hack number for FLEX.

commit 72293436c24dc0042d16d1ee9f07108c6538a972
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Sat Jan 27 03:36:54 2018 +0900

    [VM][FM77AV][DISPLAY] Separate event process around VSYNC/HSYNC.

commit 246aab1205a413f2443e4c671dc0522c173282f7
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Sat Jan 27 03:33:09 2018 +0900

    [VM][MC6809] Fix around CWAI with interrupts.
    [VM][MC6809] Make set/reset E flag at interrupt.
    [VM][MC6809] Fix bus timing.

commit c254f3e36be9e2e319d1c641a87c103f978d1a6e
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Fri Jan 26 22:01:02 2018 +0900

    [UI][Qt] BUG: Fix crash with BOOT_MODEs >= 8.

commit a8e805789751e26d5d8c427b8e5c6c7eb05a39d6
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Fri Jan 26 22:00:38 2018 +0900

    [UI][Qt][FM7] Extend boot menu.

commit 94a41282c2fea4ffd6b2b0a9193e1013d74e15f1
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Fri Jan 26 21:59:19 2018 +0900

    [VM][FM77] Fix access around BOOT-RAM.Thanks to Ryu Takegami.
    [VM][FM7] Read some (bulk) roms for FM-8/7/NEW7/77.

commit e12c4e9fceab18261f3057552e4e00c7db7176c0
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Fri Jan 26 18:35:03 2018 +0900

    [VM][FM7] .

commit 2e24a0d0d481471132346e280fd26ed6320de459
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Fri Jan 26 18:19:27 2018 +0900

    [VM][MC6809] More accurate emulation(maybe).

commit 6ca41420764ac3348be134eff01f749030d59508
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Fri Jan 26 09:13:28 2018 +0900

    [VM][MC6809] Stop to split int-sequence temporally, temporally fixed RIGLAS and helicoid (for FM-7).

commit a98fe8a02aeb95e28afe5a05c7f7a0ad64dad514
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Fri Jan 26 07:43:29 2018 +0900

    [VM][FM7] Temporally change.

commit 03503d799f9a4db53b4fa432efff31a308c11d12
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Fri Jan 26 03:51:52 2018 +0900

    [VM][FM77] Add initial support of 2HD (for FM-77).

commit 6b453c704f4c14cf53a7cc8550fc86a3dc822663
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Fri Jan 26 03:50:59 2018 +0900

    [EMU][FM7] emu.cpp : No longer use hack with some VMs.

commit 42f1370919757ee87b5253288db022a7f387924b
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Fri Jan 26 01:29:46 2018 +0900

    [Qt][OpenGL] .

commit 20cc4db343de197ee0946828742a893e5c1dc06c
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Fri Jan 26 01:28:20 2018 +0900

    [VM][FM7] Remove DUMMY CPU.Reduce host-cpu usage a lot.Related to commit a4e1a7cfef59e08f31675c6608871068d3f2c4ef.

commit a4e1a7cfef59e08f31675c6608871068d3f2c4ef
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Fri Jan 26 01:15:53 2018 +0900

    [VM][MC6809] Fix clock count with some situation, FM-8/7/77/AV don't need dummycpu.

commit cd5ebcec8bfd734e78e232c7dba0a43a13278d00
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Thu Jan 25 19:04:16 2018 +0900

    [VM][FM7][KEYBOARD] Keep BREAK key when special reset (hot start).
    [VM][FM77][FM77AV40][FM77AV40EX] Fix FTBFSs.

commit ebc1cce5f14287ce6a82b34a3f9ed53f75133b45
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Thu Jan 25 17:21:49 2018 +0900

    [VM][FM8] Add fm8_mainio.h .

commit ab31f7b109c5b80ea08284fe3c4807e839b690c9
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Thu Jan 25 17:19:36 2018 +0900

    [VM][FM8][MAINIO] Move FM-8 specified features to FM8_MAINIO:: .
    [VM][FM7][MAINIO][SOUND] Reduce hitting PCM1BIT:: when not sound buzzer.

commit 9d4fbabefff5799c837489934d4c414c0ab64705
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Thu Jan 25 05:13:17 2018 +0900

    [VM][FM77][WIP] Adding 2HD FDD.

commit 369e922aa9ee31989b8121fe26013104bd447670
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Fri Jan 19 12:15:15 2018 +0900

    [Draw][OpenGL][FM77AV] Fix blinking with some situation.

