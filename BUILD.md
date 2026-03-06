# ビルド手順

## 必要な環境

- C++17対応のコンパイラ (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.15以上
- Git (依存ライブラリのダウンロードに使用)

## ローカルビルド

### 1. ビルドディレクトリの作成

```bash
mkdir build
cd build
```

### 2. CMakeの設定

```bash
cmake ..
```

または、Releaseビルドの場合:

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

### 3. ビルド

```bash
cmake --build .
```

または並列ビルド:

```bash
cmake --build . --parallel $(nproc)
```

### 4. 実行

```bash
./smart-mobility-backend
```

サーバーは `http://localhost:8080` で起動します。

## Dockerビルド

### イメージのビルド

```bash
docker build -t smart-mobility-backend:latest .
```

### コンテナの実行

```bash
docker run -p 8080:8080 smart-mobility-backend:latest
```

## API エンドポイント

- `GET /health` - ヘルスチェック
- `GET /api/vehicle/state` - 現在の車両状態を取得
- `POST /api/vehicle/accelerate` - 加速
- `POST /api/vehicle/decelerate` - 減速
- `POST /api/vehicle/gear/{P|R|N|D}` - ギア変更
- `POST /api/vehicle/seatbelt/{true|false}` - シートベルト装着状態変更
- `POST /api/vehicle/engine-error/{true|false}` - エンジン異常状態変更
- `POST /api/vehicle/reset` - 状態リセット

## 使用例

```bash
# 状態取得
curl http://localhost:8080/api/vehicle/state

# ギアをDに変更
curl -X POST http://localhost:8080/api/vehicle/gear/D

# 加速
curl -X POST http://localhost:8080/api/vehicle/accelerate

# シートベルト装着
curl -X POST http://localhost:8080/api/vehicle/seatbelt/true
```

## 依存ライブラリ

プロジェクトは以下のライブラリを使用します（CMakeが自動的にダウンロード）:

- [cpp-httplib](https://github.com/yhirose/cpp-httplib) - HTTP サーバー
- [nlohmann-json](https://github.com/nlohmann/json) - JSON ライブラリ
