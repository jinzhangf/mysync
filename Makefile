all:
	g++ mysync.cpp -o mysync --std=c++11
	g++ lx.cpp -o lx --std=c++11
	#cp *.exe /home/dylanfang/bin/ && rm *.exe 
	mv mysync ~/bin
	mv lx ~/bin

clean:
	rm -rf mysync mysync.exe lx lx.exe
