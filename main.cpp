#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <map>

using namespace std;

void generateTABCOP(fstream&, fstream&, const map<string,map<string,string>>&);
void generateLST(const fstream&, fstream&, const map<string,map<string,string>>&);
void generateTABSIM(const fstream&, fstream&, const map<string,map<string,string>>&);

map<string,map<string,string>> instructions;


int main(){
    fstream file("P5.asm");
    fstream tbcFile("P5.TABCOP",ios::out);
    string line,label,mnemonic,symbol,value;

    //ABA Instruction
    instructions["ABA"]["INHERENT"] = "1806";

    //ADCA Instruction
    instructions["ADCA"]["INMEDIATE"] = "89ii";
    instructions["ADCA"]["DIRECT"] = "99dd";
    instructions["ADCA"]["EXTENDED"] = "B9hhll";

    //ADDD Instruction
    instructions["ADDD"]["INMEDIATE"] = "C3jjkk";
    instructions["ADDD"]["DIRECT"] = "D3dd";
    instructions["ADDD"]["EXTENDED"] = "F3hhll"; 
  
    generateTABCOP(file, tbcFile, instructions);
    

}

void generateTABCOP(fstream &asmFile, fstream &tbcFile, const map<string,map<string,string>>& ins){
    string line,mnemonic,symbol,value,MD,COP;
    int LI;
    regex inhPattern("^\\s*(\\w+)\\s*$");
    regex normPattern("^\\s*(\\w+)\\s+(#|$|@)*([0-9]+)\\s*$");
    smatch match;
    tbcFile << "MN\t\t" << "MD\t\t" << "COP\t\t" << "LI" << endl;

    while(!asmFile.eof()){
        getline(asmFile, line);

        if(regex_search(line,match,inhPattern)){
            mnemonic = match[1].str();
            for(auto it : ins){
                if(mnemonic == it.first){
                    for(auto it2 : it.second){
                        if(it2.first == "INHERENT"){
                            MD = "INH";
                            COP = it2.second;
                            break;
                        }
                    }
                }
            }
            LI = COP.length() / 2;
            tbcFile << mnemonic << "\t\t" << MD << "\t\t" << COP << "\t" << LI << endl;

        }
        else if(regex_search(line,match,normPattern)){
            mnemonic = match[1].str();
            symbol = match[2].str();
            value = match[3].str();

            for(auto it : instructions){
                if(mnemonic == it.first){
                    for(auto it2 : it.second){
                        if(it2.first == "EXTENDED"){
                            MD = "EXT";
                            COP = it2.second;
                            break;
                        }
                        else if(it2.first == "DIRECT"){
                            MD = "DIR";
                            COP = it2.second;
                            break;
                        }
                        else{
                            MD = "INM";
                            COP = it2.second;
                            break;
                        }

                    }
                }
            }
            LI = COP.length() / 2;

            tbcFile << mnemonic << "\t" << MD << "\t\t" << COP << "\t" << LI << endl;

        }
        
        
    }
}
