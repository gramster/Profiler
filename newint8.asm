	name	newint

;***********************************************
;                                              *
;       Written by G. Wheeler, July 1989       *
;                                              *
;***********************************************

PSP_size        equ     100h
EOI             equ     20h
ClkDivHi        equ     02h
ClkDivLo        equ     00h
DOS_Service     equ     21h
TSR             equ     27h
MagicHi         equ     4752h
MagicLo         equ     414Dh
TimerModes      equ     43h
Timer0Mode      equ     40h
CounterLoc      equ     512h    ; Counter loc; only used by BASIC so ok



_TEXT	segment	byte public 'CODE'
	assume	cs:_TEXT,ds:_TEXT

startlbl:

	jmp start		; jmp initial


counter_low:	dw 0
counter_high:	dw 0
oldint8:	dw 0, 0
magic:          dw MagicLo, MagicHi

newint8:
        push    es
	push    ds
        push    ax
        push    bx

        xor     bx, bx
        mov     es, bx
        mov     bx, CounterLoc
        add     word ptr es:[bx], 1
        adc     word ptr es:[bx+2], 0

   	mov     ax, word ptr es:[bx]
	and     al, 07Fh
	jnz     nochain
chain:
        pop     bx
	pop     ax
	pop     ds
        pop     es
	jmp     dword ptr cs:oldint8	; jmp oldint8

nochain:
	mov     al, EOI
	out     20h, al

        pop     bx
	pop     ax
	pop     ds
        pop     es
	iret

endlbl:

;***********************************************

start:
	mov ax,3508h		        ; get and save old int 8
	int DOS_Service
	mov word ptr cs:oldint8,bx	
	mov word ptr cs:oldint8+2,es

check:                                  ; MAGIC NUMBER CHECK
        add bx, magic-newint8           ; Move ptr from oldint8 to oldmagic
        push es
        pop dx
        adc dx, -1
        push dx
        pop es

        cmp word ptr es:[bx+2], MagicHi ; See if oldmagic is the same
        jne install
        cmp word ptr es:[bx], MagicLo
        je uninstall

install:
	mov dx, offset newint8	; Install new int 8
	push cs
	pop ds
	mov ax, 2508h		
	int DOS_Service

	mov al, 34h             ; Zap clock up
	out TimerModes, al
	nop

	mov al, ClkDivLo
	out Timer0Mode, al
	nop

	mov al, ClkDivHi
	out Timer0Mode, al
        jmp finish

uninstall:
        add bx, oldint8 - magic ; Move ptr back from magic to oldint8
        push es
        pop dx
        adc dx, -1
        push dx
        pop es

        mov dx, word ptr es:[bx]
        mov ds, word ptr es:[bx + 2]    ; get old int 8 from previous copy
	mov ax, 2508h		        ; restore old int 8
	int DOS_Service


	mov al, 34h                     ; Zap clock back down
	out TimerModes, al
	nop

	mov al, -1
	out Timer0Mode, al
	nop

	mov al, -1
	out Timer0Mode, al
        
;        int 20h                         ; Exit to DOS (crashes)

finish:
	mov dx,endlbl-startlbl+PSP_Size ; size of mem to keep	
	int TSR 			; terminate-stay-resident

_TEXT	ends
	end

