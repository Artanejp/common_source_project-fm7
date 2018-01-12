        ORG $C000
STACK   EQU $8100
RCB     EQU $9F00
RCBREG  EQU $9FFF
BYTES   EQU $00FF

BEGIN   LDS #STACK
        LDA #$80
        STA RCBREG
        BSR TEST1
WAIT    NOP
        LDA #$00
        STA RCBREG
        NOP
        NOP
        BRA WAIT
TEST1
        LDX #RCB
        LDU #BYTES
        LDA #$00
MEMSET1 STA ,X+
        INCA
        LEAU -1,U
        CMPU #$0000
        BNE MEMSET1
        RTS

        ORG $FFFE
V_RESET FDB $C000
