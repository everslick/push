all:
	# MEGA65
	$(MAKE) -C src m65
	mv src/push-m65.prg .
	$(MAKE) -C src clean
	# ZX Spectrum 32 columns
	$(MAKE) -C src zx32
	mv src/push-zx32.tap .
	$(MAKE) -C src clean
	# ZX Spectrum 64 columns
	$(MAKE) -C src zx64
	mv src/push-zx64.tap .
	$(MAKE) -C src clean
	# Spectrum Next
	$(MAKE) -C src zxn
	mv src/push-zxn.tap .
	$(MAKE) -C src clean
	# Commodore 64 with cc65
	$(MAKE) -C src c64
	mv src/push-c64.prg .
	$(MAKE) -C src clean
	# Commodore 64 with oscar64
	$(MAKE) -C src o64
	mv src/push-o64.prg .
	$(MAKE) -C src clean
	# Atari 800XL
	$(MAKE) -C src xl
	mv src/push-atarixl.prg .
	$(MAKE) -C src clean
	# Atari 800
	$(MAKE) -C src xe
	mv src/push-atari.prg .
	$(MAKE) -C src clean
	# POSIX
	$(MAKE) -C src
	mv src/push .
	$(MAKE) -C src distclean

clean:
	$(RM) push*
	$(RM) -rf foo
