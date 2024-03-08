<a name="readme-top"></a>

<h1 align="center">
  <a href="https://github.com/untrustedmodders/plugify">
    <!-- Please provide path to your logo here -->
    <img src="docs/images/logo.png" alt="Logo" width="261" height="100">
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
  - [Built With](#built-with)
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
   Embrace a dynamic programming landscape with Plugify's unique ability to effortlessly support a myriad of programming languages. Choose the languages that best suit your project, opening the door to a diverse and adaptable development environment.

2. **Language Modules:**  
   Elevate your plugin development experience by easily installing language modules for different programming languages. Plugify adapts to your preferred language, allowing you to create feature-rich plugins without language constraints.

3. **Inter-Language Communication:**  
   Enable seamless communication between plugins, transcending language barriers. Export methods and share data between plugins, fostering collaboration regardless of the underlying programming language.

4. **Package Manager:**  
   Simplify package management with Plugify's robust package manager. Install, update, downgrade, or remove packages effortlessly, ensuring your development environment remains efficient and up-to-date.

5. **Versatile Packages:**  
   The package manager supports both plugins and language modules, offering a comprehensive solution for all your development needs. Quickly share your creations with the community by packaging your plugins and modules. Easily extend the functionality of Plugify by incorporating new packages that suit your project requirements and contribute to the growing ecosystem.

### Motivation

> **[?]**
> Provide general information about your project here.
> What problem does it (intend to) solve?
> What is thwae purpose of your project?
> Why did you undertake it?
> You don't have to answer all the questions -- just the ones relevant to your project.

### Built With

> **[?]**
> Please provide the technologies that are used in the project.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

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

The same version is also available [online](https://github.com/untrustedmodders/plugify)
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
2. On Linux install CURL, otherwise build it in embedding mode with cmake options
   ```sh
   sudo apt-get install -y libcurl4-openssl-dev
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
- [C++ Language Module](https://github.com/untrustedmodders/cpp-lang-module)
- [C# Language Module](https://github.com/untrustedmodders/csharp-lang-module)
- [Python Language Module](https://github.com/untrustedmodders/py3-12-lang-module)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## In Action

While Plugify is a relatively new project, it is making waves in the realm of server-side modding, 
particularly in the Counter-Strike 2 community. As of now, Plugify is primarily utilized in the 
ance of a new project known as [CS2-Plugify](https://github.com/cs2-plugify/), which is also being developed by our team.

If you know of other resources out there that are about `Plugify`, feel free to open an issue or a PR and we will be glad to add them here.

## Roadmap

See the [open issues](https://github.com/untrustedmodders/plugify/issues) for a list of proposed features (and known issues).

- [Top Feature Requests](https://github.com/untrustedmodders/plugify/issues?q=label%3Aenhancement+is%3Aopen+sort%3Areactions-%2B1-desc) (Add your votes using the 👍 reaction)
- [Top Bugs](https://github.com/untrustedmodders/plugify/issues?q=is%3Aissue+is%3Aopen+label%3Abug+sort%3Areactions-%2B1-desc) (Add your votes using the 👍 reaction)
- [Newest Bugs](https://github.com/untrustedmodders/plugify/issues?q=is%3Aopen+is%3Aissue+label%3Abug)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Support

> **[?]**
> Provide additional ways to contact the project maintainer/maintainers.

Reach out to the maintainer at one of the following places:

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
