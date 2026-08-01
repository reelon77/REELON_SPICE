# TinySpice 构建脚本
#
#   .\scripts\build.ps1              配置(首次) + 构建
#   .\scripts\build.ps1 -Test        构建后跑 ctest
#   .\scripts\build.ps1 -Fresh       删掉 build/ 重新配置
#
# 为什么需要这个脚本：MSVC 的 cl.exe / ninja 不在普通 PowerShell 的 PATH 里，
# 必须先进 VS 开发者环境。手敲 Enter-VsDevShell 有个坑 —— 它内部会调
# vswhere.exe，而 vswhere 所在的 Installer 目录默认不在 PATH，于是 cmd.exe
# 会用系统代码页(936/GBK)吐一句"不是内部或外部命令"的报错；这段 GBK 字节
# 被按 UTF-8 读就是一屏乱码。下面第一行把 Installer 目录补进 PATH，从根上
# 消掉那个报错（而不是去调编码）。
#
# ⚠️ 本文件必须存成 **UTF-8 with BOM**：Windows PowerShell 5.1 对没有 BOM 的
# .ps1 一律按系统 ANSI(936) 解码，中文注释会被拆错字节、吃掉引号，导致语法
# 报错。同一个根因也坑过 MSVC（见根 CMakeLists 的 /utf-8）。编辑后若报
# "Unexpected token"，先查 BOM 还在不在。

param(
    [switch]$Test,
    [switch]$Fresh
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot 'build'

# 1) 让 vswhere 可见 —— 消除 Enter-VsDevShell 的 GBK 乱码报错
$installerDir = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer"
if (Test-Path $installerDir) {
    $env:PATH = "$installerDir;$env:PATH"
}

# 2) 进 VS 开发者环境（拿到 cl.exe / ninja / Windows SDK）
$vsPath = & "$installerDir\vswhere.exe" -latest -products * -property installationPath
if (-not $vsPath) { throw "找不到 Visual Studio 安装" }
Import-Module "$vsPath\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath $vsPath -SkipAutomaticLocation `
                 -DevCmdArguments '-arch=x64 -host_arch=x64' | Out-Null

Set-Location $repoRoot

if ($Fresh -and (Test-Path $buildDir)) {
    Remove-Item $buildDir -Recurse -Force
}

# 3) 配置（build/ 不存在时才跑；CMake 自己会在需要时重新生成）
if (-not (Test-Path (Join-Path $buildDir 'CMakeCache.txt'))) {
    $ninja = "$vsPath\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"
    cmake -S . -B build -G Ninja -DCMAKE_MAKE_PROGRAM="$ninja"
    if ($LASTEXITCODE -ne 0) { throw "CMake 配置失败" }
}

cmake --build build
if ($LASTEXITCODE -ne 0) { throw "构建失败" }

# 4) compile_commands.json 拷到仓库根，clangd 在这儿找最稳（已 gitignore）
Copy-Item (Join-Path $buildDir 'compile_commands.json') $repoRoot -Force

if ($Test) {
    ctest --test-dir build --output-on-failure
}
