#include <boost/filesystem.hpp>
#include "util.hpp"
#include "httplib.h"
#include "client.hpp"
#include <stdlib.h>

void helloworld(const httplib::Request &req, httplib::Response&rsp)
{
	printf("httplib server recv a req:%s\n", req.path.c_str());
	rsp.set_content("<html><h1> HelloWorld</h1></html>", "text/html");
	rsp.status = 200;
}
void Scandir()
{
	const char *ptr = "./";
	boost::filesystem::directory_iterator begin(ptr);
	boost::filesystem::directory_iterator end;
	for (; begin != end; ++begin)
	{
		//当前文件的状态信息
		if (boost::filesystem::is_directory(begin->status()))
		{
			//获取当前文件的文件名
			std::cout << begin->path().string() << "是一个目录\n";
		}
		else
		{
			std::cout << begin->path().string() << "是一个普通文件\n";
			//只获取文件名不要路径
			std::cout << "文件名" << begin->path().filename().string();
		}
	}
}
void ClientRun()
{
	Sleep(1);
	Client cli;
	cli.Start;
}
int main(int argc, char *argv[])
{
	/*httplib::Server srv;
	srv.Get("/", helloworld);
	srv.listen("0.0.0.0", 9000);*/
	/*Scandir();
	Sleep(1111111);*/
	std::thread thr_clilent(ClientRun);
	Server srv;
	srv.Start();
	return 0;

}