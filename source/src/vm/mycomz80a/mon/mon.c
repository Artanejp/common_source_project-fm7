/*
	MYCOMZ-80A MONITOR

	Author : Takeda.Toshiya
	Date   : 2009.05.01-

	WORK:
		0BFE0H		END OF STACK
		0BFE0H		KEY SHIFT
		0BFE1H		KEY GRAPH
		0BFE2H		KEY CLEAR
		0BFE3H		KEY BREAK
		0BFE4H		TEXT MODE
		0BFE8H-0BFEBH	TEMP
		0BFEEH		CURSOR X
		0BFEFH		CURSOR Y
		0BFF0H-0BFFFH	CRTC BUFFER
*/

/* init */

init()
{
#asm
	ORG	0C000H
	JMP	INIT1
	; 0C003H
	JMP	putchr
	; 0C006H
	JMP	inkey
	; 0C009H
	JMP	cnvkey
	; 0C00CH
	JMP	cursor
	; 0C00FH
	JMP	plot
	; 0C012H
	JMP	line
	; 0C015H
	JMP	clrscrn
	; 0C018H
	JMP	txtmode
	; 0C01BH
	JMP	cgmode
	; 0C01EH
	JMP	getvram
	; 0C021H
	JMP	setvram
	; 0C024H
	JMP	vramadr
	; 0C027H
	JMP	setpsg
	; 0C02AH
	JMP	getrtc
	; 0C02DH
	JMP	setrtc
INIT1:
	DI
	LXI	SP,0BFE0H
	; RESET ADDRESS MASK
	MVI	A,01H
	OUT	00H
	; COPY ROM TO RAM
	LXI	D,0C000H
INIT2:
	LDAX	D
	STAX	D
	INX	D
	MOV	A,D
	ANI	0FFH
	JNZ	INIT2
	; SWITCH ROM TO RAM
	MVI	A,03H
	OUT	00H
	; INIT PIO
	MVI	A,8AH
	OUT	07H
	MVI	A,90H
	OUT	0BH
	MVI	A,92H
	OUT	0FH
	; INIT RTC TO 24H
	LXI	D,0005H
	PUSH	D
	LXI	D,0008H
	PUSH	D
	CALL	setrtc
	POP	D
	POP	D
	; INIT SCREEN
	LXI	D,0004H
	PUSH	D
	CALL	txtmode
	POP	D
	JMP	mon
	; REMOVE THIS RET BEFORE RUN MAC.COM
#endasm
}

/*
	MON80 Version 2.1 - 8080 Monitor Program
	Language: Small-C + ASM on CP/M
	(C)1996-2006 Office TETSU
*/
mon()
{
	char *adrbgn, *adrend;
	char combuf[80], com[80], rdo[80], par0[80], par1[80], par2[80];

	adrbgn = 0;
	*combuf = *com = *rdo = *par0 = *par1 = *par2 = 0;

	putnwl();
	putstr("MON80 Version 2.1 MYCOMZ-80A Edition"); putnwl();
	putstr("Intel8080 Monitor Program"); putnwl();
	putstr("(C)1996-2006 Office TETSU"); putnwl();
	putnwl();

	while(1){
	asmpmt(adrbgn);
	getstr(combuf, 73);
	toustr(combuf);
	setcmd(combuf, com, par0);
	setpar(par0, par1, par2);
	if(*combuf != 0) strcpy(rdo, com);

	if(strcmp(rdo, "LIST") == 0){
		if(isxstr(par1)) adrbgn = xtoi(par1);
		if(isxstr(par2)) adrend = xtoi(par2);
			else adrend = adrbgn + 16;
		adrbgn = discod(adrbgn, adrend);}
	else
	if(strcmp(rdo, "DUMP") == 0){
		if(isxstr(par1)) adrbgn = xtoi(par1);
		if(isxstr(par2)) adrend = xtoi(par2);
			else adrend = adrbgn;
		adrbgn = dmpcod(adrbgn, adrend);}
	else	if(*com == 0);
	else	if(strcmp(com, "GOSUB" ) == 0)
			if(isxstr(par1)) gosub(xtoi(par1));
		else{	putstr("ERROR-");putstr(par1);putnwl();}
	else	if(strcmp(com, "DEFINE") == 0)
		adrbgn = define(adrbgn, par0);
	else	if(strcmp(com, "BASIC"  ) == 0) gosub(0xf000);

	else if(strcmp(com,"TEST")==0){
		cgmode();
		line(0,0,159,99,1);
		while(getchr()!=10);
		txtmode(0);
	}
	else	if(isxstr(com)) adrbgn = xtoi(com);
	else	adrbgn = asmlin(adrbgn, com, par1, par2);
	}
}

discod(char *s, char *e)
{
	do{ s = dislin(s);} while(s <= e);
	return(s);
}

dislin(char *s)
{
	char mcod;
	int *w;

	putchr(' ');
	putwrd(s);
	putchr(' ');
	mcod = *s++;
	putstr(opecod(mcod));
		w = s;
		if(*operd1(mcod) == 1){putchr('\t'); putbyt(*s); s += 1;}
	else	if(*operd1(mcod) == 2){putchr('\t'); putwrd(*w); s += 2;}
	else	if(*operd1(mcod) != 0){putchr('\t'); putstr(operd1(mcod));}
	else	{putnwl(); return(s);}
		w = s;
		if(*operd2(mcod) == 1){putchr(','); putbyt(*s); s += 1;}
	else	if(*operd2(mcod) == 2){putchr(','); putwrd(*w); s += 2;}
	else	if(*operd2(mcod) != 0){putchr(','); putstr(operd2(mcod));}
	putnwl( );
	return(s);
}

dmpcod(char *s, char *e)
{
	putstr("      ");
	putstr("+0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F ");
	putstr("ASCII");
	putnwl();

	s = s & 0xfff0;
	do{ s = dmplin(s);} while((s - 1) < e);
	return(s);
}

dmplin(char *s)
{
	char buf[16];
	int i;

	putchr(' ');
	putwrd(s);
	putchr(' ');
	for(i = 0; i < 16; i++) buf[i] = *s++;
	for(i = 0; i < 16; i++){
		putbyt(buf[i]);
		putchr(' ');}
	for(i = 0; i < 16; i++){
		if(isprint(buf[i])) putchr(buf[i]);
		else putchr('.');}
	putnwl();
	return(s);
}

define(char *s, char *l)
{
	char buf[80], *t;

	while(*l){
	setpar(l, buf, l);
		t = buf;
		if(isxstr(t)) *s++ = xtob(t);
	else	if(*t == 39){t++; while(*t && (*t != 39)){*s++ = *t++;}}
	else{	putstr("Error-");
		putstr(buf);
		putnwl();
		break;}
	}
	return(s);
}

asmlin(char *s, char *cd, char *r1, char *r2)
{
	char i;
	int *w, ok;

	i = 255; ok = 0;
	while(i-- && (ok == 0)){
	if (strcmp(cd, opecod(i)) == 0)
	if((strcmp(r1, operd1(i)) == 0) || (isxstr(r1) && (*operd1(i) <= 2)))
	if((strcmp(r2, operd2(i)) == 0) || (isxstr(r2) && (*operd2(i) <= 2)))
	ok = 1;}

	if(ok == 0){
	putstr("ERROR-");
	putstr(cd); putchr(' ');
	if(*r1 != 0) putstr(r1);
	if(*r2 != 0){putchr(','); putstr(r2);}
	putnwl();
	return(s);
	}

		i++; *s++ = i; w = s;
		if(*operd1(i) == 1){*s = xtob(r1); s += 1;}
	else	if(*operd2(i) == 1){*s = xtob(r2); s += 1;}
	else	if(*operd1(i) == 2){*w = xtoi(r1); s += 2;}
	else	if(*operd2(i) == 2){*w = xtoi(r2); s += 2;}
	return(s);
}

asmpmt(char *p) {putchr('['); putwrd(p); putchr(']');}

setcmd(char *buf, char *com, char *par)
{
	while(*buf != 0 && (isspace(*buf) || *buf == ',')) buf++;
	while(*buf != 0 && !isspace(*buf) && *buf != ',') *com++ = *buf++;
	*com = 0;
	while(*buf != 0 && *buf == ' ') buf++;
	while(*buf != 0) *par++ = *buf++;
	*par = 0;
}

setpar(char *buf, char *par1, char *par2)
{
	while(*buf != 0 && *buf == ' ') buf++;
	while(*buf != 0 && *buf != ',') *par1++ = *buf++;
	*par1 = 0;
	if   (*buf != 0 && *buf == ',') buf++;
	while(*buf != 0 && *buf == ' ') buf++;
	while(*buf != 0) *par2++ = *buf++;
	*par2 = 0;
}

toupper(char c) {return(c <= 'z' && c >= 'a' ? c - 32 : c);}
toustr(char *b) {while(*b){*b = toupper(*b); b++;}}
isprint(char c) {return(c >= 32  && c <= 126);}
isspace(char c) {return(c <= ' ' &&(c == ' ' || (c <= 13 && c >= 9)));}

isxdigit(char c)
{
	return	(
	(c<='f' && c>='a') || (c<='F' && c>='A') || (c<='9' && c>='0'));
}

isxstr(char *s)
{
	if((*s == 0) || (strcmp(s, "ADC") == 0)) return(0);
	if((strcmp(s, "ADD") == 0) || (strcmp(s, "CC") == 0)) return(0); 
	if((strcmp(s, "DAA") == 0) || (strcmp(s, "DAD") == 0)) return(0);
	while(isxdigit(*s)) s++;
	if(*s == 0)	return(1);
		 else return(0);
}

strcpy(char *s, char *t) {while(*s++ = *t++);}

strcmp(char *s, char *t)
{
	while(*s == *t) {
	if(*s == 0) return (0);
	++s; ++t;
	}
	return (*s - *t);
}

itoa(int value, char *buffer)
{
	int x, y;
	char sign;
	char temp[8];

	for(x = 0; x < 8; x ++) temp[x] = ' ';
	sign = (value >= 0) ? '+' : '-';
	x = 7;
	do {
		temp[x] = (value % 10) + '0';
		value = value / 10;
		x--;
	} while(value > 0);
	temp[x] = sign;
	y = 0;
	while(x < 8) buffer[y++] = temp[x++];
	buffer[y] = '\0';
}

atoi(char *s)
{
	int n, sign;
	sign=n=0;
	if(*s == '+' || *s == '-') /* sign */
		if(*s++ == '-') sign = -sign;
	while(*s)
		n = 10 * n + *s++ - '0';
	if(sign) return(-n);
	return n;
}

xtob(char *s)
{
	char n, t;

	n = 0;
	while(*s != 0){
	     if((*s >= '0') && (*s <='9'))  t = 48;
	else if((*s >= 'A') && (*s <= 'F')) t = 55;
	else if((*s >= 'a') && (*s <= 'f')) t = 87;
	else break;
	n = (n << 4) + *s - t;
	s++;
	}
	return n;
}

xtoi(char *s)
{
	int n, t;

	n = 0;
	while(*s != 0){
	     if((*s >= '0') && (*s <='9'))  t = 48;
	else if((*s >= 'A') && (*s <= 'F')) t = 55;
	else if((*s >= 'a') && (*s <= 'f')) t = 87;
	else break;
	n = (n << 4) + *s - t;
	s++;
	}
	return n;
}

putnwl( ) {putchr(10);}

putwrd(int n)
{
	char c, buf[5];
	int i;

	for(i = 3; i >= 0; i--){
		c = n & 15;
		n = (n >> 4 ) & 4095;
		if(c < 10) buf[i] = c + 48;
		      else buf[i] = c + 55;
	}
	buf[4] = 0;
	putstr(buf);
}

putbyt(char n)
{
	char c, buf[3];
	int i;

	for(i = 1; i >= 0; i--){
		c = n & 15;
		n = (n >> 4 ) & 4095;
		if(c < 10) buf[i] = c + 48;
		      else buf[i] = c + 55;
	}
	buf[2] = 0;
	putstr(buf);
}

putstr(char *b) {while(*b) putchr(*b++);}

getstr(char buf[], int max)
{
	char ch;
	int len;

	len = 0;
	while((ch = getchr()) != 10){
	if( ch == 9) ch = ' ';
	if((ch == 8) && (len > 0)){
		len--;
		putchr(8); putchr(' '); putchr(8);}
	else if(isprint(ch) && (len < max)){
		buf[len++] = ch;
		putchr(ch);}
	}
	buf[len] = 0;
	putnwl();
}

gosub(char *address)
{
#asm
	POP	B
	POP	D
	PUSH	D
	PUSH	B
	LXI	H,ADRRET
	PUSH	H
	XCHG
	PCHL
ADRRET:
#endasm
}

opecod(int cod)
{
#asm
	POP	D
	POP	B
	PUSH	B
	PUSH	D
	LXI	D,OPCPTR
	MVI	H,00H
	MOV	L,C
	DAD	H
	DAD	D
	MOV	E,M
	INX	H
	MOV	D,M
	XCHG
#endasm
}

operd1(int cod)
{
#asm
	POP	D
	POP	B
	PUSH	B
	PUSH	D
	LXI	D,OR1PTR
	MVI	H,00H
	MOV	L,C
	DAD	H
	DAD	D
	MOV	E,M
	INX	H
	MOV	D,M
	XCHG
#endasm
}

operd2(int cod)
{
#asm
	POP	D
	POP	B
	PUSH	B
	PUSH	D
	LXI	D,OR2PTR
	MVI	H,00H
	MOV	L,C
	DAD	H
	DAD	D
	MOV	E,M
	INX	H
	MOV	D,M
	XCHG
#endasm
}

/*
	Small-C libraries
*/

smallc()
{
#asm
;
;----- call1.mac   Small-C  arithmetic and logical library
;	
;	part 1		Multiply Routine
;
	;
	;MULTIPLY DE BY HL AND RETURN IN HL
	;(SIGNED MULTIPLY)
	;
CCMULT:
MULT:	MOV	B,H
	MOV	C,L
	LXI	H,0
CCMLT1:	MOV	A,C
	RRC
	JNC	CCMLT2
	DAD	D
CCMLT2:	XRA	A
	MOV	A,B
	RAR
	MOV	B,A
	MOV	A,C
	RAR
	MOV	C,A
	ORA	B
	RZ
	XRA	A
	MOV	A,E
	RAL
	MOV	E,A
	MOV	A,D
	RAL
	MOV	D,A
	ORA	E
	RZ
	JMP	CCMLT1
;
;----- call2.mac   Small-C  arithmetic and logical library
;
;
;	part 2			divide routine
;
	;DIVIDE DE BY HL AND RETURN QUOTIENT IN HL, REMAINDER IN DE
	;(SIGNED DIVIDE)
	;
CCDIV:
DIV:	MOV	B,H
	MOV	C,L
	MOV	A,D
	XRA	B
	PUSH	PSW
	MOV	A,D
	ORA	A
	CM	CCDENEG
	MOV	A,B
	ORA	A
	CM	CCBCNEG
	MVI	A,16
	PUSH	PSW
	XCHG
	LXI	D,0
CCDIV1:	DAD	H
	CALL	CCRDEL
	JZ	CCDIV2
	CALL	CCCMPBCDE
	JM	CCDIV2
	MOV	A,L
	ORI	1
	MOV	L,A
	MOV	A,E
	SUB	C
	MOV	E,A
	MOV	A,D
	SBB	B
	MOV	D,A
CCDIV2:	POP	PSW
	DCR	A
	JZ	CCDIV3
	PUSH	PSW
	JMP	CCDIV1
CCDIV3:	POP	PSW
	RP
	CALL	CCDENEG
	XCHG
	CALL	CCDENEG
	XCHG
	RET
	;
	;NEGATE THE INTEGER IN DE
	;(INTERNAL ROUTINE)
	;
CCDENEG:	MOV	A,D
	CMA
	MOV	D,A
	MOV	A,E
	CMA
	MOV	E,A
	INX	D
	RET
	;
	;NEGATE THE INTEGER IN BC
	;(INTERNAL ROUTINE)
	;
CCBCNEG:	MOV	A,B
	CMA
	MOV	B,A
	MOV	A,C
	CMA
	MOV	C,A
	INX	B
	RET
	;
	;ROTATE DE LEFT ONE BIT
	;(INTERNAL ROUTINE)
	;
CCRDEL:
	MOV	A,E
	RAL
	MOV	E,A
	MOV	A,D
	RAL
	MOV	D,A
	ORA	E
	RET
	;
	;COMPARE BC TO DE
	;(INTERNAL ROUTINE)
	;
CCCMPBCDE:
	MOV	A,E
	SUB	C
	MOV	A,D
	SBB	B
	RET
;
;----- call3.mac   Small-C  arithmetic and logical library
;
;	part 3		switch routine
;
	;
	; EXECUTE "SWITCH" STATEMENT
	;
	;  HL  =  SWITCH VALUE
	; (SP) -> SWITCH TABLE
	;         DW ADDR1, VALUE1
	;         DW ADDR2, VALUE2
	;         ...
	;         DW 0
	;        [JMP default]
	;         continuation
	;
CCSWITCH:
	XCHG	;;DE =  SWITCH VALUE
	POP	H	;;HL -> SWITCH TABLE
SWLOOP:	MOV	C,M
	INX	H
	MOV	B,M	;;BC -> CASE ADDR, ELSE 0
	INX	H
	MOV	A,B
	ORA	C
	JZ	SWEND	;;DEFAULT OR CONTINUATION CODE
	MOV	A,M
	INX	H
	CMP	E
	MOV	A,M
	INX	H
	JNZ	SWLOOP
	CMP	D
	JNZ	SWLOOP
	MOV	H,B	;;CASE MATCHED
	MOV	L,C
SWEND:	PCHL
;
;----- call4.mac   Small-C  arithmetic and logical library
;
;
;		part 4		arithmetic shift routines
;
	;
	;SHIFT DE ARITHMETICALLY RIGHT BY HL AND RETURN IN HL
	;
CCASR:
	XCHG
	DCR	E
	RM
	MOV	A,H
	RAL
	MOV	A,H
	RAR
	MOV	H,A
	MOV	A,L
	RAR
	MOV	L,A
	JMP	CCASR+1
	;
	;SHIFT DE ARITHMETICALLY LEFT BY HL AND RETURN IN HL
	;
CCASL:
	XCHG
	DCR	E
	RM
	DAD	H
	JMP	CCASL+1
	;
;
;----- call5.mac   Small-C  arithmetic and logical library
;
;
;	part 5		main routine - multiply, divide, switch and
;				shift  routines in seperate modules
;
CCDCAL:
	PCHL
CCDDGC:
	DAD	D
	JMP	CCGCHAR
	;
CCDSGC:
	INX	H
	INX	H
	DAD	SP
	;
	;FETCH A SINGLE BYTE FROM THE ADDRESS IN HL AND
	;SIGN EXTEND INTO HL
	;
CCGCHAR:
	MOV	A,M
	;
	;PUT THE ACCUM INTO HL AND SIGN EXTEND THROUGH H.
	;
CCARGC:
CCSXT:
	MOV	L,A
	RLC
	SBB	A
	MOV	H,A
	RET
	;
CCDDGI:
	DAD	D
	JMP	CCGINT
	;
CCDSGI:
	INX	H
	INX	H
	DAD	SP
	;
	;FETCH A FULL 16-BIT INTEGER FROM THE ADDRESS IN HL
	;INTO HL
	;
CCGINT:
	MOV	A,M
	INX	H
	MOV	H,M
	MOV	L,A
	RET
	;
CCDECC:
	INX	H
	INX	H
 	DAD	SP
	MOV	D,H
	MOV	E,L
	CALL	CCGCHAR
	DCX	H
	MOV	A,L
	STAX	D
	RET
	;
CCINCC:
	INX	H
	INX	H
	DAD	SP
	MOV	D,H
	MOV	E,L
	CALL	CCGCHAR
	INX	H
	MOV	A,L
	STAX	D
	RET
	;
	;
;
CCDDPC:
CDPDPC:
	;
	DAD	D
CCPDPC:
	POP	B	;;RET ADDR
	POP	D
	PUSH	B
	;
	;STORE A SINGLE BYTE FROM HL AT THE ADDRESS IN DE
	;
CCPCHAR:
PCHAR:	MOV	A,L
	STAX	D
	RET
	;
CCDECI:
	INX	H
	INX	H
	DAD	SP
	MOV	D,H
	MOV	E,L
	CALL	CCGINT
	DCX	H
	JMP	CCPINT
	;
CCINCI:
	INX	H
	INX	H
	DAD	SP
	MOV	D,H
	MOV	E,L
	CALL	CCGINT
	INX	H
 	JMP	CCPINT
	;
	;
;
CCDDPI:
CDPDPI:
	;
	DAD	D
CCPDPI:
	POP	B	;;RET ADDR
	POP	D
	PUSH	B
	;
	;STORE A 16-BIT INTEGER IN HL AT THE ADDRESS IN DE
	;
CCPINT:
PINT:	MOV	A,L
	STAX	D
	INX	D
	MOV	A,H
	STAX	D
	RET
	;
	;INCLUSIVE "OR" HL AND DE INTO HL
	;
CCOR:
	MOV	A,L
	ORA	E
	MOV	L,A
	MOV	A,H
	ORA	D
	MOV	H,A
	RET
	;
	;EXCLUSIVE "OR" HL AND DE INTO HL
	;
CCXOR:
	MOV	A,L
	XRA	E
	MOV	L,A
	MOV	A,H
	XRA	D
	MOV	H,A
	RET
	;
	;"AND" HL AND DE INTO HL
	;
CCAND:
	MOV	A,L
	ANA	E
	MOV	L,A
	MOV	A,H
	ANA	D
	MOV	H,A
	RET
	;
	;IN ALL THE FOLLOWING COMPARE ROUTINES, HL IS SET TO 1 IF THE
	;CONDITION IS TRUE, OTHERWISE IT IS SET TO 0 (ZERO).
	;
	;TEST IF HL = DE
	;
CCEQ:
	CALL	CCCMP
	RZ
	DCX	H
	RET
	;
	;TEST IF DE <> HL
	;
CCNE:
	CALL	CCCMP
	RNZ
	DCX	H
	RET
	;
	;TEST IF DE > HL (SIGNED)
	;
CCGT:
	XCHG
	CALL	CCCMP
	RC
	DCX	H
	RET
	;
	;TEST IF DE <= HL (SIGNED)
	;
CCLE:
	CALL	CCCMP
	RZ
	RC
	DCX	H
	RET
	;
	;TEST IF DE >= HL (SIGNED)
	;
CCGE:
	CALL	CCCMP
	RNC
	DCX	H
	RET
	;
	;TEST IF DE < HL (SIGNED)
	;
CCLT:
	CALL	CCCMP
	RC
	DCX	H
	RET
	;
	;COMMON ROUTINE TO PERFORM A SIGNED COMPARE
	; OF DE AND HL
	;THIS ROUTINE PERFORMS DE - HL AND SETS THE CONDITIONS: 
	; CARRY REFLECTS SIGN OF DIFFERENCE (SET MEANS DE < HL)
	; ZERO/NON-ZERO SET ACCORDING TO EQUALITY.
	;
CCCMP:
	MOV	A,H	;;INVERT SIGN OF HL
	XRI	80H
	MOV	H,A
	MOV	A,D	;;INVERT SIGN OF DE
	XRI	80H
	CMP	H	;;COMPARE MSBS
	JNZ	CCCMP1	;;DONE IF NEQ
	MOV	A,E	;;COMPARE LSBS
	CMP	L
CCCMP1:	LXI	H,1	;;PRESET TRUE COND
	RET
	;
	;TEST IF DE >= HL (UNSIGNED)
	;
CCUGE:
	CALL	CCUCMP
	RNC
	DCX	H
	RET
	;
	;TEST IF DE < HL (UNSIGNED)
	;
CCULT:
	CALL	CCUCMP
	RC
	DCX	H
	RET
	;
	;TEST IF DE > HL (UNSIGNED)
	;
CCUGT:
	XCHG
	CALL	CCUCMP
	RC
	DCX	H
	RET
	;
	;TEST IF DE <= HL (UNSIGNED)
	;
CCULE:
	CALL	CCUCMP
	RZ
	RC
	DCX	H
	RET
	;
	;COMMON ROUTINE TO PERFORM UNSIGNED COMPARE
	;CARRY SET IF DE < HL
	;ZERO/NONZERO SET ACCORDINGLY
	;
CCUCMP:
	MOV	A,D
	CMP	H
	JNZ	CCUCP1
	MOV	A,E
	CMP	L
CCUCP1:	LXI	H,1
	RET
	;
	;SUBTRACT HL FROM DE AND RETURN IN HL
	;
CCSUB:
	MOV	A,E
	SUB	L
	MOV	L,A
	MOV	A,D
	SBB	H
	MOV	H,A
	RET
	;
	;FORM THE TWO'S COMPLEMENT OF HL
	;
CCNEG:
	CALL	CCCOM
	INX	H
	RET
	;
	;FORM THE ONE'S COMPLEMENT OF HL
	;
CCCOM:
	MOV	A,H
	CMA
	MOV	H,A
	MOV	A,L
	CMA
	MOV	L,A
	RET
CCLNEG:
	MOV	A,H
	ORA	L
	JNZ	$+6
	MVI	L,1
	RET
	LXI	H,0
	;RET
#endasm
}

/* data */

opecode1()
{
#asm
OPC001:	DB	'?',00H
OPC002:	DB	'A','C','I',00H
OPC003:	DB	'A','D','C',00H
OPC004:	DB	'A','D','D',00H
OPC005:	DB	'A','D','I',00H
OPC006:	DB	'A','N','A',00H
OPC007:	DB	'A','N','I',00H
OPC008:	DB	'C','A','L','L',00H
OPC009:	DB	'C','C',00H
OPC010:	DB	'C','M',00H
OPC011:	DB	'C','M','A',00H
OPC012:	DB	'C','M','C',00H
OPC013:	DB	'C','M','P',00H
OPC014:	DB	'C','N','Z',00H
OPC015:	DB	'C','P',00H
OPC016:	DB	'C','P','E',00H
OPC017:	DB	'C','P','I',00H
OPC018:	DB	'C','P','O',00H
OPC019:	DB	'C','Z',00H
OPC020:	DB	'D','A','A',00H
OPC021:	DB	'D','A','D',00H
OPC022:	DB	'D','C','R',00H
OPC023:	DB	'D','C','X',00H
OPC024:	DB	'D','I',00H
OPC025:	DB	'E','I',00H
OPC026:	DB	'H','L','T',00H
OPC027:	DB	'I','N',00H
OPC028:	DB	'I','N','R',00H
OPC029:	DB	'I','N','X',00H
OPC030:	DB	'J','C',00H
OPC031:	DB	'J','C','Z',00H
OPC032:	DB	'J','M',00H
OPC033:	DB	'J','M','P',00H
OPC034:	DB	'J','N','Z',00H
OPC035:	DB	'J','P',00H
OPC036:	DB	'J','P','E',00H
OPC037:	DB	'J','P','O',00H
OPC038:	DB	'J','Z',00H
OPC039:	DB	'L','D','A',00H
OPC040:	DB	'L','D','A','X',00H
OPC041:	DB	'L','H','L','D',00H
OPC042:	DB	'L','X','I',00H
OPC043:	DB	'M','O','V',00H
OPC044:	DB	'M','V','I',00H
OPC045:	DB	'N','O','P',00H
OPC046:	DB	'O','R','A',00H
OPC047:	DB	'O','R','I',00H
OPC048:	DB	'O','U','T',00H
OPC049:	DB	'P','C','H','L',00H
OPC050:	DB	'P','O','P',00H
OPC051:	DB	'P','U','S','H',00H
OPC052:	DB	'R','A','L',00H
OPC053:	DB	'R','A','R',00H
OPC054:	DB	'R','C',00H
OPC055:	DB	'R','E','T',00H
OPC056:	DB	'R','L','C',00H
OPC057:	DB	'R','M',00H
OPC058:	DB	'R','N','C',00H
OPC059:	DB	'R','N','Z',00H
OPC060:	DB	'R','P',00H
OPC061:	DB	'R','P','E',00H
OPC062:	DB	'R','P','O',00H
OPC063:	DB	'R','R','C',00H
OPC064:	DB	'R','S','T','0',00H
OPC065:	DB	'R','S','T','1',00H
OPC066:	DB	'R','S','T','2',00H
OPC067:	DB	'R','S','T','3',00H
OPC068:	DB	'R','S','T','4',00H
OPC069:	DB	'R','S','T','5',00H
OPC070:	DB	'R','S','T','6',00H
OPC071:	DB	'R','S','T','7',00H
OPC072:	DB	'R','Z',00H
OPC073:	DB	'S','B','B',00H
OPC074:	DB	'S','B','I',00H
OPC075:	DB	'S','H','L','D',00H
OPC076:	DB	'S','P','H','L',00H
OPC077:	DB	'S','T','A',00H
OPC078:	DB	'S','T','A','X',00H
OPC079:	DB	'S','T','C',00H
OPC080:	DB	'S','U','B',00H
OPC081:	DB	'S','U','I',00H
OPC082:	DB	'X','C','H','G',00H
OPC083:	DB	'X','R','A',00H
OPC084:	DB	'X','R','I',00H
OPC085:	DB	'X','T','H','L',00H
;
OR1000:	DB	00H,00H
OR1001:	DB	01H,00H
OR1002:	DB	02H,00H
OR1003:	DB	'A',00H
OR1004:	DB	'B',00H
OR1005:	DB	'C',00H
OR1006:	DB	'D',00H
OR1007:	DB	'E',00H
OR1008:	DB	'H',00H
OR1009:	DB	'L',00H
OR1010:	DB	'M',00H
OR1011:	DB	'P','S','W',00H
OR1012:	DB	'S','P',00H
;
OR2000:	DB	00H,00H
OR2001:	DB	01H,00H
OR2002:	DB	02H,00H
OR2003:	DB	'A',00H
OR2004:	DB	'B',00H
OR2005:	DB	'C',00H
OR2006:	DB	'D',00H
OR2007:	DB	'E',00H
OR2008:	DB	'H',00H
OR2009:	DB	'L',00H
OR2010:	DB	'M',00H
OR2011:	DB	'P','S','W',00H
OR2012:	DB	'S','P',00H
	; REMOVE THIS RET BEFORE RUN MAC.COM
#endasm
}

static char txtparm1[14] = {
	0x0f, 0x0c,			/* 40 column, text mode */
	63, 40, 50, 0x34,
	20, 8, 16, 18, 0, 11, 0x69, 10	/* 16 lines */
};
static char txtparm2[14] = {
	0x0f, 0x0c,			/* 40 column, text mode */
	63, 40, 50, 0x34,
	31, 6, 25, 28, 0, 7, 0x20, 0	/* 25 lines */
};
static char txtparm3[14] = {
	0x0e, 0x0c,			/* 80 column, text mode */
	127, 80, 98, 0x38,
	20, 8, 16, 18, 0, 11, 0x69, 10	/* 16 lines */
};
static char txtparm4[14] = {
	0x0e, 0x0c,			/* 80 column, text mode */
	127, 80, 98, 0x38,
	31, 6, 25, 28, 0, 7, 0x20, 0	/* 25 lines */
};
static char cgparm[14] = {
	0x0e, 0x0d,			/* 80 column, cg mode */
	127, 80, 98, 0x38,
	31, 6, 25, 28, 0, 7, 0x20, 0	/* 25 lines */
};

/* bios */

bios()
{
#asm
	ORG	0E000H
	; GET BIOS NUMBER
	SHLD	0BFE8H
	POP	H
	MOV	A,M
	INX	H
	PUSH	H
	LHLD	0BFE8H
#endasm
}

/* sub */

putchr(char ch)
{
	int wdth, size, sadr, cadr, cpos, i;
	
	/* get crtc params */
	wdth = getcrtcb(1);
	size = getcrtcb(6) * wdth;
	sadr = getcrtcw(12);
	cadr = getcrtcw(14);
	cpos = (cadr - sadr) & 0x7ff;
	
	/* check character code */
	if(ch == 0x08) {
		/* bs */
		if(sadr != cadr) {
			cadr--;
			setvram(cadr, 0);
		}
	}
	else if(ch == 0x09) {
		/* htab (4chars) */
		cadr += 4 - (cpos & 3);
	}
	else if(ch == 0x0a) {
		/* cr-lf */
		cadr -= cpos % wdth;
		cadr += wdth;
	}
	else if(ch > 0xa && ch < 0x10) {
		/* ctrl code */
		return;
	}
	else {
		setvram(cadr, ch);
		cadr++;
	}
	
	/* update cursor address */
	cadr &= 0x7ff;
	setcrtcw(14, cadr);
	cpos = (cadr - sadr) & 0x7ff;
	if(cpos >= size) {
		/* scroll */
		cadr -= cpos % wdth;
		for(i = 0; i < wdth; i++) {
			setvram(cadr, 0);
			cadr++;
		}
		sadr += wdth;
		setcrtcw(12, sadr);
	}
}

getchr()
{
#asm
GETCHR1:
	CALL	inkey
	PUSH	H
	CALL	cnvkey
	POP	D
	MOV	A,L
	ANI	0FFH
	JZ	GETCHR1
#endasm
}

inkey()
{
#asm
	LXI	H,0000H
	IN	08H
	STA	0BFE0H
	ANI	01H
	RZ
	IN	05H
	MOV	L,A
INKEY1:
	IN	05H
	CMP	L
	JNZ	INKEY2
	IN	08H
	STA	0BFE0H
	ANI	01H
	JNZ	INKEY1
INKEY2:
	MOV	A,L
	CPI	61H
	JZ	INKEY3
	CPI	0E1H
	JZ	INKEY3
	RET
INKEY3:
	; GRAPH KEY
	LDA	0BFE1H
	XRI	01H
	STA	0BFE1H
#endasm
}

cnvkey(char ch)
{
#asm
	POP	B
	POP	H
	PUSH	H
	PUSH	B
	MVI	H,00H
	MOV	A,L
CNVKEY2:
	; 00H-07H -> NULL
	CPI	08H
	JC	CNVKEY3
	; 08H-0FH -> OK
	CPI	10H
	RC
	; 10H-1FH -> NULL
	; 20H     -> OK
	CPI	20H
	JC	CNVKEY3
	RZ
	; 21H-27H -> GRAPH
	CPI	28H
	JC	CNVKEY5
	; 28H-2FH -> OK
	CPI	30H
	RC
	; 30H-5FH -> GRAPH
	CPI	60H
	JC	CNVKEY5
	; 60H-7FH -> NULL
	CPI	80H
	JC	CNVKEY3
	; 80H-9FH -> 00H-1FH
	CPI	0A0H
	JC	CNVKEY4
	; A0H-DFH -> GRAPH
	CPI	0E0H
	JC	CNVKEY5
CNVKEY3:
	; E0H-FFH -> NULL
	MVI	L,00H
	RET
CNVKEY4:
	ANI	7FH
	MOV	L,A
	JMP	CNVKEY2
CNVKEY5:
	; GRAPH
	LDA	0BFE1H
	ANI	01H
	JZ	CNVKEY6
	MOV	A,L
	XRI	20H
	MOV	L,A
	RET
CNVKEY6:
	; A0H -> 20H
	MOV	A,L
	CPI	0A0H
	RNZ
	MVI	L,20H
#endasm
}

cursor(char x, char y)
{
#asm
	POP	B
	POP	H
	POP	D
	PUSH	D
	PUSH	H
	PUSH	B
	PUSH	D
	PUSH	H
	CALL	vramadr
	POP	B
	POP	B
	LXI	D,000EH
	PUSH	D
	PUSH	H
	CALL	setcrtcw
	POP	B
	POP	B
#endasm
}

plot(int x, int y, char dot)
{
	int adr;
	char bit, dat;
	
	adr = (x >> 1) + (y >> 2) * 80;
	bit = ((x & 1) * 4) + (y & 3);
	bit = 1 << bit;
	dat = getvram(adr);
	if(dot) {
		dat |= bit;
	}
	else {
		dat &= ~bit;
	}
	setvram(adr, dat);
}

line(int sx, int sy, int ex, int ey, char dot)
{
	int nx, ny, dx, dy, xx, yy, frac;
	
	nx = sx;
	ny = sy;
	if(ex < sx) {
		dx = sx - ex;
		xx = -1;
	}
	else {
		dx = ex - sx;
		xx = 1;
	}
	dx *= 2;
	if(ey < sy) {
		dy = sy - ey;
		yy = -1;
	}
	else {
		dy = ey - sy;
		yy = 1;
	}
	dy *= 2;
	
	if(dx > dy) {
		frac = dy - dx / 2;
		while(nx != ex) {
			if(frac >= 0) {
				ny += yy;
				frac -= dx;
			}
			nx += xx;
			frac += dy;
			plot(nx, ny, dot);
		}
	}
	else {
		frac = dx - dy / 2;
		while(ny != ey) {
			if(frac >= 0) {
				nx += xx;
				frac -= dy;
			}
			ny += yy;
			frac += dx;
			plot(nx, ny, dot);
		}
	}
	plot(sx, sy, dot);
	plot(ex, ey, dot);
}

clrscrn()
{
#asm
	CALL	clrvram
	CALL	rescrtca
#endasm
}

txtmode(char mode)
{
#asm
	CALL	clrscrn
	POP	B
	POP	D
	PUSH	D
	PUSH	B
	MOV	A,E
TXTMODE1:
	CPI	01H
	JZ	TXTMODE2
	CPI	02H
	JZ	TXTMODE3
	CPI	03H
	JZ	TXTMODE4
	CPI	04H
	JZ	TXTMODE5
	LDA	0BFE4H
	JMP	TXTMODE1
TXTMODE2:
	LXI	D,txtparm1
	JMP	TXTMODE6
TXTMODE3:
	LXI	D,txtparm2
	JMP	TXTMODE6
TXTMODE4:
	LXI	D,txtparm3
	JMP	TXTMODE6
TXTMODE5:
	LXI	D,txtparm4
TXTMODE6:
	STA	0BFE4H
	PUSH	D
	CALL	setcrtcs
	POP	D
#endasm
}

cgmode()
{
#asm
	CALL	clrscrn
	LXI	D,cgparm
	PUSH	D
	CALL	setcrtcs
	POP	D
#endasm
}

setcrtcs(char *p)
{
#asm
	POP	B
	POP	D
	PUSH	D
	PUSH	B
	LDAX	D
	INX	D
	OUT	0BH
	LDAX	D
	INX	D
	OUT	0BH
	LXI	H,0000H
SETCRTCS1:
	PUSH	D
	LDAX	D
	MVI	D,00H
	MOV	E,A
	PUSH	H
	PUSH	D
	CALL	setcrtcb
	POP	D
	POP	H
	POP	D
	INX	D
	INX	H
	MOV	A,L
	CPI	14
	JNZ	SETCRTCS1
	CALL	rescrtca
#endasm
}

getcrtcb(char r)
{
#asm
	POP	B
	POP	D
	PUSH	D
	PUSH	B
	MVI	D,0BFH
	MOV	A,E
	ORI	0F0H
	MOV	E,A
	LDAX	D
	MVI	H,00H
	MOV	L,A
#endasm
}

getcrtcw(char r)
{
#asm
	POP	B
	POP	D
	PUSH	D
	PUSH	B
	MVI	D,0BFH
	MOV	A,E
	ORI	0F0H
	MOV	E,A
	LDAX	D
	MOV	H,A
	MOV	A,E
	ORI	01H
	MOV	E,A
	LDAX	D
	MOV	L,A
#endasm
}

setcrtcb(char r, char v)
{
#asm
	POP	B
	POP	H
	POP	D
	PUSH	D
	PUSH	H
	PUSH	B
	MOV	A,E
	OUT	02H
	MOV	A,L
	OUT	03H
	MVI	D,0BFH
	MOV	A,E
	ORI	0F0H
	MOV	E,A
	MOV	A,L
	STAX	D
#endasm
}

setcrtcw(char r, int v)
{
#asm
	POP	B
	POP	H
	POP	D
	PUSH	D
	PUSH	H
	PUSH	B
	MOV	A,E
	OUT	02H
	MOV	A,H
	OUT	03H
	MOV	A,E
	ORI	01H
	OUT	02H
	MOV	A,L
	OUT	03H
	MVI	D,0BFH
	MOV	A,E
	ORI	0F0H
	MOV	E,A
	MOV	A,H
	STAX	D
	MOV	A,E
	ORI	01H
	MOV	E,A
	MOV	A,L
	STAX	D
	; UPDATE CURSOR X AND Y
	LHLD	0BFFEH
	XCHG
	LHLD	0BFFCH
	CALL	CCSUB
	MOV	A,H
	ANI	07H
	MOV	D,A
	MOV	E,L
	LDA	0BFF1H
	MOV	L,A
	MVI	H,00H
	CALL	CCDIV
	MOV	A,E
	STA	0BFEEH
	MOV	A,L
	STA	0BFEFH
#endasm
}

rescrtca()
{
#asm
	LXI	D,000CH
	PUSH	D
	LXI	D,0000H
	PUSH	D
	CALL	setcrtcw
	POP	D
	POP	D
	LXI	D,000EH
	PUSH	D
	LXI	D,0000H
	PUSH	D
	CALL	setcrtcw
	POP	D
	POP	D
#endasm
}

getvram(int a)
{
#asm
	POP	B
	POP	D
	PUSH	D
	PUSH	B
	MOV	A,E
	OUT	04H
	IN	06H
	ANI	0F8H
	MOV	E,A
	MOV	A,D
	ANI	07H
	ORA	E
	OUT	06H
	CALL	tvblnk
	IN	01H
	MVI	H,00H
	MOV	L,A
#endasm
}

setvram(int a, char v)
{
#asm
	POP	B
	POP	H
	POP	D
	PUSH	D
	PUSH	H
	PUSH	B
	MOV	A,E
	OUT	04H
	IN	06H
	ANI	0F8H
	MOV	E,A
	MOV	A,D
	ANI	07H
	ORA	E
	OUT	06H
	CALL	tvblnk
	MOV	A,L
	OUT	01H
#endasm
}

clrvram()
{
#asm
	LXI	H,0000H
	LXI	D,0000H
CLRSCRN1:
	PUSH	H
	PUSH	D
	CALL	setvram
	POP	D
	POP	H
	INX	H
	MOV	A,H
	ANI	08H
	JZ	CLRSCRN1
#endasm
}

vramadr(char x, char y)
{
#asm
	POP	B
	POP	H
	POP	D
	PUSH	D
	PUSH	H
	PUSH	B
	PUSH	D
	MVI	D,00H
	LDA	0BFF1H
	MOV	E,A
	CALL	CCMULT
	POP	D
	MOV	A,E
	ADD	L
	MOV	E,A
	MOV	A,D
	ADC	H
	MOV	D,A
	LHLD	0BFFCH
	MOV	A,L
	ADD	E
	MOV	L,A
	MOV	A,H
	ADC	D
	ANI	07H
	MOV	H,A
#endasm
}

tvblnk()
{
#asm
TVBLNK1:
	IN	08H
	RAL
	JC	TVBLNK1
#endasm
}

void setpsg(char v)
{
#asm
	POP	B
	POP	D
	PUSH	D
	PUSH	B
	MOV	A,E
	OUT	04H
	IN	0AH
	ORI	30H
	OUT	0AH
	XRI	30H
	OUT	0AH
	ORI	30H
	OUT	0AH
#endasm
}

void getrtc(char r)
{
#asm
	POP	B
	POP	D
	PUSH	D
	PUSH	B
	MVI	A,92H
	OUT	0FH
	MOV	A,E
	ANI	0FH
	ORI	90H
	OUT	0EH
	ORI	20H
	OUT	0EH
	IN	0DH
	MVI	H,00H
	ANI	0FH
	MOV	L,A
	MVI	A,00H
	OUT	0EH
#endasm
}

void setrtc(char r, char v)
{
#asm
	POP	B
	POP	H
	POP	D
	PUSH	D
	PUSH	H
	PUSH	B
	MVI	A,90H
	OUT	0FH
	MOV	A,L
	OUT	0DH
	MOV	A,E
	ANI	0FH
	ORI	90H
	OUT	0EH
	ORI	40H
	OUT	0EH
	MVI	A,00H
	OUT	0EH
#endasm
}

/* data */

opecode2()
{
#asm
	ORG	0EA00H
OPCPTR:
	DW	OPC045,OPC042,OPC078,OPC029,OPC028,OPC022,OPC044,OPC056
	DW	OPC001,OPC021,OPC040,OPC023,OPC028,OPC022,OPC044,OPC063
	DW	OPC001,OPC042,OPC078,OPC029,OPC028,OPC022,OPC044,OPC052
	DW	OPC001,OPC021,OPC040,OPC023,OPC028,OPC022,OPC044,OPC053
	DW	OPC001,OPC042,OPC075,OPC029,OPC028,OPC022,OPC044,OPC020
	DW	OPC001,OPC021,OPC041,OPC023,OPC028,OPC022,OPC044,OPC011
	DW	OPC001,OPC042,OPC077,OPC029,OPC028,OPC022,OPC044,OPC079
	DW	OPC001,OPC021,OPC039,OPC023,OPC028,OPC022,OPC044,OPC012
	DW	OPC043,OPC043,OPC043,OPC043,OPC043,OPC043,OPC043,OPC043
	DW	OPC043,OPC043,OPC043,OPC043,OPC043,OPC043,OPC043,OPC043
	DW	OPC043,OPC043,OPC043,OPC043,OPC043,OPC043,OPC043,OPC043
	DW	OPC043,OPC043,OPC043,OPC043,OPC043,OPC043,OPC043,OPC043
	DW	OPC043,OPC043,OPC043,OPC043,OPC043,OPC043,OPC043,OPC043
	DW	OPC043,OPC043,OPC043,OPC043,OPC043,OPC043,OPC043,OPC043
	DW	OPC043,OPC043,OPC043,OPC043,OPC043,OPC043,OPC026,OPC043
	DW	OPC043,OPC043,OPC043,OPC043,OPC043,OPC043,OPC043,OPC043
	DW	OPC004,OPC004,OPC004,OPC004,OPC004,OPC004,OPC004,OPC004
	DW	OPC003,OPC003,OPC003,OPC003,OPC003,OPC003,OPC003,OPC003
	DW	OPC080,OPC080,OPC080,OPC080,OPC080,OPC080,OPC080,OPC080
	DW	OPC073,OPC073,OPC073,OPC073,OPC073,OPC073,OPC073,OPC073
	DW	OPC006,OPC006,OPC006,OPC006,OPC006,OPC006,OPC006,OPC006
	DW	OPC083,OPC083,OPC083,OPC083,OPC083,OPC083,OPC083,OPC083
	DW	OPC046,OPC046,OPC046,OPC046,OPC046,OPC046,OPC046,OPC046
	DW	OPC013,OPC013,OPC013,OPC013,OPC013,OPC013,OPC013,OPC013
	DW	OPC059,OPC050,OPC034,OPC033,OPC014,OPC051,OPC005,OPC064
	DW	OPC072,OPC055,OPC038,OPC001,OPC019,OPC008,OPC002,OPC065
	DW	OPC058,OPC050,OPC031,OPC048,OPC014,OPC051,OPC081,OPC066
	DW	OPC054,OPC001,OPC030,OPC027,OPC009,OPC001,OPC074,OPC067
	DW	OPC062,OPC050,OPC037,OPC085,OPC018,OPC051,OPC007,OPC068
	DW	OPC061,OPC049,OPC036,OPC082,OPC016,OPC001,OPC084,OPC069
	DW	OPC060,OPC050,OPC035,OPC024,OPC015,OPC051,OPC047,OPC070
	DW	OPC057,OPC076,OPC032,OPC025,OPC010,OPC001,OPC017,OPC071
;
OR1PTR:
	DW	OR1000,OR1004,OR1004,OR1004,OR1004,OR1004,OR1004,OR1000
	DW	OR1000,OR1004,OR1004,OR1004,OR1005,OR1005,OR1005,OR1000
	DW	OR1000,OR1006,OR1006,OR1006,OR1006,OR1006,OR1006,OR1000
	DW	OR1000,OR1006,OR1006,OR1006,OR1007,OR1007,OR1007,OR1000
	DW	OR1000,OR1008,OR1002,OR1008,OR1008,OR1008,OR1008,OR1000
	DW	OR1000,OR1008,OR1002,OR1008,OR1009,OR1009,OR1009,OR1002
	DW	OR1000,OR1012,OR1002,OR1012,OR1010,OR1010,OR1010,OR1000
	DW	OR1000,OR1012,OR1002,OR1012,OR1003,OR1003,OR1003,OR1002
	DW	OR1004,OR1004,OR1004,OR1004,OR1004,OR1004,OR1004,OR1004
	DW	OR1005,OR1005,OR1005,OR1005,OR1005,OR1005,OR1005,OR1005
	DW	OR1006,OR1006,OR1006,OR1006,OR1006,OR1006,OR1006,OR1006
	DW	OR1007,OR1007,OR1007,OR1007,OR1007,OR1007,OR1007,OR1007
	DW	OR1008,OR1008,OR1008,OR1008,OR1008,OR1008,OR1008,OR1008
	DW	OR1009,OR1009,OR1009,OR1009,OR1009,OR1009,OR1009,OR1009
	DW	OR1010,OR1010,OR1010,OR1010,OR1010,OR1010,OR1000,OR1010
	DW	OR1003,OR1003,OR1003,OR1003,OR1003,OR1003,OR1003,OR1003
	DW	OR1004,OR1005,OR1006,OR1007,OR1008,OR1009,OR1010,OR1003
	DW	OR1004,OR1005,OR1006,OR1007,OR1008,OR1009,OR1010,OR1003
	DW	OR1004,OR1005,OR1006,OR1007,OR1008,OR1009,OR1010,OR1003
	DW	OR1004,OR1005,OR1006,OR1007,OR1008,OR1009,OR1010,OR1003
	DW	OR1004,OR1005,OR1006,OR1007,OR1008,OR1009,OR1010,OR1003
	DW	OR1004,OR1005,OR1006,OR1007,OR1008,OR1009,OR1010,OR1003
	DW	OR1004,OR1005,OR1006,OR1007,OR1008,OR1009,OR1010,OR1003
	DW	OR1004,OR1005,OR1006,OR1007,OR1008,OR1009,OR1010,OR1003
	DW	OR1000,OR1004,OR1002,OR1002,OR1000,OR1004,OR1001,OR1000
	DW	OR1000,OR1000,OR1002,OR1000,OR1000,OR1002,OR1001,OR1000
	DW	OR1000,OR1006,OR1002,OR1001,OR1000,OR1006,OR1001,OR1000
	DW	OR1000,OR1000,OR1002,OR1001,OR1002,OR1000,OR1001,OR1000
	DW	OR1000,OR1008,OR1002,OR1000,OR1000,OR1008,OR1001,OR1000
	DW	OR1000,OR1000,OR1002,OR1000,OR1000,OR1000,OR1001,OR1000
	DW	OR1000,OR1011,OR1002,OR1000,OR1000,OR1011,OR1001,OR1000
	DW	OR1000,OR1000,OR1002,OR1000,OR1002,OR1000,OR1001,OR1000
;
OR2PTR:
	DW	OR2000,OR2002,OR2000,OR2000,OR2000,OR2000,OR2001,OR2000
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2001,OR2000
	DW	OR2000,OR2002,OR2000,OR2000,OR2000,OR2000,OR2001,OR2000
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2001,OR2000
	DW	OR2000,OR2002,OR2000,OR2000,OR2000,OR2000,OR2001,OR2000
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2001,OR2000
	DW	OR2000,OR2002,OR2000,OR2000,OR2000,OR2000,OR2001,OR2000
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2001,OR2000
	DW	OR2004,OR2005,OR2006,OR2007,OR2008,OR2009,OR2010,OR2003
	DW	OR2004,OR2005,OR2006,OR2007,OR2008,OR2009,OR2010,OR2003
	DW	OR2004,OR2005,OR2006,OR2007,OR2008,OR2009,OR2010,OR2003
	DW	OR2004,OR2005,OR2006,OR2007,OR2008,OR2009,OR2010,OR2003
	DW	OR2004,OR2005,OR2006,OR2007,OR2008,OR2009,OR2010,OR2003
	DW	OR2004,OR2005,OR2006,OR2007,OR2008,OR2009,OR2010,OR2003
	DW	OR2004,OR2005,OR2006,OR2007,OR2008,OR2009,OR2000,OR2003
	DW	OR2004,OR2005,OR2006,OR2007,OR2008,OR2009,OR2010,OR2003
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000
	DW	OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000,OR2000
	; REMOVE THIS RET BEFORE RUN MAC.COM
#endasm
}
