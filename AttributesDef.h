#ifndef ATTRIBUTESDEF_H
#define ATTRIBUTESDEF_H

#include <string>
#include <vector>
#include "Attributes.h"

/** 
 * 数据类型枚举，表示属性的数据类型。
 * 包括字符串（STRING）、整数（INT）和浮点数（FLOAT）。
 */
enum DataType { STRING, INT, FLOAT };

/** 
 * 属性类型枚举，表示属性的种类。
 * 包括源ID（SOURCE_ID）、目标ID（DEST_ID）、节点ID（NODE_ID）、时间（TIME）和数据（DATA）。
 */
enum AttributeType { SOURCE_ID, DEST_ID, NODE_ID, TIME, DATA };

/**
 * 定义与特定图相关的属性集合。
 * 该类用于描述图中属性的类型、数据类型和名称，并提供相关操作。
 */
class AttributesDef {
public:
    AttributesDef(); // 构造函数，初始化属性定义。

    /**
     * 向当前定义中添加一个属性。
     * @param attType 属性的类型（如ID、TIME或DATA）。
     * @param dataType 属性的数据类型（如int、string或float）。
     * @param name 属性的名称。
     */
    void addAttribute(AttributeType attType, DataType dataType, const std::string &name);

    /**
     * 检查传入的Attributes对象是否与当前定义匹配。
     * @param a 要检查的Attributes对象。
     * @return 如果匹配返回true，否则返回false。
     */
    bool isSizeMatch(const Attributes &a) const;

    /**
     * 返回所有属性类型的列表（区分ID、TIME或DATA），按原始顺序排列。
     * @return 属性类型的向量。
     */
    const std::vector<AttributeType> &attributeTypes() const;

    /**
     * 返回所有属性数据类型的列表（包括非DATA属性），按原始顺序排列。
     * @return 数据类型的向量。
     */
    const std::vector<DataType> &dataTypes() const;

    /**
     * 返回所有属性名称的列表（包括非DATA属性），按原始顺序排列。
     * @return 属性名称的向量。
     */
    const std::vector<std::string> &attributeNames() const;

    /**
     * 返回字符串类型的DATA属性数量。
     * @return 字符串属性的数量。
     */
    int numStringValues() const;

    /**
     * 返回整数类型的DATA属性数量。
     * @return 整数属性的数量。
     */
    int numIntValues() const;

    /**
     * 返回浮点数类型的DATA属性数量。
     * @return 浮点数属性的数量。
     */
    int numFloatValues() const;

    /**
     * 判断两个AttributesDef对象是否相等。
     * @param rhs 要比较的另一个AttributesDef对象。
     * @return 如果相等返回true，否则返回false。
     */
    bool operator==(const AttributesDef &rhs) const;

    /**
     * 判断两个AttributesDef对象是否不相等。
     * @param rhs 要比较的另一个AttributesDef对象。
     * @return 如果不相等返回true，否则返回false。
     */
    bool operator!=(const AttributesDef &rhs) const;

private:
    std::vector<AttributeType> _attributeTypes; // 属性类型的列表。
    std::vector<DataType> _dataTypes;          // 数据类型的列表。
    std::vector<std::string> _names;           // 属性名称的列表。
    int _numStringValues, _numIntValues, _numFloatValues; // 分别记录字符串、整数和浮点数类型的DATA属性数量。
};

#endif /* ATTRIBUTESDEF_H */