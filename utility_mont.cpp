#ifndef UTILITY_MONT_CPP
#define UTILITY_MONT_CPP

#include "utility_mont.hpp"
using namespace Swombat;

//constructor for the machine
monter::monter() {
    opTable["exit"] = {0, 0};
    opTable["loadi"] = {2, 1};
    opTable["storei"] = {2, 2};
    opTable["add"] = {2, 3};
    opTable["subtract"] = {2, 4};
    opTable["multiply"] = {2, 5};
    opTable["divide"] = {2, 6};
    opTable["jump"] = {1, 7};
    opTable["jmpz"] = {2, 8};
    opTable["jmpn"] = {2, 9};
    opTable["move"] = {2, 10};
    opTable["load"] = {2, 11};
    opTable["store"] = {2, 12};
    opTable["loadc"] = {2, 13};
    opTable["clear"] = {1, 14};
    opTable["negate"] = {2, 15};
    opTable["push"] = {1, 16};
    opTable["pop"] = {1, 17};
    opTable["addi"] = {2, 18};
    opTable["call"] = {1, 19};
    opTable["return"] = {0, 20};
    opTable[".data"] = {-1, -1};
    opTable[".data"] = {2, -1};
} /*monter constructor*/

monter::~monter(){
    inputFile.close();
    outputFile.close();
    opTable.clear();
} /*monter destructor*/

//get name string, read and return file
void monter::ParseFileR(std::string filename){
    try{
        inputFile.open(filename, std::ios::in);
    } catch(...){
        std::cerr << "Could not open file" << std::endl;
    }
} /*ParseFile to Read*/

//get a string, return writeable file object
void monter::ParseFileW(std::string filename){
    try {
        outputFile.open(filename + ".o", std::ios::out);
    } catch(...){
        std::cerr << "Could not open file" << std::endl;
    }

} /*ParseFile to Write*/

//pre-process the labels
void monter::BuildAddressTable() {
    std::string output;
    std::regex labelFormat("^[ \t]*(_[^:]+):");
    std::smatch label;
    std::regex xternFormat("\\.extern");
    std::smatch xtern;
    while(getline(inputFile, output)) {
        //ilc aumenta em 4 a cada linha
        if(std::regex_search(output, label, labelFormat)){
            addressTable[label[1]] = ilc;
        }
        if(std::regex_search(output, xtern, xternFormat)){
            ilc-=2;
        }
        ilc += 2;
    }
} /*BuildAddressTable*/

//get file, calls ParseLine in each line
void monter::ParseLines(){
    std::string output;
    std::regex xternFormat("\\.extern");
    std::smatch xtern;
    inputFile.clear();
    inputFile.seekg(0, std::ios::beg);
    while(getline(inputFile, output)) {
        if(!std::regex_search(output, xtern, xternFormat))
            ParseLine(output, " \n");
    }
}

//interpreta cada linha
void monter::ParseLine(std::string &line, std::string delimiters) {
    size_t current, next = -1, first_space = -1;
    std::map<std::string, std::vector<int>>::iterator dupla;
    std::string word, label, operation, op1, op2;
    int numOperands = 0, opCode = 0, flag_operation = 0, flag_op1 = 0;

    //instruction é tudo entre uma label e um comentário
    std::regex expression("^[ \t]*(?:_[^:]+:[ \t]*)?([^;]+)");
    std::smatch instruction;
    std::regex_search(line, instruction, expression);

        //tokeniza
        do {
            current = next + 1;
            next = instruction[1].str().find_first_of(delimiters, current);
            word = instruction[1].str().substr(current, next - current);
        }while(word.empty());

        //pesquisa o opCode e o numOperands na table
        dupla = opTable.find(word);
        numOperands = dupla->second[0];
        opCode = dupla->second[1];

        //pega o resto dos operandos
        if (numOperands > 0) {
            do {
            current = next + 1;
            next = instruction[1].str().find_first_of(delimiters, current);
            word = instruction[1].str().substr(current, next - current);
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

void monter::Translate(int opCode, int numOperands, std::string op1, std::string op2){
    int op1Value = 0, op2Value = 0;
    std::string out1, out2 = "00000000";
    std::map<std::string, int>::iterator it;
    //se for um .data
    if(opCode == -1){
        std::cout << std::stoi(op2) << std::endl;
        op2Value = std::stoi(op2);
        out2 = std::bitset<8>(op2Value).to_string();
        opCode = 0;
        op1Value = 0;
    }
    else if(numOperands >= 1){
        //possibilidades para o primeiro operando
        //
        //registrador
        if(op1.at(0) == 'A'){
            op1Value = (int)(op1.at(1)) - 48;
        }
        //label
        else if(op1.at(0) == '_'){
            out2 = op1;
        }
        if(numOperands == 2){
            //registrador
            if(op2.at(0) == 'A'){
                op2Value = (int)(op2.at(1)) - 48;
                op2Value = op2Value <<5;
            }
            //IO
            else if(op2.at(0) == 'I' && op2.at(1) == 'O')
                op2Value = 254;
            //immediate
            else if(op2.at(0) != '_'){
                op2Value = std::stoi(op2);
            }
            out2 = std::bitset<8>(op2Value).to_string();
            //label
            if(op2.at(0) == '_'){
            out2 = op2;
            }
        }
    }
    int c = opCode <<3;
    c += op1Value;
    //std::cout << op1Value << " values " << op2Value << std::endl;
    out1 = std::bitset<8>(c).to_string();
    Print(out1, out2);
}


void monter::Print(std::string out1, std::string out2){
    outputFile << out1 << std::endl;
    memCount++;
    
    outputFile << out2 << std::endl;
    memCount++;
}

void monter::AddInfo(){
    outputFile << "!" << memCount << std::endl;
	std::map<std::string, int>::iterator it;
    for(it = addressTable.begin(); it != addressTable.end(); it++){
    	outputFile << "!" << it->first << " " << it->second << std::endl;
	} 
}

#endif	//UTILITY_MONT_CPP
