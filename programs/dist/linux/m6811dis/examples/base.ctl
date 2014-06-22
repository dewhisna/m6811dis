;
;    BASE ROOT Code Example
;

MemMap ROM C000 4000
MemMap RAM B000 0100
MemMap IO  A000 0040
MemMap IO  103D 0001

load C000 base.bin binary

output Disassembly base.dis
output Functions base.fnc

addresses on
;ascii on
asciibytes off
dataopbytes on
;opbytes on

tabs off

label ffd6 scivect
label ffd8 spivect
label ffda paievect
label ffdc paovect
label ffde tovfvect
label ffe0 ti4o5vect
label ffe2 to4vect
label ffe4 to3vect
label ffe6 to2vect
label ffe8 to1vect
label ffea ti3vect
label ffec ti2vect
label ffee ti1vect
label fff0 rtivect
label fff2 irqvect
label fff4 xirqvect
label fff6 swivect
label fff8 ilopvect
label fffa copvect
label fffc cmonvect
label fffe rstvect

;indirect code ffd6 scirtn
;indirect code ffd8 spirtn
;indirect code ffda paiertn
;indirect code ffdc paortn
;indirect code ffde tovfrtn
;indirect code ffe0 ti4o5rtn
;indirect code ffe2 to4rtn
;indirect code ffe4 to3rtn
;indirect code ffe6 to2rtn
;indirect code ffe8 to1rtn
indirect code ffea ti3rtn
indirect code ffec ti2rtn
indirect code ffee ti1rtn
indirect code fff0 rtirtn
indirect code fff2 irqrtn
;indirect code fff4 xirqrtn
;indirect code fff6 swirtn
indirect code fff8 iloprtn
;indirect code fffa coprtn
;indirect code fffc cmonrtn
indirect code fffe reset

; =============================================================================

;indirect code FA62                 ; fdb 2
;indirect code FA64                 ; fdb $9C47
;indirect code FA66                 ; fdb 2
;indirect code FA68                 ; fdb $93C1
;indirect code FA6A                 ; fdb 3
;indirect code FA6C                 ; fdb $8F47
;indirect code FA6E                 ; fdb 3
;indirect code FA70                 ; fdb $8FBB
;indirect code FA72                 ; fdb 3
;indirect code FA74                 ; fdb $9036
;indirect code FA76                 ; fdb 0
indirect code FA78                 ; fdb $C0BA
;indirect code FA7A                 ; fdb 0
indirect code FA7C                 ; fdb $C14A
;indirect code FA7E                 ; fdb 0
indirect code FA80                 ; fdb $C209
;indirect code FA82                 ; fdb 0
indirect code FA84                 ; fdb $C54F
;indirect code FA86                 ; fdb 0
indirect code FA88                 ; fdb $C6B2
;indirect code FA8A                 ; fdb 0
indirect code FA8C                 ; fdb $C78D
;indirect code FA8E                 ; fdb 0
indirect code FA90                 ; fdb $F189
;indirect code FA92                 ; fdb 0
;indirect code FA94                 ; fdb $B0C8
;indirect code FA96                 ; fdb 0
;indirect code FA98                 ; fdb $B0C9
;indirect code FA9A                 ; fdb 0
;indirect code FA9C                 ; fdb $B0CA
;indirect code FA9E                 ; fdb 0
indirect code FAA0                 ; fdb $C9F2
;indirect code FAA2                 ; fdb 0
indirect code FAA4                 ; fdb $CA6A
;indirect code FAA6                 ; fdb 0
indirect code FAA8                 ; fdb $CAF9
;indirect code FAAA                 ; fdb 0
indirect code FAAC                 ; fdb $CB4E
;indirect code FAAE                 ; fdb 0
indirect code FAB0                 ; fdb $CBF8
;indirect code FAB2                 ; fdb 0
indirect code FAB4                 ; fdb $CE2A
;indirect code FAB6                 ; fdb 0
indirect code FAB8                 ; fdb $CE55
;indirect code FABA                 ; fdb 0
indirect code FABC                 ; fdb $CEDD
;indirect code FABE                 ; fdb 0
indirect code FAC0                 ; fdb $CF35
;indirect code FAC2                 ; fdb 0
indirect code FAC4                 ; fdb $D0D1
;indirect code FAC6                 ; fdb 3
;indirect code FAC8                 ; fdb $8000
;indirect code FACA                 ; fdb 3
;indirect code FACC                 ; fdb $8190
;indirect code FACE                 ; fdb 3
;indirect code FAD0                 ; fdb $82B9
;indirect code FAD2                 ; fdb 3
;indirect code FAD4                 ; fdb $9317
;indirect code FAD6                 ; fdb 3
;indirect code FAD8                 ; fdb $93C5
;indirect code FADA                 ; fdb 1
;indirect code FADC                 ; fdb $962F
;indirect code FADE                 ; fdb 1
;indirect code FAE0                 ; fdb $990D
;indirect code FAE2                 ; fdb 4
;indirect code FAE4                 ; fdb $8000
;indirect code FAE6                 ; fdb 4
;indirect code FAE8                 ; fdb $8172
;indirect code FAEA                 ; fdb 4
;indirect code FAEC                 ; fdb $83B6
;indirect code FAEE                 ; fdb 3
;indirect code FAF0                 ; fdb $8580
;indirect code FAF2                 ; fdb 3
;indirect code FAF4                 ; fdb $869C
;indirect code FAF6                 ; fdb 3
;indirect code FAF8                 ; fdb $87EF
;indirect code FAFA                 ; fdb 3
;indirect code FAFC                 ; fdb $886D
;indirect code FAFE                 ; fdb 1
;indirect code FB00                 ; fdb $8000
;indirect code FB02                 ; fdb 1
;indirect code FB04                 ; fdb $8102
;indirect code FB06                 ; fdb 1
;indirect code FB08                 ; fdb $8365
;indirect code FB0A                 ; fdb 1
;indirect code FB0C                 ; fdb $8497
;indirect code FB0E                 ; fdb 1
;indirect code FB10                 ; fdb $9987
;indirect code FB12                 ; fdb 1
;indirect code FB14                 ; fdb $9B2E
;indirect code FB16                 ; fdb 2
;indirect code FB18                 ; fdb $8D46
;indirect code FB1A                 ; fdb 2
;indirect code FB1C                 ; fdb $90EF
;indirect code FB1E                 ; fdb 2
;indirect code FB20                 ; fdb $92E1
;indirect code FB22                 ; fdb 2
;indirect code FB24                 ; fdb $9336
;indirect code FB26                 ; fdb 2
;indirect code FB28                 ; fdb $9A6B
;indirect code FB2A                 ; fdb 2
;indirect code FB2C                 ; fdb $9649
;indirect code FB2E                 ; fdb 2
;indirect code FB30                 ; fdb $96A4
;indirect code FB32                 ; fdb 2
;indirect code FB34                 ; fdb $979C
;indirect code FB36                 ; fdb 1
;indirect code FB38                 ; fdb $8E04
;indirect code FB3A                 ; fdb 1
;indirect code FB3C                 ; fdb $8F53
;indirect code FB3E                 ; fdb 1
;indirect code FB40                 ; fdb $90A2
;indirect code FB42                 ; fdb 1
;indirect code FB44                 ; fdb $91F2
;indirect code FB46                 ; fdb 0
;indirect code FB48                 ; fdb $9CDB
;indirect code FB4A                 ; fdb 0
;indirect code FB4C                 ; fdb $9EA5
;indirect code FB4E                 ; fdb 0
indirect code FB50                 ; fdb $F23F
;indirect code FB52                 ; fdb 1
;indirect code FB54                 ; fdb $9267
;indirect code FB56                 ; fdb 1
;indirect code FB58                 ; fdb $927F
;indirect code FB5A                 ; fdb 1
;indirect code FB5C                 ; fdb $9298
;indirect code FB5E                 ; fdb 1
;indirect code FB60                 ; fdb $9350
;indirect code FB62                 ; fdb 1
;indirect code FB64                 ; fdb $9417
;indirect code FB66                 ; fdb 1
;indirect code FB68                 ; fdb $9516
;indirect code FB6A                 ; fdb 1
;indirect code FB6C                 ; fdb $9553
;indirect code FB6E                 ; fdb 1
;indirect code FB70                 ; fdb $95AA
;indirect code FB72                 ; fdb 0
indirect code FB74                 ; fdb $D91D
;indirect code FB76                 ; fdb 0
indirect code FB78                 ; fdb $DA87
;indirect code FB7A                 ; fdb 0
indirect code FB7C                 ; fdb $DF58
;indirect code FB7E                 ; fdb 0
indirect code FB80                 ; fdb $DF7D
;indirect code FB82                 ; fdb 0
;indirect code FB84                 ; fdb $8000
;indirect code FB86                 ; fdb 0
;indirect code FB88                 ; fdb $8AB6
;indirect code FB8A                 ; fdb 0
;indirect code FB8C                 ; fdb $8CC7
;indirect code FB8E                 ; fdb 1
;indirect code FB90                 ; fdb $84D5
;indirect code FB92                 ; fdb 1
;indirect code FB94                 ; fdb $8646
;indirect code FB96                 ; fdb 1
;indirect code FB98                 ; fdb $88E2
;indirect code FB9A                 ; fdb 3
;indirect code FB9C                 ; fdb $8A64
;indirect code FB9E                 ; fdb 3
;indirect code FBA0                 ; fdb $8C30
;indirect code FBA2                 ; fdb 3
;indirect code FBA4                 ; fdb $8DEF
;indirect code FBA6                 ; fdb 2
;indirect code FBA8                 ; fdb $8000
;indirect code FBAA                 ; fdb 2
;indirect code FBAC                 ; fdb $88E2
;indirect code FBAE                 ; fdb 2
;indirect code FBB0                 ; fdb $8D11
;indirect code FBB2                 ; fdb 0
;indirect code FBB4                 ; fdb $8E84
;indirect code FBB6                 ; fdb 0
;indirect code FBB8                 ; fdb $96BC
;indirect code FBBA                 ; fdb 0
;indirect code FBBC                 ; fdb $9BD9
;indirect code FBBE                 ; fdb 0
indirect code FBC0                 ; fdb $E301
;indirect code FBC2                 ; fdb 0
indirect code FBC4                 ; fdb $E3D8
;indirect code FBC6                 ; fdb 0
indirect code FBC8                 ; fdb $E572
;indirect code FBCA                 ; fdb 0
indirect code FBCC                 ; fdb $E8CC
;indirect code FBCE                 ; fdb 0
indirect code FBD0                 ; fdb $EA95
;indirect code FBD2                 ; fdb 0
indirect code FBD4                 ; fdb $EAFD
;indirect code FBD6                 ; fdb 0
indirect code FBD8                 ; fdb $EB53
;indirect code FBDA                 ; fdb 0
indirect code FBDC                 ; fdb $EC01
;indirect code FBDE                 ; fdb 0
indirect code FBE0                 ; fdb $EC4E
;indirect code FBE2                 ; fdb 0
indirect code FBE4                 ; fdb $ECC6
;indirect code FBE6                 ; fdb 0
indirect code FBE8                 ; fdb $ECE9
;indirect code FBEA                 ; fdb 0
indirect code FBEC                 ; fdb $ED17
;indirect code FBEE                 ; fdb 0
indirect code FBF0                 ; fdb $ED27
;indirect code FBF2                 ; fdb 0
indirect code FBF4                 ; fdb $ED49
;indirect code FBF6                 ; fdb 0
indirect code FBF8                 ; fdb $ED58
;indirect code FBFA                 ; fdb 0
indirect code FBFC                 ; fdb $ED72
;indirect code FBFE                 ; fdb 2
;indirect code FC00                 ; fdb $988B
;indirect code FC02                 ; fdb 1
;indirect code FC04                 ; fdb $898E
;indirect code FC06                 ; fdb 1
;indirect code FC08                 ; fdb $8BD5
;indirect code FC0A                 ; fdb 1
;indirect code FC0C                 ; fdb $8D6D
;indirect code FC0E                 ; fdb 1
;indirect code FC10                 ; fdb $9C69
;indirect code FC12                 ; fdb 0
indirect code FC14                 ; fdb $ED81
;indirect code FC16                 ; fdb 0
indirect code FC18                 ; fdb $EDA7
;indirect code FC1A                 ; fdb 0
indirect code FC1C                 ; fdb $EEC2
;indirect code FC1E                 ; fdb 0
indirect code FC20                 ; fdb $EFFC
;indirect code FC22                 ; fdb 0
indirect code FC24                 ; fdb $F0BA
;indirect code FC26                 ; fdb 0
indirect code FC28                 ; fdb $F163

; =============================================================================

indirect code DAC3                 ; fdb loc_0_DB28
indirect code DAC7                 ; fdb loc_0_DB20
indirect code DACB                 ; fdb loc_0_DB18
indirect code DACF                 ; fdb loc_0_DB10
indirect code DAD3                 ; fdb loc_0_DB08
indirect code DAD7                 ; fdb loc_0_DAFF
indirect code DADB                 ; fdb loc_0_DAF6
indirect code DADF                 ; fdb loc_0_DAED
indirect code DAE3                 ; fdb loc_0_DB30
indirect code DAE7                 ; fdb loc_0_DB89
entry DAED

; -----------------------------------------

indirect code DB37                 ; fdb loc_0_DB75
indirect code DB3B                 ; fdb loc_0_DB7D
indirect code DB3F                 ; fdb loc_0_DB65
indirect code DB43                 ; fdb loc_0_DB6D
indirect code DB47                 ; fdb loc_0_DB55
indirect code DB4B                 ; fdb loc_0_DB5D
indirect code DB4F                 ; fdb loc_0_DB85
entry DB55

; -----------------------------------------

indirect code DBA0                 ; fdb loc_0_DC09
indirect code DBA4                 ; fdb loc_0_DC00
indirect code DBA8                 ; fdb loc_0_DBF7
indirect code DBAC                 ; fdb loc_0_DBEE
indirect code DBB0                 ; fdb loc_0_DBE5
indirect code DBB4                 ; fdb loc_0_DBDC
indirect code DBB8                 ; fdb loc_0_DBD3
indirect code DBBC                 ; fdb loc_0_DBCA
indirect code DBC0                 ; fdb loc_0_DC11
indirect code DBC4                 ; fdb loc_0_DC8B
entry DBCA

; -----------------------------------------

indirect code DC18                 ; fdb loc_0_DC62
indirect code DC1C                 ; fdb loc_0_DC5A
indirect code DC20                 ; fdb loc_0_DC6E
indirect code DC24                 ; fdb loc_0_DC4A
indirect code DC28                 ; fdb loc_0_DC52
indirect code DC2C                 ; fdb loc_0_DC3A
indirect code DC30                 ; fdb loc_0_DC42
indirect code DC34                 ; fdb loc_0_DC87
entry DC3A

; -----------------------------------------

indirect code DCA2                 ; fdb loc_0_DD08
indirect code DCA6                 ; fdb loc_0_DD00
indirect code DCAA                 ; fdb loc_0_DCF8
indirect code DCAE                 ; fdb loc_0_DCF0
indirect code DCB2                 ; fdb loc_0_DCE7
indirect code DCB6                 ; fdb loc_0_DCDE
indirect code DCBA                 ; fdb loc_0_DCD5
indirect code DCBE                 ; fdb loc_0_DCCC
indirect code DCC2                 ; fdb loc_0_DD10
indirect code DCC6                 ; fdb loc_0_DD72
entry DCCC

; -----------------------------------------

indirect code DD17                 ; fdb loc_0_DD5A
indirect code DD1B                 ; fdb loc_0_DD66
indirect code DD1F                 ; fdb loc_0_DD45
indirect code DD23                 ; fdb loc_0_DD4D
indirect code DD27                 ; fdb loc_0_DD35
indirect code DD2B                 ; fdb loc_0_DD3D
indirect code DD2F                 ; fdb loc_0_DD6E
entry DD35

; -----------------------------------------

indirect code DD89                 ; fdb loc_0_DDEB
indirect code DD8D                 ; fdb loc_0_DDE3
indirect code DD91                 ; fdb loc_0_DDDB
indirect code DD95                 ; fdb loc_0_DDD3
indirect code DD99                 ; fdb loc_0_DDCB
indirect code DD9D                 ; fdb loc_0_DDC3
indirect code DDA1                 ; fdb loc_0_DDBB
indirect code DDA5                 ; fdb loc_0_DDB3
indirect code DDA9                 ; fdb loc_0_DDF3
indirect code DDAD                 ; fdb loc_0_DE1E
entry DDB3

; -----------------------------------------

indirect code DE35                 ; fdb loc_0_DE9E
indirect code DE39                 ; fdb loc_0_DE95
indirect code DE3D                 ; fdb loc_0_DE8C
indirect code DE41                 ; fdb loc_0_DE83
indirect code DE45                 ; fdb loc_0_DE7A
indirect code DE49                 ; fdb loc_0_DE71
indirect code DE4D                 ; fdb loc_0_DE68
indirect code DE51                 ; fdb loc_0_DE5F
indirect code DE55                 ; fdb loc_0_DEA7
indirect code DE59                 ; fdb loc_0_DF43
entry DE5F

; -----------------------------------------

indirect code DEAE                 ; fdb loc_0_DF2F
indirect code DEB2                 ; fdb loc_0_DF37
indirect code DEB6                 ; fdb loc_0_DEF4
indirect code DEBA                 ; fdb loc_0_DF02
indirect code DEBE                 ; fdb loc_0_DECC
indirect code DEC2                 ; fdb loc_0_DED4
indirect code DEC6                 ; fdb loc_0_DF3F
entry DECC

; -----------------------------------------

indirect code DF84                 ; fdb loc_0_E2A6
indirect code DF88                 ; fdb loc_0_E135
indirect code DF8C                 ; fdb loc_0_E06D
indirect code DF90                 ; fdb loc_0_E10D
indirect code DF94                 ; fdb loc_0_E0E1
indirect code DF98                 ; fdb loc_0_DFC2
indirect code DF9C                 ; fdb loc_0_DFFA
indirect code DFA0                 ; fdb loc_0_E0B1
indirect code DFA4                 ; fdb loc_0_E0C9
indirect code DFA8                 ; fdb loc_0_E093
indirect code DFAC                 ; fdb loc_0_E018
indirect code DFB0                 ; fdb loc_0_E021
indirect code DFB4                 ; fdb loc_0_E12C
indirect code DFB8                 ; fdb loc_0_E03F
indirect code DFBC                 ; fdb loc_0_E2FB
entry DFC2

; -----------------------------------------

indirect code EB5A                 ; fdb loc_0_EB87
indirect code EB5E                 ; fdb loc_0_EB78
indirect code EB62                 ; fdb loc_0_EB96
indirect code EB66                 ; fdb loc_0_EBA5
indirect code EB6A                 ; fdb loc_0_EBB4
indirect code EB6E                 ; fdb loc_0_EBC3
indirect code EB72                 ; fdb loc_0_EBD7
entry EB78

; =============================================================================

entry C0AF

entry F38D
entry F396

entry F508
entry F5FB
entry F751
entry F875
entry F92C

; =============================================================================

; MPU Ports (Non-Relocated):

label 103D INITREG

; MPU Ports (Relocated):

label A000 PORTA
label A001 DDRA
label A002 PIOC
label A003 PORTC
label A004 PORTB
label A005 PORTCL
label A006 DDRB
label A007 DDRC
label A008 PORTD
label A009 DDRD
label A00A PORTE
label A00B CFORC
label A00C OC1M
label A00D OC1D
label A00E TCNT
label A010 TIC1
label A012 TIC2
label A014 TIC3
label A016 TOC1
label A018 TOC2
label A01A TOC3
label A01C TOC4
label A01E TOC5
label A020 TCTL1
label A021 TCTL2
label A022 TMSK1
label A023 TFLG1
label A024 TMSK2
label A025 TFLG2
label A026 PACTL
label A027 PACNT
label A028 SPCR
label A029 SPSR
label A02A SPDAT
label A02B BAUD
label A02C SCCR1
label A02D SCCR2
label A02E SCSR
label A02F SCDAT
label A030 ADCTL
label A031 ADR1
label A032 ADR2
label A033 ADR3
label A034 ADR4
label A035 BPROT
;label A036 *
;label A037 *
;label A038 *
label A039 OPTION
label A03A COPRST
label A03B PPROG
label A03C HPRIO
label A03D INIT
label A03E TEST1
label A03F CONFIG

; =============================================================================

label F356 X_CALL_L09
ExitFunction F370 X_RET_L09
ExitFunction F948 VI_SWITCH_L06
ExitFunction F92C S_SWITCH_L06

entry F90B

