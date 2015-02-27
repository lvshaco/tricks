#!/bin/bash

#bash脚本获取自身路径
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo $DIR
