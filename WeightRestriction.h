#ifndef WEIGHT_RESTRICTION__H
#define WEIGHT_RESTRICTION__H

/**
 * WeightRestriction 类。
 * 定义了边匹配的权重限制条件，要求边的权重必须小于或大于特定值。
 */
class WeightRestriction
{
public:
    /**
     * 构造函数，用于初始化权重限制对象。
     * @param isLessThan 布尔值，表示权重限制是小于（true）还是大于等于（false）。
     * @param value 双精度浮点数，表示权重的限制值。
     */
    WeightRestriction(bool isLessThan, double value);

    /**
     * 获取权重限制的类型。
     * @return 如果返回 true，表示限制为小于；如果返回 false，表示限制为大于等于。
     */
    bool isLessThan() const { return _isLessThan; }

    /**
     * 获取权重限制值。
     * @return 返回权重限制值（最大值或最小值，取决于限制类型）。
     */
    double value() const { return _value; }

private:
    double _value;      // 权重限制值（最大值或最小值）
    bool _isLessThan;   // 是否为小于限制（true 表示小于，false 表示大于等于）
};

#endif