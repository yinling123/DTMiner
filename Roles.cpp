#include "Roles.h"
#include "FastReader.h"

using namespace std;

/**
 * 加载用户角色信息从指定文件。
 * 文件格式为 CSV，每行至少包含 4 列，其中第 2 列为用户名，第 3 列为角色。
 * @param fname 包含用户角色信息的文件路径。
 */
void Roles::load(const string &fname)
{
    FastReader csv(fname); // 创建一个 FastReader 对象用于读取 CSV 文件
    csv.next(); // 跳过表头
    while (csv.good()) // 循环读取文件中的每一行
    {
        char **vars = csv.next(); // 获取当前行的数据
        if (csv.rowSize() < 4) // 如果当前行的列数少于 4，则跳过该行
            continue;
        _userRoles[vars[1]] = vars[2]; // 将用户名（第 2 列）和角色（第 3 列）存入映射表
    }
    csv.close(); // 关闭文件
}

/**
 * 获取指定用户的角色。
 * 如果用户不存在，则返回默认值 UNKNOWN。
 * @param user 用户名。
 * @return 用户的角色名称。如果用户未找到，则返回 UNKNOWN。
 */
const string &Roles::getRole(const string &user) const
{
    auto pair = _userRoles.find(user); // 在映射表中查找用户
    if (pair == _userRoles.end()) // 如果用户不存在
        return UNKNOWN; // 返回默认值 UNKNOWN
    else
        return pair->second; // 返回用户对应的角色
}