#include <map>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
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
	std::string version="1.16.50";
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
	ai::print("Covariant简单人工智能聊天机器人\n版本:",ai::version,"\n");
	while (true) {
		ai::print('[', ai::owner_name, "]:");
		ai::parse(ai::getline());
	}
	return 0;
}