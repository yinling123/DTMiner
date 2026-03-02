#ifndef ROLES__H
#define ROLES__H

#include <string>
#include <unordered_map>

/**
 * 定义了一个用于管理用户角色的类。
 * 提供了加载用户角色信息和查询用户角色的功能。
 */
class Roles
{
public:
    /**
     * 默认构造函数。
     */
    Roles() {}

    /**
     * 构造函数，初始化时加载用户角色信息。
     * @param fname 包含用户角色信息的文件路径。
     */
    Roles(const std::string &fname) { this->load(fname); }

    /**
     * 从指定文件加载用户角色信息。
     * 文件格式为键值对，用户名作为键，角色作为值。
     * @param fname 包含用户角色信息的文件路径。
     */
    void load(const std::string &fname);

    /**
     * 获取指定用户的角色。
     * 如果用户不存在，则返回默认值 "UNKNOWN"。
     * @param user 用户名。
     * @return 用户的角色名称。如果用户未找到，则返回 "UNKNOWN"。
     */
    const std::string &getRole(const std::string &user) const;

private:
    std::unordered_map<std::string, std::string> _userRoles; // 存储用户名与角色的映射表
    const std::string UNKNOWN = "UNKNOWN"; // 当请求的用户角色未知时返回的默认值
};

#endif