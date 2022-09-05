/*需求描述：
编写一个完整性度量模块程序，实现对目标文件夹内文件的修改、丢失等情况的检测
要求：
1、包含两部分程序，生成程序和检测程序，用生成程序对目标文件夹生成整体的hash画像（建议使用sm3的加盐hmac算法），可自定义存储格式（建议以二进制形式连续存储，以增强不可读性及缩小文件体积），用检测程序判断目标文件夹内的文件是否被修改或遗失

2、编译形成可执行程序或dll库，实现windows命令行调用，方便项目集成

3、将生成程序和检测程序制作为简易的Qt界面，方便测试人员使用

4、检测程序提供多种结果查询接口，如管道、本地结果记录文件、自动创建简易HTTP服务提供符合RESTFUL风格的API等。
*/
#include<direct.h> 
#include<Windows.h>
#include <io.h>
#include<iostream>
#include<string>
#include<vector>
#include"sm3.h"
#include<map>
#include<fstream>
unsigned char* key = (unsigned char*)"12345678901";
using namespace std;
void hashfiledir(string path);
void check();
void dirfile(string path, string hashpath);
map<string, int> existfilecollection;//文件名映射hash值，方便计算
void check()
{
    for (map<string, int>::iterator it = existfilecollection.begin(); it != existfilecollection.end(); it++)
    {
        string filename = it->first;
        filename = filename.substr(0, filename.rfind("."));
        int flag = 0;
        flag = it->second;
        if(flag==1)
            cout << filename << "  " << ":该文件丢失" << endl;
        else if(flag==2) cout << filename << "  " << ":该文件被修改" << endl;
        else if(flag==3) cout << filename << "  " << ":该文件未被修改" << endl;
        else if(flag==4)  cout << filename << "  " << ":该文件是新增文件" << endl;
        //cout << filename << " check " << flag << endl;
    }
}
void hashfiledir(string path)
{
    long hFile = 0;
    struct _finddata_t fileInfo;
    string pathName, exdName;
    if ((hFile = _findfirst(pathName.assign(path).
        append("\\*").c_str(), &fileInfo)) == -1) {
        return;
    }
    do {
        if (fileInfo.attrib & _A_SUBDIR) {
            string fname = string(fileInfo.name);
            if (fname != ".." && fname != "." && fname != "hash") {
                hashfiledir(path + "\\" + fname);
            }
        }
        else {
            char filename[2048];//文件名比如 MD5.c
            char pathname[2048];
            char filePath[2048];
            strcpy(filename, fileInfo.name);
            strcpy(pathname, path.c_str());
            strcpy(filePath, path.c_str());
            strcat(filePath, "\\");
            strcat(filePath, filename);
            
            //cout << "file detail path" << " " << filePath << endl;
           // char buff[5];
            //strcpy(buff, ".txt");
          //  strcat(filename, buff);//以文件名加.txt为后缀的名字作为存储hash后的结果的文件的名字 eg: Md5.c.txt
           // cout << filename << endl;  //例如MD5.c.txt
            existfilecollection[filename] = 1;
        }
    } while (_findnext(hFile, &fileInfo) == 0);
    _findclose(hFile);
    return;

}

void dirfile(string path,string hashpath)
{
    long hFile = 0;
    struct _finddata_t fileInfo;
    string pathName, exdName;


    if ((hFile = _findfirst(pathName.assign(path).
        append("\\*").c_str(), &fileInfo)) == -1) {
        return;
    }
    do {
        if (fileInfo.attrib & _A_SUBDIR) {
            string fname = string(fileInfo.name);
            if (fname != ".." && fname != "." ) {
                dirfile(path + "\\" + fname,hashpath);
            }
        }
        else {
            char filename[2048];//文件名比如 MD5.c
            char filePath[2048];
            char hashtemppath[2048];
           
            strcpy(hashtemppath, hashpath.c_str());
            strcat(hashtemppath, "\\");
            strcpy(filename, fileInfo.name);
            strcpy(filePath, path.c_str());
            strcat(filePath, "\\");
            strcat(filePath, filename);
            strcat(hashtemppath, filename);
            strcat(hashtemppath, ".txt");
            strcat(filename, ".txt");
            int flag = 0;
            if (existfilecollection.count(filename)==0)
            {
                existfilecollection[filename] = 4;//说明该文件不存在是新增文件
                continue;
            }
            else 
            {   
                unsigned char hash[32];//存储的hash value
                unsigned char output[32];//存储计算后的hash value
                int ret = sm3_hmac_file(filePath, key, output);
                fstream file;
                file.open(hashtemppath, ifstream::in | ios::binary);
                char a;
                int j=0;
                while (file.read((char*)&a, sizeof(a))) {
                    hash[j] = a;
                   // printf("%02x", hash[j]);
                    if(hash[j]==output[j])
                    j++;
                    else {
                        flag = 2;//说明被修改
                        break;
                    }
                }
                if (flag != 2)
                    flag = 3;//说明没有被修改
                file.close();
            }
            //if(existfilecollection[filename]!=0)
            existfilecollection[filename] = flag;
            
        }
    } while (_findnext(hFile, &fileInfo) == 0);
    _findclose(hFile);
    return;
}

int main()
{
    char path[2048];
    char hashpath[2048];
    strcpy(path, "D:\\研一项目\\第一周8.31文件哈希\\hash_src");
    strcpy(hashpath, "D:\\研一项目\\第一周8.31文件哈希\\hash_src\\hash");
    hashfiledir(hashpath);
    dirfile(path,hashpath);
    check();
    return 0;
}

