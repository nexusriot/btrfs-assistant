# Overview
Contributions are welcome!

If you want add or change significant functionality it is better to discuss it before beginning work either in an issue or a WIP merge request.  There is also a [Telegram group](https://t.me/+fR23J4jV68MwYTgx) available for development discussions.

### Release Cycle
There is generally a WIP branch created for each new release.  The name is `wip-version`.  For example, `wip-1.5`.  When a set of features is completed the WIp branch is merged into `main` and a new WIP branch is created.  The changes are then tested by the community for a period of time.  During this period fixes are merged directly into `main`.

Once a the changes are adequately tested, the release is tagged with the version number.

When contributing, unless the MR is for a crtical bug, look for a WIP branch to point the MR at.  Critical bug fixes can be directly merged into `main`

# Coding style
Most code style guidelines are enforced by running `clang-format` using the included format file.

Here are the important guidelines for other things.

### Use of braces
All code blocks should use braces.

Wrong:
```
if (condition)
    statement;

while (condition)
    statement;
```

Horrifically Wrong:
```
if (condition)
    statement;
else {
    statement;
}
```

Correct:
```
if (condition) {
    statement;
}

while (condition) {
    statement;
}

if (condition) {
    statement;
} else {
    statement;
}
```

### Naming
Generally speaking, the project tries to follow Qt naming standards.

Almost everything should be named using camelCase.

There are a few things that use PascalCase.  This includes:
* Classes
* Enums
* Structs
* typedefs that refer to one of the above

Qt widgets should be named using objectType_objectName with both in camelCase.  For example, `pushButton_newItem`

Getters and setters also follow Qt conventions.  Members should be named starting with `m_`.  For example:

```
int monkey() { return m_monkey; }
void setMonkey(int monkey) { m_monkey = monkey; }
```

### Comments
Use comments whenever something isn't obvious and in any longer block of code.

Avoid meaningless comments.  "Iterate over the list" is fairly useless.  Instead describe why you are iterating over the list.

All functions should have a comment clock describing the function.  For class methods/functions these should be in the header file.  For free functions they can be in the .cpp file.

Function blocks for new code should use the follow convention

```
/**
 * @brief A description of what the function does
 * @param paramname A description of the parameter
 * @param paramname2 A description of the parameter
 * @return A description of what the function returns
 */
```

If the function doesn't return anything or has no parameters those sections should be omitted.

The slots for widgets do not need comments.  It should be clear what the function `on_pushButton_subvolDelete_clicked`  does.  If it isn't clear, the code probably needs to be refactored.

### Other
The functions in a class should be alphabetized in both the header and the .cpp file.  If a class file contains free functions, those should be at the top.
