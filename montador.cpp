#ifndef UTILITY_CPP
#define UTILITY_CPP

#include "swombat.hpp"
using namespace Swombat;

//constructor for the machine
swombat::swombat() {
    opTable["stop"] = {0, 0};
    opTable["load"] = {2, 1};
    opTable["store"] = {2, 2};
    opTable["read"] = {1, 3};
    opTable["write"] = {1, 4};
    opTable["add"] = {2, 5};
    opTable["subtract"] = {2, 6};
    opTable["multiply"] = {2, 7};
    opTable["divide"] = {2, 8};
    opTable["jump"] = {1, 9};
    opTable["jmpz"] = {2, 10};
    opTable["jmpn"] = {2, 11};
    opTable["move"] = {2, 12};
    opTable["push"] = {1, 13};
    opTable["pop"] = {1, 14};
    opTable["call"] = {1, 15};
    opTable["return"] = {0, 0x8000};
    opTable["load_s"] = {2, 17};
    opTable["store_s"] = {2, 18};
    opTable["loadc"] = {2, 19};
    opTable["loadi"] = {2, 20};
    opTable["storei"] = {2, 21};
    opTable["copytop"] = {1, 22};
    opTable["clear"] = {1, 14};
    opTable["negate"] = {2, 15};
    opTable["addi"] = {2, 18};
    opTable[".data"] = {-1, -1};
    opTable[".data"] = {2, -1};
} /*swombat constructor*/

swombat::~swombat(){
    inputFile.close();
    outputFile.close();
    opTable.clear();
} /*swombat destructor*/

//get name string, read and return file
void swombat::ParseFileR(std::string filename){
    try{ 
        inputFile.open(filename, std::ios::in);
    } catch(...){
        std::cerr << "Could not open file" << std::endl;
    }
} /*ParseFile to Read*/

//get a string, return writeable file object
void swombat::ParseFileW(std::string filename){
    try {
        outputFile.open(filename + ".mif", std::ios::out);
    } catch(...){
        std::cerr << "Could not open file" << std::endl;
    }

} /*ParseFile to Write*/

//pre-process the labels
void swombat::BuildAddressTable() {
    std::string output;
    std::regex labelFormat("^[ \t]*(_[^:]+):");
    std::smatch label;
    std::regex dataFormat("\\.data[ ]*([0-9]*)[ ][0-9-]*");
    std::smatch data;
    while(getline(inputFile, output)) {
        //ilc aumenta em 2 a cada linha
        if(std::regex_search(output, label, labelFormat)){
            addressTable[label[1]] = ilc;
        }
        ilc += 2;
    }
} /*BuildAddressTable*/

//get file, calls ParseLine in each line
void swombat::ParseLines(){
    std::string output;
    inputFile.clear();
    inputFile.seekg(0, std::ios::beg);
    while(getline(inputFile, output)) {
        ParseLine(output, " \n");
    }
}

//interpreta cada linha
void swombat::ParseLine(std::string &line, std::string delimiters) {
    size_t current, next = -1, first_space = -1;
    std::map<std::string, std::vector<int>>::iterator dupla;
    std::string word, label, operation, op1, op2, stripped;
    int numOperands = 0, opCode = 0, flag_operation = 0, flag_op1 = 0;

    //instruction é tudo entre uma label e um comentário
    std::regex expression("^[ \t]*(?:_[^:]+:[ \t]*)?([^;]+)");
    std::smatch instruction;
    std::regex_search(line, instruction, expression);

    stripped = std::regex_replace(instruction[1].str(), std::regex("\\s+$"), std::string(""));
    
        //tokeniza
        do {
            current = next + 1;
            next = stripped.find_first_of(delimiters, current);
            word = stripped.substr(current, next - current);
        }while(word.empty());
       
        
	//pesquisa o opCode e o numOperands na table
        dupla = opTable.find(word);
        numOperands = dupla->second[0];
        opCode = dupla->second[1];
        
        //pega o resto dos operandos
        if (numOperands > 0) {
            do {
            current = next + 1;
            next = stripped.find_first_of(delimiters, current);
            word = stripped.substr(current, next - current);
            }while(word.empty());
            op1 = word;
            if(numOperands > 1){
                do {
                current = next + 1;
                next = instruction[1].str().find_first_of(delimiters, current);
                word = instruction[1].str().substr(current, next - current);
                }while(word.empty());
                op2 = word;
            }
        }
        Translate(opCode, numOperands, op1, op2);

} /*ParseLine*/
        
void swombat::Translate(int opCode, int numOperands, std::string op1, std::string op2){
    int op1Value = 0, op2Value = 0;
    std::map<std::string, int>::iterator it;
    //se for um .data
    if(opCode == -1){
        op2Value = std::stoi(op2) % 16;
        opCode = 0;
    	op1Value = op2Value >> 8;
    }
    else if(opCode == 0x8000){
    	//
	op1Value = 0b10000000;
    }
    else if(numOperands >= 1){
        //possibilidades para o primeiro operando
        //
        //registrador
        if(op1.at(0) == 'A'){
            op1Value = (int)(op1.at(1)) - 48;
        }
        //label -> se houver uma label aqui, não tem op2
        else if(op1.at(0) == '_'){
            it = addressTable.find(op1);
            op1Value = it->second;
        }
	//immediate
	else{
	    op1Value = std::stoi(op2);
	}

        if(numOperands == 2){
            //registrador
            if(op2.at(0) == 'A'){
                op2Value = (int)(op2.at(1)) - 48;
                op2Value = op2Value;
            }
            //label
            else if(op2.at(0) == '_'){
            it = addressTable.find(op2);
            op2Value = it->second;
            }
            //immediate
            else{
                op2Value = std::stoi(op2);
            }
			
	    op1Value = op1Value << 1;
        }
	else{
	// se não tem um op2, o espaço é 11 bits, tem que trocar pra printar
	    op2Value = op1Value % 0x100000000;
	    op1Value = op1Value >> 10;
	}
    }
    
    int c = opCode <<3;
    c += op1Value;
    
    Print(std::bitset<8>(c).to_string(), std::bitset<8>(op2Value).to_string());
}

void swombat::Begin(){
    outputFile << "DEPTH = 128;" << std::endl;
    outputFile << "WIDTH = 8;" << std::endl;
    outputFile << "ADDRESS_RADIX = HEX;" << std::endl;
    outputFile << "DATA_RADIX = BIN;" << std::endl;
    outputFile << "CONTENT" << std::endl;
    outputFile << "BEGIN\n" << std::endl;
}
void swombat::End(){
    outputFile << "[" << std::uppercase << std::hex << memCount << ".." << std::uppercase << std::hex << ram-1 << "]: 00000000;" << std::endl; 
    outputFile << "END;";
}

void swombat::Print(std::string out1, std::string out2){
	char prev;
	prev = outputFile.fill('0');
	outputFile.width(2);

    outputFile << std::uppercase << std::hex << memCount << " : ";
	outputFile.fill(prev);    
    outputFile << out1 << ";" << std::endl;
    memCount++;
	
	prev = outputFile.fill ('0');
	outputFile.width(2);
    outputFile << std::uppercase << std::hex << memCount << " : ";
	outputFile.fill(prev);    
    outputFile << out2 << ";" << std::endl;
    memCount++;

}
#endif	//UTILITY_CPP
