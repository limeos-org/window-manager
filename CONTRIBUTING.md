<picture>
  <source media="(prefers-color-scheme: dark)" srcset=".github/contributing_banner_white.png">
  <source media="(prefers-color-scheme: light)" srcset=".github/contributing_banner_black.png">
  <img alt="LimeOS Banner">
</picture>

###

This document outlines the guidelines for contributing to this repository. It consists of two primary sections:  

•&emsp;Repository Contributing Guidelines - Guidelines specific to this repository.  
•&emsp;General Contributing Guidelines - Guidelines for all LimeOS repositories.  

When conflicts arise between these sections, always follow the repository-specific guidelines as they take precedence over general guidelines.

### Table of Contents

**Repository Contributing Guidelines**  
•&emsp;[Building the executable](#building-the-executable)  
•&emsp;[Running the executable](#running-the-executable)  

**General Contributing Guidelines**  
•&emsp;[Git Workflow](#git-workflow)  
•&emsp;[Versioning](#versioning)  
•&emsp;[File Structure](#file-structure)  
•&emsp;[Naming Convention](#naming-convention)  
&emsp;•&emsp;[Branch Naming](#branch-naming)  
&emsp;•&emsp;[Commit Messages](#commit-messages)  
&emsp;•&emsp;[Function Naming](#function-naming)  
&emsp;•&emsp;[Variable Naming](#variable-naming)  
&emsp;•&emsp;[Parameter Naming](#parameter-naming)  
&emsp;•&emsp;[Type Naming](#type-naming)  
&emsp;•&emsp;[Macro Naming](#macro-naming)  
&emsp;•&emsp;[File Naming](#file-naming)  
&emsp;•&emsp;[Binary Naming](#binary-naming)  
•&emsp;[Declaration Order](#declaration-order)  
•&emsp;[Documentation](#documentation)  
&emsp;•&emsp;[Header File](#header-file-h)  
&emsp;•&emsp;[Source File](#source-file-c)  
&emsp;•&emsp;[Doxygen Format](#doxygen-format)  

## Repository Contributing Guidelines

### Building the executable

To build this project locally, you will need the following dependencies:

```bash
# The following command is intended for Debian based systems.
sudo apt install \
   gcc \
   make \
   libx11-dev \
   libxi-dev \
   libcairo2-dev
```

Once the dependencies are installed, you can build the project by running:

```bash
make
```

This will compile the source code and generate an executable in the `./bin`
directory.

### Running the executable

You can either run the executable directly in order to use it as your primary window manager (Using `startx` or a display manager), or you can use a nested X server like `Xephyr` in order to run the window manager within your currently active window manager.

> **NOTE:** Testing this window manager within a Wayland compositor may cause
conflicts. We recommend using an X11-based window manager as your parent
environment to prevent unexpected behavior.

```bash
Xephyr -br -ac -noreset -screen 800x600 :1
DISPLAY=:1 ./bin/lime-os-window-manager
```

Then if you'd like, you could start an application on the new display as follows:

```bash
# xterm is being used as an example here, replace it with whatever you'd like.
DISPLAY=:1 xterm &
```

## General Contributing Guidelines

> **NOTE:** These guidelines are replicated across all LimeOS repositories. Any changes must be applied to the `CONTRIBUTING.md` files across all repositories to maintain consistency.

### Git Workflow

This repository uses two main branches:  
&emsp;•&emsp;`main` - Stable production code, must not be pushed to directly.  
&emsp;•&emsp;`develop` - Development code, must not be pushed to directly.  

In order to contribute, you must follow these steps:  
&emsp;1\. Fork the repository.  
&emsp;2\. Create a branch from `develop`, following the [branch naming conventions](#branch-naming):
```bash
git checkout develop
git checkout -b add-spectacular-feature
```  
&emsp;3\. Commit your changes, following the [commit message convention](#commit-messages).  
&emsp;4\. Push the changes to your fork.  
&emsp;5\. Submit a pull request targeting the `develop` branch.  

The changes will be reviewed by the project maintainers and contributors, after which it will be merged into the `develop` branch if approved.

When sufficient changes accumulate in `develop`, the branch will be synchronised with the `main` branch by the project maintainers, at which point, a new release is also created.

### Versioning

This repository adheres to Semantic Versioning (Semver), which uses a three-part version number in the following format:  
&emsp;•&emsp;`MAJOR` - Incremented for incompatible API changes.  
&emsp;•&emsp;`MINOR` - Incremented for backwards-compatible new features.  
&emsp;•&emsp;`PATCH` - Incremented for backwards-compatible bug fixes.  

Examples:  
&emsp;•&emsp;`1.0.0` - Initial stable release.  
&emsp;•&emsp;`1.1.0` - Added new features.  
&emsp;•&emsp;`1.1.1` - Added bug fixes.  
&emsp;•&emsp;`2.0.0` - Introduced breaking changes.  

A more in-depth guide on Semver can be found [here](https://semver.org/).

### File Structure

This repository must follow these structural guidelines:

1. **Minimal Root Directory**  
The root directory must only contain critical repository files such as build configurations, documentation, and source directory. All other files should be organized within appropriate subdirectories.

2. **Source Organization**  
•&emsp;All code must reside within the `src` directory.  
•&emsp;Organize code into subdirectories by module/feature.  
•&emsp;Source files (.c) and header files (.h) must be paired and share the same name within the same directory, except for `main.c`.

### Naming Convention

#### Branch Naming  

All Git branches in this repository must adhere to the _dash-case_ naming convention, with the exception of version numbers, which are delimited with dots. Consider these guidelines when naming a Git branch:  

1. **Action Prefix**  
   All branch names (excluding `main` and `develop`) must start with one of the following action prefixes:  
   •&emsp; `add` - Primarily adds new code, docs, files, or configurations.  
   •&emsp; `update` - Primarily modifies code, docs, files, or configurations.  
   •&emsp; `remove` - Primarily removes code, docs, files, or configurations.    
   •&emsp; `release` - Prepares codebase for a version release.  
   •&emsp; `fix` - Resolves bugs or issues.  

   Examples:  
   **✓**  `add-branch-naming-guidelines`  
   **✓**  `update-auth-tests-code-quality`  
   **✓**  `remove-gtk-dependency`  
   **✓**  `fix-slow-authentication`  
   **✓**  `release-1.0.0`  

#### Commit Messages

All commit messages in this repository must follow a specific format. Consider these guidelines when writing a commit message:

1. **Action Prefix**  
   All messages must start with one of these action words:  
   •&emsp;`Add` - When adding new code, docs, files, or configurations.  
   •&emsp;`Update` - When modifying code, docs, files, or configurations.  
   •&emsp;`Remove` - When removing code, docs, files, or configurations.  
   •&emsp;`Fix` - When resolving bugs or issues.  

2. **Message Content**  
   •&emsp;Keep messages concise but descriptive.  
   •&emsp;Focus on what changes do, not how they do it.  
   •&emsp;Do not end the message with a dot.  
   •&emsp;Write in present tense.  

   Examples:  
   **✓**  `Add user authentication module`  
   **✓**  `Update memory allocation efficiency`  
   **✓**  `Remove deprecated config parser`  
   **✓**  `Fix memory leak during window creation`  

#### Function Naming

All function names in this repository must adhere to the _snake_case_ naming convention. Consider these guidelines when naming a function:

1. **Descriptive Names**  
   •&emsp;Function names must be descriptive and clearly indicate their purpose.  
   •&emsp;Avoid unnecessary abbreviations.  
   
   Examples:  
   **✓**  `initialize_config()`  
   **𐄂**  `init()`  

2. **Verb-Noun Format**  
   Use a verb-noun structure to convey action and intent.  
   
   Examples:  
   **✓**  `parse_file()`  
   **✓**  `write_to_buffer()`  
   **𐄂**  `file_parser()`  

3. **Module Identification**  
   Functions that belong to a specific module, must include the module name within their own name to indicate their association and prevent naming conflicts in the global scope.  

   In the example below, it is assumed that we are declaring the function within a hypothetical `config` module.
   
   Examples:  
   **✓**  `parse_config_file()`  
   **𐄂**  `parse_file()`  

#### Variable Naming  

All variable names in this repository must adhere to the _snake_case_ naming convention. Consider these guidelines when naming a variable:  

1. **Descriptive Names**  
   •&emsp;Variable names must clearly indicate their purpose.  
   •&emsp;Avoid single-character names, except for loop counters.  
   •&emsp;Avoid abbreviations unless they are standard (e.g. `id` for identifier).  

   Examples:  
   **✓**  `file_descriptor`  
   **𐄂**  `fd`  

#### Parameter Naming  

All parameter names in this repository must adhere to the _snake_case_ naming convention. Consider these guidelines when naming a parameter:  

1. **Descriptive Names**  
   •&emsp;Parameter names must clearly indicate their purpose.  
   •&emsp;Always avoid single-character names.  
   •&emsp;Avoid abbreviations unless they are standard (e.g. `id` for identifier).  

   Examples:  
   **✓**  `file_descriptor`  
   **𐄂**  `fd`  

2. **Output Parameters**  
   Prefix pointer parameters with `out_` when they are used to return values from a function.
   
   Examples:  
   **✓**  `void get_name(char *out_name, int name_size)`  
   **𐄂**  `void get_name(char *name, int name_size)`  

#### Type Naming

All type names in this repository must adhere to the _PascalCase_ naming convention. Consider these guidelines when naming a type:

1. **Descriptive Names**  
   •&emsp;Type names must clearly describe the data they represent.  
   •&emsp;Use nouns or noun phrases for type names.  
   
   Examples:  
   **✓**  `UserData`  
   **𐄂**  `Data`  

2. **Module Identification**  
   Types that belong to a specific module, must include the module name within their own name to indicate their association and prevent naming conflicts in the global scope. 

   In the example below, it is assumed that we are declaring the type within a hypothetical `window` module.
   
   Examples:  
   **✓**  `WindowButtonType`  
   **𐄂**  `ButtonType`  

#### Macro Naming

All macro names in this repository must adhere to the _snake_case_ naming convention, and be written in all uppercase letters. Consider these guidelines when naming a macro:

1. **Descriptive Names**  
   •&emsp;Macro names must clearly indicate their purpose.  
   •&emsp;Avoid unnecessary abbreviations.  
   •&emsp;Add unit suffixes where applicable (e.g. `_MS`, `_PERCENT`, `_BYTES`).  
   
   Examples:  
   **✓**  `MAX_BUFFER_SIZE`  
   **✓**  `NETWORK_TIMEOUT_MS`  
   **𐄂**  `MAX_BFR_SIZE`  
   **𐄂**  `NET_TIMEOUT`  

2. **Module Identification**  
   Macros that belong to a specific module, must include the module name within their own name to indicate their association and prevent naming conflicts in the global scope. 

   In the example below, it is assumed that we are declaring the macro within a hypothetical `network` module.
   
   Examples:  
   **✓**  `NETWORK_RETRY_COUNT`  
   **𐄂**  `RETRY_COUNT`  

#### File Naming  

All file names in this repository must adhere to the _snake_case_ naming convention. Consider these guidelines when naming a file:  

1. **Concise and Contextual Names**  
   •&emsp;File names should be concise while maintaining clarity about their purpose.  
   •&emsp;Aim for 1-2 words per file name, letting the path provide additional context.  
   •&emsp;Avoid abbreviations unless they are standard (e.g `auth` for authentication).

   Examples:  
   **✓**  `user/auth.c`  
   **✓**  `user/auth/tokens.c`  
   **𐄂**  `user/authentication_tokens.c`

#### Binary Naming  

All binary files built in this repository must adhere to the _dash-case_ naming convention. Consider these guidelines when naming a binary file:  

1. **LimeOS Prefix**  
   All binary files must start with the `lime-os` prefix.

   Examples:  
   **✓**  `lime-os-window-manager`    
   **𐄂**  `lime-window-manager`

2. **Avoid Abbreviations**  
   Binary file names must use complete words rather than shortened forms to maintain clarity and prevent naming conflicts.

   Examples:  
   **✓**  `lime-os-window-manager`    
   **𐄂**  `lime-os-wm`  

3. **Suffixes**  
   When building binary files for internal libraries or LimeOS extensions, append the `lib` or `ext` suffix respectively.

   Examples:  
   **✓**  `lime-os-config-lib`    
   **✓**  `lime-os-settings-ext`    
   **𐄂**  `lime-os-config`  
   **𐄂**  `lime-os-settings`  

### Declaration Order

All header (.h) and source (.c) files must follow this specific declaration order:

1. **Includes**  
2. **Macros**  
3. **Types**  
4. **Global variables**  
5. **Functions**  

### Documentation

#### Header File (.h)  
•&emsp;Every declaration within the header file must be documented.  
•&emsp;Functions require [Doxygen Format](#doxygen-format) documentation.   
•&emsp;Other elements require a brief inline comment (1-3 lines).  
•&emsp;Keep comment line length under 80 characters (including whitespace).  
•&emsp;Write clean comments with proper punctuation and end them with a period.

Example:
```c
// Maximum number of concurrent users.
#define MAX_USERS 1000

// Stores user information and account metadata.
typedef struct {
    int id;
    char* username;
    time_t last_login;
    bool logged_in;
} UserData;

/** 
 * Validates user credentials.
 *
 * @param username The users username.
 * @param password The users password.
 *
 * @return - `0` The user credentials are valid.
 * @return - `-1` The user credentials are invalid.
 */
int validate_user(const char* username, const char* password);
```
#### Source File (.c)  
•&emsp;Don't document any declarations.  
•&emsp;Add inline comments within functions to break down complex logic into clear steps.  
•&emsp;Keep comment line length under 80 characters (including whitespace).  
•&emsp;Write clean comments with proper punctuation and end them with a period.  
•&emsp;Optionally, add a multi-line comment at the top of the file, directly after the includes, starting with "This code is responsible for" to provide critical context.  

Example:
```c
#include <stdout.h>

/**
 * This code is responsible for user authentication and session management.
 * Note that sessions timestamps use local time instead of UTC, causing
 * potential Daylight Saving Time issues.
 */

int process_user_login(UserData* user)
{
    // Verify that the user struct is valid.
    if (!validate_user_struct(user)) {
        return ERROR_INVALID_USER;
    }

    // Check if the user exists in database.
    user_record_t* record = find_user_record(user->id);

    // Update last login timestamp.
    record->last_login = get_current_time();

    return 0;
}

// Other functions...
```

#### Doxygen Format

Function documentation should follow the Doxygen-style format - a standardized way to document function signatures, parameters, return values, and behavior. It is best to stick to only using the tags below as they are widely supported. Follow these guidelines when documenting functions:

1. **`@brief` tag**  
   •&emsp;Keep it short, add additional information using the `@note` tag.  
   •&emsp;Keep implementation details out - the implementation should be self-documenting.  

2. **`@param` tag**  
   •&emsp;Document each parameter, even if seemingly obvious.  
   •&emsp;Describe valid ranges or expected formats.  
   •&emsp;Indicate if a parameter can be `NULL`.

3. **`@return` tag**  
   •&emsp;For multiple return values:  
   &emsp;•&emsp;Use separate `@return` tags.  
   &emsp;•&emsp;List each value with bullet points (`-`).  
   &emsp;•&emsp;Follow the bullet point (`-`) with the return value in backticks (e.g. `` `-1` ``).  
   &emsp;•&emsp;Follow the return value in backticks with a description.  
   •&emsp;For single return values:  
   &emsp;•&emsp;Use one `@return` tag with a simple description.

4. **`@note` tag**  
   •&emsp;Place crucial information that doesn't fit in `@brief` here.  
   •&emsp;Place any links to external documentation here.  
   •&emsp;For multiple notes:  
   &emsp;•&emsp;Use separate `@note` tags.  
   &emsp;•&emsp;List each note with bullet points (`-`).  
   &emsp;•&emsp;Follow the bullet point (`-`) with a description.  
   •&emsp;For single notes:  
   &emsp;•&emsp;Use one `@note` tag with a simple description.

5. **`@warning` tag**  
   •&emsp;For multiple warnings:  
   &emsp;•&emsp;Use separate `@warning` tags.  
   &emsp;•&emsp;List each warning with bullet points (`-`).  
   &emsp;•&emsp;Follow the bullet point (`-`) with a description.  
   •&emsp;For single notes:  
   &emsp;•&emsp;Use one `@warning` tag with a simple description.

6. **`@deprecated` tag**  
   •&emsp;Mark functions that should no longer be used.  
   •&emsp;Provide the reason for deprecation.  
   •&emsp;Reference the recommended alternative function.

Example:
```c
/**
 * @brief Validates and processes user authentication token.
 *
 * @param token Authentication token to validate.
 * @param options Configuration options struct.
 * @param timeout_ms Timeout in milliseconds, set to 0 for default.
 *
 * @return - `0` Authentication successful.
 * @return - `-1` Invalid token format.
 * @return - `-2` Token expired.
 *
 * @note - Token validation uses SHA-256 hashing.
 * @note - Cached results expire after 24 hours.
 *
 * @warning Requires minimum OpenSSL version 1.1.0.
 */
int authenticate_user(const char* token, auth_options_t* options, uint32_t timeout_ms);
```
