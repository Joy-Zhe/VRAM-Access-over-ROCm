#include <hip/hip_runtime.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <cassert>

// GPU内核函数：矩阵乘法
__global__ void matrixMultiply(float* A, float* B, float* C, int N) {
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (row < N && col < N) {
        float sum = 0.0f;
        for (int k = 0; k < N; k++) {
            sum += A[row * N + k] * B[k * N + col];
        }
        C[row * N + col] = sum;
    }
}

// 验证结果正确性
bool verifyResult(const std::vector<float>& C, const std::vector<float>& expected, int N) {
    const float tolerance = 1e-3f;
    for (int i = 0; i < N * N; i++) {
        if (std::abs(C[i] - expected[i]) > tolerance) {
            std::cout << "验证失败: C[" << i << "] = " << C[i] 
                      << ", 期望值 = " << expected[i] << std::endl;
            return false;
        }
    }
    return true;
}

// CPU矩阵乘法（用于验证）
void cpuMatrixMultiply(const std::vector<float>& A, const std::vector<float>& B, 
                      std::vector<float>& C, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            float sum = 0.0f;
            for (int k = 0; k < N; k++) {
                sum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    }
}

int main() {
    // 矩阵大小
    const int N = 1024;
    const size_t size = N * N * sizeof(float);
    
    std::cout << "=== AMD GPU 矩阵乘法 Demo ===" << std::endl;
    std::cout << "矩阵大小: " << N << "x" << N << std::endl;
    std::cout << "数据大小: " << size / (1024 * 1024) << " MB" << std::endl;
    
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
    
    // 设置设备
    hipSetDevice(0);
    hipDeviceProp_t prop;
    hipGetDeviceProperties(&prop, 0);
    std::cout << "GPU: " << prop.name << std::endl;
    // std::cout << "计算能力: " << prop.major << "." << prop.minor << std::endl;
    std::cout << "显存大小: " << prop.totalGlobalMem / (1024 * 1024) << " MB" << std::endl;
    
    // 创建随机数据
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    std::vector<float> h_A(N * N);
    std::vector<float> h_B(N * N);
    std::vector<float> h_C(N * N, 0.0f);
    std::vector<float> h_C_cpu(N * N, 0.0f);
    
    // 填充随机数据
    for (int i = 0; i < N * N; i++) {
        h_A[i] = dis(gen);
        h_B[i] = dis(gen);
    }
    
    std::cout << "\n=== 数据传输到GPU ===" << std::endl;
    
    // 分配GPU内存
    float *d_A, *d_B, *d_C;
    error = hipMalloc(&d_A, size);
    if (error != hipSuccess) {
        std::cerr << "GPU内存分配失败 (A): " << hipGetErrorString(error) << std::endl;
        return -1;
    }
    
    error = hipMalloc(&d_B, size);
    if (error != hipSuccess) {
        std::cerr << "GPU内存分配失败 (B): " << hipGetErrorString(error) << std::endl;
        hipFree(d_A);
        return -1;
    }
    
    error = hipMalloc(&d_C, size);
    if (error != hipSuccess) {
        std::cerr << "GPU内存分配失败 (C): " << hipGetErrorString(error) << std::endl;
        hipFree(d_A);
        hipFree(d_B);
        return -1;
    }
    
    // 数据传输到GPU
    auto start = std::chrono::high_resolution_clock::now();
    
    error = hipMemcpy(d_A, h_A.data(), size, hipMemcpyHostToDevice);
    if (error != hipSuccess) {
        std::cerr << "数据传输失败 (A): " << hipGetErrorString(error) << std::endl;
        hipFree(d_A);
        hipFree(d_B);
        hipFree(d_C);
        return -1;
    }
    
    error = hipMemcpy(d_B, h_B.data(), size, hipMemcpyHostToDevice);
    if (error != hipSuccess) {
        std::cerr << "数据传输失败 (B): " << hipGetErrorString(error) << std::endl;
        hipFree(d_A);
        hipFree(d_B);
        hipFree(d_C);
        return -1;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto transfer_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "数据传输时间: " << transfer_time.count() << " us" << std::endl;
    std::cout << "传输带宽: " << (8.0 * 1024 * 1024) / (transfer_time.count() * 1e-6) / 1e9 << " GB/s" << std::endl;
    
    std::cout << "\n=== GPU矩阵运算 ===" << std::endl;
    
    // 设置线程块和网格大小
    const int blockSize = 16;
    dim3 block(blockSize, blockSize);
    dim3 grid((N + blockSize - 1) / blockSize, (N + blockSize - 1) / blockSize);
    
    std::cout << "线程块大小: " << blockSize << "x" << blockSize << std::endl;
    std::cout << "网格大小: " << grid.x << "x" << grid.y << std::endl;
    
    // 执行GPU内核
    start = std::chrono::high_resolution_clock::now();
    
    hipLaunchKernelGGL(matrixMultiply, grid, block, 0, 0, d_A, d_B, d_C, N);
    
    error = hipDeviceSynchronize();
    if (error != hipSuccess) {
        std::cerr << "GPU内核执行失败: " << hipGetErrorString(error) << std::endl;
        hipFree(d_A);
        hipFree(d_B);
        hipFree(d_C);
        return -1;
    }
    
    end = std::chrono::high_resolution_clock::now();
    auto gpu_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "GPU计算时间: " << gpu_time.count() << " ms" << std::endl;
    
    std::cout << "\n=== 数据回传到主机 ===" << std::endl;
    
    // 将结果从GPU传输回主机
    start = std::chrono::high_resolution_clock::now();
    
    error = hipMemcpy(h_C.data(), d_C, size, hipMemcpyDeviceToHost);
    if (error != hipSuccess) {
        std::cerr << "数据回传失败: " << hipGetErrorString(error) << std::endl;
        hipFree(d_A);
        hipFree(d_B);
        hipFree(d_C);
        return -1;
    }
    
    end = std::chrono::high_resolution_clock::now();
    auto transfer_back_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "数据回传时间: " << transfer_back_time.count() << " us" << std::endl;
    std::cout << "回传带宽: " << (4.0 * 1024 * 1024) / (transfer_back_time.count() * 1e-6) / 1e9 << " GB/s" << std::endl;
    
    std::cout << "\n=== 结果验证 ===" << std::endl;
    
    // CPU计算用于验证
    start = std::chrono::high_resolution_clock::now();
    cpuMatrixMultiply(h_A, h_B, h_C_cpu, N);
    end = std::chrono::high_resolution_clock::now();
    auto cpu_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "CPU计算时间: " << cpu_time.count() << " ms" << std::endl;
    
    // 验证结果
    bool isCorrect = verifyResult(h_C, h_C_cpu, N);
    if (isCorrect) {
        std::cout << "✓ 结果验证通过！" << std::endl;
    } else {
        std::cout << "✗ 结果验证失败！" << std::endl;
    }
    
    // 性能统计
    std::cout << "\n=== 性能统计 ===" << std::endl;
    std::cout << "总传输时间: " << (transfer_time.count() + transfer_back_time.count()) << " ms" << std::endl;
    std::cout << "GPU加速比: " << (float)cpu_time.count() / gpu_time.count() << "x" << std::endl;
    std::cout << "GPU吞吐量: " << (2.0 * N * N * N) / (gpu_time.count() * 1e-6) / 1e9 << " GFLOPS" << std::endl;
    
    // 清理GPU内存
    hipFree(d_A);
    hipFree(d_B);
    hipFree(d_C);
    
    std::cout << "\n=== Demo完成 ===" << std::endl;
    return 0;
}
