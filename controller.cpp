#include <iostream>
#include <fstream>
#include <ctime>
#include <map>
#include <sys/stat.h>

using namespace std;

string timeToDateString(time_t time){
    string str;
    tm *_tm;
    _tm = localtime(&time);
    _tm->tm_year += 1900;
    _tm->tm_mon += 1;
    str = to_string(_tm->tm_year);
    str += "-";
    str += '0' + (_tm->tm_mon/10);
    str += '0' + (_tm->tm_mon%10);
    str += "-";
    str += '0' + (_tm->tm_mday/10);
    str += '0' + (_tm->tm_mday%10); 
    return str;
}

string trim(string s){
    int b = 0, e = s.size();
    while(b < e && s[b] == ' ') b++;
    while(e > 0 && s[e-1] == ' ') e--;
    e = e - b; 
    return s.substr(b,e);
}

string getLastInformation(string filepath){
    ifstream file;
    char l[2048];
    int b, e, state = 0;
    string line, info = "";
    file.open(filepath);
    if(file.fail()) return info;
    while(!file.eof() && state < 3){
        switch(state){
            case 0: state = 1; break;
            case 1: 
                if(l[0] == '/'){
                    l[0] = file.get();
                    if(l[0] == '*') state = 2;
                }
                break;
            case 2:
                file.getline(l+1, 2047);
                line = l;
                if(line.find("*/") != string::npos) state = 3;
                b = line.find("@");
                if(b != string::npos){
                    e = line.find(" ", b);
                    if(e != string::npos){
                        info = line.substr(b+1, e-b-1);
                        cout << info << endl;
                    }
                }
                break;
            default: break;
            }
        l[0] = file.get();
    }
    file.close();
    return info;
}

void dynamicDoxygenHeader(string filepath, map<string, string> infoList){
    ifstream file;
    ofstream temp;
    char l[2048];
    int b, e, state = 0;
    string line, lastInfo, key;
    file.open(filepath);
    if(file.fail()) state = 4;
    temp.open(filepath + ".tmp");
    if(file.fail()) state = 4;
    lastInfo = getLastInformation(filepath);
    if(lastInfo.compare("") == 0) state = 4;
    while(!file.eof() && state < 4){
        switch(state){
            case 0: state = 1; break;
            case 1: 
                if(l[0] == '/'){
                    temp << l[0];
                    l[0] = file.get();
                    if(l[0] == '*') state = 2;
                }
                temp << l[0];
                break;
            case 2:
                file.getline(l+1, 2047);
                line = l;
                if(line.find("*/") != string::npos) state = 3;
                b = line.find("@");
                if(b != string::npos){
                    e = line.find(" ", b);
                    if(e != string::npos){
                        key = line.substr(b+1, e-b-1);
                        if(key.compare(lastInfo) == 0) state = 3;
                        if(infoList[key].compare("") != 0){
                            line = line.substr(0, e+1) + infoList[key];
                        }
                    }
                }
                temp << line << endl;
                break;
            case 3:
                temp << l[0];
                break;
            default: break;
            }
        l[0] = file.get();
    }
    if(file.is_open()) file.close();
    if(temp.is_open()) temp.close();
    remove(filepath.c_str());
    rename((filepath + ".tmp").c_str(), filepath.c_str());
}

int main(){
    ifstream data;
    map<string, string> config;
    string line, key, value;
    char str[256];
    config["date"] = timeToDateString(time(NULL));
    data.open("data.txt");
    while(!data.eof()){
        int div;
        data.getline(str, 256);
        line = str;
        div = line.find(':');
        key = trim(line.substr(0,div));
        value = trim(line.substr(div+1));
        if(key.size() > 0){
            config[key] = value;
        }
    }
    /*for(pair<string, string> item : config){
        cout << "(" << item.first << ":" << item.second << ")" << endl;
    }*/
    //getLastInformation("file.test.cpp");
        /*insertDynamicInformantion("file.test.cpp", config);
    else*/
    dynamicDoxygenHeader("file.test.cpp", config);
    return 0;
}