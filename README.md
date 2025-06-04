# advent-of-code-cpp

This is the advent-of-code-cpp project.

# Building and installing

See the [BUILDING](BUILDING.md) document.

## perl dependency
There is perl dependency for Linux users that is not retrieved by vcpkg. If you see CMake errors about IPC or perl, please get perl from your package manager.

### Fedora 40
```sh
sudo dnf install perl-IPC-Cmd perl-FindBin perl-Compare perl-File-Compare
```

# Enabling downloading inputs
`StartNewDay` will download your input for you, but in order to do that, it needs your session cookie. Login to https://adventofcode.com, press F12, and find your session token. On Firefox, this is located under the "Storage" tab then the "Cookies" menu. Paste the value into a file in the "inputs" subdirectory in a file called ".adventofcode.session".

# Contributing

See the [CONTRIBUTING](CONTRIBUTING.md) document.

# Licensing
<!--
Please go to https://choosealicense.com/licenses/ and choose a license that
fits your needs. The recommended license for a project of this type is the
GNU AGPLv3.
-->
