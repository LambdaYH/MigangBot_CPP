FROM debian:bookworm
RUN  sed -i s@/archive.ubuntu.com/@/mirrors.aliyun.com/@g /etc/apt/sources.list \
    && sed -i s@/deb.debian.org/@/mirrors.aliyun.com/@g /etc/apt/sources.list \
    && sed -i s@/security.debian.org/@/mirrors.aliyun.com/@g /etc/apt/sources.list
RUN     apt update \
    &&  apt install -y \
                    build-essential \
                    git \
                    cmake \
                    libcurl4-openssl-dev \
                    libssl-dev \
                    uuid-dev \
                    default-libmysqlclient-dev \
                    libopencv-dev \
                    libboost-dev
WORKDIR /build_temp
COPY . /build_temp
RUN     git clone https://github.com/TencentCloud/tencentcloud-sdk-cpp.git \
    &&  cd tencentcloud-sdk-cpp \
    &&  mkdir build && cd build \
    &&  cmake -DBUILD_SHARED_LIBS=off -DBUILD_MODULES="nlp" .. \
    &&  make && make install \
    &&  cd /build_temp \
    &&  cmake -DCMAKE_BUILD_TYPE=Release . \
    &&  make \
    &&  mkdir /MigangBot \
    &&  cp /build_temp/bin/MigangBot /MigangBot \
    &&  cp /build_temp/assets/ -r /MigangBot \
    &&  rm -rf /build_temp/
WORKDIR /MigangBot
ENTRYPOINT ./MigangBot
EXPOSE 8080
