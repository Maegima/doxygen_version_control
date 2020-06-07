#include <iostream>
#include <fstream>
#include <ctime>
#include <map>
#include <dirent.h>
#include <sys/stat.h>
#include <list>

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

bool informantionUpToDate(string filepath, map<string, string> infoList){
    ifstream file;
    char l[2048];
    int b, e, state = 0;
    string line, key, info;
    file.open(filepath);
    if(file.fail()) return false;
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
                        key = line.substr(b+1, e-b-1);
                        info = line.substr(e+1);
                        if(infoList[key].compare("") != 0){
                            if(infoList[key].compare(info) != 0) state = 4;
                        }
                    }
                }
                break;
            default: break;
            }
        l[0] = file.get();
    }
    file.close();
    if(state == 4) return false;
    return true;
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

bool validExtension(string file){
    int size = file.size();
    if(file.find(".h", size-2) != string::npos)
        return true;
    if(file.find(".c", size-2) != string::npos)
        return true;
    if(file.find(".cpp", size-4) != string::npos)
        return true;
    if(file.find(".hpp", size-4) != string::npos)
        return true;
    return false;
}

void fileListRecursive(string name, string prefix, int maxLevel, list<string>& lst){
    struct stat info;
    string filename = prefix + name;
    if(maxLevel > 0){
        stat(filename.c_str(), &info);
        if(S_ISREG(info.st_mode)){
            if(validExtension(filename))
                lst.push_back(filename);
        } else if(S_ISDIR(info.st_mode)){
            DIR *dir = opendir(filename.c_str());
            struct dirent *entry;
            if(dir){
                while( (entry = readdir(dir)) != NULL ){
                    name = entry->d_name;
                    if(name[0] != '.')
                        fileListRecursive(name, filename + "/", maxLevel-1, lst);
                }
            }
        }
    }
}

list<string> fileList(string path){
    DIR *dir = opendir(path.c_str());
    list<string> lst;
    if(dir)
        fileListRecursive(path, "", 10, lst);
    return lst;
}

time_t lastModifiedTime(string file, time_t lastUpdate){
    struct stat st;
    stat(file.c_str(), &st);
    if(st.st_mtime > lastUpdate)
        if(timeToDateString(lastUpdate).compare(timeToDateString(st.st_mtime)) != 0)
            return st.st_mtime;
    return 0;
}

int dynamicDoxygenHeader(string filepath, map<string, string> infoList){
    ifstream file;
    ofstream temp;
    char l[2048];
    int b, e; 
    unsigned int state = 0;
    b = filepath.rfind("/");
    if(b == string::npos) b = -1;
    infoList["file"] = filepath.substr(b+1);
    string line, lastInfo, key;
    file.open(filepath);
    if(file.fail()) state += 4;
    temp.open(filepath + ".tmp");
    if(file.fail()) state += 8;
    lastInfo = getLastInformation(filepath);
    if(lastInfo.compare("") == 0) state += 16;
    if(informantionUpToDate(filepath, infoList)) state += 32;
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
    if(state == 3){
        remove(filepath.c_str());
        rename((filepath + ".tmp").c_str(), filepath.c_str());
    } 
    else if((state & 8) == 0){ 
        remove((filepath + ".tmp").c_str());
    }
    return state;
}

int main(){
    ofstream status;
    ifstream data;
    map<string, string> config;
    map<string, time_t> lastUpdate;
    string line, key, value;
    time_t tvalue;
    char str[256];
    bool writeUpdate = false;
    config["date"] = timeToDateString(time(NULL));
    data.open("data.txt");
    if(!data.fail()){ 
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
        data.close();
    }
    data.open(".status.ddh");
    if(!data.fail()){
        while(!data.eof()){
            data.getline(str, 256, ' ');
            key = str;
            data >> tvalue;
            lastUpdate[key] = tvalue;
        }
        data.close();
    }
    list<string> lst = fileList(".");
    for(string file : lst){
        cout << file << " ltime: " << lastUpdate[file] << endl;
        tvalue = lastModifiedTime(file, lastUpdate[file]);
        if(tvalue > 0){
            if(dynamicDoxygenHeader(file, config) == 3){
                cout << file << " mtime: " << tvalue << endl; 
                lastUpdate[file] = tvalue;
                writeUpdate = true;
            }
        }
    }
    if(writeUpdate){
        status.open(".status.ddh");
        if(status.fail()) return 0;
        for(string file : lst){
            if(lastUpdate[file] > 0){
                status << file << " " << lastUpdate[file] << endl;
            }
        }
    }
    return 0;
}