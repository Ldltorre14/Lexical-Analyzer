#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <map>
#include <iomanip>
#include <vector>
#include <sstream>

using namespace std;

int getSize(string val){
    int counter = 0;
    for(int i=0; i<val.length(); i++)
        counter++;
    return counter;
}


void generateTABCOP(fstream&, fstream&, const map<string,map<string,string>>&);
void generateLST(fstream&, fstream&, fstream&, map<string,string>&);
void generateTABSIM(fstream&, fstream&, map<string,string>&);

map<string,map<string,string>> instructions;
map<string,string> labels;


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
    generateLST(file, tbcFile, lstFile, labels);
    generateTABSIM(file, lstFile,labels);
    

}

void generateTABCOP(fstream &asmFile, fstream &tbcFile, const map<string,map<string,string>>& ins){
    string line,mnemonic,symbol,value,MD,COP,PC;
    int LI;
    //Problem: The inhPattern regex, detects label and the inherent instructions but also 
    //if there is a label in the END tag, it counts the END as a mnemonic
    //I have to find a way for make the END tag an exception and only catch the label
    regex inhPattern("^\\s*(\\w+:)?\\s*(\\w+)\\s*$"); //^\\s*(\\w+:)?\\s*(\\w+)\\s*$ 
    regex normPattern("^\\s*(\\w+:)?\\s+(\\w+)\\s+(#|$|@)?([0-9]+)\\s*$");
    smatch match;
    tbcFile << "MN\t\t" << "MD\t\t" << "COP\t\t" << "LI" << endl;

    while(!asmFile.eof()){
        getline(asmFile, line);
        if(regex_search(line,match,inhPattern)){
            cout<<"hello!";
            mnemonic = match[2].str();
            
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
            if(mnemonic != "END"){
                 LI = COP.length() / 2;
                tbcFile << mnemonic << "\t\t" << MD << "\t\t" << COP << "\t" << LI << endl;
            }

        }
        else if(regex_search(line,match,normPattern)){
            mnemonic = match[2].str();
            symbol = match[3].str();
            value = match[4].str();

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



void generateLST(fstream &asmFile, fstream &tbcFile, fstream &lstFile, map<string,string> &labels){
    string line,ORG,orgValue,orgSymbol,mnemonic,COP,MD;
    vector<string> values;
    int LI,PC,i=0;
    int counter = 0;
    smatch match;
    regex inhPattern("^\\s*(\\w+:)?\\s*(\\w+)\\s*$");
    regex orgPattern("^\\s*(ORG)\\s+(\\$|@|%)*([0-9]+)$");
    regex normPattern("^\\s*(\\w+:)?\\s+(\\w+)\\s+(#|$|@)?([0-9]+)\\s*$");
    regex tbcPattern("^\\s*([a-zA-Z]+)\\s+([a-zA-Z]+)\\s+([a-zA-Z0-9]+\\s*)\\s*([0-9]?)$");
    asmFile.open("P5.asm");
    tbcFile.open("P5.TABCOP");
    
    //Getting the PC value
    while(!asmFile.eof()){
        getline(asmFile,line);
        counter++;
        if(regex_search(line,match,orgPattern)){
            ORG = match[1].str();
            orgSymbol = match[2].str();
            orgValue = match[3].str();
        }
        //Getting the instruction value operand and storing in a vector
        else if(regex_search(line,match,normPattern)){
            values.push_back(match[4].str());
            //Getting the line number where it is the label
            string label = match[1].str();
            if(!label.empty()){
                labels[match[1].str()] = to_string(counter);
                cout<<"Label Normal in line:"<<counter<<endl;
            }   
        }
        else if(regex_search(line,match,inhPattern)){
            string label = match[1].str();
            if(!label.empty()){
                labels[match[1].str()] = to_string(counter);
                cout<<"Label Inherent in line:"<<counter<<endl;
            }
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
                string aux = COP;

                if(sHexValue.length()%2 != 0)
                    COP.append("0");
                    
                cout<<sHexValue<<endl;
                COP.append(sHexValue);

                if(COP.length() != LI*2){
                    for(int i=0;i<COP.length();i++){
                        if(COP[i] == '0' && i>=2){
                            while(COP.length() != LI*2){
                                COP.insert(i,"0");
                            }
                            break;
                        }
                    }
                }
                    

                lstFile <<  PC << "\t\t\t" << COP << "\t\t\t\t" << mnemonic << endl;
                PC += LI;
                
                
                i++;
            }
        }
    }
    lstFile << PC;
    lstFile.close();
    tbcFile.close();
    asmFile.close();
}


void generateTABSIM(fstream &asmFile, fstream &lstFile, map<string,string> &labels){
    asmFile.open("P5.asm");
    lstFile.open("P5.LST");
    fstream tbsFile("P5.TABSIM", ios::out);
    string line,label,PC;
    int LI;
    static int counter=0;
    regex labelPC_Pattern("^\\s*(\\w+)");
    smatch match;
    
    tbsFile << "LABEL\t\t" << endl;

    //Writing the Labels and their correspondent PC
    for(auto it : labels){
        while(!lstFile.eof()){
            getline(lstFile,line);
            counter++;
            if(regex_search(line,match,labelPC_Pattern)){
                if(counter == stoi(it.second)){
                    cout<<"found"<<endl;
                    label = it.first;
                    PC = match[1].str();
                    tbsFile << label << "=" << PC << endl;
                    break;
                }
            }
        }
    }

    /*for(auto it : labels){
        cout<<it.first<<"in Line" << it.second << endl;
    }*/

}
