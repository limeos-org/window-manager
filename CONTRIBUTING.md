<picture>
  <source media="(prefers-color-scheme: dark)" srcset=".github/contributing_banner_white.png">
  <source media="(prefers-color-scheme: light)" srcset=".github/contributing_banner_black.png">
  <img alt="LimeOS Banner">
</picture>

###

This document outlines the guidelines for contributing to this repository. Whether you're adding new features, fixing bugs, or improving the existing codebase, adhering to these standards ensures that the code remains consistent, readable, and maintainable.

> **NOTE:** These guidelines are replicated across all LimeOS repositories. Any changes must be applied to the `CONTRIBUTING.md` files across all repositories to maintain consistency.

## Naming Convention

### Function Naming

All function names in this repository must adhere to the _snake_case_ naming convention. Consider these guidelines when naming a function:

1. **Descriptive Names**  
   •  Function names must be descriptive and clearly indicate their purpose.  
   •  Avoid unnecessary abbreviations.  
   
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

### Variable Naming  

All variable names in this repository must adhere to the _snake_case_ naming convention. Consider these guidelines when naming a variable:  

1. **Descriptive Names**  
   •  Variable names must clearly indicate their purpose.  
   •  Avoid single-character names, except for loop counters.  
   •  Avoid abbreviations unless they are standard (e.g. `id` for identifier).  

   Examples:  
   **✓**  `file_descriptor`  
   **𐄂**  `fd`  

### Type Naming

All type names in this repository must adhere to the _PascalCase_ naming convention. Consider these guidelines when naming a type:

1. **Descriptive Names**  
   •  Type names must clearly describe the data they represent.  
   •  Use nouns or noun phrases for type names.  
   
   Examples:  
   **✓**  `UserData`  
   **𐄂**  `Data`  

2. **Module Identification**  
   Types that belong to a specific module, must include the module name within their own name to indicate their association and prevent naming conflicts in the global scope. 

   In the example below, it is assumed that we are declaring the type within a hypothetical `window` module.
   
   Examples:  
   **✓**  `WindowButtonType`  
   **𐄂**  `ButtonType`  

### Macro Naming

All macro names in this repository must adhere to the _snake_case_ naming convention, and be written in all uppercase letters. Consider these guidelines when naming a macro:

1. **Descriptive Names**  
   •  Macro names must clearly indicate their purpose.  
   •  Avoid unnecessary abbreviations.  
   •  Add unit suffixes where applicable (e.g. `_MS`, `_PERCENT`, `_BYTES`).  
   
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

### File Naming  

All file names in this repository must adhere to the _snake_case_ naming convention. Consider these guidelines when naming a file:  

1. **Concise and Contextual Names**  
   •  File names should be concise while maintaining clarity about their purpose.  
   •  Use directory structure to provide context rather than including it in the filename.  
   •  Aim for 1-2 words per file name, letting the path provide additional context.  
   •  Avoid abbreviations unless they are standard (e.g `auth` for authentication).

   Examples:  
   **✓**  `user/auth.c`  
   **✓**  `user/auth/tokens.c`  
   **𐄂**  `user/authentication_tokens.c`

## Declaration Order

All header (.h) and source (.c) files must follow this specific declaration order:

1. **Includes** `.c` `.h`  
2. **Macros** `.c` `.h`  
3. **Types** `.c` `.h`  
4. **Static variables** `.c`  
5. **Functions** `.c` `.h`  

## Documentation

### Header File (.h)  
   •  Every declaration within the header file must be documented.  
   •  Functions require Doxygen format documentation.   
   •  Other elements require a brief inline comment (1-3 lines).  
   •  Keep comment line length under 80 characters (including whitespace).  
   •  Write clean comments with proper punctuation and end them with a period.
   
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
    * @return 0 upon success, non-zero integer otherwise.
    */
   int validate_user(const char* username, const char* password);
   ```
### Source File (.c)  
   •  Don't document any declarations.  
   •  Add inline comments within functions to break down complex logic into clear steps.  
   •  Keep comment line length under 80 characters (including whitespace).  
   •  Write clean comments with proper punctuation and end them with a period.  
   •  Optionally, add a multi-line comment at the top of the file, directly after the includes, starting with "This code is responsible for" to provide critical context.  

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