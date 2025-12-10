#pragma once
#include <valarray>
#include "macros.h"

namespace JFMService
{
    class JFMParameters
    {
    public:
        JFMParameters()
            : m_size(0)
        {
        }

        JFMParameters(std::initializer_list<double> parameters)
            : m_parameters{parameters}
            , m_size(m_parameters.size())
        {
        }

        constexpr void Initialize(const size_t size)
        {
            m_size = size;
            m_parameters.resize(m_size);
        }

        constexpr size_t size() const { return m_size; }

        const std::valarray<double>&
        getParameters() const { return m_parameters; };

        constexpr double &operator[](size_t index)
        {
            JFM_ASSERT(index < m_parameters.size());
            return m_parameters[index];
        }
        constexpr double operator[](size_t index) const
        {
            JFM_ASSERT(index < m_parameters.size());
            return m_parameters[index];
        }

    private:
        std::valarray<double> m_parameters;
        size_t m_size;
    };
} // namespace JFMService
