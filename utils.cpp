#include "utils.h"

string strip(string str)
{
	if (str.length() > 0)
	{
		while (*str.begin() == ' '||*str.begin() == '\n'||*str.begin() == '\r')
		{
			str.erase(0, 1);
		}
		while (*str.rbegin() == ' ' || *str.rbegin() == '\n' || *str.rbegin() == '\r')
		{
			str.erase(str.length() - 1);
		}
    }
    return str;
}

bool get_filter()
{
    map_filter.clear();
	if (access(filter_file, F_OK) != 0)
	{
		cout << "The profile does not exist." << endl;
		return false;
	}
    FILE* fp = fopen(filter_file, "r");
    if (fp)
    {
        char buff[1024 + 1];
        while (fgets(buff, sizeof(buff), fp))
        {
			string str = strip(buff);
            string::size_type pos = str.find('=');
            if (str[0] != '#' && pos != string::npos)
            {
                string val = strip(str.substr(pos + 1));
                if (val.length() > 0)
                {
                    map_filter[strip(str.substr(0, pos))] = val;
                }
            }
        }
        fclose(fp);
        return true;
    }
    return false;
}

bool combining(string& options)
{
    bool key = false;
    if (map_filter.count(NAME))
    {
        string::size_type pos;
        options += "\\( ";
        string str = map_filter[NAME];
        while(str.length()>0)
        {
            if ((pos = str.find(';')) != string::npos)
            {
                options += " -name \"" + str.substr(0, pos) + "\" ";
                str = str.substr(pos + 1);
                options += " -o ";
            }
            else
            {
                options += "-name \"" + strip(str) + "\"";
                str.clear();
            }
        }
        options += " \\) ";
        key = true;
    }
    if (map_filter.count(NAME_IG))
    {
        if (key)
        {
            options += " -a ";
        }
        string::size_type pos;
        options += "\\( ";
        string str = map_filter[NAME_IG];
        while (str.length() > 0)
        {
            if ((pos = str.find(';')) != string::npos)
            {
                options += "! -name \"" + str.substr(0, pos) + "\" ";
                str = str.substr(pos + 1);
            }
            else
            {
                options += "! -name \"" + strip(str) + "\"";
                str.clear();
            }
        }
        options += " \\) ";
        key = true;
    }
    if (map_filter.count(DIR_IG))
    {
        if (key)
        {
            options += " -a ";
        }
        string::size_type pos;
        options += "\\( ";
        string str = map_filter[DIR_IG];
        while (str.length() > 0)
        {
            if ((pos = str.find(';')) != string::npos)
            {
                options += " ! -path \"*/" + str.substr(0, pos) + "/*\" ";
                str = str.substr(pos + 1);
            }
            else
            {
                options += " ! -path \"*/" + strip(str) + "/*\"";
                str.clear();
            }
        }
        options += " \\) ";
        key = true;
    }
    if (map_filter.count(PERM))
    {
        if (key)
        {
            options += " -a ";
        }
        string::size_type pos;
        options += "\\(";
        string str = map_filter[PERM];
        while (str.length() > 0)
        {
            if ((pos = str.find(';')) != string::npos)
            {
                options += "-perm " + strip(str.substr(0, pos)) + " ";
                str = str.substr(pos + 1);
                options += " -o ";
            }
            else
            {
                options += "-perm " + strip(str);
                str.clear();
            }
        }
        options += "\\) ";
        key = true;
    }
    return key;
}

bool get_arg(int& argc, char** argv, string& target, string& path, int& enhence)
{
	if (argc == 1 || argc > 4)
	{
		return false;
	}
	
	switch (argc)
	{
	case 2:
		target = argv[1];
		break;
	case 3:
		if (string(argv[1]) == "-E")
		{
			enhence = 1;
			target = argv[2];
		}
		else if (string(argv[2]) == "-E")
		{
			enhence = 1;
			target = argv[1];
		}
		else
		{
			path = argv[1];
			if (access(path.c_str(), F_OK) != 0)
			{
				cout << "The path '" + path + "' does not exist." << endl;
				return false;
			}
			target = argv[2];
		}
		break;
	case 4:
		if (string(argv[1]) == "-E")
		{
			enhence = 1;
			path = argv[2];
			target = argv[3];
		}
		else if (string(argv[2]) == "-E")
		{
			enhence = 1;
			path = argv[1];
			target = argv[3];
		}
		else if (string(argv[3]) == "-E")
		{
			enhence = 1;
			path = argv[1];
			path = argv[2];
		}
		else
		{
			usage();
			return false;
		}
		if (access(path.c_str(), F_OK) != 0)
		{
			cout << "The path '" + path + "' does not exist." << endl;
			return false;
		}
		break;
	default:
		return false;
	}
	return true;
}

bool run_cmd(vector<string>& vec_res, char* cmd)
{
	FILE* fp = popen(cmd, "r");
	if (fp)
	{
		char res_buff[1024];
		while (fgets(res_buff, sizeof(res_buff), fp))
		{
			string str = res_buff;
			while (*str.rbegin() == '\r' || *str.rbegin() == '\n')
			{
				*str.rbegin() = '\0';
			}
			string::size_type pos;
			while ((pos = str.find('\t')) != string::npos)
			{
				str.erase(pos, 1);
				str.insert(pos, 4, ' ');
			}
			vec_res.push_back(str);
		}
		pclose(fp);
		return true;
	}
	else
	{
		return false;
	}
}

void usage()
{
	cout << "Usage: search [path] [-E] <target>" << endl;
	cout << "   -E: Enhancement mode, it may be in command anywhere." << endl;
}