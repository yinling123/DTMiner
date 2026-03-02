#include "DegRestriction.h"
#include <limits.h>

using namespace std;

/**
 * DegRestriction 类的构造函数。
 * 初始化一个度数限制对象，用于定义节点的出度或入度的约束条件。
 * 
 * @param edgeType 边的类型，用于区分不同的边类别。
 * @param isOutDeg 指定是出度还是入度：
 *                 - true 表示出度。
 *                 - false 表示入度。
 * @param isLessThan 指定比较操作符：
 *                   - true 表示小于（<）。
 *                   - false 表示大于等于（>=）。
 * @param value 度数的阈值，用于与节点的实际度数进行比较。
 */
DegRestriction::DegRestriction(const std::string& edgeType, bool isOutDeg, bool isLessThan, int value)
{
    _edgeType = edgeType; // 设置边的类型。
    _isOutDeg = isOutDeg; // 设置是否为出度。
    _isLessThan = isLessThan; // 设置比较操作符。
    _value = value; // 设置度数的阈值。
}