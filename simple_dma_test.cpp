#include <hip/hip_runtime.h>
#include <iostream>
#include <vector>
#include <random>
#include <cassert>

int main() {
    std::cout << "=== 简单GPU VRAM与主机内存DMA拷贝测试 ===" << std::endl;
    
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
    
    // 测试参数
    const size_t test_size = 1024 * 1024; // 1MB数据
    const size_t data_size = test_size * sizeof(float);
    
    std::cout << "\n测试数据大小: " << data_size / (1024 * 1024) << " MB" << std::endl;
    
    // 分配主机内存
    std::vector<float> h_data(test_size);
    
    // 生成测试数据
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    for (size_t i = 0; i < test_size; i++) {
        h_data[i] = dis(gen);
    }
    
    // 分配GPU内存
    float* d_data;
    error = hipMalloc(&d_data, data_size);
    if (error != hipSuccess) {
        std::cerr << "GPU内存分配失败: " << hipGetErrorString(error) << std::endl;
        return -1;
    }
    
    std::cout << "内存分配完成" << std::endl;
    std::cout << "主机内存地址: " << h_data.data() << std::endl;
    std::cout << "GPU内存地址: " << d_data << std::endl;
    
    // 执行DMA拷贝：主机到GPU
    std::cout << "\n=== 执行DMA拷贝：主机到GPU ===" << std::endl;
    
    error = hipMemcpy(d_data, h_data.data(), data_size, hipMemcpyHostToDevice);
    
    if (error != hipSuccess) {
        std::cerr << "主机到GPU DMA拷贝失败: " << hipGetErrorString(error) << std::endl;
        hipFree(d_data);
        return -1;
    }
    
    std::cout << "主机到GPU DMA拷贝完成" << std::endl;
    
    // 执行DMA拷贝：GPU到主机
    std::cout << "\n=== 执行DMA拷贝：GPU到主机 ===" << std::endl;
    
    error = hipMemcpy(h_data.data(), d_data, data_size, hipMemcpyDeviceToHost);
    
    if (error != hipSuccess) {
        std::cerr << "GPU到主机DMA拷贝失败: " << hipGetErrorString(error) << std::endl;
        hipFree(d_data);
        return -1;
    }
    
    std::cout << "GPU到主机DMA拷贝完成" << std::endl;
    
    
    // 清理资源
    hipFree(d_data);
    
    std::cout << "\n=== 简单DMA拷贝测试完成 ===" << std::endl;
    return 0;
}
