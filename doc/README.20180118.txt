** UTF-8 **
This is test release for debugging FDC (MB8877).
This contains "Japanese Communication board"(日本語通信カード) emulation.
This also contains UART emulation.
Please check if you have ROM image.
Note:
 Some names of firmwares has changed.
 See fm7.txt and fm77av.txt .
Jan 18, 2018 K.Ohta <whatisthis.sowhat@gmail.com>

Updates:
commit 6be0565e6812d5d73d4c7b2b32a49a07ddbbd55c
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Thu Jan 18 08:13:30 2018 +0900

    [UI][Qt][FM7] Update translations.

commit 1599a0cfe883e73ea44addb609d9a7e571d89544
Author: K.Ohta <whatisthis.sowhat@gmail.com>
Date:   Thu Jan 18 07:57:14 2018 +0900

    [VM][MB8877] Make turning ON IRQ and turn OFF DRQ when seek completed.
    [VM][MB8877] Expect to work OS-9 L2 for FM77AV40 .
    [VM][MB8877] Adjust track parameter at initialize and CMD::RESTORE .

