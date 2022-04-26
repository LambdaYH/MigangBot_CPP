FROM gcc:11.3.0
RUN     apt update \
    &&  apt install -y \
                    git \
                    cmake \
                    libcurl4-openssl-dev \
                    libssl-dev \
                    uuid-dev \
                    default-libmysqlclient-dev \
                    libopencv-dev \
                    libboost-dev
WORKDIR /build_temp
RUN     git clone https://github.com/TencentCloud/tencentcloud-sdk-cpp.git \
    &&  cd tencentcloud-sdk-cpp \
    &&  mkdir build && cd build \
    &&  cmake -DBUILD_SHARED_LIBS=off -DBUILD_MODULES="nlp" .. \
    &&  make && make install
WORKDIR /build_temp/migangbot_build
COPY . /build_temp/migangbot_build
RUN     cmake -DCMAKE_BUILD_TYPE=Release . \
    &&  make
WORKDIR /MigangBot
RUN     cp /build_temp/migangbot_build/bin/MigangBot . \
    &&  cp /build_temp/migangbot_build/assets/ -r . \
    &&  rm -rf /build_temp/
ENTRYPOINT ./MigangBot
EXPOSE 8080
