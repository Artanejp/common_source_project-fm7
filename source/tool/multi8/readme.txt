- how to create rom image

	1. start monitor on real MULTI8.

		MON

	2. output 0000h-7fffh to data recorder.

		*W 0,7FFF

	3. record sound of the data recorder on PC.
	   format must be 8bit, mono, 48kHz.

	4. convert wave file to rom image file on PC.

		> wav2rom *.wav

	5. rename "dest.rom" to "basic.rom".

	6. run "mkrom.exe" to create "kanji.rom".


- how to create data recorder image

	1. record sound of the data recorder on PC.
	   format must be 8bit, mono, 48kHz.

	2. convert wave file to data recorder image file on PC.

		> wav2cas *.wav

	3. the data recorder image "dest.cas" is created.


- when *.cas is not correctly converted

	1. edit "tmp2.txt"

	2. convert "tmp2.txt" to data recorder image.

		> wav2cas tmp2.txt

	"tmp2.txt" format:

		N:	2400Hz pulses * 4, means bit=1
		w:	1200Hz pulses * 2, means bit=0
		[n?]:	2400Hz pulses * ?
		[W?]	1200Hz pulses * ?
		[l,h]	illegal pulse

		1byte data must be NNw????????.

