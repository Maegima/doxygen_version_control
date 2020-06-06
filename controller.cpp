#include <iostream>
#include <fstream>
#include <ctime>
#include <map>
#include <sys/stat.h>

using namespace std;

string getDate(){
    string res;
    time_t timer;
    tm *now_tm;
    time(&timer);  /* get current time; same as: timer = time(NULL)  */
    now_tm = localtime(&timer);
    now_tm->tm_year += 1900;
    now_tm->tm_mon += 1;
    res = to_string(now_tm->tm_year);
    res += "-";
    res += '0' + (now_tm->tm_mon/10);
    res += '0' + (now_tm->tm_mon%10);
    res += "-";
    res += '0' + (now_tm->tm_mday/10);
    res += '0' + (now_tm->tm_mday%10); 
    return res;
}

string trim(string s){
    int b = 0, e = s.size();
    while(b < e && s[b] == ' ') b++;
    while(e > 0 && s[e-1] == ' ') e--;
    e = e - b; 
    return s.substr(b,e);
}

bool testIfHasDynamicInformation(string filepath){
    ifstream file;
    char l[2048];
    int b;
    string line;
    file.open(filepath);
    if(file.fail()) return false;
    while(!file.eof()){
        file.getline(l, 2048);
        line = l;
        b = line.find("$[");
        if(b != string::npos && line.find("]", b)){ 
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

void insertDynamicInformantion(string filepath, map<string, string> conf){
    int state = 0, b, e;
    char l[2048], c = 0;
    string line, word, temppath;
    ofstream temp;
    ifstream file;
    conf["file"] = filepath;
    file.open(filepath);
    if(file.fail()) return;
    temppath = filepath + ".temp.tmp";
    temp.open(temppath, std::fstream::out);
    if(temp.fail()) return;
    while(!file.eof()){
        switch(state){
            case 0: state = 1; break;
            case 1:
                if(c == '/'){
                    temp << c;
                    c = file.get();
                    if(c == '/')
                        state = 2;
                    else if(c == '*')
                        state = 3;
                }
                temp << c;
                break;
            case 2:
                file.getline(l, 2048);
                line = c;
                line += l;
                //cout << line << endl;
                b = line.find("$[");
                e = line.find("]");
                if(b != string::npos && e != string::npos){
                    temp << line.substr(0,b);
                    word = trim(line.substr(b+2, e-b-2));
                    temp << conf[word];
                    //cout << line << endl;
                    temp << line.substr(e+1) << endl;
                }
                else{
                    temp << line << endl;
                }
                state = 1;    
                break;
            case 3:
                file.getline(l, 2048);
                line = c;
                line += l;
                //cout << "(" << line << ")" << endl;
                if(line.find("*/") != string::npos)
                    state = 1;
                b = line.find("$[");
                e = line.find("]");
                if(b != string::npos && e != string::npos){
                    temp << line.substr(0,b);
                    word = trim(line.substr(b+2, e-b-2));
                    temp << conf[word];
                    cout << word << endl;
                    temp << line.substr(e+1) << endl;
                }
                else{
                    temp << line << endl;
                }
                break;
        }
        c = file.get();
    }
    file.close();
    temp.close();
    remove(filepath.c_str());
    rename(temppath.c_str(), filepath.c_str());
}

int main(){
    ifstream data;
    map<string, string> config;
    string line, key, value;
    char str[256];
    config["date"] = getDate();
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
    for(pair<string, string> item : config){
        cout << "(" << item.first << ":" << item.second << ")" << endl;
    }
    if(testIfHasDynamicInformation("file.test.cpp"))
        insertDynamicInformantion("file.test.cpp", config);
    else
    {
        cout << "up to date" << endl;
    }
    
    return 0;
}