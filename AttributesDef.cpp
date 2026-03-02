#include "AttributesDef.h"
#include "Attributes.h"

using namespace std;

// 构造函数，初始化字符串、浮点数和整数值的数量为0。
AttributesDef::AttributesDef() {
    _numStringValues = 0;
    _numFloatValues = 0;
    _numIntValues = 0;
}

// 添加一个属性，包括属性类型、数据类型和名称。
void AttributesDef::addAttribute(AttributeType attributeType, DataType dataType, const std::string &name) {
    _attributeTypes.push_back(attributeType); // 将属性类型添加到列表中。
    _dataTypes.push_back(dataType);          // 将数据类型添加到列表中。
    _names.push_back(name);                  // 将属性名称添加到列表中。
    if (attributeType == AttributeType::DATA) { // 如果是DATA类型的属性：
        if (dataType == DataType::STRING)       // 如果数据类型是字符串：
            _numStringValues++;                 // 增加字符串值计数。
        else if (dataType == DataType::FLOAT)   // 如果数据类型是浮点数：
            _numFloatValues++;                  // 增加浮点数值计数。
        else if (dataType == DataType::INT)     // 如果数据类型是整数：
            _numIntValues++;                    // 增加整数值计数。
        else                                    // 如果数据类型未知：
            throw "Can't add attribute to definition. Unknown type."; // 抛出异常。
    }
}

// 检查传入的Attributes对象的大小是否与当前定义匹配。
bool AttributesDef::isSizeMatch(const Attributes &a) const {
    return a.floatValues().size() == _numFloatValues && // 浮点数值数量匹配。
           a.intValues().size() == _numIntValues &&     // 整数值数量匹配。
           a.stringValues().size() == _numStringValues; // 字符串值数量匹配。
}

// 返回属性类型的列表。
const std::vector<AttributeType> &AttributesDef::attributeTypes() const {
    return _attributeTypes;
}

// 返回数据类型的列表。
const std::vector<DataType> &AttributesDef::dataTypes() const {
    return _dataTypes;
}

// 返回属性名称的列表。
const std::vector<std::string> &AttributesDef::attributeNames() const {
    return _names;
}

// 返回字符串值的数量。
int AttributesDef::numStringValues() const { return _numStringValues; }

// 返回整数值的数量。
int AttributesDef::numIntValues() const { return _numIntValues; }

// 返回浮点数值的数量。
int AttributesDef::numFloatValues() const { return _numFloatValues; }

// 比较两个AttributesDef对象是否相等，比较属性数量，而非比较顶点和边数量
bool AttributesDef::operator==(const AttributesDef &rhs) const {
    int n = this->_names.size(); // 获取当前对象的属性名称数量。
    if (n != rhs.attributeNames().size()) // 如果属性名称数量不匹配：
        return false;                     // 返回false。
    for (int i = 0; i < n; i++) {         // 遍历所有属性：
        if (_names[i] != rhs._names[i])   // 如果属性名称不匹配：
            return false;                 // 返回false。
        if (this->_dataTypes[i] != rhs._dataTypes[i]) // 如果数据类型不匹配：
            return false;                 // 返回false。
        if (this->_attributeTypes[i] != rhs._attributeTypes[i]) // 如果属性类型不匹配：
            return false;                 // 返回false。
    }
    return true; // 如果所有属性都匹配，返回true。
}

// 比较两个AttributesDef对象是否不相等。
bool AttributesDef::operator!=(const AttributesDef &rhs) const {
    return !(*this == rhs); // 调用==运算符的结果并取反。
}