#include <hip/hip_runtime.h>
#include <iostream>
#include <vector>

int main() {
    // 初始化HIP
    hipInit(0);
    hipSetDevice(0);
    
    // 测试参数
    const size_t test_size = 1024 * 1024; // 1MB数据
    const size_t data_size = test_size * sizeof(float);
    
    // 分配内存
    std::vector<float> h_data(test_size, 1.0f);
    float* d_data;
    hipMalloc(&d_data, data_size);
    
    // 执行DMA拷贝：主机到GPU
    hipMemcpy(d_data, h_data.data(), data_size, hipMemcpyHostToDevice);
    
    // 清理资源
    hipFree(d_data);
    
    std::cout << "主机到GPU DMA拷贝完成" << std::endl;
    return 0;
}
