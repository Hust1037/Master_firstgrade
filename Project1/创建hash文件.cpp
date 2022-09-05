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
map<string, unsigned char*> filecollection;//文件名映射hash值，方便计算
void getfileshash(char storePath[], char filePath[], char filename[]);
void dir(string path);
void dir(string path)
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
                dir(path + "\\" + fname);
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
            char buff[5];
            strcpy(buff ,".txt");
            strcat(filename, buff);//以文件名加.txt为后缀的名字作为存储hash后的结果的文件的名字 eg: Md5.c.txt
           
            //cout << pathname << endl;
            getfileshash(pathname, filePath,filename);
        }
    } while (_findnext(hFile, &fileInfo) == 0);
    _findclose(hFile);
    return;
}
void getfileshash(char storePath[],char filePath[],char filename[] )
//filename->文件的具体路径 storePath->hash文件存储的位置
{
    unsigned char output[32];//存储的hash value
    int ret = sm3_hmac_file(filePath, key, output);
    strcat(storePath, "\\hash");
    _mkdir(storePath);
    strcat(storePath, "\\");
    strcat(storePath, filename);
    fstream file(storePath, /*ofstream::app|*/  ios::binary | ofstream::out);
    if (!file) {
        cout << "file open failed" << endl;
        exit(0);
    }
    for (int i = 0; i < 32; i++)
    {
        unsigned char tmp;
        tmp = output[i];
        file.write((char*)&tmp, sizeof(tmp));
    }
    file.close();
}
int main()
{
    char path[2048];
    cout << "请输入你要检查的文件夹所在的地址" << endl;
    //strcpy(path,"D:\\研一项目\\第一周8.31文件哈希\\hash_src");
    cin >> path;
    dir(path);
    cout << "生成摘要成功：默认在检查文件夹下创建hash文件夹并存储" << endl;
    return 0;
}

