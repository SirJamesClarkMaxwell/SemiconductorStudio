#include "CalculatorGpu.h"
#include "macros.h"
#include "cuda_lambert.h"

#include <math.h>
#include <array>
#include <span>
#include <type_traits>
#include "macros.h"

#include <cuda_runtime.h>

#define pad_n(__n)      std::right << std::setw(__n)

namespace
{
using namespace JFMService;
using namespace JFMService::FittingService;

typedef double cuda_float_t;

static const int g_params_count = 4; // Note: this is per model type (e.g. 4, 5, etc.)
} // namespace anonymous

namespace cuda_hidden
{
__host__ void
inspectGpuMemory(const char *tag="")
{
    size_t freeMemoryInBytes;
    size_t totalMemoryInBytes;
    constexpr static const size_t MB_1 = 1024 * 1024;

    cudaMemGetInfo(&freeMemoryInBytes, &totalMemoryInBytes);

    Info() << "Gpu device memory: [ " << tag << " ]\n";
    Info() << pad_n(8) << "total: " << pad_n(8) << (totalMemoryInBytes/MB_1) << "MB\n";
    Info() << pad_n(8) << "free: "  << pad_n(8) << (freeMemoryInBytes/MB_1) << "MB\n";
}

enum class ArrType {
    // Device array fitted by Host, output it copied back to the original array
    InputOutput,
    // Device array fitted by Host, output is ignored
    Input,
    // Device array it preinitialized with zeros, output it copied back to the original array
    Output,
};

template <typename T>
struct Arr {
private:
    T *m_data;
    size_t m_singleSize;
    size_t m_size;
    size_t m_count;
    T *m_dData;
    ArrType m_type;
    // Indicates number of copies of `m_data` that will make up the `m_dData`
    // This is used to enable mulitple threads access same array that is modified inside kernel
    // Fixed value-arrays have this value as 1 (no spares created)
    int m_duplicateCount;

    void InitializeCudaArray()
    {
        cudaError_t err;
        JFM_ASSERT(m_data != nullptr);
        // we don't want empty arrays
        JFM_ASSERT(m_size);

        JFM_ASSERT_WITH_MSG(
                ((err = cudaMalloc((void **)&m_dData, m_size)) == cudaSuccess),
                "Cuda malloc for array of size : %zu failed"
                " (err : %d , host array [ addr : 0x%lX ])\n",
                m_size, err, (uintptr_t)m_data);

        Reset();

        Verbose() << "[Cuda][Debug] Created device buffer [ addr : " << std::hex << m_dData
                  << " ] for host array [ addr : " << std::hex << m_data << " , size : " << m_size << " ]\n";
    }

    void Copy()
    {
        if ((m_type == ArrType::InputOutput) || (m_type == ArrType::Output))
        {
            cudaError_t err;
            JFM_ASSERT(m_data != nullptr);
            JFM_ASSERT_WITH_MSG(
                ((err = cudaMemcpy(m_data, m_dData, m_size, cudaMemcpyDeviceToHost)) == cudaSuccess),
                "Cuda memcpy(Device->Host) for arr : 0x%lX, of size : %zu failed"
                " (err : %d, host array [ addr : 0x%lX ])\n",
                (uintptr_t)m_dData, m_size, err, (uintptr_t)m_data);
        }
    }

    void Reset()
    {
        if ((m_type == ArrType::Input) || (m_type == ArrType::InputOutput))
        {
            cudaError_t err;
            size_t off = 0;

            for (int ndx = 0; ndx < m_duplicateCount; ++ndx, off += m_singleSize)
            {
                JFM_ASSERT_WITH_MSG(
                        ((err = cudaMemcpy(m_dData+off, m_data, m_singleSize, cudaMemcpyHostToDevice)) == cudaSuccess),
                        "Cuda memcpy(Host->Device) for arr : 0x%lX, of size : %zu failed"
                        " (err : %d)\n",
                        (uintptr_t)m_data, m_size, err);
            }
        }
    }

protected:
    Arr(ArrType type, size_t count, size_t duplicateCount)

        : m_data(nullptr)
        , m_singleSize(count * sizeof(T))
        , m_size(m_singleSize * duplicateCount)
        , m_count(count)
        , m_type(type)
        , m_duplicateCount(duplicateCount)
    {
    }

    void Initialize(T *data)
    {
        JFM_ASSERT(data);
        this->m_data = data;
        InitializeCudaArray();
    }

public:
    Arr(ArrType type, T *data, size_t count, size_t duplicateCount=1)
        : m_data(data)
        , m_singleSize(count * sizeof(T))
        , m_size(m_singleSize * duplicateCount)
        , m_count(count)
        , m_type(type)
        , m_duplicateCount(duplicateCount)
    {
        InitializeCudaArray();
    }

    Arr(const T *data, size_t count)
        : m_data(const_cast<T *>(data))
        , m_singleSize(count * sizeof(T))
        , m_size(m_singleSize)
        , m_count(count)
        , m_type(ArrType::Input)
        , m_duplicateCount(1)
    {
        InitializeCudaArray();
    }

    ~Arr()
    {
        cudaError_t err;
        JFM_ASSERT(m_dData != nullptr);

        // before be destroy _this_ object we want to first transfer data back to host
        Copy();

        JFM_ASSERT_WITH_MSG(
                ((err = cudaFree((void *)m_dData)) == cudaSuccess),
                "Cuda free(m_dData) failed"
                " (err : %d, host array [ addr : 0x%lX , size : %zu ])\n",
                err, (uintptr_t)m_data, m_size);
    }

    T *Get() { return m_dData; }

    size_t Count() const { return m_count; }
};

template <typename T>
struct ArrLazy : public Arr<T>
{
    ArrLazy(ArrType type, size_t count, size_t duplicateCount=1)
        : Arr<T>(type, count, duplicateCount)
    {
    }

    void Reinitialize(T *data)
    {
        this->Initialize(data);
    }
};

template <typename T>
struct ArrEmpty : public Arr<T>
{
    ArrEmpty(ArrType type, size_t count, size_t duplicateCount=1)
        : Arr<T>(type, count, duplicateCount)
    {
        static_assert(std::is_fundamental_v<T> == true,
                      "ArrEmpty only supports primitive types ( { int , double, etc. } )");
        std::vector<T> emptyVec(count);
        this->Initialize(emptyVec.data());
    }
};

__device__ void
model_calculate_current(
    cuda_float_t *current,
    cuda_float_t *voltage,
    size_t length,
    const cuda_float_t T,
    const cuda_float_t I0,
    const cuda_float_t A,
    const cuda_float_t Rs,
    const cuda_float_t Rsh)
{
    // TODO: think about delegating _this_ function for per-thread execution with the count
    //       of threads being `length`

    static const cuda_float_t k = 8.6e-5;
    cuda_float_t I, V;
    cuda_float_t x, I_lw;

    for (size_t ndx = 0; ndx < length; ++ndx) {
        V = voltage[ndx];
        I = current[ndx];

        x = ((I0 * Rs) / (A * k * T)) * exp(V / (A * k * T));

        I_lw = lambert_w0f(x);
        I_lw *= (A * k * T) / Rs;

        I = I_lw + (V - I_lw * Rs) / Rsh;

        current[ndx] = I;
    }
}

__device__ void
calculate_error(
    cuda_float_t *current,
    cuda_float_t *reference_current,
    size_t length,
    cuda_float_t noise,
    cuda_float_t *error)
{
    cuda_float_t accumulatedError = 0.0;
    size_t ndx;

    for (ndx = 0; ndx < length; ++ndx)
    {
        accumulatedError += pow(((log(current[ndx]) - log(reference_current[ndx])) / noise), 2);
    }

    *error = accumulatedError;
}

__global__ void calculate_data(
    cuda_float_t *current,
    cuda_float_t *reference_current,
    cuda_float_t *voltage,
    size_t length,
    cuda_float_t *result_parameters,
    size_t result_parameters_count,
    const cuda_float_t *reference_parameters,
    const cuda_float_t T,
    const cuda_float_t noise,
    cuda_float_t *errors)
{
    static const size_t parameter_count = 4;
    size_t ndx;
    size_t off;

    ndx = blockDim.x * blockIdx.x + threadIdx.x;
    // printf("block dim : %d, block idx : %d, thread idx : %d\n", blockDim.x, blockIdx.x, threadIdx.x);

    if (ndx < result_parameters_count / 4)
    {
        /* <Implements :
             .  NumericStorm::Fitting::Parameters<parameter_size> params;
             .  for (const auto &[id, val] : data.parameters)
             .      params[id] = val;
           >
        */
        size_t param_ndx = ndx * parameter_count;
        // printf("%s:%d : param_ndx : %ld, result parameter count : %ld\n", __func__, __LINE__, param_ndx, result_parameters_count);
        off = ndx * length;

        model_calculate_current(current + off,
                                voltage,
                                length,
                                T,
                                result_parameters[param_ndx + 0],
                                result_parameters[param_ndx + 1],
                                result_parameters[param_ndx + 2],
                                result_parameters[param_ndx + 3]);

        model_calculate_current(reference_current + off,
                                voltage,
                                length,
                                T,
                                reference_parameters[0],
                                reference_parameters[1],
                                reference_parameters[2],
                                reference_parameters[3]);

        calculate_error(current + off,
                        reference_current + off,
                        length,
                        noise,
                        errors + ndx);
    }
}
} // namespace cuda_hidden

void calculatorGpuCall(
    CudaParams calculationParams,
    const MCInput &input,
    std::vector<MCResult> *output,
    size_t totalLength,
    double Temperature,
    double Noise)
{
    constexpr static const size_t parameter_count = 4;

    JFM_UNUSED const size_t memoryPoolSize = calculationParams.memoryPoolSize;
    const int threadsPerBlock = calculationParams.threadsPerBlock;

    const auto &characteristic = input.startingData.initialData.characteristic;
    // TODO: Is `current` output parameter in every model ? Is there a need to treat it as an input ??
    std::vector<cuda_float_t> current{ characteristic.currentData.begin(), characteristic.currentData.end() };
    std::vector<cuda_float_t> voltage{ characteristic.voltageData.begin(), characteristic.voltageData.end() };
    JFM_ASSERT(current.size() == voltage.size());

    std::vector<cuda_float_t> errors(totalLength);
    JFM_ASSERT(errors.size() == output->size());

    const auto referenceParameters =
    [&]() -> std::vector<cuda_float_t> {
        std::vector<cuda_float_t> arr(parameter_count);
        // JFM_ASSERT(input.trueParameters.size() == arr.size());
        ParameterMap param = input.trueParameters;

        for (size_t ndx = 0; ndx < arr.size(); ++ndx)
        {
            arr[ndx] = param[ndx];
        }

        return arr;
    }();

    std::vector<cuda_float_t> currentTotal(totalLength * current.size());
    {
        size_t off = 0;

        for (size_t ndx = 0; ndx < totalLength; ++ndx, off += current.size())
            memcpy(currentTotal.data() + off, current.data(), sizeof(decltype(current[0])) * current.size());
    }

    jfm_debug::DataDumper("gpu : before", current.data(), current.size());
    {
        using namespace cuda_hidden;

        JFM_UNUSED cudaError_t err = cudaGetLastError();
        if (err != cudaSuccess)
        {
            Info() << "Cuda last error : " << err << "\n";
        }

        const size_t result_parameters_count = totalLength * g_params_count;
        std::vector<cuda_float_t> result_parameters(result_parameters_count);
        for (size_t resultNdx = 0; resultNdx < totalLength; ++resultNdx) {
            MCResult& result = output->at(resultNdx);
            ParameterMap& param = result.foundParameters;
            JFM_ASSERT(param.size() == g_params_count); // TODO: move `map` to `array`

            for (size_t i = 0; i < g_params_count; ++i)
                result_parameters[resultNdx * g_params_count + i] = param[i];
        }

        // size_t voltageSize = voltage.size() * sizeof(decltype(voltage[0]));
        // size_t currentSize = current.size() * sizeof(decltype(current[0]));
        // size_t singleResultParametersSize = parameter_count * sizeof(decltype(result_parameters[0]));
        // size_t referenceParametersSize = referenceParameters.size() * sizeof(decltype(referenceParameters[0]));

        // size_t duplicateSize = singleResultParametersSize + currentSize;
        // size_t remainingMemory = memoryPoolSize - voltageSize - referenceParametersSize;
        // size_t duplicateCount = remainingMemory / duplicateSize + !!(remainingMemory % duplicateSize);
        // size_t iterationCount = totalLength / duplicateCount + !!(totalLength % duplicateCount);
        // Info() << "Duplicate size : " << duplicateSize << ", remainingMemory : " << remainingMemory << "\n";
        // Info() << "Duplicate count : " << duplicateCount << ", iterationCount : " << iterationCount << "\n";

        size_t executionDimension = totalLength;

        const size_t blocksPerGrid = executionDimension / threadsPerBlock + !!(executionDimension % threadsPerBlock);
        Info() << "------------------------------------------------------\n";
        Info() << "GPU Calculation Parameters :\n";
        Info() << "\tblocks per grid : " << blocksPerGrid << "\n";
        Info() << "\tthreads per block : " << threadsPerBlock << "\n";
        Info() << "------------------------------------------------------\n";

        ArrLazy<cuda_float_t> dCurrent(ArrType::Input, currentTotal.size());
        ArrEmpty<cuda_float_t> dReferenceCurrent(ArrType::Input, currentTotal.size());
        Arr dVoltage(ArrType::Input, voltage.data(), voltage.size());
        ArrLazy<cuda_float_t> dResultParameters(ArrType::Input, parameter_count * executionDimension);
        Arr dReferenceParameters(referenceParameters.data(), referenceParameters.size());
        Arr dErrors(ArrType::Output, errors.data(), errors.size());

        dResultParameters.Reinitialize(result_parameters.data());
        dCurrent.Reinitialize(currentTotal.data());
        JFM_ASSERT(cudaGetLastError() == cudaSuccess);

        cuda_hidden::inspectGpuMemory("before");

        calculate_data<<<blocksPerGrid, threadsPerBlock>>>(
            dCurrent.Get(),
            dReferenceCurrent.Get(),
            dVoltage.Get(),
            current.size(),
            dResultParameters.Get(),
            dResultParameters.Count(),
            dReferenceParameters.Get(),
            static_cast<cuda_float_t>(Temperature),
            static_cast<cuda_float_t>(Noise/100.0),
            dErrors.Get());

        JFM_ASSERT_WITH_MSG(
            ((err = cudaDeviceSynchronize()) == cudaSuccess),
            "Cuda device synchronize failed (err : %d)\n", err);
    }
    jfm_debug::DataDumper("gpu : after", currentTotal.data(), current.size());
    jfm_debug::DataDumper("gpu : errors", errors.data(), errors.size());

    cuda_hidden::inspectGpuMemory("after");

    // rewrite results back
    for (size_t ndx = 0; ndx < output->size(); ++ndx)
    {
        output->at(ndx).error = errors[ndx];
    }
}
