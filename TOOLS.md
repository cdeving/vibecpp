# Custom Tool System Documentation

## Overview
This system allows you to define custom tools that can be called by LLM. These tools are configured through a `~/.custom.vibecpp` file and can execute system commands while providing structured communication between your application and LLMs.

## Configuration Format

Each tool is defined as a JSON object with the following fields:

```json
{
  "tools": [
    {
        "name": "ToolName",
        "description": "Description of what this tool does",
        "arguments": "argument_definitions",
        "realCommand": "system_command",
        "format": "format_string"
    }
  ]
}
```

## Field Details

### `name` (required)
- Unique identifier for the tool
- Used by LLM to reference this tool

### `description` (required)
- Clear explanation of what the tool does
- Helps LLM determine when to use this tool

### `arguments` (required)
- String describing each argument in format: `name: type - description`
- For flag arguments, specify as `name: string "flag_name" - description`
- Example: `"gpu_num: int - gpu id to get (0 for all), debug: string \"debug\" - is a flag, don't set if not needed"`

### `format` (required)
- Template string using argument names for parameter placement
- Arguments are bound by name, maintaining order and preventing LLM errors
- Example: `"{gpu_num} {debug}"`

### `realCommand` (required)
- System command to execute via `popen`
- Must exist on the target system
- Example: `"gpudata"`

## Usage Example

The system will execute commands like:
```
gpudata 0 debug
```