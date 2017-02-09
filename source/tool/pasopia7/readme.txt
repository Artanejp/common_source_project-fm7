Dump BIOS ROM

	100 data &hf5, &hdb, &h22, &he6, &h03, &hf5, &hf6, &h01
	110 data &hd3, &h3c, &h3a, &h00, &h00, &h32, &h00, &h00
	120 data &hf1, &hd3, &h3c, &hf1, &hc9
	130 clear ,&hdfff
	140 for a=&he000 to &he014
	150  read d:poke a,d
	160 next a
	170 sum=0
	180 for u=0 to &hf
	190  print u
	200  poke &he00c,&h40+u		// 40,50,60,70
	210  poke &he00f,&he1+u
	220  for l=0 to &hff
	230   poke &he00b,l
	240   poke &he00e,l
	250   a=&he000:call a
	260   sum=sum+peek(&he100+u*256+l)
	270   sum=sum and &hff
	280  next l
	290 next u
	300 print sum

	Run to copy the bios image to $e100
	bsave #-1,"name",&he100,&h100
	Record the data recorder sound to wav file (48KHz/8bit/mono)
	Change &h40 in line.200 to &h50,&h60,&h70 and record again

	Convert each wav file to binary with wav2bin and convine them

Dump BASIC ROM

	100 data &hf5, &hdb, &h22, &he6, &h03, &hf5, &he6, &h02
	200  poke &he00c,&h00+u		// 00,10,20,30,40,50,60,70

	Change &h00 in line.200 to &h10,&h20,&h30,&h40,&h50,&H60,&h70

