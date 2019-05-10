all:
	g++ mysync.cpp -o mysync --std=c++11
	g++ lx.cpp -o lx --std=c++11
	#cp *.exe /home/dylanfang/bin/ && rm *.exe 
	mv mysync /Users/linfu/bin
	mv lx /Users/linfu/bin

clean:
	rm *.exe
