constexpr auto _s1 = __TIME__[6] - 48;
constexpr auto _s2 = __TIME__[7] - 48;
constexpr auto _h1 = __TIME__[0] - 48;
constexpr auto _h2 = __TIME__[1] - 48;
constexpr auto _m1 = __TIME__[3] - 48;
constexpr auto _m2 = __TIME__[4] - 48;
constexpr auto __tvtodw(__int64 high, __int64 low) { return high * 10 + low; }
constexpr auto _HOURS = __tvtodw(_h1, _h2) + 1;
constexpr auto _MINUTES = __tvtodw(_m1, _m2);
constexpr auto _SECONDS = __tvtodw(_s1, _s2);
constexpr auto _TIMEVAL = (_SECONDS + (_MINUTES * 60) + (_HOURS * 60 * 60));

inline auto fantastic(volatile __int64 _param_value)
{
    volatile auto x = _param_value ^ _SECONDS;
    volatile auto part1 = _SECONDS;
    volatile auto part2 = (_MINUTES ^ _m2) * _s2;
    volatile auto part3 = (_s1 * _MINUTES / _HOURS);
    volatile auto partEnd1 = (part1 + part2 + 1);
    volatile auto partEnd2 = x - part3;
    volatile auto partEnd = partEnd2 / partEnd1;

    return partEnd;
}

#define fantastic_arg(x) (((x * _SECONDS) + (x * (_MINUTES ^ _m2) * _s2) + (_s1 * _MINUTES / _HOURS) + x) ^ _SECONDS)
#define xor_fantastic(x) (fantastic(fantastic_arg(x)))
#define junk_code(x) (xor_fantastic(x+_TIMEVAL))
