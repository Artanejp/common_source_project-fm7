DUMP LIST CHECKER (and EXTRACTOR)
=================================

        Oct 15, 2019 Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	   
## About:

 This is checking and decoding "Dump list" of 80's (or earlier) computer magazines.
 
 Now, don't generate binary data from dump list, cheking only yet.

## Building:

    cc -o dumpcheck dumplist.c
	
## Synopsis:

    ./dumpcheck -i input_file [--checkonly]
	
*  "input file" is plain text (see format).
*  --checkonly : Don't generate binary file

## Dumplist format:

 This should plain text with below format:

> [ADDR+0x00] xx yy (repeat 16 times) :[XSUM0]
> [ADDR+0x10] (Same format)
>            (repeat 16 times)
> [ADDR+0xf0] ...                     :[XSUMF]
> Sum:        [YSUM0] ... [YSUMF]    :[TOTAL SUM]
 
 ADDR is hexadecimal, not decimal.

>      [XSUMn] = ([Value of +0xn0] + [Value of +0xn1] +...+ [Value 0f +0xnF]) & 0xFF .
>      [YSUMm] = ([Value of +0x0m] + [Value of +0x1m] +...+ [Value of +0xFm]) & 0xFF .
>  [TOTAL SUM] = ([VAlue of +0x00] + [Value of +0x01] +...+ [Value of +0xFF]) & 0xFF.
  
 for example:
> 1000 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F :78 
> 1010 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F :78 
> ...
> 10F0 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F :78 
> Sum: 00 10 20 30 40 50 60 70 80 90 A0 B0 C0 D0 E0 F0 :80

### Have fun!!
Ohta.
