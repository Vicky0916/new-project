#pragma once
#include <stdio.h>
#ifdef _WIN32
//windows头文件
#include <iostream>
#include <WS2tcpip.h>
#include <fstream>
#include <sstream>
#include <boost/filesystem.hpp>
#include <vector>
#include <Iphlpapi.h>  //获取网卡信息接口的头文件
#pragma comment(lib,"Iphlpapi.lib")  //获取网卡信息接口的库文件包含
#pragma comment(lib,"ws2_32.lib")  //windows下的socket库

#else
//linux头文件
#endif

class StringUtil
{
public:
	static int64_t Str2Dig(const std::string &num)
	{
		std::stringstream tmp;
		tmp << num;
		int64_t res;
		tmp >> res;
		return res;

	}
};


class FileUtil
{
public:
	static int64_t GetFileSize(const std::string &name)
	{
		return boost::filesystem::file_size(name);
	}
	static bool Write(const std::string &name, const std::string &body, int64_t offset=0)
	{
		FILE *fp = NULL;
		fopen_s(&fp, name.c_str(), "wb+");
		if (fp == NULL)
		{
			std::cerr << "打开文件失败\n";
			return false;
		}
		fseek(fp, offset, SEEK_SET);
		int ret = fwrite(body.c_str(), 1, body.size(), fp);
		if (ret != body.size())
		{
			std::cerr<< "向文件写入数据失败\n";
			fclose(fp);
			return false;
		}
		fclose(fp);
		return true;
		//std::cout << "写入文件数据:" << name << "size:" << body.size << "\n";
		//std::ofstream ofs(name);
		//if (ofs.is_open() == false)
		//{
		//	std::cerr << "打开文件失败\n";
		//	return false;
		//}
		//ofs.seekp(offset, std::ios::beg);  //读写位置调转到相对于文件起始位置开始偏移offset
		//ofs.write(&body[0], body.size());
		//if (ofs.good() == false)
		//{
		//	std::cerr << "向文件写入失败\n";
		//	ofs.close();
		//	return false;
		//}
		//ofs.close();
		//return true;
	}
	//指针参数表示这是一个输出型参数
	//const & 表示这是一个输入型参数
	static bool Read(const std::string &name, std::string *body)
	{
		int64_t filesize = boost::filesystem::file_size(name);
		body->resize(filesize);
		std::cout << "读取文件数据：" << name << "size:" << filesize << "\n";
		FILE *fp = NULL;
		fopen_s(&fp,name.c_str(), "rb+");
		if (fp == NULL)
		{
			std::cerr << "打开文件失败\n";
			return false;
		}
		int ret = fread(&(*body)[0], 1, filesize, fp);
		if (ret != filesize)
		{
			std::cerr << "从文件读取数据失败\n";
			fclose(fp);
			return false;
		}
		fclose(fp);
		return true;
		
		/*
		std::ifstream ifs(name);
		if (ifs.is_open == false)
		{
			std::cerr << "打开文件失败\n";
			return false;
		}
    	
		//std::cout << "要读取的文件大小" << name << ":" << filesize << std::endl;
		ifs.read(&(*body)[0], filesize);
		if (ifs.good() == false)
		{
			std::cerr << "读取文件数据失败\n";
			ifs.close();
			return false;
		}
		ifs.close();
		return true;*/
	}
	static bool ReadRange(const std::string &name, std::string *body, int64_t len, int64_t offset)
	{
		body->resize(len);
		FILE *fp = NULL;
		fopen_s(&fp, name.c_str(), "rb+");
		if(fp==NULL)
		{
			std::cerr << "从文件读取数据失败\n";
			return false;
		}
		fseek(fp, offset, SEEK_SET);
		int ret = fread(&(*body)[0], 1, len, fp);
		if (ret != len)
		{
			std::cerr << "从文件读取数据失败\n";
			fclose(fp);
			return false;
		}
		fclose(fp);
		return true; 
	}
};


class Adapter
{
public:
	uint32_t _ip_addr;  //网卡上的IP地址
	uint32_t _mask_addr;  //网卡上的子网掩码
};

class AdapterUtil
{
public:
#ifdef _WIN32
	//windows下获取的网卡信息实现
	static bool GetAllAdapter(std::vector<Adapter> *list)
	{
		PIP_ADAPTER_INFO p_adapters = new IP_ADAPTER_INFO();
		
		//GetAdaptersInfo win下获取网卡信息的接口--网卡信息可能有多个
		//获取网卡信息有可能会失败，因为空间不足，因此有一个输出型参数
		uint64_t all_adapter_size = sizeof(IP_ADAPTER_INFO);
		int ret = GetAdaptersInfo(p_adapters, (PULONG)&all_adapter_size);
		if (ret == ERROR_BUFFER_OVERFLOW) //这个错误表示缓冲区空间不足
		{
			//重新给指针申请空间
			delete p_adapters;
			p_adapters = (PIP_ADAPTER_INFO)new BYTE[all_adapter_size];
			//重新获取网卡信息
			GetAdaptersInfo(p_adapters, (PULONG)&all_adapter_size);
		}

		while (p_adapters)
		{
			std::cout << "网卡名称：" << p_adapters->AdapterName << std::endl;
			std::cout << "网卡描述：" << p_adapters->Description << std::endl;
			std::cout<<"IP地址："<<p_adapters->IpAddressList.IpAddress.String<<std::endl;
			std::cout << "子网掩码：" << p_adapters->IpAddressList.IpMask.String << std::endl;
			std::cout << std::endl;
			p_adapters = p_adapters->Next;
		}
		return true;
	}
#else
//linux下获取网卡信息实现
bool GetAllAdapter(std::vector<Adapter>*list)
{
	return true;
}
#endif
};
