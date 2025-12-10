
local_includes = []
foreach inc : JFM_includes
    local_includes += join_paths('../../', inc)
endforeach

local_args = []
if use_debug
    local_args += '-DJFM_DEBUG'
endif

libcalculator_gpu = shared_library('calculator_gpu',
    sources: 'CalculatorGpu/CalculatorGpu.cu',
    include_directories: local_includes,
    cuda_args: ['-std=c++20', local_args, '-gencode=arch=compute_86,code=sm_86', '-DJFM_COMPILER_CUDA'],
    install: true)

libcalculator_gpu_dep = declare_dependency(
    link_with: libcalculator_gpu)
