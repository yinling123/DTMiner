#include "WeightRestriction.h"

/**
 * WeightRestriction 构造函数，用于初始化权重限制对象。
 * @param isLessThan 布尔值，表示权重限制是小于（true）还是大于等于（false）。
 * @param value 双精度浮点数，表示权重的限制值。
 */
WeightRestriction::WeightRestriction(bool isLessThan, double value)
{
    _isLessThan = isLessThan; // 初始化是否为小于限制
    _value = value;           // 初始化权重限制值
}