#include "file_util.h"

#include <sys/stat.h>
#include <io.h>
#include <fstream>
#include <sstream>
#include "str_util.h"
#include "echoutil.h"

using namespace std;

namespace fileutil
{
    bool is_dir_exist(const std::string &dir)
    {
        struct _stat fileStat;
        if ((_stat(dir.c_str(), &fileStat) == 0) && (fileStat.st_mode & _S_IFDIR)){
            return true;
        }

        return false;
    }

    bool exist(const std::string &path)
    {
        return _access(path.c_str(), 0) != -1;
    }

    bool dir(const std::string &path, /* out */filevec_t &files)
    {
        struct _finddata_t filefind;

        int handle = 0;
        if(-1 == (handle = _findfirst(path.c_str(), &filefind))){
            return false;
        }

        do{
            if(_A_SUBDIR != filefind.attrib){
                // ����Ŀ¼�����ļ�
                files.push_back(filefind.name);
            }
        } while(!_findnext(handle, &filefind));

        _findclose(handle);
        return true;
    }

    bool dir_no_ext(const std::string &path, /* out */filevec_t &files)
    {
        filevec_t ext_files;
        if(false == dir(path, ext_files)){
            return false;
        }

        for (filevec_t::iterator itr = ext_files.begin(); itr != ext_files.end(); ++itr){
            string &filename = *itr;
            files.push_back(strutil::strip_ext(filename));
        }

        return !files.empty();
    }

    char* get_whole_file_buf(const std::string &path)
    {
        if(false == exist(path)){
            return NULL;
        }

        /* ��Ҫһ��byte��©�ض��������ļ���ֻ�ܲ��ö����Ʒ�ʽ�� */
        errno_t err;
        FILE *file = NULL;
        err = fopen_s(&file, path.c_str(), "rb" );  
        if (NULL == file || err > 0){
            ECHO_ERR("��<%s>�ļ�ʧ�ܣ������:%u", path.c_str(), err);
            return NULL;
        }  

        /* ��ȡ�ļ���С */  
        fseek (file , 0 , SEEK_END);  
        long size = ftell (file);  
        rewind (file);  

        /* �����ڴ�洢�����ļ� */   
        char *buf = new char[size];  
        if (buf == NULL)        {  
            ECHO_ERR("����<%s>�ļ�ʧ�ܣ�����ԭ��:����<%ubyte>���ڴ�ʧ��", path.c_str(), size);
            return NULL;
        }  

        /* ���ļ�������buffer�� */  
        size_t ret = fread (buf, 1, size, file);  
        if (ret != size){  
            ECHO_ERR("����<%s>�ļ�ʧ�ܣ�����ԭ��:�ļ���ȡ��С����<%u> != <%u>", path.c_str(), ret, size);
            return NULL;
        }  

        /* ���������ļ��Ѿ���buffer�� */
        /* �ر��ļ����ͷ��ڴ� */
        fclose (file);
        return buf;
    }

    bool get_whole_file_str(const std::string &path, std::string &text)
    {
        if(false == exist(path)){
            text = "";
            return false;
        }

        std::ifstream file(path.c_str());
        std::stringstream buf;
        buf << file.rdbuf();

        text = buf.str();
        return true;
    }

    bool overwrite_file(const std::string &path, const std::string &new_text)
    {
        string old_text;
        get_whole_file_str(path, old_text);
        if (old_text == new_text){
            return true;
        }

        ofstream o(path);
        if (!o.good()){
            ECHO_ERR("�޷������ļ���·������:<%s>", path.c_str());
            return false;
        }

        o << new_text;
        o.flush();
        o.close();

        return true;
    }
}