# Smart Mobility Dashboard (C++)

車両制御システムのダッシュボードを模したデモアプリケーションです。
自動化テスト（単体テスト・GUIテスト）のデモンストレーション用途で開発しています。

## 概要

スピードメーター、ギアインジケーター、警告灯、各種制御ボタンを備えた車両ダッシュボードを Web UI で提供します。
バックエンドは C++ で実装し、REST API 経由でフロントエンドと連携します。

## 技術スタック

| 区分 | 技術 |
|------|------|
| 言語 | C++17 |
| ビルドツール | CMake 3.15+ |
| HTTP サーバー | [cpp-httplib](https://github.com/yhirose/cpp-httplib) v0.18.3 |
| JSON | [nlohmann-json](https://github.com/nlohmann/json) v3.11.3 |
| フロントエンド | HTML / CSS / JavaScript (Vanilla) |
| コンテナ | Docker / OpenShift |

## プロジェクト構成

```
smart-mobility-dashboard-c/
├── CMakeLists.txt          # ビルド設定
├── Dockerfile              # コンテナビルド用
├── include/                # ヘッダーファイル
│   ├── VehicleState.h      # 車両状態モデル
│   ├── VehicleService.h    # ビジネスロジック
│   └── VehicleResource.h   # REST API エンドポイント
├── src/                    # ソースファイル
│   ├── main.cpp            # エントリーポイント
│   ├── VehicleState.cpp
│   ├── VehicleService.cpp
│   └── VehicleResource.cpp
└── public/                 # 静的ファイル（フロントエンド）
    ├── index.html
    ├── css/dashboard.css
    └── js/dashboard.js
```

## 必要な環境

- C++17 対応コンパイラ (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.15 以上
- Git（依存ライブラリの自動ダウンロードに使用）

## ビルドと実行

```bash
mkdir build && cd build
cmake ..
cmake --build . --parallel $(nproc)
cd ..
./build/smart-mobility-backend
```

サーバーが `http://localhost:8080` で起動します。

## Docker

```bash
docker build -t smart-mobility-backend:latest .
docker run -p 8080:8080 smart-mobility-backend:latest
```

## API エンドポイント

| メソッド | パス | 説明 |
|----------|------|------|
| GET | `/health` | ヘルスチェック |
| GET | `/api/vehicle/state` | 現在の車両状態を取得 |
| POST | `/api/vehicle/accelerate` | 加速 |
| POST | `/api/vehicle/decelerate` | 減速 |
| POST | `/api/vehicle/gear/{P\|R\|N\|D}` | ギア変更 |
| POST | `/api/vehicle/seatbelt/{true\|false}` | シートベルト装着状態変更 |
| POST | `/api/vehicle/engine-error/{true\|false}` | エンジン異常状態変更 |
| POST | `/api/vehicle/reset` | 状態リセット |

## 使用例

```bash
# 状態取得
curl http://localhost:8080/api/vehicle/state

# ギアを D に変更
curl -X POST http://localhost:8080/api/vehicle/gear/D

# 加速
curl -X POST http://localhost:8080/api/vehicle/accelerate

# シートベルト装着
curl -X POST http://localhost:8080/api/vehicle/seatbelt/true
```

## ライセンス

社内デモ用アプリケーション

## テストコメント

CIパイプライン検証 v1
