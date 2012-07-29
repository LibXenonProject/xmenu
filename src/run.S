.section ".elfldr"

.globl run
run:
	lis %r3, 0x9000
	rldicr %r3, %r3, 32, 31
	ori %r3, %r3, 0x1000
	mtsrr1 %r3

	li %r3, 0
	oris %r3, %r3, 0x1b00
	ori %r3, %r3, 1f - run

	mtsrr0 %r3

	rfid


waitcpu:
	lis %r3, 0x1b00
	lwz %r4, -4(%r3)
	cmpwi %r4, 0
	beq waitcpu
	mtctr %r4
	bctr

1:
	mfspr %r4, 1023
	cmpwi %r4, 0
	bne waitcpu

	li %r3, 0
	oris %r3, %r3, 0x1C00

	subis %r1, %r3, 1

	bl load_elf_image

	mtctr %r3
	ori %r3, %r3, 0x60
	lis %r4, 0x1b00
	stw %r3, -4(%r4)

	bctr

.globl rputc
rputc:
	lis %r4, 0x8000
	ori %r4, %r4, 0x200
	rldicr  %r4, %r4, 32,31
  oris  %r4, %r4, 0xea00

  slwi %r3, %r3, 24
  stw %r3, 0x1014(%r4)
1:
  lwz %r3, 0x1018(%r4)
  rlwinm. %r3, %r3, 0, 6, 6
  beq 1b

  blr

.globl puthex
puthex:
	mflr 0
	mr 30,3
	li 31,0
.L5:
	rldicl 9,30,4,60
	addi 31,31,1
	sldi 30,30,4
	addi 3,9,55+32
	cmpdi 7,9,9
	bgt 7,.L3
	addi 3,9,48
.L3:
	bl rputc
	cmpdi 7,31,16
	bne 7,.L5
	mtlr 0
	blr
