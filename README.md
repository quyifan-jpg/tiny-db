# 安装依赖

apt update && apt upgrade -y && apt install cmake make git g++ gcc -y && cd ~
&& git clone https://github.com/gabime/spdlog.git && cd spdlog && mkdir build && cd build && cmake .. && make -j && sudo make install && cd ~
&& git clone https://github.com/google/googletest && cd googletest && mkdir build && cd build && cmake .. && make -j && sudo make install && cd ~
&& git clone https://github.com/nlohmann/json && cd json && mkdir build && cd build && cmake .. && make -j && sudo make install && cd ~
&& git clone https://github.com/abseil/abseil-cpp.git && cd abseil-cpp && mkdir build && cd build && cmake .. && make -j && make install && cd ~
&& rm -rf spdlog googletest json

cd smallkv
./build.sh         ## 编译
./main_run.sh      ## 主程序
./unittest_run.sh  ## 单元测试
