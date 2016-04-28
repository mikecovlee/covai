#include <map>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include "covnet.h"
namespace ai {
	template < typename T > void print(const T &);
	std::string getline();
	void clrscr();
	template < typename T, typename ... Elements >
	void print(const T & dat, const Elements & ... args)
	{
		print(dat);
		print(args ...);
	}

	std::vector < std::string > split(const std::string & s)
	{
		std::vector < std::string > input;
		typedef std::string::size_type string_size;
		string_size i = 0;
		while (i != s.size()) {
			while (i != s.size() && std::isspace(s[i]))
				++i;
			string_size j = i;
			while (j != s.size() && !std::isspace(s[j]))
				++j;
			if (i != j) {
				input.push_back(s.substr(i, j - i));
				i = j;
			}
		}
		return input;
	}

	class neuron {
	protected:
		int mType;
		std::string mData;
	public:
		enum type {
			data = 0,
			action = 1
		};
		neuron():mType(type::data), mData("NULL")
		{
		}
		neuron(int typ, const std::string & dat):mType(typ), mData(dat)
		{
		}
		int ntype() const
		{
			return mType;
		}
		std::string ndata() const
		{
			return mData;
		}
		bool operator<(const neuron & dat)const
		{
			return format(*this) < format(dat);
		}
		static std::string format(const neuron & dat)
		{
			if (dat.ntype() == type::data)
				return "#DAT?" + dat.ndata();
			else
				return "#ACT?" + dat.ndata();
		}
		static neuron unformat(const std::string & dat)
		{
			if (dat.substr(0, dat.find('?')) == "#ACT")
				return neuron(type::action, dat.substr(dat.find('?') + 1));
			else
				return neuron(type::data, dat.substr(dat.find('?') + 1));
		}
	};

	bool silent = false;
	std::string version = "1.16.50";
	std::string robot_name = "Covariant Ai";
	std::string owner_name = "用户";
	std::string record_dir = "record.txt";
	std::map < std::string, std::map < neuron, int >>data;

	void learn(const std::string & keyword, const neuron & answer, int weight = 1)
	{
		if (data[keyword].find(answer) == data[keyword].end()) {
			if (weight >= 0)
				data[keyword][answer] = weight;
			else
				data[keyword][answer] = 1;
		} else {
			data[keyword][answer] += weight;
		}
	}

	void learn()
	{
		print("[学习模式]请输入您要匹配的关键词:\n");
		std::string keys = getline();
		print("[学习模式]请输入您要匹配的回答:\n");
		std::string answer = getline();
		auto args = split(keys);
		for (auto & it:args) {
react:
			print("[学习模式]请输入关键词[\"", it,
			      "\"]回答的类型和权值(权值默认为1):\n");
			std::string input = getline();
			auto v = split(input);
			if (v.size() < 1) {
				print("[Error]参数数量不足\n");
				goto react;
			}
			if (v[0] == "#DAT") {
				if (v.size() > 1)
					learn(it, neuron(neuron::type::data, answer), atoi(v[1].c_str()));
				else
					learn(it, neuron(neuron::type::data, answer));
				continue;
			}
			if (v[0] == "#ACT") {
				if (v.size() > 1)
					learn(it, neuron(neuron::type::action, answer), atoi(v[1].c_str()));
				else
					learn(it, neuron(neuron::type::action, answer));
				continue;
			}
			print("[Error]未知参数\n");
			goto react;
		}
	}

	void parse_macro(const std::string &);
	void match(const std::string & usrinput)
	{
		if (data.empty())
			throw std::out_of_range("匹配关键词失败。");
		bool isFit = false;
		std::map < neuron, int >match_list;
		for (auto & it:data) {
			if (usrinput.find(it.first) != std::string::npos) {
				isFit = true;
				for (auto & result:it.second) {
					if (match_list.find(result.first) == match_list.end())
						match_list[result.first] = result.second;
					else
						match_list[result.first] += result.second;
				}
			}
		}
		if (!isFit)
			throw std::out_of_range("匹配关键词失败。");
		auto max = match_list.begin();
		for (auto it = ++match_list.begin(); it != match_list.end(); ++it) {
			if (it->second > max->second)
				max = it;
		}
		if (max->first.ntype() == neuron::type::data) {
			print(max->first.ndata(), '\n');
		} else {
			if (max->first.ndata()[0] == '#')
				parse_macro(max->first.ndata());
			else
				match(max->first.ndata());
		}
	}

	void save_data()
	{
		std::ofstream out(record_dir);
		if (!out) {
			print("[Error]无法读取存档文件。\n");
			return;
		}
		out << "#Robot_Name " << robot_name << std::endl;
		out << "#Owner_Name " << owner_name << std::endl;
		for (auto & it:data) {
			out << '$' << it.first << std::endl;
			for (auto & vals:it.second) {
				out << vals.second << '&' << neuron::format(vals.first) << std::endl;
			}
		}
		print("[Message]保存数据成功。\n");
	}

	void load_data()
	{
		std::ifstream in(record_dir);
		if (!in) {
			print("[Error]无法读取存档文件。\n");
			return;
		}
		std::string key;
		std::string tmp;
		std::vector < std::string > record;
		while (std::getline(in, tmp))
			record.push_back(tmp);
		for (int i = 0; i < record.size();) {
			if (record[i][0] == '$') {
				key = record[i].substr(1);
				++i;
				for (; i < record.size() && record[i][0] != '$'; ++i) {
					tmp.clear();
					std::string::size_type pos = record[i].find('&');
					int count = atoi(record[i].substr(0, pos).c_str());
					data[key][neuron::unformat(record[i].substr(pos + 1))] = count;
				}
				continue;
			}
			if (record[i][0] == '#') {
				silent = true;
				parse_macro(record[i]);
				silent = false;
				++i;
				continue;
			}
		}
		print("[Message]加载数据成功。\n");
	}

	void download_data()
	{
		try {
			cov::net::Http socket("ldc.atd3.cn", "/covai/record.txt");
			if (!socket.download()) {
				print("[Error]无法下载数据，请检查您的网络。\n");
				return;
			}
			std::string key;
			std::string tmp;
			std::vector < std::string > record;
			while (std::getline(socket.stream(), tmp)) {
				record.push_back(tmp);
			}
			print("[Message]数据下载成功。\n");
			for (int i = 0; i < record.size();) {
				if (record[i][0] == '$') {
					key = record[i].substr(1);
					++i;
					for (; i < record.size() && record[i][0] != '$'; ++i) {
						tmp.clear();
						std::string::size_type pos = record[i].find('&');
						int count = atoi(record[i].substr(0, pos).c_str());
						data[key][neuron::unformat(record[i].substr(pos + 1))] = count;
					}
					continue;
				}
				if (record[i][0] == '#') {
					silent = true;
					parse_macro(record[i]);
					silent = false;
					++i;
					continue;
				}
				++i;
			}
			print("[Message]加载数据成功。\n");
		} catch(...) {
			print("[Error]无法下载数据，请检查您的网络。\n");
			return;
		}
	}

	void help()
	{
		print("Covariant人工智能聊天机器人 使用说明\n"
		      "Covariant人工智能机器人是基于关键词加权数据库模拟神经元实现的，理论上具有初级智力。\n"
		      "您需要不断的帮助机器人学习才能增加机器人的智商。\n"
		      "机器人指令说明:\n"
		      "#Save FILE 将数据库保存至本地文件，将抹除文件内所有内容，如文件不存在则会被创建。如果不指定文件将保存至预设文件内\n"
		      "#Load FILE 从本地文件中读取数据，数据库中现有内容将不会被抹除。如果不指定文件将从预设文件中读取\n"
		      "#Sync 从Internet上下载官方数据库，数据库中现有内容将不会被抹除\n"
		      "#Record FILE 更改预设本地文件路径\n"
		      "#Robot_Name NAME 更改机器人的名字，将被保存到本地数据库中\n"
		      "#Owner_Name NAME 更改用户的名字，将被保存到本地数据库中\n"
		      "#Learn 学习模式\n"
		      "#Help 获取支持\n#Clrscr 清理当前屏幕\n");
		print("学习模式说明:\n"
		      "您需要将需要提供被匹配的关键词和这些关键词统一的回答。您将可以为每一个关键词选择这个回答对于这个关键词是什么类型以及权值。\n"
		      "一个关键词的回答有两种类型，一种是数据，对应的代码是#DAT\n"
		      "另一种是跳转，对应的代码是#ACT\n"
		      "数据将被直接输出到屏幕上，而跳转将被重新放入解析器中匹配。通过跳转您能够构建复杂的逻辑体系\n"
		      "权值则是关键词匹配的关键。数据库将根据一个回答的权值来决定是否执行这个回答。您要减少回答的权值只需设置为负数即可。您对于一个回答强调的次数越多则机器人会更偏向于给予这个回答\n"
		      "跳转类型的回答可以是机器人指令，您可以教机器人一些话来代替输入指令。\n"
		      "版权所有 2016 (C) Covariant Studio\n"
		      "感谢@星翼 提供人工智能思路，@Kiva 提供网络通信支持\n"
		      "获取支持请访问 https://github.com/mikecovlee/covai\n");
	}

	void parse_macro(const std::string & usrinput)
	{
		std::vector < std::string > args = split(usrinput);
		if (args[0] == "#Save") {
			if (args.size() < 2) {
				print("[Message]存档文件目录为:\"", record_dir, "\"\n");
			} else {
				record_dir = args[1];
				print("[Message]存档文件目录现在是:\"", record_dir, "\"\n");
			}
			save_data();
			return;
		}
		if (args[0] == "#Load") {
			if (args.size() < 2) {
				print("[Message]存档文件目录为:\"", record_dir, "\"\n");
			} else {
				record_dir = args[1];
				print("[Message]存档文件目录现在是:\"", record_dir, "\"\n");
			}
			load_data();
			return;
		}
		if (args[0] == "#Sync") {
			print("[Message]正在从服务器下载数据…\n");
			download_data();
			return;
		}
		if (args[0] == "#Record") {
			if (args.size() < 2) {
				print("[Message]存档文件目录为:\"", record_dir, "\"\n");
			} else {
				record_dir = args[1];
				print("[Message]存档文件目录现在是:\"", record_dir, "\"\n");
			}
			return;
		}
		if (args[0] == "#Robot_Name") {
			if (args.size() < 2) {
				print("我叫", robot_name, '\n');
			} else {
				robot_name = args[1];
			}
			return;
		}
		if (args[0] == "#Owner_Name") {
			if (args.size() < 2) {
				print("你是", owner_name, '\n');
			} else {
				owner_name = args[1];
			}
			return;
		}
		if (args[0] == "#Learn") {
			learn();
			return;
		}
		if (args[0] == "#Help") {
			help();
			return;
		}
		if (args[0] == "#Clrscr") {
			clrscr();
			return;
		}
		print("[Error]未知参数\n");
	}

	void parse(const std::string & usrinput)
	{
		if (usrinput.size() == 0)
			return;
		if (usrinput[0] == '#') {
			parse_macro(usrinput);
			return;
		}
		try {
			print('[', robot_name, "]:");
			match(usrinput);
		} catch(std::out_of_range e) {
			print(e.what(), '\n');
			learn();
		}
	}
}

#include <iostream>
template < typename T > void ai::print(const T & dat)
{
	if (!silent)
		std::cout << dat << std::flush;
}

std::string ai::getline()
{
	std::string input;
	std::getline(std::cin, input);
	return input;
}

void ai::clrscr()
{
	print("\x1B[2J\x1B[0;0f");
}

int main()
{
	std::ios_base::sync_with_stdio(false);
	ai::print("Covariant人工智能聊天机器人\n版本:", ai::version,
	          "\n[Message]正在从服务器下载数据…\n");
	ai::download_data();
	while (true) {
		ai::print('[', ai::owner_name, "]:");
		ai::parse(ai::getline());
	}
	return 0;
}