#include "Attributes.h"
#include <float.h>
#include <limits.h>

using namespace std;

// Restrictions类的构造函数，初始化默认值。
// _exact：是否需要精确匹配，默认为true。
// _any：是否允许任意值，默认为false。
// _hasMin 和 _hasMax：是否有最小值或最大值限制，默认为false。
Restrictions::Restrictions() {       
    _exact = true;
    _any = false;
    _hasMin = false;
    _hasMax = false;
}

// 允许任意值，设置_any为true，并禁用_exact。
void Restrictions::allowAny() {
    _any = true;
    _exact = false;
}

// 设置最小值限制，启用_hasMin并禁用_exact。
void Restrictions::setMin() {
    _hasMin = true;
    _exact = false;
}

// 设置最大值限制，启用_hasMax并禁用_exact。
void Restrictions::setMax() {
    _hasMax = true;
    _exact = false;
}

// 返回是否允许任意值。
bool Restrictions::any() const { return _any; }

// 返回是否有最小值限制。
bool Restrictions::hasMin() const { return _hasMin; }

// 返回是否有最大值限制。
bool Restrictions::hasMax() const { return _hasMax; }

// 返回是否需要精确匹配。
bool Restrictions::exact() const { return _exact; }

// FloatRestrictions类的构造函数，初始化浮点数的最小值和最大值。
FloatRestrictions::FloatRestrictions() {       
    _min = -FLT_MAX; // FLT_MIN实际上是接近0的值，因此使用-FLT_MAX表示最小值。
    _max = FLT_MAX;  // 最大值为FLT_MAX。
}

// 设置浮点数的最小值限制。
void FloatRestrictions::setMin(float value) {
    Restrictions::setMin(); // 调用基类方法启用最小值限制。
    _min = value;         // 设置新的最小值。
}

// 设置浮点数的最大值限制。
void FloatRestrictions::setMax(float value) {
    Restrictions::setMax(); // 调用基类方法启用最大值限制。
    _max = value;         // 设置新的最大值。
}

// 返回浮点数的最小值。
float FloatRestrictions::min() const { return _min; }

// 返回浮点数的最大值。
float FloatRestrictions::max() const { return _max; }

// IntRestrictions类的构造函数，初始化整数的最小值和最大值。
IntRestrictions::IntRestrictions() {       
    _min = INT_MIN; // 整数最小值为INT_MIN。
    _max = INT_MAX; // 整数最大值为INT_MAX。
}

// 设置整数的最小值限制。
void IntRestrictions::setMin(int value) {
    Restrictions::setMin(); // 调用基类方法启用最小值限制。
    _min = value;         // 设置新的最小值。
}

// 设置整数的最大值限制。
void IntRestrictions::setMax(int value) {
    Restrictions::setMax(); // 调用基类方法启用最大值限制。
    _max = value;         // 设置新的最大值。
}

// 返回整数的最小值。
int IntRestrictions::min() const { return _min; }

// 返回整数的最大值。
int IntRestrictions::max() const { return _max; }

// Attributes类的构造函数。
Attributes::Attributes() {}

// 添加一个具体的字符串值。
void Attributes::addStringValue(const std::string &value) {
    _stringValues.push_back(value);          // 将字符串值添加到列表中。
    _stringRestrictions.push_back(StringRestrictions()); // 添加默认的字符串限制。
}

// 添加一个字符串限制而非具体值。
void Attributes::addStringValue(StringRestrictions r) {
    _stringValues.push_back(string());      // 添加一个空字符串值。
    _stringRestrictions.push_back(r);       // 添加指定的字符串限制。
}

// 添加一个具体的浮点数值。
void Attributes::addFloatValue(float value) {
    _floatValues.push_back(value);          // 将浮点数值添加到列表中。
    _floatRestrictions.push_back(FloatRestrictions()); // 添加默认的浮点数限制。
}

// 添加一个浮点数限制而非具体值。
void Attributes::addFloatValue(FloatRestrictions r) {
    _floatValues.push_back(0.0);            // 添加默认浮点数值（0.0）。
    _floatRestrictions.push_back(r);        // 添加指定的浮点数限制。
}

// 添加一个具体的整数值。
void Attributes::addIntValue(int value) {
    _intValues.push_back(value);            // 将整数值添加到列表中。
    _intRestrictions.push_back(IntRestrictions()); // 添加默认的整数限制。
}

// 添加一个整数限制而非具体值。
void Attributes::addIntValue(IntRestrictions r) {
    _intValues.push_back(0);                // 添加默认整数值（0）。
    _intRestrictions.push_back(r);          // 添加指定的整数限制。
}

// 返回字符串值的列表。
const std::vector<std::string> &Attributes::stringValues() const { return _stringValues; }

// 返回浮点数值的列表。
const std::vector<double> &Attributes::floatValues() const { return _floatValues; }

// 返回整数值的列表。
const std::vector<int> &Attributes::intValues() const { return _intValues; }

// 返回字符串限制的列表。
const std::vector<StringRestrictions> &Attributes::stringRestrictions() const { return _stringRestrictions; }

// 返回浮点数限制的列表。
const std::vector<FloatRestrictions> &Attributes::floatRestrictions() const { return _floatRestrictions; }

// 返回整数限制的列表。
const std::vector<IntRestrictions> &Attributes::intRestrictions() const { return _intRestrictions; }