#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <map>
#include <iomanip>
#include <vector>
#include <sstream>

using namespace std;




void generateTABCOP(fstream&, fstream&, const map<string,map<string,string>>&);
void generateLST(fstream&, fstream&, fstream&, const map<string,map<string,string>>&);
void generateTABSIM(fstream&, fstream&, const map<string,map<string,string>>&);

map<string,map<string,string>> instructions;


int main(){
    fstream file("P5.asm");
    fstream tbcFile("P5.TABCOP",ios::out);
    fstream lstFile("P5.LST",ios::out);
    string line,label,mnemonic,symbol,value;

    //ABA Instruction
    instructions["ABA"]["INHERENT"] = "1806";

    //ADCA Instruction
    instructions["ADCA"]["INMEDIATE"] = "89  ";
    instructions["ADCA"]["DIRECT"] = "99  ";
    instructions["ADCA"]["EXTENDED"] = "B9    ";

    //ADDD Instruction
    instructions["ADDD"]["INMEDIATE"] = "C3    ";
    instructions["ADDD"]["DIRECT"] = "D3  ";
    instructions["ADDD"]["EXTENDED"] = "F3    "; // 
  
    generateTABCOP(file, tbcFile, instructions);
    generateLST(file, tbcFile, lstFile, instructions);
    generateTABSIM(file, lstFile,instructions);
    

}

void generateTABCOP(fstream &asmFile, fstream &tbcFile, const map<string,map<string,string>>& ins){
    string line,mnemonic,symbol,value,MD,COP,PC;
    int LI;
    regex inhPattern("^\\s*(\\w+)\\s*$");
    regex normPattern("^\\s*(\\w+)\\s+(#|$|@)?([0-9]+)\\s*$");
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

            if(value.length() > 2){
                MD = "EXT";
                COP = instructions[mnemonic]["EXTENDED"];
            }
            else if(symbol.empty()){
                MD = "DIR";
                COP = instructions[mnemonic]["DIRECT"];
            }
            else if(symbol == "#"){
                MD = "INM";
                COP = instructions[mnemonic]["INMEDIATE"];
            }

            LI = COP.length() / 2;

            tbcFile << mnemonic << "\t" << MD << "\t\t" << COP << "\t" << LI << endl;

        }
             
    }
    asmFile.close();
    tbcFile.close();
}



void generateLST(fstream &asmFile, fstream &tbcFile, fstream &lstFile, const map<string,map<string,string>> &ins){
    string line,ORG,orgValue,orgSymbol,mnemonic,COP,MD;
    vector<string> values, COPvalues;
    int LI,PC,i=0;
    smatch match;
    regex orgPattern("^\\s*(ORG)\\s+(\\$|@|%)*([0-9]+)$");
    regex normPattern("^\\s*(\\w+)\\s+(#|$|@)?([0-9]+)\\s*$");
    regex tbcPattern("^\\s*([a-zA-Z]+)\\s+([a-zA-Z]+)\\s+([a-zA-Z0-9]+\\s*)\\s*([0-9]?)$");
    asmFile.open("P5.asm");
    tbcFile.open("P5.TABCOP");
    
    //Getting the PC value
    while(!asmFile.eof()){
        getline(asmFile,line);
        if(regex_search(line,match,orgPattern)){
            ORG = match[1].str();
            orgSymbol = match[2].str();
            orgValue = match[3].str();
        }
        //Getting the instruction value operand and storing in a vector
        else if(regex_search(line,match,normPattern)){
            values.push_back(match[3].str());  
        }
        
    }
    asmFile.close();
    PC = stoi(orgValue);
    lstFile << "PC\t\t\t\t" << "COP\t\t" << "\t\t\tMnemonic" << endl;

    //Reading .TABCOP for generating .LST
    while(!tbcFile.eof()){
        getline(tbcFile,line);
        
        if(regex_search(line,match,tbcPattern)){
            mnemonic = match[1].str();
            MD = match[2].str();
            COP = match[3].str();
            LI = stoi(match[4].str());
            
            if(MD == "INH"){
                lstFile << PC << "\t\t\t" << COP << "\t\t\t" << mnemonic << endl;
                PC += LI;
            }
            else{
                for(int i=0; i<COP.length(); i++){
                    if(COP[i] == ' '){
                        COP.erase(i);
                    }
                }
                string val = values[i];
                stringstream hexValue;
                hexValue << hex << stoi(val);
                string sHexValue = hexValue.str();
                if(sHexValue.length()%2 != 0){COP.append("0");}
                cout<<sHexValue<<endl;
                COP.append(sHexValue);

                lstFile << PC << "\t\t\t" << COP << "\t\t\t\t" << mnemonic << endl;
                PC += LI;
                i++;
            }
            



        }
    }
    lstFile.close();
    tbcFile.close();
    asmFile.close();
}


void generateTABSIM(fstream &asmFile, fstream &lstFile, const map<string,map<string,string>> &ins){
    asmFile.open("P5.asm");
    lstFile.open("P5.LST");
    fstream tbsFile("P5.TABSIM", ios::out);
    string line,label,PC;
    int LI;
    regex inhPattern("^\\s*(\\w+)\\s*$");
    regex normPattern("^\\s*(\\w+)\\s+(#|$|@)?([0-9]+)\\s*$");
    regex labelPattern("^\\s*([0-9a-zA-Z]+)");
    smatch match;
    
    tbsFile << "LABEL\t\t" << endl;

    //Getting the label from the asm file
    while(!asmFile.eof()){
        getline(asmFile,line);
        if(regex_search(line,match,labelPattern)){
            label = match[1].str();
        }
    }
    tbsFile << label << "=" << "4014" << endl;

}
