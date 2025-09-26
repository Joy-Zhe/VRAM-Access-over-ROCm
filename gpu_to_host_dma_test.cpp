#include <hip/hip_runtime.h>
#include <iostream>
#include <vector>

int main() {
    // 初始化HIP
    hipInit(0);
    hipSetDevice(0);
    
    // 测试参数
    const size_t test_size = 4;
    const size_t data_size = test_size * sizeof(float);
    
    // 分配内存
    std::vector<float> h_data(test_size);
    float* d_data;
    hipMalloc(&d_data, data_size);
    
    // 打印hipMalloc得到的GPU地址
    std::cout << "hipMalloc分配的GPU地址: 0x" << std::hex << d_data << std::dec << std::endl;
    
    // 初始化GPU数据
    // std::vector<float> init_data(test_size, 2.0f);
    // hipMemcpy(d_data, init_data.data(), data_size, hipMemcpyHostToDevice);
    
    // 执行DMA拷贝：GPU到主机
    hipMemcpy(h_data.data(), d_data, data_size, hipMemcpyDeviceToHost);
    
    // 打印拷贝到的数据
    // std::cout << "拷贝到主机的数据: ";
    // for (size_t i = 0; i < test_size; ++i) {
    //     std::cout << h_data[i] << " ";
    // }
    // std::cout << std::endl;
    
    // 清理资源
    hipFree(d_data);
    
    std::cout << "GPU到主机DMA拷贝完成" << std::endl;
    return 0;
}
