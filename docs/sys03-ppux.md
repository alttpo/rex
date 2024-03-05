SUBSYSTEM 03: PPUX
---

TODO: convert to binary protocol sepc

PPU Extensions (ppux):
<ppux-write-cmd>  := <vram-write-cmd>
|| <cgram-write-cmd>

<vram-write-cmd>  := 'pvw' ',' <args>
	write bitmap data to ppux extra VRAM memory
	args:
		address: int
		size:    int
		data:    binary

<cgram-write-cmd> := 'pcw' ',' <args>
	write palette data to ppux extra CGRAM memory
	args:
		address: int
		size:    int
		data:    binary

<ppux-exec-cmd>   := 'px' ';' <exec-cmdseq>
	specifies the drawing command sequence which renders every game frame
<exec-cmdseq> := <cmdseq> where commands are
	's': set render state
		args:
			layer:   int -- PPU layer to draw on
			sub:     bit -- main (0) or sub (1) screen
			replace: bit -- overlay (0) or replace (1) pixels
	'p': set palette address in extra CGRAM
		args:
			cgram-addr: int
	'o': set x/y offset pointers
		repeated
		zero args means clear offsets
		each addrx,addry pair points to the addresses of a pair of 16-bit unsigned ints in the memory chip to read and then use to subtract from subsequent x,y drawing coords
		order matters because subtraction is used
		args:
			(
				chip:   str
				addrx:  int
				addry:  int
			)*
	'b': draw 16bpp bitmap with 15bpp direct color
		ignores palette address
		args:
			x:      int
			y:      int
			width:  int
			bitmap: binary
	'2','4','8': draw n-bpp tile from extra VRAM
		args:
			x:         int
			y:         int
			width:     int
			height:    int
			flip-h:    bit
			flip-v:    bit
			vram-addr: int
