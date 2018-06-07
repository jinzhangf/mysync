all:
	g++ mysync.cpp -o mysync
	g++ lx.cpp -o lx
	cp *.exe /home/dylanfang/bin/ && rm *.exe 

clean:
	rm *.exe
