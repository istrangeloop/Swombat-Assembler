#ifndef	COMPILER_CPP
#define COMPILER_CPP

#include "utility.hpp"

using namespace Swombat;

int main(int argc, char *argv[]) {
	if(argc != 3) 
		std::cerr << "Incorrect parameters" << std::endl;

	std::string filename_input, filename_output;
    swombat s;
	
	filename_input = argv[1];
	filename_output = argv[2];

	try{
        s.ParseFileR(filename_input);
    }
    catch(...){
        std::cerr << "Could not open "<< filename_input << " for reading" << std::endl;
    }

	try{
        s.ParseFileW(filename_output);
	}
    catch(...){
        std::cerr << "Could not open " << filename_output << "for writing" << std::endl;
    }
    //ordem de execução
    //print beginning of file
    s.Begin();
    s.BuildAddressTable();
    s.ParseLines();
    s.End();
    //print end of file

	/*size_t lastindex = filename_input.find_last_of('.');
	filename_output = filename_input.substr(0, lastindex);*/
    return 0;
}

#endif	//COMPILER_CPP

