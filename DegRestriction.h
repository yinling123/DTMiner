/* 
 * File:   DegRestriction.h
 * Author: D3M430
 *
 * Created on February 28, 2017, 9:18 AM
 */

#ifndef DEGRESTRICTION_H
#define DEGRESTRICTION_H

#include <string>

/**
 * DegRestriction.
 * 定义节点匹配的限制条件，其中顶点的度数（出度或入度）必须满足特定边类型的限制。
 * 限制可以是大于或小于某个值。
 */
class DegRestriction {
public:
    /**
     * 构造函数，用于初始化DegRestriction对象。
     * @param edgeType 边的类型（如“friend”、“follow”等）。
     * @param isOutDeg 是否为出度（true表示出度，false表示入度）。
     * @param isLessThan 是否为小于限制（true表示小于，false表示大于）。
     * @param value 限制的度数值。
     */
    DegRestriction(const std::string &edgeType, bool isOutDeg, bool isLessThan, int value);

    /**
     * 返回边的类型。
     * @return 边类型的字符串。
     */
    const std::string &edgeType() const { return _edgeType; }

    /**
     * 判断是否为出度限制。
     * @return 如果是出度限制返回true，否则返回false。
     */
    bool isOutDeg() const { return _isOutDeg; }

    /**
     * 判断是否为小于限制。
     * @return 如果是小于限制返回true，否则返回false。
     */
    bool isLessThan() const { return _isLessThan; }

    /**
     * 返回限制的度数值。
     * @return 度数的整数值。
     */
    int value() const { return _value; }

private:
    std::string _edgeType; // 边的类型。
    int _value;            // 限制的度数值。
    bool _isOutDeg;        // 是否为出度限制。
    bool _isLessThan;      // 是否为小于限制。
};

#endif /* DEGRESTRICTION_H */