FROM ubuntu:22.04 AS builder

# 必要なパッケージのインストール
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*

# 作業ディレクトリ
WORKDIR /app

# ソースコードのコピー
COPY CMakeLists.txt ./
COPY include/ ./include/
COPY src/ ./src/

# ビルド
RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . --parallel $(nproc)

# 実行用の軽量イメージ
FROM ubuntu:22.04

# 必要な実行時ライブラリのインストール
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# 作業ディレクトリ
WORKDIR /app

# ビルドされたバイナリをコピー
COPY --from=builder /app/build/smart-mobility-backend .

# ポート公開
EXPOSE 8080

# ヘルスチェック
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/health || exit 1

# 実行
CMD ["./smart-mobility-backend"]
