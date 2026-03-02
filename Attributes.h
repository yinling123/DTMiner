#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#pragma once

#include <vector>
#include <string>

/**
 * 属性的限制（如最大值、最小值等）
 */
class Restrictions {
public:
    Restrictions(); // 构造函数，初始化限制标志。
    void allowAny(); // 允许任意值，设置_any标志并禁用_exact。
    void setMin();   // 设置最小值限制，并禁用_exact。
    void setMax();   // 设置最大值限制，并禁用_exact。
    bool any() const; // 返回是否允许任意值。
    bool hasMin() const; // 返回是否有最小值限制。
    bool hasMax() const; // 返回是否有最大值限制。
    bool exact() const;  // 返回是否需要精确匹配。
private:
    bool _any, _hasMin, _hasMax, _exact; // 限制标志：任意值、最小值、最大值、精确匹配。
};

/**
 * 针对浮点数属性的限制。
 */
class FloatRestrictions : public Restrictions {
public:
    FloatRestrictions(); // 构造函数，初始化浮点数的最小值和最大值。
    void setMin(float value); // 设置浮点数的最小值限制。
    void setMax(float value); // 设置浮点数的最大值限制。
    float min() const; // 返回浮点数的最小值。
    float max() const; // 返回浮点数的最大值。
private:
    float _min, _max; // 浮点数的最小值和最大值。
};

/**
 * 针对整数属性的限制。
 */
class IntRestrictions : public Restrictions {
public:
    IntRestrictions(); // 构造函数，初始化整数的最小值和最大值。
    void setMin(int value); // 设置整数的最小值限制。
    void setMax(int value); // 设置整数的最大值限制。
    int min() const; // 返回整数的最小值。
    int max() const; // 返回整数的最大值。
private:
    int _min, _max; // 整数的最小值和最大值。
};

/**
 * 针对字符串属性的限制。
 */
class StringRestrictions : public Restrictions {
    // 当前没有超出基类Restrictions的额外功能。
};

/**
 * 存储图中节点或边的属性。
 */
class Attributes {
public:
    Attributes(); // 默认构造函数。

    /** 添加一个具体的字符串值 */
    void addStringValue(const std::string &value); // 添加一个具体的字符串值到列表中。

    /** 添加一个字符串限制而非具体值 */
    void addStringValue(StringRestrictions r); // 添加字符串值的限制。

    /** 添加一个具体的浮点数值 */
    void addFloatValue(float value); // 添加一个具体的浮点数值到列表中。

    /** 添加一个浮点数限制而非具体值 */
    void addFloatValue(FloatRestrictions r); // 添加浮点数值的限制。

    /** 添加一个具体的整数值 */
    void addIntValue(int value); // 添加一个具体的整数值到列表中。

    /** 添加一个整数限制而非具体值 */
    void addIntValue(IntRestrictions r); // 添加整数值的限制。

    /** 与字符串类型的DATA属性关联的值（按文件中的顺序） */
    const std::vector<std::string> &stringValues() const; // 返回字符串值的列表。

    /** 与浮点数类型的DATA属性关联的值（按文件中的顺序） */
    const std::vector<double> &floatValues() const; // 返回浮点数值的列表。

    /** 与整数类型的DATA属性关联的值（按文件中的顺序） */
    const std::vector<int> &intValues() const; // 返回整数值的列表。

    /** 字符串类型DATA属性的限制 */
    const std::vector<StringRestrictions> &stringRestrictions() const; // 返回字符串限制的列表。

    /** 浮点数类型DATA属性的限制 */
    const std::vector<FloatRestrictions> &floatRestrictions() const; // 返回浮点数限制的列表。

    /** 整数类型DATA属性的限制 */
    const std::vector<IntRestrictions> &intRestrictions() const; // 返回整数限制的列表。

private:
    std::vector<std::string> _stringValues; // 字符串值的列表。
    std::vector<double> _floatValues;       // 浮点数值的列表。
    std::vector<int> _intValues;           // 整数值的列表。
    std::vector<StringRestrictions> _stringRestrictions; // 字符串限制的列表。
    std::vector<FloatRestrictions> _floatRestrictions;   // 浮点数限制的列表。
    std::vector<IntRestrictions> _intRestrictions;       // 整数限制的列表。
};

#endif