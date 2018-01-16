** UTF-8 **
This is test release for debugging DMA and FDC (MB8877).
This contains "Japanese Communication board"(日本語通信カード) emulation.
This also contains UART emulation.
Please check if you have ROM image.
Note:
 Some names of firmwares has changed.
 See fm7.txt and fm77av.txt .
Jan 17, 2018 K.Ohta <whatisthis.sowhat@gmail.com>

Updates:
commit 2350969f27f089a9a3df39f8fb456cad4adeda2d
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Wed Jan 17 05:09:43 2018 +0900

    [UI][Qt][FM7] Update UIs, revision.

commit 24f54c39da49dabf7bec34946c4fe46fe3550ee8
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Wed Jan 17 05:07:18 2018 +0900

    [VM][MB8877] Fix DRQ/IRQ timing.
    [VM][MB8877] Make IRQ with disk not inserted or not connected when processing command(s).
    [VM][MB8877] Expect to work OS-9 L2 for FM77AV40.

commit 82b9e9120514dafd785803ebf8c4494d8ea8d397
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Wed Jan 17 02:58:32 2018 +0900

    [VM][FM7][JCOMMCARD] JCOMMCARD has no backup battery.Not save ram.Thanks to Ryu Takegami.

commit ae5cabae3c0d98ec9f9516b784cc95faf4500e88
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Wed Jan 17 02:55:07 2018 +0900

    [VM][FM7][JCOMMCARD] Change name of dictionary rom to "DICROM.ROM".This rom is same as FM77-211.

commit d4dffe4e97eb51313a24761f7e8add9c1f1dd5dd
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Wed Jan 17 02:45:21 2018 +0900

    [VM][FM77AV40][HD6844] HD6844 has only a one interrupt line, not multiple.
    [VM][FM77AV40][HD6844] Rename drq_line to busreq_line.
    [VM][FM77AV40EX][HD6844] FM77AV40EX has only one DMA channel.Not multiple channels.
    [VM][HD6844] Add "__USE_MULTIPLE_CHAINING" and "__USE_CHAINING" flags for FM77AV40/EX/SX series.
    [VM][HD6844] Add special flags for FM77AV40/EX/SX.
    [VM][FM77AV40][HD6844] Expect to complete booting OS-9 L2 for FM77AV40.

commit 56b317e5727a3fbcf4d73e3871e2d16c4bc45020
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Wed Jan 17 02:42:53 2018 +0900

    [VM][FM7][MB8877] Not clear status when force-interrupt with TYPE-1 Command(seek etc).Fix (Add hack) for FUKU*.d77 (えびふく's music disks).


