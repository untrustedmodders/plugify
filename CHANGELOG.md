# Changelog

## [1.2.6](https://github.com/untrustedmodders/plugify/compare/v1.2.5...v1.2.6) (2025-05-29)


### Bug Fixes

* action build ([05e73b1](https://github.com/untrustedmodders/plugify/commit/05e73b10e679a58a6593a8925c1edca036862618))
* add missing packages to build ([dee72fd](https://github.com/untrustedmodders/plugify/commit/dee72fd126dccc77f1a76f1ffcf20e9830e962a3))
* add missing windows headers ([2d34e92](https://github.com/untrustedmodders/plugify/commit/2d34e925d648ce35c88fe2a8628aec7ba6af14ab))
* add new package naming strategy ([1acf428](https://github.com/untrustedmodders/plugify/commit/1acf428745fb060389d299a2251055158e04d92a))
* build on windows (2) ([b0696e0](https://github.com/untrustedmodders/plugify/commit/b0696e0d4dc7e415e57919612ea08aee65bf9490))
* disable tests on mingw ([59bec62](https://github.com/untrustedmodders/plugify/commit/59bec6261efbac4770a5ed75e6604f144c49a0ed))
* Mac crashpad build issue ([fe2283a](https://github.com/untrustedmodders/plugify/commit/fe2283a09e9b6fa40db3b7090ad7c669a8c976b8))
* make CRASHPAD_USE_BORINGSSL global avail ([40ae148](https://github.com/untrustedmodders/plugify/commit/40ae14826040fbeaeb7428a0b90d0cf31537ec21))
* mingw build ([0eedf7d](https://github.com/untrustedmodders/plugify/commit/0eedf7d9c72bb773a2400a813ed5b12f417b8fa9))
* more crashpad improvements ([a5df7c6](https://github.com/untrustedmodders/plugify/commit/a5df7c6a9adaeae1b3e19d59b9a6b0d3e71b3b05))
* move from mingw to ucrt ([dfa2be4](https://github.com/untrustedmodders/plugify/commit/dfa2be48c8f7171f6751d2696bf2c6e4a9489b15))
* msys2 build ([ab94f24](https://github.com/untrustedmodders/plugify/commit/ab94f24f649d9715de3efd5cd7b032062f93033d))
* small change in cmake ([f75d70a](https://github.com/untrustedmodders/plugify/commit/f75d70a860af199aad31a05a1eebcad455de59ac))

## [1.2.5](https://github.com/untrustedmodders/plugify/compare/v1.2.4...v1.2.5) (2025-05-25)


### Bug Fixes

* build on windows ([5f6ac40](https://github.com/untrustedmodders/plugify/commit/5f6ac4070349a77af0f1894318275a93cae8b0f3))

## [1.2.4](https://github.com/untrustedmodders/plugify/compare/v1.2.3...v1.2.4) (2025-05-25)


### Bug Fixes

* add acceleration for sha256 on arm ([28e849e](https://github.com/untrustedmodders/plugify/commit/28e849ed71d9268d0c7afc7604b9a4a88d5297d5))
* add asmjit patch for arm64 ([cd7ce41](https://github.com/untrustedmodders/plugify/commit/cd7ce4109193f1e458f487e5f2611c9bab58bb41))
* add cmake version parse ([0596375](https://github.com/untrustedmodders/plugify/commit/059637536747ff1977e8af455fc976e430f9923d))
* add compiler macros ([944f1a7](https://github.com/untrustedmodders/plugify/commit/944f1a7a0a877cb759bc0a087099f84ef5521ae4))
* add cpplogs to test app ([aeeb346](https://github.com/untrustedmodders/plugify/commit/aeeb34611eccb36d7fb027ebf05ce07fcf3963c8))
* add cpptrace ([cf7d6e4](https://github.com/untrustedmodders/plugify/commit/cf7d6e47e8fcdc3b8a77044e191a7c6a690f84b6))
* add crashlogs ([6871b78](https://github.com/untrustedmodders/plugify/commit/6871b78dcf83955f8f0c887f4edbe58210df6c80))
* add crashpad support ([8771371](https://github.com/untrustedmodders/plugify/commit/8771371490336d68987bd7d4f61c15b595444030))
* add debugging header ([801e84c](https://github.com/untrustedmodders/plugify/commit/801e84ca98b97a5112573285173eb6acd873b32d))
* add defer instead of function scope ([ed7cd45](https://github.com/untrustedmodders/plugify/commit/ed7cd4516ca75b0ed60e74208ef2ab1baa734cd5))
* add error handler to x86 ([0154594](https://github.com/untrustedmodders/plugify/commit/01545945d0f4678dc82a29c9a1ba8761f7d1bae6))
* add handle ctor to assembly object ([dee4661](https://github.com/untrustedmodders/plugify/commit/dee46616040f83fab4687c7b93a82c526a75d2ed))
* add miniz patch ([e1ba267](https://github.com/untrustedmodders/plugify/commit/e1ba267c9c551a16fa1d45697968a8de134b55f2))
* add missing macro ([d33be7e](https://github.com/untrustedmodders/plugify/commit/d33be7e8d939380fac63a691cad8c8b88b9435ff))
* add other platforms to debugging.hpp ([7987369](https://github.com/untrustedmodders/plugify/commit/7987369461e4496d2696428b82f00f4fd659f48b))
* add PARENT_SCOPE to platform function ([99dee2b](https://github.com/untrustedmodders/plugify/commit/99dee2bae16c793b397fe70fa25880d54de4def5))
* add url links to fetch contents ([ba337e4](https://github.com/untrustedmodders/plugify/commit/ba337e4ddc9d1259e8f962c103313a4de96c62a3))
* AppleClang add export methods ([a40390b](https://github.com/untrustedmodders/plugify/commit/a40390ba433b6850bd9e61403e8057926634fd1a))
* AppleClang build fixes ([3ef3a56](https://github.com/untrustedmodders/plugify/commit/3ef3a568c84108906923da8d294edc6c735c547f))
* AppleClang remove export methods ([99ba337](https://github.com/untrustedmodders/plugify/commit/99ba3374e8e594d167103ae7ae49770f25f2ca9b))
* arch detection ([05a36d4](https://github.com/untrustedmodders/plugify/commit/05a36d49c7350dc21f32c87870c34c0f0d333174))
* arm typo in cmake ([506f805](https://github.com/untrustedmodders/plugify/commit/506f80509afe4d9552b64a88f53070e3e50c3970))
* build on linux ([fa76102](https://github.com/untrustedmodders/plugify/commit/fa761029ec1f21cedd9e271476b89c0d905b3a4f))
* cpptrace problem on Linux and Mac ([c74139a](https://github.com/untrustedmodders/plugify/commit/c74139ab492830368eca7aaf7dcca9dba697d593))
* dlinfo error handling typo ([705c3f9](https://github.com/untrustedmodders/plugify/commit/705c3f98b5813bf880c4cca00bc2913dc29565ce))
* final arm64 fixes of jit ([ad3d186](https://github.com/untrustedmodders/plugify/commit/ad3d186ca87881bcedfbb1bd2c8fb8739d119f54))
* fix clang build and more refactor ([c04fe67](https://github.com/untrustedmodders/plugify/commit/c04fe670866445a64a3c660c41d8f3b44b6bc3d0))
* function typo ([e79b176](https://github.com/untrustedmodders/plugify/commit/e79b176636b99d0dc51970280e69abe5e75a6c33))
* git script to separate file ([1163311](https://github.com/untrustedmodders/plugify/commit/1163311e6dc84185d84dd40b5c2ebf2a9e72f410))
* linux logger ([437b1a4](https://github.com/untrustedmodders/plugify/commit/437b1a47303885ffa0b30b40402fbb7293b96987))
* make crashpad build with test always ([7e551d2](https://github.com/untrustedmodders/plugify/commit/7e551d23e134a0adef57d4f00c1fa737bf95eabe))
* massive CMake rework ([5489698](https://github.com/untrustedmodders/plugify/commit/5489698e23f1fdd6f9412196fd23e1d3faa55d5c))
* MINGW build ([9447813](https://github.com/untrustedmodders/plugify/commit/94478139acef983e23c1345feb9eae818acdb28a))
* remove crashlogs ([0aae3c4](https://github.com/untrustedmodders/plugify/commit/0aae3c423f3e20b365009549ef4d176982fe9061))
* remove deprecated asmjit functions ([30068e4](https://github.com/untrustedmodders/plugify/commit/30068e47050ef520b928da52b42f2df75e7ee68b))
* remove miniz patch ([4d43983](https://github.com/untrustedmodders/plugify/commit/4d4398325a931794ffdba599b9bb9daae7ca623a))
* remove sstream in date format ([1fb3fbf](https://github.com/untrustedmodders/plugify/commit/1fb3fbf7467e42ce90b0b3ae23a39155b1397072))
* remove stacktrace include ([b1301d3](https://github.com/untrustedmodders/plugify/commit/b1301d3e51a2672ff7c49cfab0e31a214d9ce16b))
* remove swap ordering on big endian for sha ([42719a6](https://github.com/untrustedmodders/plugify/commit/42719a67f7465c9f990c1b235ff152aa6eee63b3))
* replace assert by default one ([c578381](https://github.com/untrustedmodders/plugify/commit/c5783811f00b6c435018f4c780a45611d38c91a6))
* revert some changes for x86 ([dadc246](https://github.com/untrustedmodders/plugify/commit/dadc2463c8630dfb88e7628ce998983ae1311019))
* shadow declaration ([b21c0f4](https://github.com/untrustedmodders/plugify/commit/b21c0f4100d1cb5eeb4605eed8e53247f6959aaf))
* small linux fixes ([3fbc407](https://github.com/untrustedmodders/plugify/commit/3fbc4078e89809d8f150aec0f672049ec5c5a58b))
* small x86 improvements of generated code at runtime ([625a548](https://github.com/untrustedmodders/plugify/commit/625a548d26fb652a445001ad5d2ea7dce7c6a35f))
* some cmake fixes for arm ([26d3609](https://github.com/untrustedmodders/plugify/commit/26d36097abfa907155ec2280d1045de6c061ae1d))
* typo in debugging ([35d8a70](https://github.com/untrustedmodders/plugify/commit/35d8a7039492216f9aa320f515555d576f602170))
* update int format in plug ([8fb0905](https://github.com/untrustedmodders/plugify/commit/8fb0905a7972005d02250574f056724a791de1e1))
* update plug ([6407e57](https://github.com/untrustedmodders/plugify/commit/6407e57040eb4119fcae997ef463663e7ade7b16))
* win build ([577f4c4](https://github.com/untrustedmodders/plugify/commit/577f4c449ee8c5765eed0815e2a38975b48f0607))
* win build (2) ([a0a51f6](https://github.com/untrustedmodders/plugify/commit/a0a51f6ed0cc71c2bc6b78465d2195a8d317573b))
* wsl patching ([767bb3a](https://github.com/untrustedmodders/plugify/commit/767bb3a2007d13ddb34b380d9440d5dbed3079e1))

## [1.2.3](https://github.com/untrustedmodders/plugify/compare/v1.2.2...v1.2.3) (2025-03-27)


### Bug Fixes

* another mingw fix ([b1b5669](https://github.com/untrustedmodders/plugify/commit/b1b5669e1dedf90e29ff72f5555a81a6a3a2c4a3))

## [1.2.2](https://github.com/untrustedmodders/plugify/compare/v1.2.1...v1.2.2) (2025-03-27)


### Bug Fixes

* change plug build to powershell ([8a4addc](https://github.com/untrustedmodders/plugify/commit/8a4addc0f0afaa86bda611ec7d14cca9bec93bf1))
* MINGW missing macros ([2900443](https://github.com/untrustedmodders/plugify/commit/29004438d7ec11afd858de32fe33508806464fd8))

## [1.2.1](https://github.com/untrustedmodders/plugify/compare/v1.2.0...v1.2.1) (2025-03-27)


### Bug Fixes

* a lot of code issues ([5c76c10](https://github.com/untrustedmodders/plugify/commit/5c76c1049d791cbe97f0f5ed0dd86744c1a53ff6))
* curl build error ([e36b0b0](https://github.com/untrustedmodders/plugify/commit/e36b0b003b8fef11a6380f2935160280d348962d))

## [1.2.0](https://github.com/untrustedmodders/plugify/compare/v1.1.2...v1.2.0) (2025-03-24)


### Features

* base paths ([7388fc2](https://github.com/untrustedmodders/plugify/commit/7388fc24639ceb9a26df8540c951bbe9675fd736))


### Bug Fixes

* add missing getters to provider ([fe73eba](https://github.com/untrustedmodders/plugify/commit/fe73ebae8c98462c421a77bf97e87094ff6c3353))
* base paths ([3adacb4](https://github.com/untrustedmodders/plugify/commit/3adacb40542bcbf918f6b4f2e5b6f4c1b6b0f689))
* **glaze:** update to `v2.9.5` ([1134f33](https://github.com/untrustedmodders/plugify/commit/1134f334d7e64e8da9d2de4ef857471d090d8c52))
* improve downloader with more error handling ([e6c077f](https://github.com/untrustedmodders/plugify/commit/e6c077fce37600b7a2da50e5a289e9e080e3bee0))
* missing plugin handle methods ([fbd07aa](https://github.com/untrustedmodders/plugify/commit/fbd07aa0b924b2b7d56c231c4c22bff50da9cc65))
* MSVC issues on cpp versioning macro ([3b11316](https://github.com/untrustedmodders/plugify/commit/3b11316c3fc270e57ba09794c2720140842c403c))
* optional access ([ddba5e0](https://github.com/untrustedmodders/plugify/commit/ddba5e0a031e3af7a1aaa718c9d3b535c9e37c24))
* ranges typo in macro ([cada73d](https://github.com/untrustedmodders/plugify/commit/cada73d6bcdb9750a3ab43e4b3f0ba2331e68325))
* remove noexcept for some vertor methods ([6351535](https://github.com/untrustedmodders/plugify/commit/6351535d7106ef597c79776322190a1b98a80c4f))
* revert includes to cpp20 ([78059c9](https://github.com/untrustedmodders/plugify/commit/78059c98acb81f8f13a487ab13be966fd037259a))
* update schema with descriptions ([b53adba](https://github.com/untrustedmodders/plugify/commit/b53adbafedbec66d118c5b8db6e338d9f72b58f2))

## [1.1.2](https://github.com/untrustedmodders/plugify/compare/v1.1.1...v1.1.2) (2025-02-16)


### Bug Fixes

* refactor cmake stripping ([8bd22b4](https://github.com/untrustedmodders/plugify/commit/8bd22b42379c98f4d34d465eb661da6b9e6262a5))
* update to abi1 on linux ([623d857](https://github.com/untrustedmodders/plugify/commit/623d85770f3e8d0efe198eb3a005f228175e2411))

## [1.1.1](https://github.com/untrustedmodders/plugify/compare/v1.1.0...v1.1.1) (2025-02-16)


### Bug Fixes

* add spacing ([a028c47](https://github.com/untrustedmodders/plugify/commit/a028c47bfb5f73f3983caefa39f93234c7379426))
* add static test for any size ([24aed00](https://github.com/untrustedmodders/plugify/commit/24aed00924008d5b7ff2d9c5c960ecb1c4d59313))
* plugin update state typo ([4ac75c0](https://github.com/untrustedmodders/plugify/commit/4ac75c07271af24c8493a546f4808f863e6af515))
* **variant:** remove `detail::variant_tag` inherits ([a795b47](https://github.com/untrustedmodders/plugify/commit/a795b4797ac30a95333fcfa71b0fb75c19394fcf))
* **vector:** include missing `memory_resource` header ([44a07d8](https://github.com/untrustedmodders/plugify/commit/44a07d86fd8f2a893b9875b33d931ec839e39801))

## [1.1.0](https://github.com/untrustedmodders/plugify/compare/v1.0.1...v1.1.0) (2025-02-10)


### Features

* **GitHub workflows - Clang:** comment MacOS build ([41236ae](https://github.com/untrustedmodders/plugify/commit/41236ae592a8d63139b6d0252e4d62e07fcd2555))
* **GitHub workflows - MSVC:** add msbuild architecture value ([528666d](https://github.com/untrustedmodders/plugify/commit/528666de4fef9b2d6a0ed392de6521eee9c3ac24))


### Bug Fixes

* **CMake:** cleanup typical file characters into `PLUGIFY_VERSION` ([66d9413](https://github.com/untrustedmodders/plugify/commit/66d9413695403b16b5805abba9f4c62d6e5425ff))
* **fmt:** using `fmt::detail` namespace ([4c7aee4](https://github.com/untrustedmodders/plugify/commit/4c7aee4e0e3414a07bb2cf6f5f3bddda55824cb5))
* **GitHub workflows - MSVC:** update setup msbuild action version ([6829236](https://github.com/untrustedmodders/plugify/commit/6829236e64307a64868218a0e35a06e5736305f4))
* **GitHub workflows - MSVC:** update VS version to `[17.12,17.13)` ([df04ef2](https://github.com/untrustedmodders/plugify/commit/df04ef282986845f37e3c15f1fb73e88cbd7df92))
* **GitHub workflows - MSVC:** update VS version to `17.13` ([c12cb49](https://github.com/untrustedmodders/plugify/commit/c12cb4907b40f309333cb3270bd4c75ad4f79ca7))
* **GitHub Workflows - Ubuntu:** remove update alternatives ([ef40aee](https://github.com/untrustedmodders/plugify/commit/ef40aeece513085a9a79e44dccd7d9e039ac4520))

## [1.0.1](https://github.com/untrustedmodders/plugify/compare/v1.0.0...v1.0.1) (2025-02-06)


### Bug Fixes

* add syncstream to logger ([0c3a394](https://github.com/untrustedmodders/plugify/commit/0c3a394915d5dba53ad6989b92ad9ef6e3071305))
* add version macro ([47db4e7](https://github.com/untrustedmodders/plugify/commit/47db4e77c6051dd1d0dba5e8ffcd455211df79a1))
* change workflows ([8653012](https://github.com/untrustedmodders/plugify/commit/8653012ed957af19c7f6adec13a283e31b15169b))
* module id dublication ([dcddaa0](https://github.com/untrustedmodders/plugify/commit/dcddaa0206d708f660f9752104e33021ebe0f1af))
* version formatting ([8d4b814](https://github.com/untrustedmodders/plugify/commit/8d4b814cbf3fa2bd8b9f8d45a7f54b1c20824be5))
* version print ([157a5e4](https://github.com/untrustedmodders/plugify/commit/157a5e4e2178c6394efa8c0fffce891d526832ed))
