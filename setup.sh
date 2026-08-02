#!/usr/bin/env bash
set -e

echo "📦 Installing required C++ system dependencies..."
sudo apt-get update && sudo apt-get install -y \
    build-essential \
    cmake \
    libasio-dev \
    libboost-dev \
    libsqlite3-dev \
    libssl-dev \
    libcurl4-openssl-dev \
    nlohmann-json3-dev

echo "🧹 Cleaning previous build artifacts..."
rm -rf build/

echo "🔨 Configuring and building project..."
cmake -B build
cmake --build build

echo "✅ Setup and Build Complete!"