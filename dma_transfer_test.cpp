#include <hip/hip_runtime.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <cassert>

// 简单的测试内核
__global__ void testKernel(float* data, size_t size) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < size) {
        data[idx] = data[idx] * 2.0f; // 简单的操作
    }
}

// DMA传输测试类
class DMATransferTest {
private:
    size_t test_size;
    std::vector<float> h_data;
    float* d_data;
    float* h_pinned_data;
    
public:
    DMATransferTest(size_t size) : test_size(size), d_data(nullptr), h_pinned_data(nullptr) {
        h_data.resize(test_size);
        
        // 生成随机测试数据
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(0.0f, 1.0f);
        
        for (size_t i = 0; i < test_size; i++) {
            h_data[i] = dis(gen);
        }
    }
    
    ~DMATransferTest() {
        cleanup();
    }
    
    bool initialize() {
        std::cout << "=== 初始化DMA传输测试 ===" << std::endl;
        
        // 分配GPU内存
        hipError_t error = hipMalloc(&d_data, test_size * sizeof(float));
        if (error != hipSuccess) {
            std::cerr << "GPU内存分配失败: " << hipGetErrorString(error) << std::endl;
            return false;
        }
        
        // 分配固定内存（pinned memory）
        error = hipHostMalloc(&h_pinned_data, test_size * sizeof(float), hipHostMallocDefault);
        if (error != hipSuccess) {
            std::cerr << "固定内存分配失败: " << hipGetErrorString(error) << std::endl;
            hipFree(d_data);
            return false;
        }
        
        // 复制数据到固定内存
        memcpy(h_pinned_data, h_data.data(), test_size * sizeof(float));
        
        std::cout << "测试数据大小: " << test_size * sizeof(float) / (1024 * 1024) << " MB" << std::endl;
        std::cout << "初始化完成" << std::endl;
        
        return true;
    }
    
    void testRegularTransfer() {
        std::cout << "\n=== 常规传输测试 ===" << std::endl;
        
        // CPU到GPU传输
        auto start = std::chrono::high_resolution_clock::now();
        hipError_t error = hipMemcpy(d_data, h_data.data(), test_size * sizeof(float), hipMemcpyHostToDevice);
        auto end = std::chrono::high_resolution_clock::now();
        
        if (error != hipSuccess) {
            std::cerr << "CPU到GPU传输失败: " << hipGetErrorString(error) << std::endl;
            return;
        }
        
        auto cpu_to_gpu_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double cpu_to_gpu_bandwidth = (test_size * sizeof(float)) / (cpu_to_gpu_time.count() * 1e-6) / 1e9;
        
        std::cout << "CPU到GPU传输时间: " << cpu_to_gpu_time.count() << " us" << std::endl;
        std::cout << "CPU到GPU带宽: " << cpu_to_gpu_bandwidth << " GB/s" << std::endl;
        
        // GPU到CPU传输
        start = std::chrono::high_resolution_clock::now();
        error = hipMemcpy(h_data.data(), d_data, test_size * sizeof(float), hipMemcpyDeviceToHost);
        end = std::chrono::high_resolution_clock::now();
        
        if (error != hipSuccess) {
            std::cerr << "GPU到CPU传输失败: " << hipGetErrorString(error) << std::endl;
            return;
        }
        
        auto gpu_to_cpu_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double gpu_to_cpu_bandwidth = (test_size * sizeof(float)) / (gpu_to_cpu_time.count() * 1e-6) / 1e9;
        
        std::cout << "GPU到CPU传输时间: " << gpu_to_cpu_time.count() << " us" << std::endl;
        std::cout << "GPU到CPU带宽: " << gpu_to_cpu_bandwidth << " GB/s" << std::endl;
        
        std::cout << "带宽比例 (GPU→CPU / CPU→GPU): " << gpu_to_cpu_bandwidth / cpu_to_gpu_bandwidth << "x" << std::endl;
    }
    
    void testPinnedMemoryTransfer() {
        std::cout << "\n=== 固定内存传输测试 ===" << std::endl;
        
        // 使用固定内存进行CPU到GPU传输
        auto start = std::chrono::high_resolution_clock::now();
        hipError_t error = hipMemcpy(d_data, h_pinned_data, test_size * sizeof(float), hipMemcpyHostToDevice);
        auto end = std::chrono::high_resolution_clock::now();
        
        if (error != hipSuccess) {
            std::cerr << "固定内存CPU到GPU传输失败: " << hipGetErrorString(error) << std::endl;
            return;
        }
        
        auto pinned_cpu_to_gpu_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double pinned_cpu_to_gpu_bandwidth = (test_size * sizeof(float)) / (pinned_cpu_to_gpu_time.count() * 1e-6) / 1e9;
        
        std::cout << "固定内存CPU到GPU传输时间: " << pinned_cpu_to_gpu_time.count() << " us" << std::endl;
        std::cout << "固定内存CPU到GPU带宽: " << pinned_cpu_to_gpu_bandwidth << " GB/s" << std::endl;
        
        // 使用固定内存进行GPU到CPU传输
        start = std::chrono::high_resolution_clock::now();
        error = hipMemcpy(h_pinned_data, d_data, test_size * sizeof(float), hipMemcpyDeviceToHost);
        end = std::chrono::high_resolution_clock::now();
        
        if (error != hipSuccess) {
            std::cerr << "固定内存GPU到CPU传输失败: " << hipGetErrorString(error) << std::endl;
            return;
        }
        
        auto pinned_gpu_to_cpu_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double pinned_gpu_to_cpu_bandwidth = (test_size * sizeof(float)) / (pinned_gpu_to_cpu_time.count() * 1e-6) / 1e9;
        
        std::cout << "固定内存GPU到CPU传输时间: " << pinned_gpu_to_cpu_time.count() << " us" << std::endl;
        std::cout << "固定内存GPU到CPU带宽: " << pinned_gpu_to_cpu_bandwidth << " GB/s" << std::endl;
        
        std::cout << "固定内存带宽比例 (GPU→CPU / CPU→GPU): " << pinned_gpu_to_cpu_bandwidth / pinned_cpu_to_gpu_bandwidth << "x" << std::endl;
    }
    
    void testAsyncTransfer() {
        std::cout << "\n=== 异步传输测试 ===" << std::endl;
        
        hipStream_t stream;
        hipError_t error = hipStreamCreate(&stream);
        if (error != hipSuccess) {
            std::cerr << "创建流失败: " << hipGetErrorString(error) << std::endl;
            return;
        }
        
        // 异步CPU到GPU传输
        auto start = std::chrono::high_resolution_clock::now();
        error = hipMemcpyAsync(d_data, h_pinned_data, test_size * sizeof(float), hipMemcpyHostToDevice, stream);
        if (error != hipSuccess) {
            std::cerr << "异步CPU到GPU传输失败: " << hipGetErrorString(error) << std::endl;
            hipStreamDestroy(stream);
            return;
        }
        
        // 等待传输完成
        hipStreamSynchronize(stream);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto async_cpu_to_gpu_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double async_cpu_to_gpu_bandwidth = (test_size * sizeof(float)) / (async_cpu_to_gpu_time.count() * 1e-6) / 1e9;
        
        std::cout << "异步CPU到GPU传输时间: " << async_cpu_to_gpu_time.count() << " us" << std::endl;
        std::cout << "异步CPU到GPU带宽: " << async_cpu_to_gpu_bandwidth << " GB/s" << std::endl;
        
        // 异步GPU到CPU传输
        start = std::chrono::high_resolution_clock::now();
        error = hipMemcpyAsync(h_pinned_data, d_data, test_size * sizeof(float), hipMemcpyDeviceToHost, stream);
        if (error != hipSuccess) {
            std::cerr << "异步GPU到CPU传输失败: " << hipGetErrorString(error) << std::endl;
            hipStreamDestroy(stream);
            return;
        }
        
        hipStreamSynchronize(stream);
        end = std::chrono::high_resolution_clock::now();
        
        auto async_gpu_to_cpu_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double async_gpu_to_cpu_bandwidth = (test_size * sizeof(float)) / (async_gpu_to_cpu_time.count() * 1e-6) / 1e9;
        
        std::cout << "异步GPU到CPU传输时间: " << async_gpu_to_cpu_time.count() << " us" << std::endl;
        std::cout << "异步GPU到CPU带宽: " << async_gpu_to_cpu_bandwidth << " GB/s" << std::endl;
        
        std::cout << "异步带宽比例 (GPU→CPU / CPU→GPU): " << async_gpu_to_cpu_bandwidth / async_cpu_to_gpu_bandwidth << "x" << std::endl;
        
        hipStreamDestroy(stream);
    }
    
    void testZeroCopy() {
        std::cout << "\n=== 零拷贝传输测试 ===" << std::endl;
        
        // 分配零拷贝内存
        float* zero_copy_data;
        hipError_t error = hipHostMalloc(&zero_copy_data, test_size * sizeof(float), hipHostMallocMapped);
        if (error != hipSuccess) {
            std::cerr << "零拷贝内存分配失败: " << hipGetErrorString(error) << std::endl;
            return;
        }
        
        // 获取GPU端指针
        void* d_zero_copy_data_void;
        error = hipHostGetDevicePointer(&d_zero_copy_data_void, zero_copy_data, 0);
        float* d_zero_copy_data = static_cast<float*>(d_zero_copy_data_void);
        if (error != hipSuccess) {
            std::cerr << "获取GPU端零拷贝指针失败: " << hipGetErrorString(error) << std::endl;
            hipHostFree(zero_copy_data);
            return;
        }
        
        // 复制数据到零拷贝内存
        memcpy(zero_copy_data, h_data.data(), test_size * sizeof(float));
        
        std::cout << "零拷贝内存分配成功" << std::endl;
        std::cout << "CPU端指针: " << zero_copy_data << std::endl;
        std::cout << "GPU端指针: " << d_zero_copy_data << std::endl;
        
        // 测试零拷贝访问性能
        auto start = std::chrono::high_resolution_clock::now();
        
        // 在GPU上访问零拷贝内存
        hipLaunchKernelGGL(testKernel, dim3(1), dim3(1), 0, 0, d_zero_copy_data, test_size);
        hipDeviceSynchronize();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto zero_copy_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "零拷贝GPU访问时间: " << zero_copy_time.count() << " us" << std::endl;
        
        hipHostFree(zero_copy_data);
    }
    
    void cleanup() {
        if (d_data) {
            hipFree(d_data);
            d_data = nullptr;
        }
        if (h_pinned_data) {
            hipHostFree(h_pinned_data);
            h_pinned_data = nullptr;
        }
    }
};

int main() {
    std::cout << "=== DMA传输性能测试 ===" << std::endl;
    
    // 初始化HIP
    hipError_t error = hipInit(0);
    if (error != hipSuccess) {
        std::cerr << "HIP初始化失败: " << hipGetErrorString(error) << std::endl;
        return -1;
    }
    
    // 获取设备信息
    int deviceCount;
    hipGetDeviceCount(&deviceCount);
    std::cout << "检测到 " << deviceCount << " 个AMD GPU" << std::endl;
    
    if (deviceCount == 0) {
        std::cerr << "未检测到AMD GPU设备" << std::endl;
        return -1;
    }
    
    hipSetDevice(0);
    hipDeviceProp_t prop;
    hipGetDeviceProperties(&prop, 0);
    std::cout << "GPU: " << prop.name << std::endl;
    std::cout << "显存大小: " << prop.totalGlobalMem / (1024 * 1024) << " MB" << std::endl;
    
    // 测试不同大小的数据传输
    std::vector<size_t> test_sizes = {1024 * 1024, 4 * 1024 * 1024, 16 * 1024 * 1024}; // 1MB, 4MB, 16MB
    
    for (size_t size : test_sizes) {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "测试数据大小: " << size * sizeof(float) / (1024 * 1024) << " MB" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
        DMATransferTest test(size);
        
        if (!test.initialize()) {
            std::cerr << "初始化失败" << std::endl;
            continue;
        }
        
        // 运行各种传输测试
        test.testRegularTransfer();
        test.testPinnedMemoryTransfer();
        test.testAsyncTransfer();
        test.testZeroCopy();
    }
    
    std::cout << "\n=== 测试完成 ===" << std::endl;
    return 0;
}
