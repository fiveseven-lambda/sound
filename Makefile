CFLAGS = -lm
sound:

play: test
	./sound test
	aplay a.wav
