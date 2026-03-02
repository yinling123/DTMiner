#ifndef FASTREADER_H
#define FASTREADER_H

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

/**
 * 更快的 CSV 文件读取器，但灵活性稍低，
 * 因为对变量的数量和大小设置了限制。
 */
class FastReader
{
public:
    /**
     * 最大变量数量（列数）。
     */
    static constexpr int MAX_VARS = 100;

    /**
     * 每个变量的最大长度。
     */
    static constexpr int MAX_VAR_SIZE = 200;

    /**
     * 构造函数：初始化一个 FastReader 对象。
     * @param delim 指定分隔符，默认为逗号 (',')。
     */
    FastReader(char delim = ',');

    /**
     * 构造函数：初始化一个 FastReader 对象并打开指定文件。
     * @param fname 要读取的文件名。
     * @param delim 指定分隔符，默认为逗号 (',')。
     */
    FastReader(const std::string &fname, char delim = ',');

    /**
     * 析构函数：释放分配的资源并关闭文件流。
     */
    ~FastReader();

    /**
     * 打开指定文件以供读取。
     * @param fname 要打开的文件名。
     * @param delim 指定分隔符，默认为逗号 (',')。
     */
    void open(const std::string &fname, char delim = ',');

    /**
     * 检查文件流是否处于良好状态。
     * @return 如果文件流可以继续读取，则返回 true；否则返回 false。
     */
    bool good() const;

    /**
     * 读取文件中的下一行，并将其解析为多个字段。
     * @return 包含解析结果的二维字符数组。
     */
    char** next();

    /**
     * 返回当前行的数据。
     * @return 当前行的二维字符数组。
     */
    char** row() const;

    /**
     * 返回当前行的字段数量。
     * @return 当前行的字段数量。
     */
    int rowSize() const;

    /**
     * 返回指定变量的长度。
     * @param var 变量的索引。
     * @return 变量的长度。
     */
    int varSize(int var) const;

    /**
     * 关闭文件流。
     */
    void close();

    /**
     * 打印当前行的内容。
     */
    void dispRow() const;

private:
    char _delim;          // 分隔符，用于解析 CSV 文件。
    char** _row;          // 存储当前行数据的二维字符数组。
    //std::vector<char> _buf; // 注释掉的部分：用于存储文件内容的缓冲区。
    //int _bufIndex;          // 缓冲区的当前读取位置。
    int _rowSize;         // 当前行的字段数量。
    int _lineNum;         // 当前行号。
    std::ifstream _ifs;   // 文件输入流。
};

#endif /* FASTREADER_H */