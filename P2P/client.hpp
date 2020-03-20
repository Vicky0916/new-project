#pragma once
#include <thread>
#include <boost/filesystem.hpp>
#include  "util.hpp"
#include "httplib.h"

#define P2P_PORT 9000
#define MAX_IPBUFFER 16
#define MAX_RANGE (100*1024*1024) 
#define SHARED_PATH "./Shared"
#define DOWNLOAD_PATH "./Download/"

class Host
{
public:
	uint32_t _ip_addr; //要配对的主机的IP地址
	bool _pair_ret;  //用于存放配对结果，成功返回true,失败返回false
};

class Client
{
public:
	bool Start()
	{
		//客户端程序需要循环运行，因为下载文件不止是下载一次
		while (1)
		{
			GetOnlineHost();
		}
		return true;
	}
	//主机配对的线程入口函数
	void HostPair(Host* host)
	{
		//1、组织http协议格式的请求数据
		//2、搭建一个tcp的客户端，将数据发送
		//3、等待服务器的回复，并进行解析
		host->_pair_ret = false;
		char buf[MAX_IPBUFFER] = { 0 };
		inet_ntop(AF_INET, &host->_ip_addr, buf, MAX_IPBUFFER);
		httplib::Client cli(buf, P2P_PORT);
		auto rsp = cli.Get("/hostpair");
		if (rsp && rsp->status == 200)  //判断主机撇对结果
		{
			host->_pair_ret = true;
		}
		return;
	}
	
	bool GetOnlineHost()
	{
		char ch='Y';  //是否重新匹配，默认是进行匹配，若已经匹配，online主机不为空，则让用户选择
		if (!_online_host.empty())
		{
			std::cout << "是否重新查看在线主机?（Y/N）";
			fflush(stdout);
			std::cin >> ch;
		}
		if (ch == 'Y')
		{
			std::cout << "开始主机匹配......\n";
			//1、获取网卡信息，进而得到局域网中所有的IP地址列表
			std::vector<Adapter> list;
			AdapterUtil::GetAllAdapter(&list);
			std::vector<Host> host_list;
			for (int i = 0; i < list.size(); i++) //得到所有的主机ip列表
			{
				uint32_t ip = list[i]._ip_addr;
				uint32_t mask = list[i]._mask_addr;
				//计算网络号
				uint32_t net = (ntohl(ip & mask));
				//计算最大主机号
				uint32_t max_host = (~ntohl(mask));
				for (int j = 1; (int32_t)j < max_host; j++)
				{
					uint32_t host_ip = net + j;//这个主机IP的计算应该使用主机字节序的网络号和主机号
					Host host;
					host._ip_addr = htonl(host_ip);  //将这个主机字节序的IP地址转化成网络字节序
					host._pair_ret = false;
					host_list.push_back(host);
				}
			}
			//对host_list中的主机创建线程进行配对
			std::vector<std::thread*> thr_list(host_list.size());
			for (int i = 0; i < host_list.size(); i++)
			{
				thr_list[i] = new std::thread(&Client::HostPair, this, &host_list[i]);
			}
			std::cout << "正在主机配对中,请稍后......\n" ;
			//等待所有县城主机配对完成，判断配对结果，疆在线主机添加到online_host中
			for (int i = 0; i < host_list.size(); i++)
			{
				thr_list[i]->join();
				if (host_list[i]._pair_ret == true)
				{
					_online_host.push_back(host_list[i]);
				}
				delete thr_list[i];
			}
		}
		//将所有在线主机打印出来，供用户选择
		for (int i = 0; i < _online_host.size(); i++)
		{
			char buf[MAX_IPBUFFER] = { 0 };
			inet_ntop(AF_INET, &_online_host[i]._ip_addr, buf, MAX_IPBUFFER);
			std::cout << "\t" << buf << std::endl;
		}
		// 3、若配对请求得到相应，则对应主机位在线主机，则将IP添加到_online_host列表中
		// 4、打印在线主机列表，是用户选择
		std::cout << "请选择配对主机，获取共享文件列表：";
		fflush(stdout);
		std::string select_ip;
		std::cin >> select_ip;
		GetShareList(select_ip);  //用户选择主机后，调用获取文件列表接口
		return true;
	}
	
	//获取文件列表
	bool GetShareList(const std::string &host_ip)
	{
	//向服务器发送一个文件列表获取请求
	//1、先发送请求
	//2、得到相应之后，解析正文
		httplib::Client cli(host_ip, P2P_PORT);
		auto rsp = cli.Get("/list");
		if (rsp == NULL || rsp->status != 200)
		{
			std::cerr << "获取文件列表响应错误\n";
			return false;
		}
		//打印正文--打印服务器响应的文件名称列表公用户选择
		//body:filename1\r\nfilename2...
		std::cout << rsp->body << std::endl;
		std::cout << "\n请选择要下载的文件：" ;
		fflush(stdout);
		std::string filename;
		std::cin >> filename;
		RangeDownload(host_ip, filename);
		return true;
	}
	//下载文件
	bool DownloadFile(const std::string &host_ip, const std::string&filename)
	{
		//1\向服务端发送文件下载请求
		//2\得到相应结果，响应结果中的body正文就是文件数据
		//3、创建文件，将文件数据写入文件中，关闭文件
		std::string req_path = "/download/" + filename;
		httplib::Client cli(host_ip,P2P_PORT);
		//std::cout << "向服务器发送文件下载请求：" <<host_ip<<":"<< req_path << std::endl;
		auto rsp = cli.Get(req_path.c_str());
		if (rsp == NULL || rsp->status != 200)
		{
			std::cout << "下载文件，获取响应信息失败\n"<<rsp->status<<std::endl;
			return false;
		}
		if (boost::filesystem::exists(DOWNLOAD_PATH))
		{
			boost::filesystem::create_directory(DOWNLOAD_PATH);
		}
		std::string realpath = DOWNLOAD_PATH + '/' + filename;
		if (FileUtil::Write(realpath, rsp->body) == false)
		{
			std::cerr << "文件下载失败\n";
			return false;
		}
		std::cout << "文件下载成功！" << std::endl;
		return true;
	}
	bool RangeDownload(const std::string &host_ip, const std::string &name)
	{
		//1、发送head请求，通过响应中的Content-Length获取文件大小
		std::string req_path = "/download/" + name;
		httplib::Client cli(host_ip, P2P_PORT);
		auto rsp = cli.Head(req_path.c_str());
		if (rsp == NULL || rsp->status != 200)
		{
			std::cout << "获取文件大小信息失败\n";
			return false;
		}
		std::string clen = rsp->get_header_value("Content-Length");
		int64_t filesize = StringUtil::Str2Dig(clen);
		//2、根据文件大小进行分块
		//int range_count=filesize/MAX_RANGE
		// 若文件大小小于块大小，直接进行下载
		if (filesize < MAX_RANGE)
		{
			std::cout << "文件较小，直接下载文件\n";
			return DownloadFile(host_ip, name);
		}
		// 若文件大小不能整除块,泽芬块个数等于文件大小除以分块代销然后+1
		std::cout << "文件过大，分块进行下载\n";
		int range_count = 0;
		if ((filesize % MAX_RANGE) == 0)
		{
			range_count = filesize / MAX_RANGE;
		}
		else
		{
			range_count = (filesize / MAX_RANGE) + 1;
		}
		int64_t range_start = 0, range_end = 0;
		for (int i = 0; i < range_count; i++)
		{
			range_start = i * MAX_RANGE;
			if (i == (range_count - 1))  //末尾分块
			{
				range_end = filesize - 1;
			}
			else
			{
				range_end = ((i + 1)*MAX_RANGE) - 1;
			}
			std::cout << "客户端请求分块：" << range_start << "-" << range_end << std::endl;
			//3、逐一请求分块数据得到相应之后写入文件的指定位置
			std::stringstream tmp;
			tmp << "bytes=" << range_start << "-" << range_end;   //组织一个range头信息的区间值
			httplib::Headers header;
			header.insert(std::make_pair("Range", tmp.str())); 
			header.insert(httplib::make_range_header({ {range_start,range_end} }));
			httplib::Client cli(host_ip.c_str(), P2P_PORT);
			//httplib::Headers header;
			//header.insert(std::make_pair("Range", tmp.str()));
			auto rsp=cli.Get(req_path.c_str(), header);
			// 若文件大小能整除块,泽芬块个数等于文件大小除以分块代销
			if (rsp == NULL || rsp->status != 206)
			{
				std::cout << "区间下载失败\n";
				return false;
			}
			std::cout << "客户端分块写入文件"<<range_start<<"-"<<range_end<<std::endl;
			FileUtil::Write(name, rsp->body, range_start);
			std::cout << "客户端分块写入文件 成功\n"; 
		}
		std::cout << "文件下载成功\n";
		return true;
	}
private:
	std::vector<Host> _online_host;
};

class Server
{
public:
	bool Start()
	{
		//添加针对客户端请求处理方式对应关系‘
		_srv.Get("/hostpair", HostPair);
		_srv.Get("/list", ShareList);
		_srv.Get("/download/.*",Download); //正则表达式 
		_srv.listen("0.0.0.0", P2P_PORT);
		return true;
	}
private:
	static void HostPair(const httplib::Request &req, httplib::Response &rsp)
	{
		rsp.status = 200;
		return;
	}
	//获取共享文件列表--在主机设置一个共享目录，凡是这个目录下的都要共享给别人
	static void ShareList(const httplib::Request &req, httplib::Response &rsp)
	{
		//1、查看目录是否存在，若目录不存在，则创建目录
		if (boost::filesystem::exists(SHARED_PATH))
		{
			boost::filesystem::create_directory(SHARED_PATH);
		}
		boost::filesystem::directory_iterator begin(SHARED_PATH);  //实例化目录迭代器
		boost::filesystem::directory_iterator end;  //实例化目录迭代器的末尾
		//开始迭代目录
		for (; begin != end; ++begin)
		{
			if (boost::filesystem::is_directory(begin->status()))
			{
				//当前版本我们只获取普通文件
				continue;
			}
			std::string name = begin->path().filename().string();
			rsp.body += name + "\r\n";
		}
		rsp.status = 200;
		return;
	}
	static void Download(const httplib::Request &req, httplib::Response &rsp)
	{
		//std::cout << "服务端受到文件下载请求：" << req.path << std::endl;
			//req_path--客户端请求的资源路径
		boost::filesystem::path req_path(req.path);
		std::string name = req_path.filename().string();
		//std::cout << "服务端收到的实际文件下载名称：" << name << "路径：" << SHARED_PATH << std::endl;
		std::string realpath = SHARED_PATH + '/' + name;  //只获取文件名称

		//boost::filesystem::exist() 判断文件是否存在
		//std::cout << "服务端收到的实际文件下载路径" << realpath<<std::endl;
		if (!boost::filesystem::exists(realpath) || boost::filesystem::is_directory(realpath))
		{
			rsp.status = 404;
			return;
		}
		if (req.method == "GET")
		{
			//以前的GET请求，就是直接下载完整文件，但是现在不一样了，现在有了分块传输这个功能
			//判断是否是分块传输的依据，就是这次请求中是否有Range头部字段
			if (req.has_header("Range"))
			{
				//这就是一个分块传输
				//需要知道分块区间是多少
				std::string range_str = req.get_header_value("Range");  //bytes=start-end;
				httplib::Ranges ranges;   //
				httplib::detail::parse_range_header(range_str,ranges);
				int64_t range_start = ranges[0].first;
				int64_t range_end = ranges[0].second;
				int64_t range_len = range_end - range_start + 1;
				std::cout <<realpath<< ":range:" << range_start << "-" << std::endl;
				FileUtil::ReadRange(realpath, &rsp.body, range_len, range_start);
				rsp.status = 206;
				std::cout << "服务端响应区间完毕！" << std::endl;
			}
			else
			{
				//没有Range字段，则是一个完整的文件下载
				if (FileUtil::Read(realpath, &rsp.body) == false);//实际文件的路径应该是共享的
				{
					rsp.status = 500;
					return;
				}
				rsp.status = 200;
			}
		}
		else  //这个是针对HEAD请求的--客户端只要头部不要正文
		{
			int64_t filesize = FileUtil::GetFileSize(realpath);
			rsp.set_header("Content-Length", std::to_string(filesize));  //设置响应头部信息
			rsp.status = 200;
		}
		
		return;
	}
private:
	httplib::Server _srv;
};














