	        ; Ordering of segments for the linker.
	        ; WRS: Note we list all our segments here, even though
	        ; we don't use them all, because their ordering is set
	        ; when they are first seen.	
	        .area _CODE
	        .area _CODE2
		.area _VIDEO
	        .area _CONST
	        .area _INITIALIZED
	        .area _DATA
	        .area _BSEG
	        .area _BSS
	        .area _HEAP
	        .area _GSINIT
	        .area _GSFINAL
	        .area _COMMONMEM
		.area _DISCARD
	        ; note that areas below here may be overwritten by the heap at runtime, so
	        ; put initialisation stuff in here
	        .area _INITIALIZER

        	; imported symbols
        	.globl _fuzix_main
	        .globl init_early
	        .globl init_hardware
	        .globl s__DATA
	        .globl l__DATA
	        .globl s__DISCARD
	        .globl l__DISCARD
	        .globl s__COMMONMEM
	        .globl l__COMMONMEM
		.globl s__INITIALIZER
	        .globl kstack_top

		.globl _ubee_model

	        ; startup code
	        .area _CODE

;
;	Once the loader completes it jumps here. We are loaded between
;	low memory and DFFF. Above us at this moment is ROM and VRAM is
;	at F800 overlapping the top of RAM
;
;	There are lots of uBee's we might have been loaded on but we
;	only have to care about those that have enough memory (128K+_
;
		.word 0xC0DE
start:
		di
		ld sp, #kstack_top
		;
		; Figure out our hardware type. We need to work this out
		; before we can shuffle the memory map and set up video
		;
		; Start with the easy case (we expect our loader to preserve
		; these - we might need to move the model detection and
		; support check into the loader
	        ld a, (0xEFFD)
		cp #'2'
	        jr nz, not256tc
		ld a, (0xEFFE)
		cp #'5'
	        jr nz, not256tc
		ld a, (0xEFFF)
		cp #'6'
	        jr nz, not256tc
		ld a,#2
		ld (_ubee_model),a
		; Do we need to touch ROM etc - not clear we need do
		; anything as we are already in RAM mode. Turn on video
		; mapping at Fxxx just while we boot up so we can poke it
		; for debug. Some day we can map the video just for
		; the video writes.
		ld a,#0x04
		out (0x50),a
		jr relocate

not256tc:	; The uBee might have colour support
		ld hl, (0xF7FF)
		ld (hl),#0x90		; zero the last byte
		ld a, #0x10
		out (0x1C),a		; attribute latch
		ld (hl),#0xFF		; set to 0xFF
		ld a,#0x00		; back to video
		out (0x1c),a
		ld a,(hl)
		ld (_ubee_model),a	; 1 - premium
		or a
		jr z, unsupported_model	; 128K required and premium
		ld a,#0x04		; ROMs off video at Fxxx
		out (0x50),a

		; FIXME: support SBC (128K non premium 5.25 or 3.5
		; drives, flicker on video writes)

relocate:
		;
		; move the common memory where it belongs    
		ld hl, #s__DATA
		ld de, #s__COMMONMEM
		ld bc, #l__COMMONMEM
		ldir
		; then the discard
		ld de, #s__DISCARD
		ld bc, #l__DISCARD
		ldir
		; then zero the data area
		ld hl, #s__DATA
		ld de, #s__DATA + 1
		ld bc, #l__DATA - 1
		ld (hl), #0
		ldir

		call init_early
		call init_hardware
		call _fuzix_main
		di
stop:		halt
		jr stop

;
;	We still have ROM at this point
;
unsupported_model:
	        ld hl,#unsupported_txt
	        call 0xE033
	        jp 0xE000
unsupported_txt:
		.ascii "Unsupported platform"
		.byte 0x80

_ubee_model:	.byte 0
