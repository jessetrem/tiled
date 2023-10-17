qbs setup-toolchains --detect
qbs setup-qt --detect
qbs generate -g visualstudio2022 config:debug profile:MSVC2019-x64
