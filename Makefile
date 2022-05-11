
SRC_FILES_CPP = main.cpp file.cpp

cpp: $(SRC_FILES_CPP)
	g++ -std=c++2a -fconcepts -pthread -o copy_file $^
