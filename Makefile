# AMD GPU 测试应用 Makefile
# 支持矩阵运算Demo和BAR0 VRAM测试

# 编译器设置
CXX = hipcc
HIPCC = hipcc
GCC = g++

# 目标程序
MATRIX_TARGET = matrix_multiply_demo
MATRIX_SOURCE = matrix_multiply_demo.cpp

BAR0_TARGET = bar0_vram_test
BAR0_SOURCE = bar0_vram_test.cpp

DMA_TARGET = dma_transfer_test
DMA_SOURCE = dma_transfer_test.cpp

SIMPLE_DMA_TARGET = simple_dma_test
SIMPLE_DMA_SOURCE = simple_dma_test.cpp

HOST_TO_GPU_TARGET = host_to_gpu_dma_test
HOST_TO_GPU_SOURCE = host_to_gpu_dma_test.cpp

GPU_TO_HOST_TARGET = gpu_to_host_dma_test
GPU_TO_HOST_SOURCE = gpu_to_host_dma_test.cpp

# 编译选项
CXXFLAGS = -O3 -Wall -std=c++14
HIPFLAGS = --offload-arch=gfx906,gfx908,gfx90a,gfx1030,gfx1100
BAR0FLAGS = -O3 -Wall -std=c++14 -I/usr/include/hip

# 默认目标
all: $(MATRIX_TARGET) $(BAR0_TARGET) $(DMA_TARGET) $(SIMPLE_DMA_TARGET) $(HOST_TO_GPU_TARGET) $(GPU_TO_HOST_TARGET)

# 编译矩阵运算Demo
$(MATRIX_TARGET): $(MATRIX_SOURCE)
	@echo "=== 编译AMD GPU矩阵运算Demo ==="
	@echo "编译器: $(HIPCC)"
	@echo "源文件: $(MATRIX_SOURCE)"
	@echo "目标文件: $(MATRIX_TARGET)"
	@echo ""
	$(HIPCC) $(CXXFLAGS) $(HIPFLAGS) -o $(MATRIX_TARGET) $(MATRIX_SOURCE)
	@echo ""
	@echo "矩阵运算Demo编译完成！"
	@echo "运行: ./$(MATRIX_TARGET)"

# 编译BAR0 VRAM测试
$(BAR0_TARGET): $(BAR0_SOURCE)
	@echo "=== 编译BAR0 VRAM测试应用 ==="
	@echo "编译器: $(GCC)"
	@echo "源文件: $(BAR0_SOURCE)"
	@echo "目标文件: $(BAR0_TARGET)"
	@echo ""
	$(GCC) $(BAR0FLAGS) -o $(BAR0_TARGET) $(BAR0_SOURCE)
	@echo ""
	@echo "BAR0 VRAM测试编译完成！"
	@echo "运行: sudo ./$(BAR0_TARGET)"

# 编译DMA传输测试
$(DMA_TARGET): $(DMA_SOURCE)
	@echo "=== 编译DMA传输测试应用 ==="
	@echo "编译器: $(HIPCC)"
	@echo "源文件: $(DMA_SOURCE)"
	@echo "目标文件: $(DMA_TARGET)"
	@echo ""
	$(HIPCC) $(CXXFLAGS) $(HIPFLAGS) -o $(DMA_TARGET) $(DMA_SOURCE)
	@echo ""
	@echo "DMA传输测试编译完成！"
	@echo "运行: ./$(DMA_TARGET)"

# 编译简单DMA测试
$(SIMPLE_DMA_TARGET): $(SIMPLE_DMA_SOURCE)
	@echo "=== 编译简单DMA拷贝测试应用 ==="
	@echo "编译器: $(HIPCC)"
	@echo "源文件: $(SIMPLE_DMA_SOURCE)"
	@echo "目标文件: $(SIMPLE_DMA_TARGET)"
	@echo ""
	$(HIPCC) $(CXXFLAGS) $(HIPFLAGS) -o $(SIMPLE_DMA_TARGET) $(SIMPLE_DMA_SOURCE)
	@echo ""
	@echo "简单DMA拷贝测试编译完成！"
	@echo "运行: ./$(SIMPLE_DMA_TARGET)"

# 编译主机到GPU DMA测试
$(HOST_TO_GPU_TARGET): $(HOST_TO_GPU_SOURCE)
	@echo "=== 编译主机到GPU DMA拷贝测试应用 ==="
	@echo "编译器: $(HIPCC)"
	@echo "源文件: $(HOST_TO_GPU_SOURCE)"
	@echo "目标文件: $(HOST_TO_GPU_TARGET)"
	@echo ""
	$(HIPCC) $(CXXFLAGS) $(HIPFLAGS) -o $(HOST_TO_GPU_TARGET) $(HOST_TO_GPU_SOURCE)
	@echo ""
	@echo "主机到GPU DMA拷贝测试编译完成！"
	@echo "运行: ./$(HOST_TO_GPU_TARGET)"

# 编译GPU到主机DMA测试
$(GPU_TO_HOST_TARGET): $(GPU_TO_HOST_SOURCE)
	@echo "=== 编译GPU到主机DMA拷贝测试应用 ==="
	@echo "编译器: $(HIPCC)"
	@echo "源文件: $(GPU_TO_HOST_SOURCE)"
	@echo "目标文件: $(GPU_TO_HOST_TARGET)"
	@echo ""
	$(HIPCC) $(CXXFLAGS) $(HIPFLAGS) -o $(GPU_TO_HOST_TARGET) $(GPU_TO_HOST_SOURCE)
	@echo ""
	@echo "GPU到主机DMA拷贝测试编译完成！"
	@echo "运行: ./$(GPU_TO_HOST_TARGET)"

# 运行程序
run-matrix: $(MATRIX_TARGET)
	@echo "=== 运行矩阵运算Demo ==="
	@echo ""
	./$(MATRIX_TARGET)

run-bar0: $(BAR0_TARGET)
	@echo "=== 运行BAR0 VRAM测试 ==="
	@echo "注意: 需要root权限访问PCI设备"
	@echo ""
	sudo ./$(BAR0_TARGET)

run-dma: $(DMA_TARGET)
	@echo "=== 运行DMA传输测试 ==="
	@echo ""
	./$(DMA_TARGET)

run-simple-dma: $(SIMPLE_DMA_TARGET)
	@echo "=== 运行简单DMA拷贝测试 ==="
	@echo ""
	./$(SIMPLE_DMA_TARGET)

run-host-to-gpu: $(HOST_TO_GPU_TARGET)
	@echo "=== 运行主机到GPU DMA拷贝测试 ==="
	@echo ""
	./$(HOST_TO_GPU_TARGET)

run-gpu-to-host: $(GPU_TO_HOST_TARGET)
	@echo "=== 运行GPU到主机DMA拷贝测试 ==="
	@echo ""
	./$(GPU_TO_HOST_TARGET)

# 清理
clean:
	@echo "清理构建文件..."
	rm -f $(MATRIX_TARGET) $(BAR0_TARGET) $(DMA_TARGET) $(SIMPLE_DMA_TARGET) $(HOST_TO_GPU_TARGET) $(GPU_TO_HOST_TARGET)
	@echo "清理完成"

# 检查环境
check:
	@echo "=== 检查ROCm环境 ==="
	@echo "检查rocminfo..."
	@rocminfo | head -20
	@echo ""
	@echo "检查hipcc版本..."
	@hipcc --version
	@echo ""
	@echo "检查GPU设备..."
	@rocminfo | grep -A 3 "Device Type: GPU" || echo "未检测到GPU设备"

# 性能测试
perf-matrix: $(MATRIX_TARGET)
	@echo "=== 矩阵运算性能测试 ==="
	@echo "运行3次测试..."
	@for i in 1 2 3; do \
		echo "第$$i次运行:"; \
		./$(MATRIX_TARGET) | grep -E "(GPU计算时间|GPU加速比|GPU吞吐量)"; \
		echo ""; \
	done

perf-bar0: $(BAR0_TARGET)
	@echo "=== BAR0 VRAM性能测试 ==="
	@echo "运行3次测试..."
	@for i in 1 2 3; do \
		echo "第$$i次运行:"; \
		sudo ./$(BAR0_TARGET) | grep -E "(写入带宽|读取带宽)"; \
		echo ""; \
	done

perf-dma: $(DMA_TARGET)
	@echo "=== DMA传输性能测试 ==="
	@echo "运行3次测试..."
	@for i in 1 2 3; do \
		echo "第$$i次运行:"; \
		./$(DMA_TARGET) | grep -E "(带宽|比例)"; \
		echo ""; \
	done

perf-simple-dma: $(SIMPLE_DMA_TARGET)
	@echo "=== 简单DMA拷贝性能测试 ==="
	@echo "运行3次测试..."
	@for i in 1 2 3; do \
		echo "第$$i次运行:"; \
		./$(SIMPLE_DMA_TARGET) | grep -E "(带宽|比例)"; \
		echo ""; \
	done

perf-host-to-gpu: $(HOST_TO_GPU_TARGET)
	@echo "=== 主机到GPU DMA拷贝测试 ==="
	@echo "运行3次测试..."
	@for i in 1 2 3; do \
		echo "第$$i次运行:"; \
		./$(HOST_TO_GPU_TARGET) | grep -E "(完成|失败)"; \
		echo ""; \
	done

perf-gpu-to-host: $(GPU_TO_HOST_TARGET)
	@echo "=== GPU到主机DMA拷贝测试 ==="
	@echo "运行3次测试..."
	@for i in 1 2 3; do \
		echo "第$$i次运行:"; \
		./$(GPU_TO_HOST_TARGET) | grep -E "(完成|失败)"; \
		echo ""; \
	done

# 帮助信息
help:
	@echo "AMD GPU 测试应用 Makefile"
	@echo ""
	@echo "可用目标:"
	@echo "  all               编译所有程序 (默认)"
	@echo "  run-matrix        编译并运行矩阵运算Demo"
	@echo "  run-bar0          编译并运行BAR0 VRAM测试 (需要root权限)"
	@echo "  run-dma           编译并运行DMA传输测试"
	@echo "  run-simple-dma    编译并运行简单DMA拷贝测试"
	@echo "  run-host-to-gpu   编译并运行主机到GPU DMA拷贝测试"
	@echo "  run-gpu-to-host   编译并运行GPU到主机DMA拷贝测试"
	@echo "  clean             清理构建文件"
	@echo "  check             检查ROCm环境"
	@echo "  perf-matrix       运行矩阵运算性能测试"
	@echo "  perf-bar0         运行BAR0 VRAM性能测试"
	@echo "  perf-dma          运行DMA传输性能测试"
	@echo "  perf-simple-dma   运行简单DMA拷贝性能测试"
	@echo "  perf-host-to-gpu  运行主机到GPU DMA拷贝测试"
	@echo "  perf-gpu-to-host  运行GPU到主机DMA拷贝测试"
	@echo "  help              显示此帮助信息"
	@echo ""
	@echo "示例:"
	@echo "  make                   # 编译所有程序"
	@echo "  make run-matrix        # 编译并运行矩阵运算Demo"
	@echo "  make run-bar0          # 编译并运行BAR0 VRAM测试"
	@echo "  make run-dma           # 编译并运行DMA传输测试"
	@echo "  make run-simple-dma    # 编译并运行简单DMA拷贝测试"
	@echo "  make run-host-to-gpu   # 编译并运行主机到GPU DMA拷贝测试"
	@echo "  make run-gpu-to-host   # 编译并运行GPU到主机DMA拷贝测试"
	@echo "  make clean             # 清理文件"
	@echo "  make check             # 检查环境"
	@echo "  make perf-matrix       # 矩阵运算性能测试"
	@echo "  make perf-bar0         # BAR0 VRAM性能测试"
	@echo "  make perf-dma          # DMA传输性能测试"
	@echo "  make perf-simple-dma   # 简单DMA拷贝性能测试"
	@echo "  make perf-host-to-gpu  # 主机到GPU DMA拷贝测试"
	@echo "  make perf-gpu-to-host  # GPU到主机DMA拷贝测试"

# 声明伪目标
.PHONY: all run-matrix run-bar0 run-dma run-simple-dma run-host-to-gpu run-gpu-to-host clean check perf-matrix perf-bar0 perf-dma perf-simple-dma perf-host-to-gpu perf-gpu-to-host help
