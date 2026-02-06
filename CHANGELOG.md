# Changelog

## [2.1.0](https://github.com/untrustedmodders/plugify/compare/v2.0.0...v2.1.0) (2026-02-06)


### Features

* add class support ([b25a038](https://github.com/untrustedmodders/plugify/commit/b25a03817823a569fb008b011307ffd757b6fa54))


### Bug Fixes

* add case_insensitive_compare ([3fa4b41](https://github.com/untrustedmodders/plugify/commit/3fa4b41eb40d72397ef827d79f90952957b87d0d))
* add class binding serialization ([065f790](https://github.com/untrustedmodders/plugify/commit/065f7904cd317ab9b0d4b292d1c476669c0d4fe7))
* add class bindings structs ([ef245c1](https://github.com/untrustedmodders/plugify/commit/ef245c1e4bcfaf99b3fd5e519e4e4856fd1858cc))
* add class validation to manifest ([419d98e](https://github.com/untrustedmodders/plugify/commit/419d98e3c86892e7ac1c6468d564bde35d26ba29))
* add constant iters to hybrid vector ([1196475](https://github.com/untrustedmodders/plugify/commit/1196475177d5a1ac7359e406c0f18376c5a7ad22))
* add deprecated fields to methods and classes ([7a11430](https://github.com/untrustedmodders/plugify/commit/7a114308dfc43ff3c017c9c200e2509eb1402066))
* add deprecated fields to schema ([27cd6a5](https://github.com/untrustedmodders/plugify/commit/27cd6a55a878fc51ad522ec614ae106e4e6db0c3))
* add directory exclusion for manifest search ([6396037](https://github.com/untrustedmodders/plugify/commit/6396037e80982497c0d2686ae37f4115c9116b28))
* add float cast_to for apple clang ([ae5cb77](https://github.com/untrustedmodders/plugify/commit/ae5cb77bb027f3586511b45fdea0f7c261540438))
* add hybrid_vector helper ([be69959](https://github.com/untrustedmodders/plugify/commit/be69959b10391afa80f91a84c5f1e7134eab0d79))
* add missing move ([b5f6a9b](https://github.com/untrustedmodders/plugify/commit/b5f6a9bb45be27a340429e803e7be7d9edf8a9c0))
* add module support ([8f6c623](https://github.com/untrustedmodders/plugify/commit/8f6c6236b0d5378400f6c3e427b45da03645938b))
* add nullability to alias in bindings ([06b2284](https://github.com/untrustedmodders/plugify/commit/06b2284ef4d21dc79113e561e26c9e94f9e5b4f7))
* add plg types for VS natvis ([f0b7aa9](https://github.com/untrustedmodders/plugify/commit/f0b7aa9213b675d6122a52256e1bed42d8125ecf))
* add support of cmake install ([7988a9b](https://github.com/untrustedmodders/plugify/commit/7988a9b840cdd545c3cac05a97e05fb495429934))
* add support of handless wrappers ([de726de](https://github.com/untrustedmodders/plugify/commit/de726de07a2fcb9d9e1efaad4c6f4b33eb227c8b))
* add unsafe view for raw ptr ([32794d6](https://github.com/untrustedmodders/plugify/commit/32794d6ac151f91e7e1c99024c0604aec581c689))
* add vector parser util func ([cbd8a2f](https://github.com/untrustedmodders/plugify/commit/cbd8a2f8e26fa2448c778353accec742197458c1))
* allow alias member for types ([b08027b](https://github.com/untrustedmodders/plugify/commit/b08027b4bde6dc2c2b370a6f723991fdd034d227))
* allow constructors list be empty ([c9a2b10](https://github.com/untrustedmodders/plugify/commit/c9a2b10b4c20bb9853f9af26f688b718b62fc69c))
* allow description in class data ([e5a7232](https://github.com/untrustedmodders/plugify/commit/e5a72328b8976c1a2a9cef0aa85914185168a492))
* avoid alias copy on return ([27f47c6](https://github.com/untrustedmodders/plugify/commit/27f47c6ffd996a04b3abf7625ed42f961a34b3ed))
* build issue ([6ef9731](https://github.com/untrustedmodders/plugify/commit/6ef97315de70fb2f0c3f30ae7ebbc1859a466180))
* case_insensitive_equal issue ([7d96f53](https://github.com/untrustedmodders/plugify/commit/7d96f53c2358cd8d15b2fdfddcd713a109392553))
* dereferencing type-punned pointer ([d0b82ce](https://github.com/untrustedmodders/plugify/commit/d0b82ce85b0c1ffb4f1bde36e978b0a40da07a4f))
* exclude deps from install ([1dc632a](https://github.com/untrustedmodders/plugify/commit/1dc632a7dfb8d45b2126957833778e65be3aaab8))
* improve class bindings ([f045038](https://github.com/untrustedmodders/plugify/commit/f045038ff943066800613c0128e500bafcad54e7))
* improve enum prettify ([fc46ac3](https://github.com/untrustedmodders/plugify/commit/fc46ac351c9935a643d113444f27b13cbb5c8cef))
* improve ipvbase ([2c72070](https://github.com/untrustedmodders/plugify/commit/2c720701596de48169ddf89ad203f77d0e561a58))
* improve plg formatter for primitive structs ([35b9a16](https://github.com/untrustedmodders/plugify/commit/35b9a165a9b12fb334c6a6d7a13dae9fa13f35c4))
* make relaxed setter for atomic ([5dc203c](https://github.com/untrustedmodders/plugify/commit/5dc203c179eb9435b9f1fdb6e074c1d707b1a5ef))
* performance metrics output in plg app ([08eade5](https://github.com/untrustedmodders/plugify/commit/08eade52c6b1e6b43c65522ba71fae4a38c43745))
* remove assume valid in string header ([8ad7f39](https://github.com/untrustedmodders/plugify/commit/8ad7f3981e9bafda7dca42e123a46c01466c8530))
* remove dublicates validation for enum values ([b0399a7](https://github.com/untrustedmodders/plugify/commit/b0399a752f1fc115d994b470a921ae15710686a1))
* remove git macros ([ded6061](https://github.com/untrustedmodders/plugify/commit/ded6061d4d00c9e9f8b207335c1f21ab367d4098))
* rework allocator for alignment mode ([aea99c2](https://github.com/untrustedmodders/plugify/commit/aea99c2a0fa562895c8b3383c591bc8fa626076d))
* rework event bus little bit ([361398f](https://github.com/untrustedmodders/plugify/commit/361398fd37af66771781fee9a33fa4ff3033295f))
* rework manifest lookup ([e842305](https://github.com/untrustedmodders/plugify/commit/e842305f01d4304ff6714bcfbbdfe74b27381a3e))
* rework safecall to static method ([57bc377](https://github.com/untrustedmodders/plugify/commit/57bc377e0474bc7ce7bfcaa4cf975af2541f94b5))
* rework scoped service locator ([ac5c383](https://github.com/untrustedmodders/plugify/commit/ac5c38377a60260b22af56c235b1bb6428744ad7))
* small refactor ([c536899](https://github.com/untrustedmodders/plugify/commit/c5368993c14e42243392f6b312a37a25a8f4477c))
* some refactor of new classes ([d3b0dd3](https://github.com/untrustedmodders/plugify/commit/d3b0dd3c73a886091eb970b88cfebea84c7112c7))
* static analysis warnings ([3f2dbfd](https://github.com/untrustedmodders/plugify/commit/3f2dbfd61521d3c6a2bc35f144ddb8e54e8346ec))
* string in constexpr on GCC 15 ([d9b99ea](https://github.com/untrustedmodders/plugify/commit/d9b99ea80eb031a845f294de0079b63ce84b7b0a))
* typo ([c2b28f2](https://github.com/untrustedmodders/plugify/commit/c2b28f2d3055395f6913ee91f31508787f4e00cf))

## [2.0.0](https://github.com/untrustedmodders/plugify/compare/v1.2.8...v2.0.0) (2025-10-07)


### ⚠ BREAKING CHANGES

* major update of core

### Features

* add more inplace vectors ([ce4922c](https://github.com/untrustedmodders/plugify/commit/ce4922c6a0d090c5a6080de66e95572b8859aa32))
* major update of core ([4e0b49a](https://github.com/untrustedmodders/plugify/commit/4e0b49aa8b0d8c5ed6c4093768a50067ce1cb832))


### Bug Fixes

* actions ([87ead71](https://github.com/untrustedmodders/plugify/commit/87ead71c121f59346b140796d1894012d6c46a08))
* add __VA_OPT__ ([ed2271f](https://github.com/untrustedmodders/plugify/commit/ed2271ffa4b9ce9ad89184d7295b856f50c57824))
* Add _GNU_SOURCE for steamrt ([46eefc8](https://github.com/untrustedmodders/plugify/commit/46eefc8e17e0e873e64251ebc30077c8bf12facb))
* add -fno-omit-frame-pointer to san mode ([a12add3](https://github.com/untrustedmodders/plugify/commit/a12add30600a9c05fe54dba01472447d61c30b26))
* add 32bit support to hash structs ([ef84dc0](https://github.com/untrustedmodders/plugify/commit/ef84dc070a3d54917761e925102bfbe8ef4c622f))
* add char* support to join ([3a1726c](https://github.com/untrustedmodders/plugify/commit/3a1726ca699fd74d963e61422f4091cef03316d2))
* add concepts to inplace_vector ([608a3aa](https://github.com/untrustedmodders/plugify/commit/608a3aa0c5c6229b226ae61acd861cdeae298141))
* add constexpr to flat_map ([14a36ce](https://github.com/untrustedmodders/plugify/commit/14a36ce6c1b67a90874717b5652cc0e6020129f0))
* add improvements to containers ([fd9f019](https://github.com/untrustedmodders/plugify/commit/fd9f019b684963ee90b4d02ce709142c83259af1))
* add inplace vector container ([2f6f05b](https://github.com/untrustedmodders/plugify/commit/2f6f05be9645ea95091e76a99f22fa5014293090))
* add missing include for gcc ([fe9b87e](https://github.com/untrustedmodders/plugify/commit/fe9b87eb1dca56d8f3bc8141484d41d895e87908))
* add missing macro ([4b68753](https://github.com/untrustedmodders/plugify/commit/4b687534ad0957dd8319755f289ad058cf0ba125))
* add more flags to test app ([7b231e6](https://github.com/untrustedmodders/plugify/commit/7b231e630874c85795efb15edf3e345e19e889fa))
* add plg string to heterogeneous map ([17f13ed](https://github.com/untrustedmodders/plugify/commit/17f13eddb49999617122cc9e6c0d3ca885817fd4))
* add re2 for test ([ae3f66b](https://github.com/untrustedmodders/plugify/commit/ae3f66b93a65aca6a7f78dd83337efd62d726d62))
* add sanitizer improvements ([aeadddd](https://github.com/untrustedmodders/plugify/commit/aeadddd98ffbc8586e33f491a43445403db0db70))
* add smart ptr ([df3298e](https://github.com/untrustedmodders/plugify/commit/df3298ef363adec30bb621004cebd4f0ebd9ee19))
* add some warning supressions ([38d6e9f](https://github.com/untrustedmodders/plugify/commit/38d6e9f5452fee901586219974cd7b177847d7bf))
* add string reverse for join in advance ([1fa5d71](https://github.com/untrustedmodders/plugify/commit/1fa5d719829ee809ad7de5dda1c7a64716df302d))
* add versioning support for language ([df7b22a](https://github.com/untrustedmodders/plugify/commit/df7b22a4da41f4f603961f60ee76bfff410eb654))
* assembly lookup by name ([108f16b](https://github.com/untrustedmodders/plugify/commit/108f16b97e9cd6380b83c083296e0a40d8d1f43d))
* build issue ([fbef7ea](https://github.com/untrustedmodders/plugify/commit/fbef7ea01c69e03af6cf0b474a0a9aa1db494e9b))
* build on windows using cmake 4.0 ([a760dc9](https://github.com/untrustedmodders/plugify/commit/a760dc99faccec2a1d4a208c4091685b461397f7))
* change default build type ([48e6e0a](https://github.com/untrustedmodders/plugify/commit/48e6e0ac974c2a44dffcfc327bc71871c5fae0c9))
* clang build ([464e32f](https://github.com/untrustedmodders/plugify/commit/464e32f8f04f3eed9d149fdfc9db937093f29e68))
* clang build ([bb5f59b](https://github.com/untrustedmodders/plugify/commit/bb5f59b92bdffb7b16a44efde23102082f7ea90e))
* clang fixes ([77628c3](https://github.com/untrustedmodders/plugify/commit/77628c388c302ac04c6443aab97821abb53636ba))
* clang warning ([89c63a4](https://github.com/untrustedmodders/plugify/commit/89c63a4c7b7e3be3f81434454d0c7eb996f64a24))
* clang warnings ([c11deb8](https://github.com/untrustedmodders/plugify/commit/c11deb8d03d3bb479ea2198de851e4f5a9250611))
* clang warnings ([7ceb64f](https://github.com/untrustedmodders/plugify/commit/7ceb64f5fcd23378ad6f59bf86a3ef0d6a5257cc))
* disable reports by default ([c4e37bf](https://github.com/untrustedmodders/plugify/commit/c4e37bf5c113084f26892e38aefb5160f0af23b0))
* disable sanitizer for short strings ([1a7f39c](https://github.com/untrustedmodders/plugify/commit/1a7f39cc3927905c694d5685e4b2137b93990c8a))
* gcc shadow warnings ([e4a1a09](https://github.com/untrustedmodders/plugify/commit/e4a1a0963ea4f75898e0fae150f1de61007328df))
* improve features tests ([79c7734](https://github.com/untrustedmodders/plugify/commit/79c77348cd82b4f8c3330132a1e9456cb7abfe50))
* improve fs class ([cc02696](https://github.com/untrustedmodders/plugify/commit/cc02696ecc9468d5025f58d951a722a7198bd901))
* improve sting assigning for non continuous iterators ([01e9e41](https://github.com/untrustedmodders/plugify/commit/01e9e4133bc08e19e3f2d9b53737aee605ed60dd))
* improvements of jit helpers ([35d0cfa](https://github.com/untrustedmodders/plugify/commit/35d0cfa1e402e4202935b349b0c49946e524a652))
* insensitive equals casts ([3eb0b05](https://github.com/untrustedmodders/plugify/commit/3eb0b0572b03bb35f205dca0291fc78af159cddb))
* make property safer on coping ([72f8265](https://github.com/untrustedmodders/plugify/commit/72f826507ad0302f3a828cd972252425453603cf))
* mingw build ([186b4d2](https://github.com/untrustedmodders/plugify/commit/186b4d2bafa39706e8e9d827f8cffefd6911c52b))
* mingw build ([c36531d](https://github.com/untrustedmodders/plugify/commit/c36531d2f6575c43b162b3ed07d27725f5370e6a))
* minor changes ([480d6d3](https://github.com/untrustedmodders/plugify/commit/480d6d3f3ed21905c0c4e3b747812015d20b1a26))
* more clang fixes ([6ae8e4c](https://github.com/untrustedmodders/plugify/commit/6ae8e4c0b39301798fb4dbfc27349cfe52465429))
* move to thread from jthread for apple clang ([085ca32](https://github.com/untrustedmodders/plugify/commit/085ca32d2cc6950dd2d468cf1449b91eaf3829dd))
* regex for name check ([ac293e3](https://github.com/untrustedmodders/plugify/commit/ac293e343f820b2a77062e284c904b9c0c190c6f))
* regex name check ([e92fdaa](https://github.com/untrustedmodders/plugify/commit/e92fdaa8dbee78e14f940a8b1f9be3c92742fa14))
* remove crashpad ([df8674e](https://github.com/untrustedmodders/plugify/commit/df8674e83014b99ddc50a74eeb213c30a233ac58))
* remove explicit ctors from string ([dd42bbc](https://github.com/untrustedmodders/plugify/commit/dd42bbc57c2fdfbac936b04521dd6205c3b8e668))
* remove export symbol ([36c7347](https://github.com/untrustedmodders/plugify/commit/36c7347fd5da6f13f185daf2e0597ffe3ac69e27))
* remove flat maps ([81b17e8](https://github.com/untrustedmodders/plugify/commit/81b17e8351ae932db801ccaece4600acd6a32ec6))
* remove format macro ([de68c09](https://github.com/untrustedmodders/plugify/commit/de68c09cf65c7e94a7a932ac78c2af9f7bd7f0e5))
* remove resourceDirectories and resources from packages ([73786a8](https://github.com/untrustedmodders/plugify/commit/73786a82b02753e3c32997f6efa183c9dfb0dc5a))
* remove shared_ptr_access ([3357a12](https://github.com/untrustedmodders/plugify/commit/3357a1211f0b47744eac16dc0a35e67c3bc4597f))
* remove some unnecessary checks ([db61558](https://github.com/untrustedmodders/plugify/commit/db61558772b5dfe5abeb1d9a26d20839d9d2c45a))
* remove unused ([b2f76af](https://github.com/untrustedmodders/plugify/commit/b2f76af2d773c81a0eb52c152045776a1a4558c4))
* remove unused includes ([e2ef25f](https://github.com/untrustedmodders/plugify/commit/e2ef25fcd9fc63232829c2ca79425a3b414523f3))
* remove unused token ([f87fab4](https://github.com/untrustedmodders/plugify/commit/f87fab4e2e98200f345d5851e0aabed78908ed7d))
* rename local members for LLVM style ([caf5a84](https://github.com/untrustedmodders/plugify/commit/caf5a84e1674d5194abb07eb8ea791b2a60a8ba6))
* rename requestedVersion to version ([e859ba8](https://github.com/untrustedmodders/plugify/commit/e859ba8f167d4959b0bfcfc7b747cd9fb705cb59))
* replace __VA_OPT__ by  ##__VA_ARGS__ ([25b9ceb](https://github.com/untrustedmodders/plugify/commit/25b9ceb716b466f970df53965844bf821005c8ad))
* replace inplace_vector ([bb1d6e3](https://github.com/untrustedmodders/plugify/commit/bb1d6e399039d1b2f1070161d0b7bbac756d0d40))
* replace loops by std::copy in inplace ([fd3c882](https://github.com/untrustedmodders/plugify/commit/fd3c882219937a80e02dcbc4b68c12d9cc9256ab))
* rework case_insensitive_equal ([19910f7](https://github.com/untrustedmodders/plugify/commit/19910f705894762bbdb0fe4ac7b7a10a5d0c3209))
* rework exceptions ([1f2cac3](https://github.com/untrustedmodders/plugify/commit/1f2cac39bc06c61ca99f7d7f7769550c463009ae))
* rework runtime path for modules using manifest name ([106c577](https://github.com/untrustedmodders/plugify/commit/106c5776dcfdfb022e310e6da03ed9e9e297b04a))
* rework sanitizer + add win san ([18c0758](https://github.com/untrustedmodders/plugify/commit/18c075881459025b92001e94e3ca576d9edebc50))
* set cxx ver to tests ([9ae49a2](https://github.com/untrustedmodders/plugify/commit/9ae49a2d2695801b044759f8045bf33110550176))
* small cmake fixes ([b30977c](https://github.com/untrustedmodders/plugify/commit/b30977c0d0fa617c499cdac0d845ed1f3ef88c55))
* small improvements ([4dff788](https://github.com/untrustedmodders/plugify/commit/4dff7887d12a1a0622f3b6c3263a61aaf0d0ecb2))
* small refactor of stl container replacements ([c9a1274](https://github.com/untrustedmodders/plugify/commit/c9a12740213fd5afaf62a3797d8dfba149667a12))
* some allocator related tests ([bfcc0ec](https://github.com/untrustedmodders/plugify/commit/bfcc0ec1640a6ccd4f677a384161a87bb0b50138))
* some build errors ([e4537f6](https://github.com/untrustedmodders/plugify/commit/e4537f6dd18d592892d3c1ee483acbbf62b41d47))
* some container improvements ([03f517f](https://github.com/untrustedmodders/plugify/commit/03f517f98cef904a099bda893cbccedad34f6b3f))
* some improvements ([f93c9f0](https://github.com/untrustedmodders/plugify/commit/f93c9f071ced2bc0ad4b48352ef23330259e62e4))
* some macro improvements ([d9049a3](https://github.com/untrustedmodders/plugify/commit/d9049a3dcd9a0e555acdffa93968f65e79c613de))
* some plug test app improvements ([a3e6f65](https://github.com/untrustedmodders/plugify/commit/a3e6f6548575873bdf25547743cf294ff3524389))
* some refactor ([839665f](https://github.com/untrustedmodders/plugify/commit/839665ff89a33a5c8f8dac648a47b94c9dc4fa5b))
* stacktrace and remove format from assembly ([c1b39b4](https://github.com/untrustedmodders/plugify/commit/c1b39b46fc29c50e8bb7ccab8a10a3f6b1456b32))
* static strings on linux ([63f5ef1](https://github.com/untrustedmodders/plugify/commit/63f5ef1b553512496d2a73c08947cba34b7e2aad))
* typo ([75212ea](https://github.com/untrustedmodders/plugify/commit/75212eaee647cb64117acfe08d2df9486d685858))
* update .clang-format ([c9e5451](https://github.com/untrustedmodders/plugify/commit/c9e545194564f94f3686fd191a562fe7b34e7161))
* update actions build ([c1d5890](https://github.com/untrustedmodders/plugify/commit/c1d589060cce1f80dbb9ba0759b05c2911578151))
* update containers ([3eab09a](https://github.com/untrustedmodders/plugify/commit/3eab09a389bd03b854887d9c054e6108dd33cb30))
* update libdwarf ([05edb7c](https://github.com/untrustedmodders/plugify/commit/05edb7c3918329a0eb1f2044f260336f951f73fe))
* update schemas ([304348f](https://github.com/untrustedmodders/plugify/commit/304348f2f7a24675e35d2587643234c21103cb40))
* win build ([c99398a](https://github.com/untrustedmodders/plugify/commit/c99398a7a8edd18c7c3ca32cd773ea8c62adb6a9))
* win build ([1a86b85](https://github.com/untrustedmodders/plugify/commit/1a86b853501505f7b652838b14d697c7e779fbf5))

## [1.2.8](https://github.com/untrustedmodders/plugify/compare/v1.2.7...v1.2.8) (2025-07-31)


### Bug Fixes

* cxx version and add macos to build ([c1eaf04](https://github.com/untrustedmodders/plugify/commit/c1eaf04efbb3d7a136716607112b6609a87050dd))

## [1.2.7](https://github.com/untrustedmodders/plugify/compare/v1.2.6...v1.2.7) (2025-07-31)


### Bug Fixes

* add C++23 and above support ([10ffdcd](https://github.com/untrustedmodders/plugify/commit/10ffdcda328befe88178b515b80de7d8ab1ce666))
* add debugging include checker ([394b931](https://github.com/untrustedmodders/plugify/commit/394b931508faf860d7d37044aa4a44e682752d69))
* add default value to types ([beb95e9](https://github.com/untrustedmodders/plugify/commit/beb95e909840579dc98c30d151e0baeec4c8829f))
* add fs::absolute error handling ([8cc6d6e](https://github.com/untrustedmodders/plugify/commit/8cc6d6e61fd6b9fa168cec61684ef370279bf601))
* add Kahn's Algorithm for plugin order detection instead of DFS ([d628016](https://github.com/untrustedmodders/plugify/commit/d6280161c04c34745e8e1d8fadbdde5a750acb7f))
* add plg::allocator ([e416259](https://github.com/untrustedmodders/plugify/commit/e4162599db93e484092166af52d193847e6e6edb))
* another improve of zip slip detection ([7227b04](https://github.com/untrustedmodders/plugify/commit/7227b04782045781605ab1f9cfdf94e765cb9da0))
* avoid double checks in fs ([8ec9cbc](https://github.com/untrustedmodders/plugify/commit/8ec9cbccc955f1ad880c92f84ca7e1ff32e3585f))
* bump steamrt compiler for build ([506b8b7](https://github.com/untrustedmodders/plugify/commit/506b8b779e72b156cf5bd4082406b53b503d9d9d))
* change msvc build for action ([46f677e](https://github.com/untrustedmodders/plugify/commit/46f677e0e69b9ebbf065ffd448344fefe429e45b))
* change regs for cb return ([a4f936f](https://github.com/untrustedmodders/plugify/commit/a4f936f4a7a4f0cd1fb000525917fed44776fef1))
* improve allocator ([202cc10](https://github.com/untrustedmodders/plugify/commit/202cc101c0c0891f47c00f5086814d771e7c6301))
* improve content type lookup ([16ad781](https://github.com/untrustedmodders/plugify/commit/16ad781c026d690f58d17c4a56a1ca5e93ad6385))
* inline plugify_throw ([b8a3810](https://github.com/untrustedmodders/plugify/commit/b8a3810e882557c30c64614d0df0d46d68466164))
* make allocator constexpr friendly and add ability to align properly ([8009720](https://github.com/untrustedmodders/plugify/commit/8009720dec3ff2241a04ca1a79a9df29b9b58d1f))
* more cleanup of code ([5d58ac6](https://github.com/untrustedmodders/plugify/commit/5d58ac6c6e7329aa66118bef6327cad532b51cf7))
* move validations to separate methods ([5ac1299](https://github.com/untrustedmodders/plugify/commit/5ac1299372e605aa7883ef04dc82d6269214514a))
* msvc build ([2ef709c](https://github.com/untrustedmodders/plugify/commit/2ef709ca4efe553e81f26a239f72aa1f4f67afb6))
* refactoring extraction to a single loop ([31a3ecd](https://github.com/untrustedmodders/plugify/commit/31a3ecd74c5e5d3d7a362326d17479fd5740b318))
* remove additional buffer copying ([86fa63d](https://github.com/untrustedmodders/plugify/commit/86fa63d6223ef9d8e1a699ba40a2cc5a03c5fa2f))
* remove constexpr ([203d385](https://github.com/untrustedmodders/plugify/commit/203d3859f6a57eaa4492f292ed5d04df45af2b15))
* remove deprecated CallConvId::kHost ([12c770a](https://github.com/untrustedmodders/plugify/commit/12c770a20d22c2601da30fe352c9945c1a8fbbeb))
* remove funcName check for inner prototypes ([186728e](https://github.com/untrustedmodders/plugify/commit/186728e7bef6982a1c5f00428417bed1b4ec572b))
* remove legacy platform check ([ab02860](https://github.com/untrustedmodders/plugify/commit/ab02860425bde2d2431f79e40c6d4676037ad653))
* remove magic strings ([b285e1c](https://github.com/untrustedmodders/plugify/commit/b285e1cd0700d5b7f363e974c97177f4a458fe75))
* remove memaddr operators ([cf0b2d1](https://github.com/untrustedmodders/plugify/commit/cf0b2d198651ed1aa5fbf505f4486ece38d55947))
* rework os detection ([0df2b39](https://github.com/untrustedmodders/plugify/commit/0df2b39cdc99ff7e29f70be393956ddeb5f3aa42))
* some build errors ([d1528ab](https://github.com/untrustedmodders/plugify/commit/d1528abec534f443a5c99e5a904f2d3df41781fc))
* update arch naming (again) ([ffc0e89](https://github.com/untrustedmodders/plugify/commit/ffc0e893d0721901ba3058f813418c96277384e2))
* update arch tags for action build ([d0f4264](https://github.com/untrustedmodders/plugify/commit/d0f426449b29c93ded3cdd609068e37d9da6a8a0))
* update asmjit ([bfeaca6](https://github.com/untrustedmodders/plugify/commit/bfeaca6a59f903389cb7cc2169909618d29d9b5b))
* update asmjit HasHiArgSlot ([723f9e8](https://github.com/untrustedmodders/plugify/commit/723f9e868f10c120c3a42086a091d7a59d76d04f))
* update asmjit headers ([955e1bd](https://github.com/untrustedmodders/plugify/commit/955e1bdb3232be88cd5f7b743220cbc0c0799af0))
* update mem addr with constexpr ([15c005d](https://github.com/untrustedmodders/plugify/commit/15c005df3a51f6b0327324be8b73171be02a9d9a))
* update readme and add missing permission ([cc5c1f6](https://github.com/untrustedmodders/plugify/commit/cc5c1f661ae0cecf95b19a631972ade2a76dfafe))
* update sig lookup ([3931ecf](https://github.com/untrustedmodders/plugify/commit/3931ecfa60a2b5a793d99a066be024e1b487ab54))
* win build ([aa2f3fe](https://github.com/untrustedmodders/plugify/commit/aa2f3fede307e606f0cea0bde5e1a054b88d5a15))
* x86 build ([cb9e880](https://github.com/untrustedmodders/plugify/commit/cb9e8806c2a0f12a35cfaa38beae45e2e42fccd0))
* x86 float return ([911bd19](https://github.com/untrustedmodders/plugify/commit/911bd198a1856070b95f7951be83fc7f44cd6505))
* zip slip vulnerability ([0a03f5a](https://github.com/untrustedmodders/plugify/commit/0a03f5a133567640631e8c7ba86f4500be454057))

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
