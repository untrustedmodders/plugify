<a name="readme-top"></a>

<h1 align="center">
  <a href="https://github.com/untrustedmodders/plugify">
    <!-- Please provide path to your logo here -->
    <img src="docs/images/logo.png" alt="Logo" width="261" height="100">
  </a>
</h1>

![Build Status](https://github.com/untrustedmodders/plugify/actions/workflows/cmake-multi-platform.yml/badge.svg)

<div align="center">
  A Modern C++ Plugin and Package Manager with Multi-Language Support
  <br />
  <a href="#about"><strong>Explore the screenshots »</strong></a>
  <br />
  <br />
  <a href="https://github.com/untrustedmodders/plugify/issues/new?assignees=&labels=bug&template=01_BUG_REPORT.md&title=bug%3A+">Report a Bug</a>
  ·
  <a href="https://github.com/untrustedmodders/plugify/issues/new?assignees=&labels=enhancement&template=02_FEATURE_REQUEST.md&title=feat%3A+">Request a Feature</a>
  .
  <a href="https://github.com/untrustedmodders/plugify/issues/new?assignees=&labels=question&template=04_SUPPORT_QUESTION.md&title=support%3A+">Ask a Question</a>
</div>

<div align="center">
<br />

[![Project license](https://img.shields.io/github/license/untrustedmodders/plugify.svg?style=flat-square)](LICENSE)

[![Pull Requests welcome](https://img.shields.io/badge/PRs-welcome-ff69b4.svg?style=flat-square)](https://github.com/untrustedmodders/plugify/issues?q=is%3Aissue+is%3Aopen+label%3A%22help+wanted%22)
[![code with love by untrustedmodders](https://img.shields.io/badge/%3C%2F%3E%20with%20%E2%99%A5%20by-untrustedmodders-ff1414.svg?style=flat-square)](https://github.com/untrustedmodders)

</div>

<details open="open">
<summary>Table of Contents</summary>

- [About](#about)
  - [Built With](#built-with)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Installation](#installation)
- [Usage](#usage)
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

<details>


<summary>Screenshots</summary>
<br>

<p align="right">(<a href="#readme-top">back to top</a>)</p>

> **[?]**
> Please provide your screenshots here.

|                               Home Page                               |                               Login Page                               |
| :-------------------------------------------------------------------: | :--------------------------------------------------------------------: |
| <img src="docs/images/screenshot.png" title="Home Page" width="100%"> | <img src="docs/images/screenshot.png" title="Login Page" width="100%"> |

</details>

### Built With

> **[?]**
> Please provide the technologies that are used in the project.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Getting Started


### Prerequisites

> **[?]**
> What are the project requirements/dependencies?

### Installation

1. Clone the repo
   ```sh
   git clone https://github.com/untrustedmodders/plugify.git
   ```
2. Install dependencies
   ```sh
   git submodule update --init --recursive
   ```
   > On Linux install CURL, otherwise build it in embedding mode with cmake options
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
   cmake --build . --preset Release
   ```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Usage

> **[?]**
> How does one go about using it?
> Provide various use cases and code examples here.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

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
