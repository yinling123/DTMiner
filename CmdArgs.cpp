#include "CmdArgs.h"
#include "FileIO.h"
#include <iostream>

using namespace std;

/**
 * 构造函数：解析命令行参数并验证其有效性。
 * @param argc 命令行参数的数量。
 * @param argv 命令行参数的数组。
 */
CmdArgs::CmdArgs(int argc, char **argv)
{
    _success = true; // 默认成功，除非被证明失败
    _unordered = false; // 默认情况下考虑时间顺序

    // 如果没有提供任何参数，则显示帮助信息并标记为失败
    if(argc <= 1)
    {
        this->dispHelp();
        _success = false;
        return;
    }

    // 遍历所有命令行参数
    for(int i=1; i<argc; i++)
    {
        string arg = argv[i];
        if(arg == "-g") // 指定图文件
        {
            i++;
            if(i == argc) // 如果缺少文件名
            {
                cout << "Missing graph file after -g argument." << endl;
                _success = false;
                continue;
            }
            _graphFname = argv[i]; // 设置图文件名
            if(FileIO::fileExists(_graphFname)==false) // 检查文件是否存在
            {
                cout << "Graph file \"" << _graphFname << "\" does not exist, or cannot be opened." << endl;
                _success = false;		
            }
        }
        else if(arg == "-q") // 指定查询图文件
        {
            i++;
            if(i == argc) // 如果缺少文件名
            {
                cout << "Missing query graph file after -q argument." << endl;
                _success = false;
                continue;
            }
            _queryFnames.push_back(argv[i]); // 添加查询图文件名
            if(FileIO::fileExists(_queryFnames.back())==false) // 检查文件是否存在
            {
                cout << "Query file \"" << _queryFnames.back() << "\" does not exist, or cannot be opened." << endl;
                _success = false;		
            }
        }
        else if(arg == "-qf") // 指定查询图文件夹
        {
            i++;
            if(i == argc) // 如果缺少文件夹路径
            {
                cout << "Missing query graph folder after -qf argument." << endl;
                _success = false;
                continue;
            }
            _queryFnames = FileIO::getFileNames(argv[i],".txt"); // 获取文件夹中所有 .gdf 文件
            if(_queryFnames.empty()) // 如果未找到文件
            {
                cout << "No files with a *.gdf extension could be found in the given folder: " << argv[i] << endl;
                _success = false;
            }
        }
        else if(arg == "-o") // 指定输出文件
        {
            i++;
            if(i == argc) // 如果缺少文件名
            {
                cout << "Missing query out file after -o argument." << endl;
                _success = false;
                continue;
            }
            _outFname = argv[i]; // 设置输出文件名
        }
        else if(arg == "-delta") // 指定时间窗口大小
        {
            i++;
            if(i == argc) // 如果缺少值
            {
                cout << "Missing delta value after -delta argument." << endl;
                _success = false;
                continue;
            }
            _deltaValues.push_back(parseDuration(argv[i])); // 解析时间窗口值
            if(_deltaValues.back() <= 0) // 检查值是否有效
            {
                cout << "Delta value must be an integer >= 1." << endl;
                _success = false;
            }
        }
        else // 未知参数
        {
            cout << "Unknown command line argument: " << arg << endl;
            _success = false;
        }
    }

    // 如果未指定时间窗口，默认设置为 24 小时
    if(_deltaValues.empty())
        _deltaValues.push_back(60*60*24);

    // 检查必要参数是否缺失
    if(_graphFname.empty())
    {
        cout << "Missing graph file. Specify with the -g argument." << endl;
        _success = false;
    }
    if(_queryFnames.empty())
    {
        cout << "Missing query file. Specify with the -q argument." << endl;
        _success = false;
    }
    if(_deltaValues.size() > 1 && _outFname.empty() == false)
    {
        cout << "If using multiple delta values, you need to use the computer generated output filenames" << endl;
        _success = false;
    }
}

/**
 * 显示帮助信息，说明命令行参数的用法。
 */
void CmdArgs::dispHelp() const
{
    cout << endl;
    cout << "--- dynamo_search ---" << endl;
    cout << endl;
    cout << "Graph input arguments:" << endl;
    cout << "  -g [filename]" << endl;
    cout << "       Specifies the name of the GDF graph we want to search against." << endl;
    cout << endl;
    cout << "Query input arguments:" << endl;
    cout << "  -q [filename]" << endl;
    cout << "       Specifies the GDF subgraph we want to find in our graph." << endl;
    cout << endl;
    cout << "Output arguments:" << endl;
    cout << "  -o [filename]" << endl;
    cout << "       Filename of the GDF file to save the combined results to." << endl;
    cout << "       If no name is specified, then a default filename is used, based" << endl;
    cout << "       on the input filenames and the delta value used." << endl;
    cout << endl;
    cout << "Other arguments:" << endl;
    cout << "  -delta [t]" << endl;
    cout << "       Specifies max duration (in seconds) between matched temporal edges." << endl;
    cout << "       (Default is 24 hours)." << endl;
    cout << "NOTE:" << endl;
    cout << "   As a shortcut, instead of entering duration times in seconds, you can use" << endl;
    cout << "   the following abbreviations for different time scales:" << endl;
    cout << "      w = week, d = day, h = hour, m = minute" << endl;
    cout << "   So a delta duration of one hour can be represented as 1h instead of 3600." << endl;
    cout << endl;
}

/**
 * 解析时间窗口字符串，支持多种时间单位（秒、分钟、小时、天、周）。
 * @param str 时间窗口字符串。
 * @return 解析后的时间窗口值（以秒为单位）。
 */
int CmdArgs::parseDuration(string str) const
{
    if(str.empty()) // 如果字符串为空，返回 0
        return 0;

    char c = str.back(); // 获取最后一个字符
    if(c >= '0' && c <= '9') // 如果是数字，直接解析为秒
        return atoi(str.c_str());

    str = str.substr(0,str.size()-1); // 去掉最后一个字符
    int n = atoi(str.c_str()); // 解析数值部分

    // 根据时间单位转换为秒
    if(c == 's' || c == 'S')
        return n; // 秒
    n *= 60;
    if(c == 'm' || c == 'M')
        return n; // 分钟
    n *= 60;
    if(c == 'h' || c == 'H')
        return n; // 小时
    n *= 24;
    if(c == 'd' || c == 'D')
        return n; // 天
    n *= 7;
    if(c == 'w' || c == 'W')
        return n; // 周

    // 如果时间单位无效，报错并返回 0
    cerr << "ERROR. Unknown suffix on duration: " << str << c << endl;
    cerr << "Possible options are:" << endl;
    cerr << "  * s = seconds" << endl;
    cerr << "  * m = minutes" << endl;
    cerr << "  * h = hours" << endl;
    cerr << "  * d = days" << endl;
    cerr << "  * w = weeks" << endl;
    return 0;
}

/**
 * 根据输入文件名和时间窗口生成默认输出文件名。
 * @param gFname 图文件名。
 * @param hFname 查询图文件名。
 * @param delta 时间窗口值。
 * @return 生成的输出文件名。
 */
string CmdArgs::createOutFname(const string &gFname, const string &hFname, time_t delta)
{
    string gRoot = FileIO::getFname(gFname); // 获取图文件的基础名
    string hRoot = FileIO::getFname(hFname); // 获取查询图文件的基础名

    // 去掉扩展名
    size_t pos = gRoot.rfind('.');
    if(pos != string::npos && pos != 0)
        gRoot = gRoot.substr(0,pos);
    pos = hRoot.rfind('.');
    if(pos != string::npos && pos != 0)
        hRoot = hRoot.substr(0,pos);

    // 生成输出文件名
    string newFname = "G_" + gRoot + "_Q_" + hRoot + "_D_" + std::to_string(delta) + ".gdf";
    return newFname;
}