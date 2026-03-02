#include "FastReader.h"
#include <iostream>

using namespace std;

/**
 * 构造函数：初始化一个 FastReader 对象。
 * @param delim 指定分隔符（例如逗号）用于解析 CSV 文件。
 */
FastReader::FastReader(char delim) : _delim(delim)
{
    // 初始化行号为 0。
    _lineNum = 0;
    // 分配存储每行数据的二维字符数组。
    _row = new char*[MAX_VARS];
    for (int i = 0; i < MAX_VARS; i++)
    {
        _row[i] = new char[MAX_VAR_SIZE];
        _row[i][0] = '\0'; // 初始化每个变量为空字符串。
    }
}

/**
 * 构造函数：初始化一个 FastReader 对象并打开指定文件。
 * @param fname 要读取的文件名。
 * @param delim 指定分隔符（例如逗号）用于解析 CSV 文件。
 */
FastReader::FastReader(const std::string& fname, char delim) : _delim(delim)
{
    // 初始化行号为 0。
    _lineNum = 0;
    // 分配存储每行数据的二维字符数组。
    _row = new char*[MAX_VARS];
    for (int i = 0; i < MAX_VARS; i++)
    {
        _row[i] = new char[MAX_VAR_SIZE];
        _row[i][0] = '\0'; // 初始化每个变量为空字符串。
    }
    // 打开指定文件。
    this->open(fname);
}

/**
 * 析构函数：释放分配的内存并关闭文件流。
 */
FastReader::~FastReader()
{
    // 关闭文件流。
    this->close();
    // 释放二维字符数组的内存。
    if (_row != 0)
    {
        for (int i = 0; i < MAX_VARS; i++)
        {
            if (_row[i] != 0)
            {
                delete[] _row[i];
                _row[i] = 0;
            }
        }
        delete[] _row;
        _row = 0;
    }
}

/**
 * 打开指定文件以供读取。
 * @param fname 要打开的文件名。
 * @param delim 指定分隔符（例如逗号）用于解析 CSV 文件。
 */
void FastReader::open(const std::string& fname, char delim)
{
    // 如果文件流已打开，则先关闭它。
    if (_ifs.is_open())
        _ifs.close();

    // 设置分隔符并打开文件。
    _delim = delim;
    _ifs.open(fname);

    /* 
     * 注释掉的部分是另一种实现方式：
     * 以二进制模式打开文件并一次性读取到缓冲区中。
     */
}

/**
 * 关闭文件流。
 */
void FastReader::close()
{
    _ifs.close();
    //_buf.clear(); // 清空缓冲区（如果使用缓冲区实现）。
}

/**
 * 检查文件流是否处于良好状态。
 * 出现文件读取完毕等情况，才返回false
 * @return 如果文件流可以继续读取，则返回 true；否则返回 false。
 */
bool FastReader::good() const
{
    return _ifs.good();
}

/**
 * 返回当前行的数据。
 * @return 当前行的二维字符数组。
 */
char** FastReader::row() const
{
    return _row;
}

/**
 * 读取文件中的下一行，并将其解析为多个字段。
 * @return 包含解析结果的二维字符数组。
 */
char** FastReader::next()
{
    // 初始化行大小和变量大小。
    _rowSize = 0;
    int varSize = 0;
    char* var = _row[_rowSize];
    bool inQuote = false; // 标记是否在引号内。
    _lineNum++;           // 增加行号。
    var[0] = '\0';        // 初始化当前变量为空字符串。
    bool inText = false;  // 标记是否在文本中。
    bool badVar = false;  // 标记变量是否超出最大长度。

    while (true)
    {
        char c = _ifs.get(); // 从文件流中读取一个字符。

        // 如果文件流结束或读取失败，处理最后一行。
        if (!_ifs.good())
        {
            var[varSize] = '\0';
            varSize = 0;
            _rowSize++;
            break;
        }

        // 处理引号内的内容。
        if (c == '"')
        {
            inQuote = !inQuote;
            inText = inQuote;
            continue;
        }

        // 处理不同操作系统的换行符（DOS vs Unix）。
        if (c == '\r')
            c = _ifs.get();

        // 处理换行符。
        if (c == '\n')
        {
            if (!inQuote)
            {
                var[varSize] = '\0';
                varSize = 0;
                _rowSize++;
                break;
            }
            else
            {
                // 如果换行符出现在引号内，抛出异常。
                cout << "Line Number: " << _lineNum << endl;
                cout << "Row: ";
                this->dispRow();
                throw "Line break found inside quotation marks.";
            }
        }

        // 处理分隔符。
        if (c == _delim && !inQuote)
        {
            inText = false;
            badVar = false;
            _rowSize++;
            var[varSize] = '\0';

            // 如果列数超过最大限制，抛出异常。
            if (_rowSize >= MAX_VARS)
            {
                _rowSize = MAX_VARS;
                cout << "Line Number: " << _lineNum << endl;
                cout << "Row: ";
                this->dispRow();
                throw "Number of columns in CSV file exceeded number available for FastReader.";
            }

            var = _row[_rowSize];
            var[0] = '\0';
            varSize = 0;
        }
        else if (inText || c != ' ')
        {
            inText = true;

            // 如果变量未超出最大长度，继续存储字符。
            if (!badVar)
            {
                var[varSize] = c;
                varSize++;
            }

            // 如果变量超出最大长度，跳过剩余内容。
            if (varSize >= MAX_VAR_SIZE)
            {
                var[MAX_VAR_SIZE - 1] = '\0';
                cout << "Line Number: " << _lineNum << endl;
                cout << "Row: ";
                this->dispRow();
                cout << "Variable size = " << varSize << endl;
                cout << "Max variable size = " << MAX_VAR_SIZE << endl;
                cout << "Skipping remainding content of variable" << endl;
                badVar = true;
            }
        }
    }

    return _row;
}

/**
 * 返回当前行的字段数量。
 * @return 当前行的字段数量。
 */
int FastReader::rowSize() const
{
    return _rowSize;
}

/**
 * 打印当前行的内容。
 */
void FastReader::dispRow() const
{
    for (int i = 0; i < _rowSize - 1; i++)
    {
        cout << _row[i] << ",";
    }
    cout << _row[_rowSize - 1] << endl;
}