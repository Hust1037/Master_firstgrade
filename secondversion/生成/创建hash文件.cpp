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
unsigned char key[2048] /*= (unsigned char*)"12345678901"*/;
using namespace std;
void dir(string path, string rootdir, string storepath);
string un_chartostring(unsigned char* array);
string size_ttostring(size_t n);
void convertStrToUnChar(char* str, unsigned char* UnChar);

void convertStrToUnChar(char* str, unsigned char* UnChar)
{
    int i = strlen(str), j = 0, counter = 0;
    char c[2];
    unsigned int bytes[2];

    for (j = 0; j < i; j += 2)
    {
        if (0 == j % 2)
        {
            c[0] = str[j];
            c[1] = str[j + 1];
            sscanf(c, "%02x", &bytes[0]);
            UnChar[counter] = bytes[0];
            counter++;
        }
    }
    return;
}
string size_ttostring(size_t n)
{
    string str;
    char strl[2048];
    snprintf(strl, 2048, "%zu", n);
    str = strl;
    return str;
}
string un_chartostring(unsigned char* array)
{
    string str;
    for (int i = 0; i < 32; i++)
        str += to_string(array[i]);
    return str;
}
void dir(string path,string rootdir,string storepath)
{
    long hFile = 0;
    struct _finddata_t fileInfo;
    string pathName, exdName;
    
    //只取自己的根文件夹
   // path = filename1;
    if ((hFile = _findfirst(pathName.assign(path).
        append("\\*").c_str(), &fileInfo)) == -1) {
        return;
    }
    do {
        if (fileInfo.attrib & _A_SUBDIR) {
            string fname = string(fileInfo.name);
            if (fname != ".." && fname != "." && fname != "hash") {
                dir(path + "\\" + fname,rootdir+"\\"+fname,storepath);
              
            }
        }
        else {
            char filename[2048];//文件名比如 MD5.c
            char pathname[2048];
            char filePath[2048];
            strcpy(filename, fileInfo.name);
            rootdir += "\\";
            rootdir += filename;
            strcpy(pathname, path.c_str());
            strcpy(filePath, path.c_str());
            strcat(filePath, "\\");
            strcat(filePath, filename);
            //cout << "file from root detail path:" << " " << rootdir << endl;
            
            size_t n;
            hash<string> h;
            n = h(rootdir);
            string rootdirvalue = size_ttostring(n);
            string::size_type iPos = rootdir.find_last_of('\\');
            rootdir = rootdir.substr(0, iPos);
            unsigned char output[32];
            int ret = sm3_hmac_file(filePath, key, output);
            string hashvalue;
            hashvalue = un_chartostring(output);
            ofstream outFile;
            outFile.open(storepath, ios::app);
            outFile << rootdirvalue << endl;
            outFile << hashvalue << endl;
        }
        
    } while (_findnext(hFile, &fileInfo) == 0);
    _findclose(hFile);
    return;
}

int main(int argc, char* argv[])
{
    string path;
    char patharr[2048];
    strcpy(patharr, argv[2]);
    //cout << "请输入你要检查的文件夹所在的地址" << endl;
    //path="D:\\研一项目\\第一周8.31文件哈希\\hash_src";
    path = patharr;
    string storepath;
    char test[2048];
    strcpy(test, argv[1]);
    convertStrToUnChar(test, key);
    storepath = path + "\\hash.txt";
  //  cout << path << " " << storepath << endl;
    //cin >> path;
    string::size_type iPos = path.find_last_of('\\');
    string filename = path.substr(iPos + 1, filename.length() - iPos - 1);
    string copy_filename1 = filename;
    dir(path,filename, storepath);
    cout << "生成摘要成功：默认在检查文件夹下根目录创建hash.txt并存储" << endl;
    return 0;
}

