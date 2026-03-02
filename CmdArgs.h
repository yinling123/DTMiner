#ifndef CMDARGS_H
#define CMDARGS_H

#include <string>
#include <vector>
#include <time.h>

/**
 * 自定义命令行参数解析类。
 * 用于解析和验证命令行参数，并提供访问这些参数的方法。
 */
class CmdArgs
{
public:
    /**
     * 构造函数：解析命令行参数并初始化类成员变量。
     * @param argc 命令行参数的数量。
     * @param argv 命令行参数的数组。
     */
    CmdArgs(int argc, char **argv);

    /**
     * 获取图文件名。
     * @return 图文件名。
     */
    const std::string &graphFname() const { return _graphFname; }

    /**
     * 获取查询图文件名列表。
     * @return 查询图文件名的向量。
     */
    const std::vector<std::string> &queryFnames() const { return _queryFnames; }

    /**
     * 获取输出文件名。
     * @return 输出文件名。
     */
    const std::string &outFname() const { return _outFname; }

    /**
     * 获取时间窗口值列表（以秒为单位）。
     * @return 时间窗口值的向量。
     */
    const std::vector<time_t> &deltaValues() const { return _deltaValues; }

    /**
     * 检查命令行参数解析是否成功。
     * @return 如果解析成功返回 true，否则返回 false。
     */
    bool success() const { return _success; }

    /**
     * 检查是否忽略时间顺序。
     * @return 如果忽略时间顺序返回 true，否则返回 false。
     */
    bool unordered() const { return _unordered; }

    /**
     * 显示帮助信息，说明命令行参数的用法。
     */
    void dispHelp() const;

    /**
     * 解析时间字符串并将其转换为秒数。
     * 支持的时间单位包括：
     * - 秒 (s)
     * - 分钟 (m)
     * - 小时 (h)
     * - 天 (d)
     * - 周 (w)
     * @param str 包含时间的字符串。
     * @return 解析后的时间值（以秒为单位）。
     */
    int parseDuration(std::string str) const;

    /**
     * 根据输入文件名和时间窗口生成默认输出文件名。
     * @param gFname 图文件名。
     * @param hFname 查询图文件名。
     * @param delta 时间窗口值（以秒为单位）。
     * @return 生成的输出文件名。
     */
    std::string createOutFname(const std::string &gFname, const std::string &hFname, time_t delta);

private:
    std::string _graphFname; // 图文件名
    std::string _outFname;   // 输出文件名
    std::vector<std::string> _queryFnames; // 查询图文件名列表
    std::vector<time_t> _deltaValues; // 时间窗口值列表（以秒为单位）
    time_t _delta; // 单个时间窗口值（备用）
    bool _success; // 标记命令行参数解析是否成功
    bool _unordered; // 标记是否忽略时间顺序
};

#endif