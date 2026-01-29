main:
	g++ -std=c++17 src/main.cpp src/detection/detection.cpp -o savemanager -lzip

mac:
	g++ -std=c++17 src/main.cpp src/detection/detection.cpp -o savemanager \
	-I/opt/homebrew/include -L/opt/homebrew/lib -lzip
