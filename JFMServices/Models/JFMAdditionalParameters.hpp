#pragma once
#include "macros.h"
#include "JFMParameters.hpp"

namespace JFMService
{
    enum FixingConfiguration : uint32_t
    {
        A = BIT(0),
        I0 = BIT(1),
        Rs = BIT(2),
        Rsh = BIT(3),
        alpha = BIT(4),
        Rsh2 = BIT(5),
        Isc = BIT(6),
    };

    inline uint32_t toUnderlying(FixingConfiguration val) {return static_cast<uint32_t>(val);}

    inline FixingConfiguration operator|(FixingConfiguration lhs, FixingConfiguration rhs) {return static_cast<FixingConfiguration>(toUnderlying(lhs) | toUnderlying(rhs));}
    inline FixingConfiguration operator|=(FixingConfiguration& lhs, FixingConfiguration rhs)
    {
        lhs = static_cast<FixingConfiguration>(toUnderlying(lhs) | toUnderlying(rhs));
        return lhs;
    }
    inline FixingConfiguration operator&(FixingConfiguration lhs, FixingConfiguration rhs) {return static_cast<FixingConfiguration>(toUnderlying(lhs) & toUnderlying(rhs));}

    inline FixingConfiguration operator>>(FixingConfiguration lhs, uint32_t shift) {return static_cast<FixingConfiguration>(toUnderlying(lhs) >> shift);}

    inline FixingConfiguration& operator>>=(FixingConfiguration& lhs, uint32_t shift) {lhs = lhs >> shift;return lhs;}

    struct JFMAdditionalParameters
    {
    public:
        JFMAdditionalParameters();
        JFMAdditionalParameters(double Temperature);
        JFMAdditionalParameters(double Temperature,
                                JFMParameters fixingValues,
                                FixingConfiguration configuration);

        double Temperature;
        JFMParameters fixingValues;
        FixingConfiguration fixingConfiguration;
    };
} // namespace JFMService
