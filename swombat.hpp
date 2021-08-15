#ifndef SWOMBAT_HPP
#define SWOMBAT_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <regex>
#define MAXOP 3

namespace Swombat {

	class swombat {
		private:
			int ilc = 0, memCount = 0, ram = 256;
			std::map<std::string, std::vector<int>> opTable;
			std::map<std::string, int> addressTable;
            std::ifstream inputFile;
            std::ofstream outputFile;

			void ParseLine(std::string &line, std::string delimiters);
			int Label(std::string name);
			int Operation(std::string name);
			void TreatOps();
            void Translate(int opCode, int numOperands, std::string op1, std::string op2);
            void Print(std::string out1, std::string out2);
		public:
			swombat();
            ~swombat();
            void BuildAddressTable();
			void ParseLines();
            void ParseFileR(std::string filename);
            void ParseFileW(std::string filename);
            void Begin();
            void End();
	};
}


#endif	//SWOMBAT_HPP
