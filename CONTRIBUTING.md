<picture>
  <!-- The media queries determine the image based on website theme -->
  <source media="(prefers-color-scheme: dark)" srcset=".assets/contributing_banner/dark_mode.png">
  <source media="(prefers-color-scheme: light)" srcset=".assets/contributing_banner/light_mode.png">
  <!-- Fallback to light mode variant if no match -->
  <img alt="LimeOS Banner" src=".assets/contributing_banner/light_mode.png">
</picture>

######

This document outlines the guidelines for contributing to this repository,
including best practices for code contributions and other relevant procedures.
It is divided into multiple sections, which you can explore in the table of
contents below. In the event of any conflicts between these sections, the
section listed first will take precedence.

### Table of Contents

**Repository Contributing Guidelines**

- [Building the window manager](#building-the-window-manager)
- [Running the window manager](#running-the-window-manager)

**General Contributing Guidelines**

- [Understanding the Git workflow](#understanding-the-git-workflow)
- [Determining version numbers](#determining-version-numbers)
- [Writing commit messages](#writing-commit-messages)

**C Language Contributing Guidelines**

- [Writing documentation](#writing-documentation)  
  &nbsp; • &nbsp; [Writing directive comments](#writing-directive-comments)  
  &nbsp; • &nbsp; [Writing indicative comments](#writing-indicative-comments)
- [Naming code elements and files](#naming-code-elements-and-files)  
  &nbsp; • &nbsp; [Naming functions](#naming-functions)  
  &nbsp; • &nbsp; [Naming variables](#naming-variables)  
  &nbsp; • &nbsp; [Naming parameters](#naming-parameters)  
  &nbsp; • &nbsp; [Naming types](#naming-types)  
  &nbsp; • &nbsp; [Naming macros](#naming-macros)  
  &nbsp; • &nbsp; [Naming files](#naming-files)  
  &nbsp; • &nbsp; [Naming binaries](#naming-binaries)
- [Ordering code declarations](#ordering-code-declarations)
- [Structuring files](#structuring-files)
- [Formatting code](#formatting-code)

**Document Contributing Guidelines**

- [Storing document assets](#storing-document-assets)
- [Modifying CONTRIBUTING.md](#modifying-contributingmd)
- [Modifying README.md](#modifying-readmemd)

&nbsp;

## Repository Contributing Guidelines

### Building the window manager

This subsection explains how to build the window manager from source.

First, install the required dependencies. For Debian-based Linux distributions,
run:

```bash
sudo apt install \
   gcc \
   make \
   libx11-dev \
   libxi-dev \
   libxfixes-dev \
   libxrandr-dev \
   libxcomposite-dev \
   libcairo2-dev
```

If you're not using a Debian-based distribution, package names may differ.
In that case, consult your distribution's package repositories
(package listings) to find the correct package names for your system.

Then, from the root directory of the repository, build the project by running:

```bash
make
```

Finally, verify the build succeeded by checking that the executable exists at
`./bin/limeos-window-manager`.

### Running the window manager

This subsection documents how to run the window manager after building it.
Multiple launch methods exist, each suited to different use cases.

When running the window manager, you have three options: you can launch the
window manager executable using the `startx` command, a display manager, or you
can use a nested X server like Xephyr to run the window manager within your
currently active window manager.

For development purposes, it is advisable to use the nested X server approach.
However, be aware that running an X11 window manager within a Wayland
environment may cause conflicts. To avoid unexpected behavior, it is
recommended to use an X11-based window manager as your parent environment.

<details>
   <summary>&ensp;<b>Launching the window manager using startx.</b></summary>

&nbsp;  
The `startx` command-line utility is part of the X11 toolchain, and its purpose
is to initiate an X server and launch a window manager within it
simultaneously.

This command cannot be executed from an already running graphical session. You
must first switch to a TTY (non-graphical terminal) session. You can do this by
pressing `Ctrl + Alt + F1` through `F6`, depending on your system configuration.

Once in the TTY, simply run the following command:

```bash
startx /path/to/limeos-window-manager
```

</details>

<details>
   <summary>&ensp;<b>Launching the window manager using a display manager.</b></summary>

&nbsp;  
A display manager is a graphical interface that manages user sessions and
provides a login screen for authentication, after which it launches a window
manager for the user’s session.

First, you must identify the display manager being used on your system, and
whether it supports launching custom window managers. The following command
checks if you have a common display manager process running:

```bash
ps -e | grep -E 'sddm|gdm|kdm|mdm|xdm|lightdm|lxdm'
```

If the **output is empty**, it may suggest that you are using an uncommon
display manager. In this case, you can still proceed, but you will need to
check whether your display manager supports launching a custom window manager
yourself. If the **output is not empty**, you can consult the table below:

| Process Name | Display Manager                | Supports X11? | Supports Custom WM? |
| ------------ | ------------------------------ | ------------- | ------------------- |
| `sddm`       | Simple Desktop Display Manager | **✓**         | **✓**               |
| `gdm`        | GNOME Display Manager          | **✓**         | **✓**               |
| `kdm`        | KDE Display Manager            | **✓**         | **𐄂**               |
| `mdm`        | MDM Display Manager            | **✓**         | **✓**               |
| `xdm`        | X Display Manager              | **✓**         | **✓**               |
| `lightdm`    | LightDM                        | **✓**         | **✓**               |
| `lxdm`       | LXDM                           | **✓**         | **✓**               |

Next, you must create a session file. Session files are used by display managers
to identify the available window managers on the system. Create a session file
in `/usr/share/xsessions/` named `limeos-window-manager.desktop` with the
following contents:

```ini
[Desktop Entry]
Name=LimeOS Window Manager             # Display name
Exec=/path/to/limeos-window-manager    # Path to executable
Type=Application                       # Desktop entry type
```

Ensure that the session file is executable. You can do this with the following
command:

```bash
chmod +x /usr/share/xsessions/limeos-window-manager.desktop
```

From now on, the login screen provided by your display manager should have an
option to launch the custom window manager.

</details>

<details>
   <summary>&ensp;<b>Running the window manager inside of Xephyr.</b></summary>

&nbsp;  
Xephyr is a nested X server that allows you to run an X session within an
existing X session.

If you haven't already installed Xephyr, you can do so using your package
manager. For example, on Debian-based systems, you can run:

```bash
sudo apt install xserver-xephyr
```

If you're not using a Debian-based Linux distribution, the package name may
differ. In that case, you must consult the package repositories for your
specific distribution to identify the appropriate name.

Open a terminal and start Xephyr with a specified display size. For example, to
create a display with a `1280x800` resolution, you can use the following
command:

```bash
Xephyr -br -ac -noreset -screen 1280x800 :1
```

The `-br` flag sets the background color to black, the `-ac` flag disables
Access Control (simplifying development), the `-noreset` flag prevents the
server from closing when all clients disconnect, and the `:1` indicates the
display number, which you can choose as needed.

Now you can start the window manager within the Xephyr session by running:

```bash
DISPLAY=:1 /path/to/limeos-window-manager
```

</details>

&nbsp;

## General Contributing Guidelines

### Understanding the Git workflow

This subsection documents the branching strategy and contribution process.
Following this workflow ensures smooth collaboration and maintains code quality
across releases.

This repository uses two main branches:

- `main` - Stable release code, must not be pushed to directly.
- `develop` - Development code, must not be pushed to directly.

**Rules**

1. External contributors **must** fork the repository and create a branch from
   `develop`.

   _**Why?**_ Forks isolate work-in-progress without granting write access to
   the main repository.

   _**Exception!**_ Project members **may** branch directly in the main
   repository, but **must** still follow the branching and review rules below.

2. Branches **must** use one of the following prefixes: `feature/`, `fix/`,
   `docs/`, or `chore/`.

   _**Why?**_ Prefixes categorize work at a glance: `feature/` for new
   functionality, `fix/` for bug fixes, `docs/` for documentation, and `chore/`
   for refactors or build changes.

3. Contributors **must not** push directly to `main` or `develop`.

   _**Why?**_ Protected branches ensure all changes go through review,
   maintaining code quality.

4. Pull requests **must** target the `develop` branch.

   _**Why?**_ Targeting develop allows integration testing before changes reach
   the stable main branch.

**Example**

```bash
git checkout develop
git checkout -b feature/audio-support  # or fix/, docs/, chore/
# Make changes and commit
git push origin feature/audio-support
# Submit pull request targeting develop
```

Changes will be reviewed by project maintainers. Approved changes are merged or
squashed into `develop`. When sufficient changes accumulate, maintainers merge
`develop` into `main` and create a new release tag on `main` following SemVer
conventions.

### Determining version numbers

This subsection defines the versioning scheme used for releases. Semantic
Versioning provides clear expectations about compatibility between versions.

This repository adheres to Semantic Versioning (SemVer), which uses a three-part
version number:

- `MAJOR` - Incremented for incompatible API changes.
- `MINOR` - Incremented for backwards-compatible new features.
- `PATCH` - Incremented for backwards-compatible bug fixes.

**Example**

- `1.0.0` - Initial stable release.
- `1.1.0` - Added new features.
- `1.1.1` - Added bug fixes.
- `2.0.0` - Introduced breaking changes.

A more in-depth guide on SemVer can be found [here](https://semver.org/).

### Writing commit messages

This subsection defines conventions for writing clear and consistent commit
messages. Well-written commit messages make project history easier to navigate
and simplify debugging with tools like `git bisect`.

**Template**

```
[VERB] [ACTION]
```

Where:

- `[VERB]` - An imperative verb (e.g., "Add", "Fix", "Remove", "Update").
- `[ACTION]` - What the commit accomplishes.

To produce this format, think: "This commit will ..." and complete the sentence.
The completion is your subject line.

**Example**

| Mental test              | Subject line                          |
| ------------------------ | ------------------------------------- |
| "This commit will ..." → | `Add disk selection step to wizard`   |
| "This commit will ..." → | `Fix memory leak in partition parser` |
| "This commit will ..." → | `Remove deprecated locale functions`  |

**Rules**

1. The subject line **must** complete the sentence "This commit will ...".

   _**Why?**_ This naturally produces imperative mood, matching Git's
   auto-generated messages and describing what applying the commit does.

2. The subject line **must not** exceed 72 characters.

   _**Why?**_ Short subjects remain fully visible in `git log --oneline`
   and GitHub's commit list without truncation.

3. The subject line **must not** end with a period.

   _**Why?**_ The subject is a title, not a sentence; omitting the period
   is the standard convention.

4. The body, if included, **should** be separated from the subject by a blank
   line and wrap at 72 characters.

   _**Why?**_ Git tooling treats the first line as the subject; a blank line
   ensures proper parsing, and wrapped lines display correctly without
   horizontal scrolling.

&nbsp;

## C Language Contributing Guidelines

### Writing documentation

This subsection establishes guidelines for documenting code through comments and
Doxygen annotations. Clear documentation reduces onboarding time for new
ontributors, prevents bugs, and ensures the long-term sustainability of the
project.

**Rules**

1. Comment lines **must not** exceed 80 characters in length, including
   whitespace.

   _**Why?**_ Research suggests ~50-75 characters is easiest to read. 80 is a
   practical cap that works well in terminals and side-by-side diffs.

   _**Exception!**_ Comments **may** exceed 80 characters when wrapping would
   harm readability (e.g., URLs, long identifiers, code-like snippets, or
   Doxygen tag lines that become worse when wrapped).

2. Comments using sentence grammar **must** follow natural language conventions
   (e.g., end with a full stop).

   _**Why?**_ Inconsistent punctuation looks sloppy and undermines the
   professionalism of the codebase.

Additionally, these guidelines enforce the usage of **directive** and
**indicative** commenting, which you can read more about below.

#### Writing directive comments

Directive comments are inline annotations written in a _directive_ tone that
describe each logical step within function implementations. They transform code
into a readable narrative by pairing language with code, allowing reviewers and
maintainers to understand the algorithm without parsing every line. This is
stricter than most C projects; expect more time spent on readability than raw
velocity.

**Template**

```c
// [VERB] [ACTION].
```

Where:

- `[VERB]` - Indicates the sentence must start with a verb (e.g., "Ensure",
  "Validate", "Calculate").
- `[ACTION]` - What the step accomplishes.

**Example**

```c
int update_item_value(Item *item, int new_value)
{
    // Ensure the item pointer is not NULL.
    if (item == NULL)
    {
        return -1;
    }

    // Validate the new value against allowed range.
    if (new_value < MIN_VALUE || new_value > MAX_VALUE)
    {
        return -2;
    }

    // Assign the new value to the item.
    item->value = new_value;

    return SUCCESS;
}
```

**Rules**

1. Each logical step **must** have a directive comment above it.

   _**Why?**_ Directive comments let readers skim logic without parsing
   implementation. Remove the code, and the comments alone should convey
   the algorithm.

   _**Exception!**_ The first or last step **may** be omitted if self-evident
   (e.g., a simple return statement).

2. Steps **must** be separated by a blank line; code within a step **must not**
   contain blank lines.

   _**Why?**_ Blank lines separate steps into skimmable chunks; blank lines
   inside a step break the grouping.

3. Comments **must** start with a verb (e.g., "Validate...", "Calculate...",
   "Ensure...").

   _**Why?**_ Verbs describe actions; nouns describe state. Code is action.

4. Comments **must** use `//`, not `/* */` or `/** */`.

   _**Why?**_ Single-line comments signal inline guidance, not API documentation.

5. Comments **should** describe the action, but **may** explain _how_ when the
   implementation is non-obvious.

   _**Why?**_ Usually the code shows how; `// Increment i` above `i++` is noise,
   not signal.

#### Writing indicative comments

Indicative comments document code element declarations (functions, types,
macros, global variables) written in an _indicative_ tone by stating their
identity and purpose. These comments appear at point-of-declaration in header
files and provide intent that signatures alone cannot express.

**Templates**

1\. Identity template, for things that _are_ (types, macros, global variables):

```c
/** A/The [THING] [RELATIONSHIP] [ROLE / PURPOSE]. */
```

Where:

- `A/The` - `"A"` for instances (types), `"The"` for singletons (macros, global
  variables).
- `[THING]` - What it literally is (`"type"`, `"maximum retry count"`,
  `"configuration struct"`).
- `[RELATIONSHIP]` - How it relates to its role (`"representing"`, `"for"`,
  `"defining"`).
- `[ROLE / PURPOSE]` - What it's for.

2\. Action template, for things that _do_ (functions):

```c
/** [VERB] [ACTION]. */
```

Where:

- `[VERB]` - Indicates the sentence must start with a verb (e.g., "Validates",
  "Calculates", "Fetches").
- `[ACTION]` - What it does, including its primary output or effect.

3\. Expanded template, which extends either the identity or action templates:

```c
/**
 * [IDENTITY or ACTION].
 *
 * [PURPOSE].
 *
 * @param [NAME] [DESCRIPTION].
 *
 * @return - `[VALUE]` - [DESCRIPTION].
 *
 * @note [ADDITIONAL CONTEXT].
 */
```

Where:

- `[IDENTITY or ACTION]` - The first line, following either template above.
- `[PURPOSE]` - Additional context explaining why, if not obvious from the
  first line.
- `[NAME] [DESCRIPTION]` - The parameter name, and what the parameter is used
  for.
- `[VALUE] [DESCRIPTION]` - What the function returns, and what that value
  means.
- `[ADDITIONAL CONTEXT]` - Optional. Any caveats or assumptions that don't fit
  elsewhere.

**Example**

```c
/** The maximum number of connection retry attempts. */
#define MAX_RETRIES 3

/** A type representing a point in 2D space. */
typedef struct
{
    int x;
    int y;
} Vector2;

/** Calculates the linear distance between two points. */
float calculate_distance(Vector2 p1, Vector2 p2);

/**
 * Attempts to establish a connection to the specified server.
 *
 * Retries up to MAX_RETRIES times before failing.
 *
 * @param host The server hostname.
 *
 * @return - `0` Indicates successful connection.
 * @return - `-1` Indicates a connection failure.
 * @return - `-2` Indicates an invalid hostname.
 *
 * @note The caller is responsible for closing the connection.
 */
int connect_to_server(const char *host);
```

**Rules**

1. All declarations in header files **must** have an indicative comment stating
   what it is and what role it plays.

   _**Why?**_ Names don't always capture intent. Requiring comments removes
   ambiguity about what should be documented, reducing decision fatigue.

2. The comment **should** explain why.

   _**Why?**_ The "purpose" captures intent code cannot express, but forcing it
   on trivial cases creates noise.

   _**Exception!**_ The explanation **may** be omitted when the role is
   self-evident; trivial declarations **may** use a single-line form.

3. The comment **must not** explain how; reserve that for directive comments in
   the implementation.

   _**Why?**_ Indicative comments are for consumers. Implementation details
   belong where the logic lives.

4. Doxygen tags **must** follow these conventions: use only `@param`, `@return`,
   `@note`, and `@warning`; include `@param` for all parameters; separate tag
   groups with blank lines.

   _**Why?**_ Limiting tags ensures broad tooling support; requiring `@param`
   captures intent that types cannot express; blank lines make the structure
   skimmable.

5. Declaration comments **must** use `/**` and end with `*/` (Doxygen format).

   _**Why?**_ Doxygen comments enable automatic documentation generation and
   appear at point-of-use in editors.

6. Complex source files **should** begin with a file header comment placed above
   any includes. The file header comment **must** use `/** ... */` but
   **must not** include any Doxygen tags and **must** begin with "This
   code is responsible for".

   _**Why?**_ File-level context helps readers understand the module's purpose
   immediately, before any implementation details.

7. Functions with multiple failure modes **should** use distinct negative return
   values for each error type (e.g., `-1`, `-2`, `-3`).

   _**Why?**_ Distinct codes let callers identify specific failures without
   inspecting global state or logs.

8. Functions with multiple return values **should** document each with a
   separate `@return` entry.

   _**Why?**_ Separate entries make all possible outcomes scannable at a glance.

### Naming code elements and files

This subsection defines naming conventions for functions, variables, types,
macros, and files. Unlike many modern programming languages, C lacks an
official style guide. We follow a C naming style broadly reminiscent of
established projects (e.g., Linux kernel and GNU), with additional
project-specific conventions.

#### Naming functions

This subsection defines naming conventions for functions.

1. Function names **must** follow `snake_case()` convention.

   _**Why?**_ Snake case is the traditional C convention, keeping naming
   consistent with standard library functions and established C projects.

2. Function names **should** follow a verb-noun structure
   (e.g., `write_to_buffer()`).

   _**Why?**_ Verb-noun makes intent explicit at the call site, reducing
   cognitive load when scanning code.

3. Public function names **should** include the module name after the verb
   (e.g., `load_config()` in the `config` module).

   _**Why?**_ Putting the module name in the function name prevents collisions
   and makes call sites self-explanatory, while keeping names readable because
   they start with an action.

   _**Exception!**_ Static (private) functions **may** omit the module name.

#### Naming variables

This subsection defines naming conventions for variables.

1. Variable names **must** follow `snake_case` convention.

   _**Why?**_ Consistent casing with functions keeps the codebase visually
   uniform and predictable.

2. Variable names **must** clearly indicate the variable's contents or purpose.

   _**Why?**_ Ambiguous names force readers to trace assignments, breaking
   their reading flow and forcing unnecessary context-switching.

3. Variable names **must not** use abbreviations.

   _**Why?**_ Abbreviations create project-specific vocabulary that readers
   must memorize and reduce discoverability in search.

   _**Exception!**_ Standard abbreviations **may** be used
   (e.g., `id` for identifier).

#### Naming parameters

This subsection defines naming conventions for function parameters.

1. Parameter names **must** follow `snake_case` convention.

   _**Why?**_ Consistent casing with variables keeps function signatures
   visually uniform.

2. Parameter names **must** clearly indicate the parameter's contents or
   purpose.

   _**Why?**_ Vague parameter names force readers to jump to the function
   definition to understand what to pass.

3. Parameter names **must not** use abbreviations.

   _**Why?**_ Parameters are read most often at the call site. Avoiding
   abbreviations keeps intent obvious.

   _**Exception!**_ Standard abbreviations **may** be used
   (e.g., `id` for identifier).

4. Output parameters **must** be prefixed with `out_`.

   _**Why?**_ The prefix signals that the function modifies the parameter,
   preventing accidental misuse.

#### Naming types

This subsection defines naming conventions for types (structs, enums, typedefs).

1. Type names **must** follow `PascalCase` convention.

   _**Why?**_ PascalCase distinguishes types from variables and functions at
   a glance.

2. Type names **should** use descriptive nouns or noun phrases
   (e.g., `UserData`).

   _**Why?**_ Descriptive names reduce cognitive load by making a type's
   contents clear at a glance.

3. Type names **should** incorporate the name of their module (e.g.,
   `ImageProperties` in `image` module).

   _**Why?**_ Module prefixes prevent name collisions across the codebase.

   _**Exception!**_ Private types **may** omit the module prefix since they are
   not exposed externally, though including it is still preferred for
   consistency.

#### Naming macros

This subsection defines naming conventions for preprocessor macros.

1. Macro names **must** follow `SCREAMING_SNAKE_CASE` convention (all uppercase).

   _**Why?**_ Uppercase signals "compile-time constant" or "macro" at a glance,
   distinguishing them from runtime values.

2. Macro names **must** clearly indicate the macro's contents or purpose.

   _**Why?**_ Magic values without context force readers to guess intent.

3. Macro names **should** add unit suffixes where applicable (e.g., `_MS`,
   `_PERCENT`, `_BYTES`).

   _**Why?**_ Unit suffixes prevent unit mismatch bugs and make the value's
   meaning explicit.

4. Macro names **should** incorporate the name of their module (e.g.,
   `NETWORK_RETRY_COUNT` in `network` module).

   _**Why?**_ Module prefixes prevent name collisions and group related
   constants visually.

   _**Exception!**_ Private macros **may** omit the module prefix since they are
   not exposed externally, though including it is still preferred for
   consistency.

#### Naming files

This subsection defines naming conventions for source and header files.

1. File names **must** follow `snake_case` convention.

   _**Why?**_ Snake case matches the function naming convention and avoids
   case-sensitivity issues across file systems.

2. File names **should** use concise names, typically 1-2 words.

   _**Why?**_ Short names scan faster in directory trees and include statements.

3. File names **must not** use abbreviations.

   _**Why?**_ Abbreviations make files harder to find via search and increase
   inconsistent naming.

   _**Exception!**_ Widely understood abbreviations **may** be used
   (e.g., `auth`).

4. File names **should** let directory structure provide context
   (e.g., `user/auth/tokens.c`).

   _**Why?**_ Long filenames with multiple concepts signal that directory
   structure should carry that meaning instead.

#### Naming binaries

This subsection defines naming conventions for compiled binaries and libraries.

1. Binary names **must** follow `dash-case` convention.

   _**Why?**_ Dash case is the standard convention for Unix executables and
   command-line tools.

2. Binary names **must** start with the `limeos-` prefix
   (e.g., `limeos-window-manager`).

   _**Why?**_ The prefix identifies the binary as part of the LimeOS project
   and prevents name collisions with system tools.

3. Binary names **must not** use abbreviations.

   _**Why?**_ Users type binary names directly; full names are more
   discoverable and memorable.

4. Library binaries **must** append the `-lib` suffix
   (e.g., `limeos-config-lib`).

   _**Why?**_ The suffix distinguishes libraries from executables at a glance.

### Ordering code declarations

This subsection defines the order in which code elements should appear within
source and header files. Consistent ordering helps contributors quickly locate
specific elements, reducing cognitive load when navigating unfamiliar code.

**Rules**

1. Code elements **must** be organized in the following order: **Includes**,
   **Macros**, **Types**, **Global variables**, **Functions**.

   _**Why?**_ This order ensures dependencies are defined before use: includes
   bring in external dependencies, macros are available for type definitions,
   types are available for variable declarations, and functions come last after
   all their dependencies are established.

### Structuring files

This subsection defines how to organize files and directories within the
repository. A well-organized structure enables contributors to quickly locate
and understand code components.

**Rules**

1. The root directory **must** contain only essential files: build
   configurations (`Makefile`), documentation (`README.md`, `CONTRIBUTING.md`),
   important directories (`src`, `bin`, `obj`, `tests`, `assets`, `.assets`),
   and license information.

   _**Why?**_ A minimal, consistent root keeps the entry point clean and
   scannable; clutter confuses contributors and is a bad first impression.

2. All source code **must** reside within the `src` directory.

   _**Why?**_ A single source root makes build configuration simpler and keeps
   the repository organized.

3. Source code **should** be organized into logical subdirectories by module or
   feature.

   _**Why?**_ Modular organization groups related code, making navigation
   intuitive.

4. When a `.c` file has a corresponding interface header, they **must** share
   the same base name and directory.

   _**Why?**_ Paired naming makes finding the interface for any implementation
   trivial.

   _**Exception!**_ Unpaired `.c` or `.h` files **may** exist for legitimate
   patterns (e.g., `main.c` without a header, test files, header-only utilities,
   aggregator headers like `all.h`, platform-specific headers, or generated
   headers).

**Example**

```
src/
├── module/
│   ├── feature.c
│   └── feature.h
```

### Formatting code

This subsection defines code formatting conventions for consistency across the
codebase.

**Example**

```c
if (condition)
{
    do_something();
}
else
{
    do_other();
}
```

**Rules**

1. Braces **must** follow Allman style, with opening and closing braces on their
   own lines.

   _**Why?**_ Allman style visually aligns braces, making block boundaries easy
   to scan and reducing errors when modifying code.

2. Indentation **must** use 4 spaces (no tabs).

   _**Why?**_ Spaces render consistently across editors and tools; 4 spaces
   provide clear visual hierarchy without excessive horizontal drift.

&nbsp;

## Document Contributing Guidelines

### Storing document assets

This subsection defines where to store assets used in documentation files like
images, or other media.

**Rules**

1. Document assets **must** be stored in the `.assets` directory at the
   repository root.

   _**Why?**_ Using a neutral, repo-local assets directory keeps docs portable
   across hosting providers and avoids coupling to GitHub-specific conventions
   like `.github`.

2. Assets **should** be organized into subdirectories that group related files
   (e.g., `.assets/banner/`, `.assets/icons/`).

   _**Why?**_ Assets often need light/dark mode variants or other related
   versions; subdirectories keep these variants together rather than scattered
   across a flat structure.

### Modifying CONTRIBUTING.md

This subsection defines rules for authoring and updating `CONTRIBUTING.md`.
Consistent structure keeps the guide scannable and ensures that contributors
can extend it predictably without degrading quality over time.

Subsections are primarily divided into three variants: **Guiding** subsections,
**Convention** subsections and **Informational** subsections.

**Templates**

**Guiding**

```markdown
### [VERB] [TITLE]

> This subsection [RELATIONSHIP] [SECTION] ► [SUBSECTION].

This subsection [ROLE & PURPOSE].

First, [STEP].

Then, [STEP].

Finally, [STEP].
```

Where:

- `[VERB] [TITLE]` - A gerund-led title describing the action (e.g., "Running
  the development environment").
- `[RELATIONSHIP]` - How this subsection relates to another (`"extends"`,
  `"gets extended by"`, `"overrides"`, `"gets overridden by"`); omit the entire
  note line if not applicable.
- `[SECTION] ► [SUBSECTION]` - The target section and subsection being extended
  or overridden.
- `[ROLE & PURPOSE]` - What this subsection does and why it exists.
- `[STEP]` - A discrete action in the workflow.

**Convention**

```markdown
### [VERB] [TITLE]

> This subsection [RELATIONSHIP] [SECTION] ► [SUBSECTION].

This subsection [ROLE & PURPOSE].

**Template**

```[TEMPLATE]```

Where:

- `[PLACEHOLDER]` - [DESCRIPTION].

**Example**

```[EXAMPLE]```

**Rules**

1. [RULE]

   _**Why?**_ [RATIONALE].
```

Where:

- `[VERB] [TITLE]` - A gerund-led title describing the pattern (e.g., "Naming
  React components").
- `[RELATIONSHIP]` - How this subsection relates to another (`"extends"`,
  `"gets extended by"`, `"overrides"`, `"gets overridden by"`); omit the entire
  note line if not applicable.
- `[SECTION] ► [SUBSECTION]` - The target section and subsection being extended
  or overridden.
- `[ROLE & PURPOSE]` - What this subsection does and why it exists.
- `[TEMPLATE]` - A reusable structure contributors should follow.
- `[PLACEHOLDER]` - A variable in the template requiring explanation.
- `[DESCRIPTION]` - What the placeholder means or how to fill it in.
- `[EXAMPLE]` - A concrete demonstration of the template in use.
- `[RULE]` - A rule statement using RFC modality (must, should, may).
- `[RATIONALE]` - The concrete reason the rule exists.

**Informational**

```markdown
### [VERB] [TITLE]

[INFORMATION].
```

Where:

- `[VERB] [TITLE]` - A gerund-led title describing the pattern (e.g.,
  "Understanding the authentication system").
- `[INFORMATION]` - The information, which could range in size from a few
  sentences to a few paragraphs.

**Rules**

1. Rules **must** include exactly one _**Why?**_ paragraph per rule.

   _**Why?**_ The rule states the contract; the why preserves intent so future
   edits don't regress.

2. Rules **may** include an _**Exception!**_ paragraph after the _**Why?**_
   paragraph when legitimate cases exist that should not follow the rule.

   _**Why?**_ Exceptions prevent rules from being overly rigid; placing them
   after the rationale keeps the rule's intent clear while acknowledging
   edge cases.

3. "Why" paragraphs **should** explain the causal link between the rule and its
   benefit, not just the benefit alone.

   _**Why?**_ "Consistent naming makes searches faster" explains causation;
   "improves code quality" just names a vague outcome without showing how the
   rule achieves it.

4. Rules **must** use consistent modality: **must**, **must not**, **should**,
   **may**.

   _**Why?**_ Standard wording makes requirement strength explicit to readers,
   reviewers, and AI.

5. The `extends`/`overrides` note **must** be included when the subsection
   modifies or builds upon another.

   _**Why?**_ Without the note, readers won't know rules conflict or stack;
   explicit precedence prevents misapplication.

6. Convention subsections **must** include **Template**, **Example**, and
   **Rules** sections.

   _**Why?**_ Templates provide structure, examples demonstrate usage, and
   rules codify requirements; together they make conventions actionable.

   _**Exception!**_ **Template** and **Example** sections **may** be omitted
   when the rules are self-evident, as forcing examples on simple rule sets
   adds bulk without clarifying intent.

### Modifying README.md

This subsection defines rules for updating `README.md`. The README acts as a
menu, a short, scannable entry point that directs readers to deeper
documentation rather than duplicating it.

**Rules**

1. The README **must** stay short and act as a navigation hub, directing to docs
   like `CONTRIBUTING.md` or `LICENSE.md`.

   _**Why?**_ Readers land on the README first. A concise overview with clear
   links gets them to the right place faster than a wall of text.

2. Content **should** be moved to dedicated docs when it exceeds a few
   paragraphs.

   _**Why?**_ Long READMEs become hard to maintain and bury the essential
   "what is this / how do I run it" information.
