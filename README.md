<a name="readme-top"></a>

<h1 align="center">
  <a href="https://github.com/untrustedmodders/plugify">
    <!-- Please provide path to your logo here -->
    <img src="https://github.com/untrustedmodders/plugify/blob/main/docs/images/logo.png?raw=true" alt="Logo" width="261" height="100">
  </a>
</h1>

<div align="center">
  A Modern C++ Plugin and Package Manager with Multi-Language Support
  <br />
  <a href="https://github.com/untrustedmodders/plugify/issues/new?assignees=&labels=bug&template=01_BUG_REPORT.md&title=bug%3A+">Report a Bug</a>
  ·
  <a href="https://github.com/untrustedmodders/plugify/issues/new?assignees=&labels=enhancement&template=02_FEATURE_REQUEST.md&title=feat%3A+">Request a Feature</a>
  .
  <a href="https://github.com/untrustedmodders/plugify/issues/new?assignees=&labels=question&template=04_SUPPORT_QUESTION.md&title=support%3A+">Ask a Question</a>
</div>

<div align="center">
<br />

</div>

<details open="open">
<summary>Table of Contents</summary>

- [About](#about)
  - [Key Features](#key-features)
  - [Motivation](#motivation)
- [Integration](#integration)
  - [Requirements](#requirements)
  - [CMake](#cmake)
  - [Example](#example)
- [Documentation](#documentation)
- [Tests](#tests)
- [Extensions](#extensions)
- [In Action](#in-action)
- [Roadmap](#roadmap)
- [Support](#support)
- [Project assistance](#project-assistance)
- [Contributing](#contributing)
- [Authors & contributors](#authors--contributors)
- [Security](#security)
- [License](#license)
- [Acknowledgements](#acknowledgements)

</details>

---

## About

Unlock a new era in plugin development with Plugify, a revolutionary library designed to streamline and enhance
the way plugins are created and utilized. Plugify goes beyond traditional plugin management by introducing innovative language modules,
redefining the boundaries of cross-language communication.

### Key Features

1. **Dynamic Language Diversity:**  
Plugify’s power is in its flexibility. It allows you to use multiple programming languages. Choose the languages you need for the project, and go. This will make your development environment more diverse and flexible.

2. **Language Modules:**  
Plugify allows you to install language modules for various programming languages, so you can use the language of your choice and develop plugins with the core without any doubt about language incompatibilities.

3. **Inter-Language Communication:**  
Make plugins communicate to each other in any existing language. Export methods, and share data between plugins, without regard for the programming language used.

5. **Package Manager:**  
Lets your system manage packages. It has a robust package manager. Install, remove, update, downgrade packages easily and make your development environment keep up with the latest packages to read.

6. **Versatile Packages:**  
The package system handles plugins and language modules, and covers most of a developer’s needs. Package your plugins and modules for easy sharing. Easily extend Plugify with new packages that meet the requirements of your project and fit the ecosystem.

### Motivation

The plan was to build a new plugin system for CS in C#, but some of the decisions that were made in the early version led to a decision to ‘remove the training wheels’, unshackle the system from any game or embeddable language and create a system which allows the developers of plugins to build them in whatever language they like. For as long as that language module is installed, the system is able to support it. So by no longer trying to force it to be deemed a ‘good’ solution to be embedded in a variety of games, Plugify now goes far beyond the logical limitations a C#-specific solution has. What this means is that Plugify not only has the potential to have a larger community of developers, but is now flexible from the beginning to last a long time. Longevity was always a selling point, since if it was to work, it always had to be a ‘forever’ feature. It’s not just games development that stands to benefit. The flexibility of Plugify should mean it has uses in many projects, not just in games.

## Integration

### Requirements

- CMake version 3.14 or later.
- Doxygen version 1.8 or later.
- Requires C++20 or later.
- Only designed and tested for 64bit little-endian systems.

[Actions](https://github.com/untrustedmodders/plugify/actions) build and test with [Clang](https://clang.llvm.org) (15+), [MSVC](https://visualstudio.microsoft.com/vs/features/cplusplus/) (2022), and [GCC](https://gcc.gnu.org) (11+) on apple, windows, and linux.

![clang build](https://github.com/untrustedmodders/plugify/actions/workflows/clang.yml/badge.svg) ![gcc build](https://github.com/untrustedmodders/plugify/actions/workflows/gcc.yml/badge.svg) ![msvc build](https://github.com/untrustedmodders/plugify/actions/workflows/msvc.yml/badge.svg) ![msys build](https://github.com/untrustedmodders/plugify/actions/workflows/msys2.yml/badge.svg)

### CMake

You can also use the `plugify::plugify` interface target in CMake.

#### External

To use this library from a CMake project, you can locate it directly with `find_package()` and use the namespaced imported target from the generated package configuration:

```cmake
# CMakeLists.txt
find_package(plugify REQUIRED)
...
add_library(foo ...)
...
target_link_libraries(foo PRIVATE plugify::plugify)
```

#### Embedded

To embed the library directly into an existing CMake project, place the entire source tree in a subdirectory and call `add_subdirectory()` in your `CMakeLists.txt` file:

```cmake
# Typically you don't care so much for a third party library's tests to be
# run from your own project's code.
set(PLUGIFY_BUILD_TESTS OFF CACHE INTERNAL "")

# Don't use include(plugify/CMakeLists.txt) since that carries with it
# unintended consequences that will break the build.  It's generally
# discouraged (although not necessarily well documented as such) to use
# include(...) for pulling in other CMake projects anyways.
add_subdirectory(plugify)
...
add_library(foo ...)
...
target_link_libraries(foo PRIVATE plugify::plugify)
```

##### Embedded (FetchContent)

Since CMake v3.11,
[FetchContent](https://cmake.org/cmake/help/v3.11/module/FetchContent.html) can
be used to automatically download a release as a dependency at configure time.

Example:
```cmake
include(FetchContent)

FetchContent_Declare(plugify URL https://github.com/untrustedmodders/plugify/releases/download/v1.0.0/plugify.tar.xz)
FetchContent_MakeAvailable(plugify)

target_link_libraries(foo PRIVATE plugify::plugify)
```

**Note**: It is recommended to use the URL approach described above which is supported as of version 1.0.0. See
[wiki](https://github.com/untrustedmodders/plugify/wiki/cmake/#fetchcontent) for more information.

#### Supporting Both

To allow your project to support either an externally supplied or an embedded Plugify library, you can use a pattern akin to the following:

``` cmake
# Top level CMakeLists.txt
project(FOO)
...
option(FOO_USE_EXTERNAL_PLUGIFY "Use an external Plugify library" OFF)
...
add_subdirectory(thirdparty)
...
add_library(foo ...)
...
# Note that the namespaced target will always be available regardless of the
# import method
target_link_libraries(foo PRIVATE plugify::plugify)
```
```cmake
# thirdparty/CMakeLists.txt
...
if(FOO_USE_EXTERNAL_PLUGIFY)
  find_package(plugify REQUIRED)
else()
  add_subdirectory(plugify)
endif()
...
```

`thirdparty/plugify` is then a complete copy of this source tree.

### Example

This code creates an instance of the object implementing the plugify::IPlugify interface. It sets up logging, initializes the instance, and then interacts with a package manager and a plugin manager if they are available. Error handling is included for initialization failures.

```c++
std::shared_ptr<plugify::IPlugify> instance = plugify::MakePlugify();
if (instance) {
	auto logger = std::make_shared<StdLogger>();
	instance->SetLogger(logger);
	logger->SetSeverity(plugify::Severity::Debug);
	
	if (!instance->Initialize()) {
		std::cout << "No feet, no sweets!");
		return EXIT_FAILURE;
	}

	if (auto packageManager = instance->GetPackageManager().lock()) {
		packageManager->Initialize();

		if (packageManager->HasMissedPackages()) {
			std::cerr << "Plugin manager has missing packages." << std::endl;
			packageManager->InstallMissedPackages();
			continue;
		}
		if (packageManager->HasConflictedPackages()) {
			std::cerr << "Plugin manager has conflicted packages." << std::endl;
			packageManager->UninstallConflictedPackages();
			continue;
		}
	}

	if (auto pluginManager = instance->GetPluginManager().lock()) {
		pluginManager->Initialize();
	}
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Documentation

The documentation is based on [doxygen](http://www.doxygen.nl/). To build it:

   ```sh
   cd build
   cmake .. -DPLUGIFY_BUILD_DOCS=ON
   cmake --build . --target docs
   ```

The API reference is created in HTML format in the `build/docs/html` directory.
To navigate it with your favorite browser:

   ```sh
   cd build
   your_favorite_browser docs/html/index.html
   ```

The same version is also available [online](https://untrustedmodders.github.io/plugify)
for the latest release, that is the last stable tag.<br/>
Moreover, there exists a [wiki](https://github.com/untrustedmodders/plugify/wiki) dedicated
to the project where users can find all related documentation pages.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Tests

To build the basic testing app:

1. Clone the repo
   ```sh
   git clone https://github.com/untrustedmodders/plugify.git
   ```
2. Install CURL, otherwise build it in embedding mode with cmake options  
   a. Linux:
   ```sh
   sudo apt-get install -y libcurl4-openssl-dev
   ```
   b. Mac:
   ```sh
   brew install curl
   ```
   > The dependencies can be used as external `(find_package)` and embedding `(add_subdirectory)`, you can use plugify options to choose what you suits your need.
3. Create build folder
   ```sh
   mkdir build
   cd build
   ```
4. Build the project
   ```sh
   cmake .. 
   cmake --build . --target plug
   ```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Extensions

Here is a list of all already implemented language modules:
- [C++ Language Module](https://github.com/untrustedmodders/plugify-module-cpp)
- [C# Language Module](https://github.com/untrustedmodders/csharp-lang-module)
- [Python Language Module](https://github.com/untrustedmodders/plugify-module-python3.12)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## In Action

While Plugify is a relatively new project, it is making waves in the realm of server-side modding, 
particularly in the Counter-Strike 2 community. As of now, Plugify is primarily utilized in the 
ance of a new project known as [CS2-Plugify](https://github.com/mms2-plugify/), which is also being developed by our team.

If you know of other resources out there that are about `Plugify`, feel free to open an issue or a PR and we will be glad to add them here.

## Roadmap

See the [open issues](https://github.com/untrustedmodders/plugify/issues) for a list of proposed features (and known issues).

- [Top Feature Requests](https://github.com/untrustedmodders/plugify/issues?q=label%3Aenhancement+is%3Aopen+sort%3Areactions-%2B1-desc) (Add your votes using the 👍 reaction)
- [Top Bugs](https://github.com/untrustedmodders/plugify/issues?q=is%3Aissue+is%3Aopen+label%3Abug+sort%3Areactions-%2B1-desc) (Add your votes using the 👍 reaction)
- [Newest Bugs](https://github.com/untrustedmodders/plugify/issues?q=is%3Aopen+is%3Aissue+label%3Abug)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Support

Reach out to the maintainer at one of the following places:

- [Discord](https://discord.gg/ZT8SXzrY9a)
- [GitHub issues](https://github.com/untrustedmodders/plugify/issues/new?assignees=&labels=question&template=04_SUPPORT_QUESTION.md&title=support%3A+)
- Contact options listed on [this GitHub profile](https://github.com/untrustedmodders)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Project assistance

If you want to say **thank you** or/and support active development of plugify:

- Add a [GitHub Star](https://github.com/untrustedmodders/plugify) to the project.
- Tweet about the plugify.
- Write interesting articles about the project on [Dev.to](https://dev.to/), [Medium](https://medium.com/) or your personal blog.

Together, we can make plugify **better**!

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Contributing

First off, thanks for taking the time to contribute! Contributions are what make the open-source community such an amazing place to learn, inspire, and create. Any contributions you make will benefit everybody else and are **greatly appreciated**.


Please read [our contribution guidelines](docs/CONTRIBUTING.md), and thank you for being involved!

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Authors & contributors

The original setup of this repository is by [untrustedmodders](https://github.com/untrustedmodders).

For a full list of all authors and contributors, see [the contributors page](https://github.com/untrustedmodders/plugify/contributors).

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Security

Plugify follows good practices of security, but 100% security cannot be assured.
Library is provided **"as is"** without any **warranty**. Use at your own risk.

_For more information and to report security issues, please refer to our [security documentation](docs/SECURITY.md)._

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## License

This project is licensed under the **MIT license**.

See [LICENSE](LICENSE) for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Acknowledgements

> **[?]**
> If your work was funded by any organization or institution, acknowledge their support here.
> In addition, if your work relies on other software libraries, or was inspired by looking at other work, it is appropriate to acknowledge this intellectual debt too.

<p align="right">(<a href="#readme-top">back to top</a>)</p>
