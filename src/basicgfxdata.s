;
; BASIC-style GFX for drawing the playfield.
;
; in the future i hope to swap this out for something that will render 2x2 tiles
; and make everything continue to work
;
.PSC02

.EXPORT _print_gr

CHROUT = $FFD2

; assumes C64 kernal
ROWSTART = 209
SCRCOL = 214

COLROWSTART = 251

COLOR_RAM = 55296

; this routine will print a baba tile
; at the current kernal cursor position

_print_gr:
	; store starting index x=a*4;
	ASL
	ASL
	TAX
	; create color ram vector
	LDA ROWSTART
	STA COLROWSTART
	LDA ROWSTART+1
	AND #$F3
	ORA #>COLOR_RAM
	STA COLROWSTART+1
	LDY SCRCOL
.REPEAT 4
	LDA GFXSCREEN,X
	STA (ROWSTART),Y
	LDA GFXCOLOR,X
	STA (COLROWSTART),Y
	INX
	INY
.ENDREPEAT
	CPY #40
	BCC DONT_ADVANCE
	TYA
	SBC #40
	TAY
	CLC ; ???
	LDA ROWSTART
	ADC #40
	STA ROWSTART
	LDA ROWSTART+1
	ADC #0
	STA ROWSTART+1
	INC 211 ; kernal row number
DONT_ADVANCE:
	STY SCRCOL
	RTS

.RODATA
; GRAPHICS

GFXSCREEN:
	.BYTE 46 , 46 , 46 , 46
	.BYTE 46 , 90 , 90 , 46
	.BYTE 108 , 247 , 247 , 123
	.BYTE 242 , 241 , 242 , 241
	.BYTE 32 , 78 , 120 , 32
	.BYTE 47 , 47 , 47 , 47
	.BYTE 32 , 87 , 114 , 32
	.BYTE 207 , 160 , 173 , 208
	.BYTE 32 , 160 , 184 , 32
	.BYTE 2 , 1 , 2 , 1
	.BYTE 18 , 15 , 3 , 11
	.BYTE 23 , 1 , 12 , 12
	.BYTE 6 , 12 , 1 , 7
	.BYTE 23 , 1 , 20 , 18
	.BYTE 11 , 5 , 25 , 32
	.BYTE 4 , 15 , 15 , 18
	.BYTE 153 , 143 , 149 , 160
	.BYTE 151 , 137 , 142 , 161
	.BYTE 147 , 148 , 143 , 144
	.BYTE 144 , 149 , 147 , 136
	.BYTE 147 , 136 , 149 , 148
	.BYTE 143 , 144 , 133 , 142
	.BYTE 147 , 137 , 142 , 139
	.BYTE 140 , 143 , 147 , 133
	.BYTE 136 , 143 , 148 , 160
	.BYTE 141 , 133 , 140 , 148
	.BYTE 160 , 178 , 182 , 160
	.BYTE 160 , 178 , 183 , 160
	.BYTE 160 , 178 , 184 , 160
	.BYTE 160 , 178 , 185 , 160
	.BYTE 8 , 1 , 19 , 32
	.BYTE 32 , 9 , 19 , 32

.macro NOUN_COLOR
	.BYTE 1,1,1,1 ; all white
.endmacro
.macro PROP_COLOR
	.BYTE 3,3,3,3 ;all cyan
.endmacro

GFXCOLOR:
	.BYTE 0 , 0 , 0 , 0
	.BYTE 1 , 1 , 1 , 1
	.BYTE 15 , 15 , 15 , 15
	.BYTE 8 , 8 , 8 , 8
	.BYTE 7 , 7 , 7 , 7
	.BYTE 14 , 14 , 14 , 14
	.BYTE 7 , 7 , 7 , 7
	.BYTE 2 , 2 , 2 , 2
	.BYTE 2 , 2 , 2 , 2
	NOUN_COLOR
	NOUN_COLOR
	NOUN_COLOR
	NOUN_COLOR
	NOUN_COLOR
	NOUN_COLOR
	NOUN_COLOR
	PROP_COLOR
	PROP_COLOR
	PROP_COLOR
	PROP_COLOR
	PROP_COLOR
	PROP_COLOR
	PROP_COLOR
	PROP_COLOR
	PROP_COLOR
	PROP_COLOR
	PROP_COLOR
	PROP_COLOR
	PROP_COLOR
	PROP_COLOR
	.BYTE 1 , 1 , 1 , 1
	.BYTE 1 , 1 , 1 , 1

