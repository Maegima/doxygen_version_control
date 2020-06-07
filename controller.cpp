#include <iostream>
#include <fstream>
#include <ctime>
#include <map>
#include <dirent.h>
#include <sys/stat.h>
#include <list>

using namespace std;

struct FileInfo{
    std::string name;
    time_t mtime;
};

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
    size_t b = 0, e = s.size();
    while(b < e && s[b] == ' ') b++;
    while(e > 0 && s[e-1] == ' ') e--;
    e = e - b; 
    return s.substr(b,e);
}

bool informantionUpToDate(string filepath, map<string, string> infoList){
    ifstream file;
    char l[2048];
    size_t b, e;
    unsigned int state = 0;
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
    size_t b, e;
    unsigned int state = 0;
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
    size_t size = file.size();
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

void fileListRecursive(string name, string prefix, int maxLevel, list<FileInfo>& lst){
    struct stat info;
    string filename = prefix + name;
    if(maxLevel > 0){
        stat(filename.c_str(), &info);
        if(S_ISREG(info.st_mode)){
            if(validExtension(filename))
                lst.push_back({filename, info.st_mtime});
        } else if(S_ISDIR(info.st_mode)){
            DIR *dir = opendir(filename.c_str());
            struct dirent *entry;
            if(dir){
                while( (entry = readdir(dir)) != NULL ){
                    name = entry->d_name;
                    if(name[0] != '.')
                        fileListRecursive(name, filename + "/", maxLevel-1, lst);
                }
                closedir(dir);
            }
        }
    }
}

list<FileInfo> fileList(string path){
    DIR *dir = opendir(path.c_str());
    list<FileInfo> lst;
    if(dir){
        fileListRecursive(path, "", 10, lst);
        closedir(dir);
    }
    return lst;
}

bool isUpToDate(FileInfo file, time_t lastUpdate){
    if(file.mtime > lastUpdate)
        return (timeToDateString(lastUpdate).compare(timeToDateString(file.mtime)) == 0);
    return true;
}

time_t getMTime(string file){
    struct stat st;
    if(stat(file.c_str(), &st) == 0)
        return st.st_mtime;
    return 0;
}

unsigned int dynamicDoxygenHeader(FileInfo& fileInfo, map<string, string> &infoList){
    ifstream file;
    ofstream temp;
    char l[2048];
    size_t b, e; 
    unsigned int state = 0;
    b = fileInfo.name.rfind("/");
    if(b == string::npos) b = -1;
    infoList["file"] = fileInfo.name.substr(b+1);
    infoList["date"] = timeToDateString(fileInfo.mtime);
    string line, lastInfo, key;
    file.open(fileInfo.name);
    if(file.fail()) state += 4;
    temp.open(fileInfo.name + ".tmp");
    if(file.fail()) state += 8;
    lastInfo = getLastInformation(fileInfo.name);
    if(lastInfo.compare("") == 0) state += 16;
    if(informantionUpToDate(fileInfo.name, infoList)) state += 32;
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
        remove(fileInfo.name.c_str());
        rename((fileInfo.name + ".tmp").c_str(), fileInfo.name.c_str());
        fileInfo.mtime = getMTime(fileInfo.name);
    } 
    else if((state & 8) == 0){ 
        remove((fileInfo.name + ".tmp").c_str());
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
    unsigned int state;
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
            if(key.compare("") != 0){
                data >> tvalue;
                lastUpdate[key] = tvalue;
            }
            data.getline(str, 256);
        }
        data.close();
    }
    list<FileInfo> lst = fileList(".");
    for(FileInfo& file : lst){
        cout << file.name << " "; 
        cout << "ltime: " << lastUpdate[file.name] << " ";
        cout << "mtime: " << file.mtime << " ";
        if(!isUpToDate(file, lastUpdate[file.name])){
            state = dynamicDoxygenHeader(file, config);
            cout << "etime: " << file.mtime << " ";
            cout << "state: " << state << " ";
            writeUpdate = true;
        }
        cout << endl;
    }
    if(writeUpdate){
        status.open(".status.ddh");
        if(status.fail()) return 0;
        for(FileInfo file : lst){
            status << file.name << " " << file.mtime << endl;
        }
        status.close();
    }
    return 0;
}