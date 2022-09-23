/*需求描述：
编写一个完整性度量模块程序，实现对目标文件夹内文件的修改、丢失等情况的检测
要求：
1、包含两部分程序，生成程序和检测程序，用生成程序对目标文件夹生成整体的hash画像（建议使用sm3的加盐hmac算法），可自定义存储格式（建议以二进制形式连续存储，以增强不可读性及缩小文件体积），用检测程序判断目标文件夹内的文件是否被修改或遗失

2、编译形成可执行程序或dll库，实现windows命令行调用，方便项目集成

3、将生成程序和检测程序制作为简易的Qt界面，方便测试人员使用

4、检测程序提供多种结果查询接口，如管道、本地结果记录文件、自动创建简易HTTP服务提供符合RESTFUL风格的API等。
*/
#include<direct.h> 
#include <io.h>
#include<iostream>
#include<string>
#include<vector>
#include"sm3.h"
#include<map>
#include<fstream>
#include<filesystem>
namespace fs = std::filesystem;
unsigned char key[1024]  /*(unsigned char*)"12345678901"*/;
using namespace std;
string un_chartostring(unsigned char*);
string size_ttostring(size_t n);
void dir(string path, string rootdir);
map<string, int> existfilecollection;//文件名映射hash值，方便计算
map<string, string> filehash;
map<string, string> filehashtoname;
map<string, string> hashtxt;
void check(string path, string filename);
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
//检查待检查文件夹下的文件，通过核对map中的映射值，检查是否文件丢失或者新增。
//如果不存在丢/增的情况再进去hash文件夹下读取对应文件，核对是否修改
//默认根目录下有hash文档
void dir(string path, string rootdir)
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
    //int i = 1;
    do {
        if (fileInfo.attrib & _A_SUBDIR) {
            string fname = string(fileInfo.name);
            if (fname != ".." && fname != "." && fname != "hash") {
                dir(path + "\\" + fname, rootdir + "\\" + fname);

            }
        }
        else  {
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
            size_t n;
            hash<string> h;
            n = h(rootdir);
            string rootdirvalue = size_ttostring(n); 
            unsigned char output[32];
            int ret = sm3_hmac_file(filePath, key, output);
            string hashvalue;
            hashvalue = un_chartostring(output);
            filehash[rootdirvalue]= hashvalue;
            filehashtoname[rootdirvalue] = rootdir;
            //递归返回时候进行还原
            string::size_type iPos = rootdir.find_last_of('\\');
            rootdir = rootdir.substr(0, iPos);
          /*  ofstream outFile;
            outFile.open("hash.txt", ios::app);
            outFile << hashvalue << endl;*/
        }

    } while (_findnext(hFile, &fileInfo) == 0);
    _findclose(hFile);
    return;
}
void check(string path,string filename)
{
    ifstream inFile(path + "\\hash.txt", ios::in);
    if (!inFile) {
        cout << "hash 文件丢失或者hash文件打开失败，请重新检查" << endl;
        exit(0);
    }
    string namestr;
    string hashstr;
    //int j = 0;
    while (getline(inFile, namestr))
    {
        getline(inFile, hashstr);
        string hashfile;
        hashtxt[namestr] = hashstr;
        hashfile = filehash[namestr];
        if (hashfile != hashstr&&hashfile!="")
        {
            cout << "文件被修改" << endl;
            //cout << "被修改的文件是：" << filehashtoname[namestr] << endl;
            exit(0);
        }
        else if (filehashtoname.count(namestr) == 0)
        {
            cout << "文件丢失" << endl;
           // cout << "丢失的文件是：" << filehashtoname[namestr] << endl;
            exit(0);
        }
        //cout << "contexthash: " << hashstr << endl;
    }
    for (map<string, string>::iterator it = filehash.begin(); it != filehash.end(); it++)
    {
        if (hashtxt.count(it->first) == 0)
        {
            filename += "\\hash.txt";
            if (filename != filehashtoname[it->first])
            {
                cout << "存在文件新增     " << filehashtoname[it->first] << endl;
                exit(0);
            }
        }
    }
    cout << "全部文件通过校验" << endl;
    int i = 0;
    //for (map<string, string>::iterator it = filehashtoname.begin(); it != filehashtoname.end(); it++)
    //{
    //    cout << "hash result: " << it->first << endl;
    //    cout << "name result: " << it->second << endl;
    //    //cout << i++<<endl;
    //}
   /* for (map<string, string>::iterator it = filehash.begin(); it != filehash.end(); it++)
    {
        cout << "hash name result: " << it->first << endl;
        cout << "hash file result: " << it->second << endl;
        cout << endl;
    }*/

}
using namespace std;
int main(int argc, char* argv[])
{
    string path;
    char hashpath[2048];
    //cout << "请输入需要检查的文件夹所在的位置\n";
    //cin >> path;
   strcpy(hashpath,argv[2]);
   // cout << path << endl;
   path = hashpath;
  // path = "D:\\研一项目\\第一周8.31文件哈希\\hash_src";
    //cout << "请输入需要检查的文件夹经过计算后存储hash值所在文件夹的位置\n";
    //strcpy(hashpath, "D:\\研一项目\\第一周8.31文件哈希\\hash_src\\hash.txt");
    //cin >> hashpath;
    //strcpy(hashpath, "D:\\研一项目\\第一周8.31文件哈希\\hash_src\\hash");
    string::size_type iPos = path.find_last_of('\\');
    string filename = path.substr(iPos + 1, filename.length() - iPos - 1);
    string copy_filename1 = filename;
    char test[2048] ;
    strcpy(test, argv[1]);
    convertStrToUnChar(test, key);
    dir(path, filename);
    check(path,filename);

    
    return 0;
}

