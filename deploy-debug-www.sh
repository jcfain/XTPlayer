#!/bin/bash
rm -rf ./build-debug/debug/www
cp -ruf ../XTEngine/src/www/ ./build-debug/debug/
rm -rf ./build-release/release/www
cp -ruf ../XTEngine/src/www/ ./build-release/release/
