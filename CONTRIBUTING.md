<picture>
  <source media="(prefers-color-scheme: dark)" srcset=".github/contributing_banner_white.png">
  <source media="(prefers-color-scheme: light)" srcset=".github/contributing_banner_black.png">
  <img alt="LimeOS Banner">
</picture>

######

This document outlines the guidelines for contributing to this repository, including best practices for code contributions and other relevant procedures. It is primarily divided into three sections: the repository guidelines, the language guidelines and the general guidelines. In the event of any conflicts between these sections, the section listed first will take precedence.

### Table of Contents

#### Repository Contributing Guidelines  

 - [Building the window manager](#building-the-window-manager)  
 - [Running the window manager](#running-the-window-manager)  

#### Language Contributing Guidelines  

 - [Writing documentation](#writing-documentation)    
 - [Naming files and code elements](#naming-files-and-code-elements)  
 - [Ordering code declarations](#ordering-code-declarations)  
 - [Structuring files](#structuring-files)  

#### General Contributing Guidelines  

 - [Understanding the Git Workflow](#understanding-the-git-workflow)  
 - [Determining version numbers](#determining-version-numbers)  

## Repository Contributing Guidelines

### Building the window manager

Building the window manager locally is a straightforward process. To get started, you simply need to install the required dependencies and execute the `make` command.

For Debian-based Linux distributions, you can install the necessary dependencies using the following command:

```bash
sudo apt install \
   gcc \
   make \
   libx11-dev \
   libxi-dev \
   libcairo2-dev
```

If you're not using a Debian-based Linux distribution, the package names may differ. In that case, you must consult the package repositories for your specific distribution to identify the appropriate names.

After the dependencies are installed, and assuming you are in the root directory of the repository, you can build the project by running:

```bash
make
```

This will compile the source code and generate the window manager executable in the `./bin` directory.

### Running the window manager

When running the window manager, you have three options: you can launch the window manager executable using the `startx` command, a display manager, or you can use a nested X server like Xephyr to run the window manager within your currently active window manager.

For development purposes, it is advisable to use the nested X server approach. However, be aware that running an X11 window manager within a Wayland environment may cause conflicts. To avoid unexpected behavior, it is recommended to use an X11-based window manager as your parent environment.

<details>
   <summary>&ensp;<b>Launching the window manager using startx.</b></summary>

   &nbsp;  
   The `startx` command-line utility is part of the X11 toolchain, and its purpose is to initiate an X server and launch a window manager within it simultaneously.

   This command cannot be executed from an already running graphical session. You must first switch to a TTY (non-graphical terminal) session. You can do this by pressing `Ctrl + Alt + F1` through `F6`, depending on your system configuration.

   Once in the TTY, simply run the following command:

   ```bash
   startx /path/to/limeos-window-manager
   ```
</details>

<details>
   <summary>&ensp;<b>Launching the window manager using a display manager.</b></summary>

   &nbsp;  
   A display manager is a graphical interface that manages user sessions and provides a login screen for authentication, after which it launches a window manager for the user’s session.

   First, you must identify the display manager being used on your system, and whether it supports launching custom window managers. The following command checks if you have a common display manager process running:

   ```bash
   ps -e | grep -E 'sddm|gdm|kdm|mdm|xdm|lightdm|lxdm'
   ```

   If the **output is empty**, it may suggest that you are using an uncommon display manager. In this case, you can still proceed, but you will need to check whether your display manager supports launching a custom window manager yourself. If the **output is not empty**, you can consult the table below:

   | Process Name     | Display Manager                | Supports X11? | Supports Custom WM? |
   |------------------|--------------------------------|---------------|---------------------|
   | `sddm`           | Simple Desktop Display Manager | **✓**         | **✓**               |
   | `gdm`            | GNOME Display Manager          | **✓**         | **✓**               |
   | `kdm`            | KDE Display Manager            | **✓**         | **𐄂**               |
   | `mdm`            | MDM Display Manager            | **✓**         | **✓**               |
   | `xdm`            | X Display Manager              | **✓**         | **✓**               |
   | `lightdm`        | LightDM                        | **✓**         | **✓**               |
   | `lxdm`           | LXDM                           | **✓**         | **✓**               |

   Next, you must create a session file. Session files are used by display managers to identify the available window managers on the system. Create a session file in `/usr/share/xsessions/` named `limeos-window-manager.desktop` with the following contents:

   ```ini
   [Desktop Entry]
   Name=LimeOS Window Manager             # Display name
   Exec=/path/to/limeos-window-manager    # Path to executable
   Type=Application                       # Desktop entry type
   ```

   Ensure that the session file is executable. You can do this with the following command:

   ```bash
   chmod +x /usr/share/xsessions/limeos-window-manager.desktop
   ```

   From now on, the login screen provided by your display manager should have an option to launch the custom window manager.
</details>

<details>
   <summary>&ensp;<b>Running the window manager inside of Xephyr.</b></summary>

   &nbsp;  
   Xephyr is a nested X server that allows you to run an X session within an existing X session.

   If you haven't already installed Xephyr, you can do so using your package manager. For example, on Debian-based systems, you can run:

   ```bash
   sudo apt install xserver-xephyr
   ```

   If you're not using a Debian-based Linux distribution, the package name may differ. In that case, you must consult the package repositories for your specific distribution to identify the appropriate name.

   Open a terminal and start Xephyr with a specified display size. For example, to create a display with a `1280x800` resolution, you can use the following command:

   ```bash
   Xephyr -br -ac -noreset -screen 1280x800 :1
   ```

   The `-br` flag sets the background color to black, the `-ac` flag disables Access Control (simplifying development), the `-noreset` flag prevents the server from closing when all clients disconnect, and the `:1` indicates the display number, which you can choose as needed.

   Now you can start the window manager within the Xephyr session by running:

   ```bash
   DISPLAY=:1 /path/to/limeos-window-manager
   ```
</details>

## Language Contributing Guidelines

**Important:** These guidelines are replicated across all LimeOS repositories that primarely use the C programming language. Any changes made here, must also be applied to the `CONTRIBUTING.md` files across similar repositories to maintain consistency.

### Writing documentation

Clear, comprehensive documentation is a cornerstone principle across all LimeOS codebases. It reduces onboarding time for new contributors, prevents bugs, and ensures the long-term sustainability of the project.

When writing documentation, ensure that comment lines do not exceed 80 characters in length, including whitespace. This improves readability across different viewing environments. All comments should be written with proper punctuation and grammar, where sentences end with a period.

The following guidelines outline how to properly document code for both header (.h) and source (.c) files.

<details>
   <summary>&ensp;<b>Writing documentation within header (.h) files.</b></summary>

   &nbsp;  
   Every declaration within the header file must be documented using the Doxygen format, which allows for automatic documentation generation and standardized presentation.

   Below are some examples demonstrating the Doxygen format:
   
   ```c
   /**
    * Represents a point in a 2D coordinate system.
    */
   typedef struct {
      int x;
      int y;
   } Vector2;

   /** 
   * Computes the linear distance between two Vector2 points
   * in 2D space.
   *
   * @param p1 The first point.
   * @param p2 The second point.
   *
   * @return The calculated distance as a floating-point number.
   */
   float calculate_distance(int p1, int p2);
   ```

   Note how the documentation block uses the Doxygen-compatible `/** */` style rather than regular comments and how each section (description, parameters, returns) is separated by a single blank line.

   For functions with a limited number of discrete return values (e.g., error codes), use the dash notation (`-`) within the `@return` tag to list each possible value and its meaning:

   ```c
   /**
   * @return - `0` Indicates successful execution.
   * @return - `-1` Indicates a general failure.
   * @return - `-2` Indicates a specific error condition (e.g., invalid input).
   */
   ```

   Please stick to the `@param`, `@return`, `@note`, and `@warning` tags exclusively, as these tags are guaranteed to be widely supported.
</details>

<details>
   <summary>&ensp;<b>Writing documentation within source (.c) files.</b></summary>

   &nbsp;  
   Documentation within source files must purely focus on documenting the implementation details with inline comments that explain the logic and algorithmic steps within functions:

   ```c
   int update_item_value(Item* item, int new_value)
   {
      // Ensure the item pointer is not NULL.
      if (item == NULL) {
         return ERROR_NULL_POINTER;
      }

      // Validate the new value against allowed range.
      if (new_value < MIN_VALUE || new_value > MAX_VALUE) {
         return ERROR_VALUE_OUT_OF_RANGE;
      }

      // Assign the new value to the item.
      item->value = new_value;

      return SUCCESS;
   }
   ```

   For complex source files, consider adding a multi-line comment at the top of the file, directly after the includes, that begins with "This code is responsible for" to provide critical context:

   ```c
   #include <stdout.h>

   /**
   * This code is responsible for user authentication and session management.
   * Note that sessions timestamps use local time instead of UTC, causing
   * potential Daylight Saving Time issues.
   */
   ```
</details>

### Naming files and code elements

Consistent naming conventions are crucial for maintaining code readability. Adhering to these standards ensures that code elements and files are easily identifiable and understandable.

Unlike many modern programming languages, C lacks an official style guide or standard convention. Instead, different projects and organizations have developed their own sets of conventions over time. In our codebase, we've adopted naming practices that align closely with traditional C programming patterns found in established projects like the Linux kernel and GNU software.

Below are the specific file and code element naming conventions to follow within this project:

#### Function Names

 - Follow `snake_case()` convention.  
 - Typically follow a verb-noun structure (e.g., `write_to_buffer()`).  
 - Functions inside of a module must incorporate that module's name.  
 - Incorporate the name of the module they're in (e.g., `read_config()` in `config` module).

#### Variable Names

 - Follow `snake_case` convention.
 - Clearly indicate the variable's contents or purpose.
 - Avoid abbreviations, except for standard ones (e.g., `id` for identifier).

#### Parameter Names

 - Follow `snake_case` convention.
 - Clearly indicate the parameter's contents or purpose.
 - Avoid abbreviations, except for standard ones (e.g., `id` for identifier).
 - Are prefixed with `out_` if its values are to be modified by the function.

#### Type Names

 - Follow `PascalCase` convention.
 - Use descriptive nouns or noun phrases that represent the data structure (e.g., `UserData`).
 - Incorporate the name of the module they're in (e.g., `ImageProperties` in `image` module).

#### Macro Names

 - Follow `SNAKE_CASE` convention (all uppercase).
 - Clearly indicate the macro's contents or purpose.
 - Add unit suffixes where applicable for clarity (e.g., `_MS`, `_PERCENT`, `_BYTES`).
 - Incorporate the name of the module they're in (e.g., `NETWORK_RETRY_COUNT` in `network` module).

#### File Names

 - Follow `snake_case` convention.
 - Use concise names, typically 1-2 words.
 - Avoid abbreviations unless widely understood (e.g., `auth`).
 - Let directory structure provide context (e.g., `user/auth/tokens.c`).

#### Binary Names

 - Follow `dash-case` convention.
 - Always start with the `limeos-` prefix (e.g., `limeos-window-manager`).
 - Avoid abbreviations entirely (Excluding the suffix below).
 - Append `-lib` suffix for libraries (e.g., `limeos-config-lib`).

### Ordering code declarations

Maintaining a consistent order of declarations within source and header files helps contributors quickly locate specific elements within a file, reducing the cognitive load associated with navigating unfamiliar code.

Code elements should be organized in the following order:

1. **Includes**  
2. **Macros**  
3. **Types**  
4. **Global variables**  
5. **Functions**  

### Structuring files

A well-organized repository structure enables contributors to quickly locate and understand code components. The following structural guidelines must be followed:

#### Minimal Root Directory

Limit the root directory to essential files only:  
 - Build configurations (`Makefile`)  
 - Documentation (`README.md`, `CONTRIBUTING.md`)  
 - Important directories (`src`, `bin`, `obj`)  
 - License information  

#### Source Organization

 - All source code must reside within the `src` directory  
 - Organize code into logical subdirectories by module/feature  

#### File Pairing  

 - Source files (.c) and header files (.h) must share the same name and directory  
 - Exception: `main.c` doesn't require a header file  

```
src/
├── module/
│   ├── feature.c
│   └── feature.h
```

## General Contributing Guidelines

**Important:** These guidelines are replicated across all LimeOS repositories. Any changes made here, must also be applied to the `CONTRIBUTING.md` files across all other repositories to maintain consistency.

### Understanding the Git Workflow

This repository uses two main branches:  

 - `main` - Stable release code, must not be pushed to directly.  
 - `develop` - Development code, must not be pushed to directly.  

In order to contribute, you must follow these steps:  

1. Fork the repository and clone it onto your system.  
2. Create a branch from `develop` prefixed with `feature-`:
```bash
git checkout develop
git checkout -b feature-audio-support
```  
3. Commit your changes with an informative message.
4. Push the changes to your fork.  
5. Submit a pull request targeting the `develop` branch.  

The changes will be reviewed by the project maintainers and contributors, after which it will be _merged or squashed_ into the `develop` branch if approved.

When sufficient changes accumulate in `develop`, the branch will be _rebased_ onto `main` by the project maintainers, at which point, a new release tag is also created.

### Determining version numbers

This repository adheres to Semantic Versioning (Semver), which uses a three-part version number in the following format:  

 - `MAJOR` - Incremented for incompatible API changes.  
 - `MINOR` - Incremented for backwards-compatible new features.  
 - `PATCH` - Incremented for backwards-compatible bug fixes.  

Examples:  

 - `1.0.0` - Initial stable release.  
 - `1.1.0` - Added new features.  
 - `1.1.1` - Added bug fixes.  
 - `2.0.0` - Introduced breaking changes.  

A more in-depth guide on Semver can be found [here](https://semver.org/).
