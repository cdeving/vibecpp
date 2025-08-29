# vibecpp - AI Task Automation Tool
A powerful CLI tool for automating tasks using AI, primarily focused on code writing but customizable for any task through its flexible architecture.

## Quick Start

Recommended: We recommend using Docker as it provides the necessary file access permissions for vibecpp to interact with files in your project directory.
Get started quickly with:
```bash
docker pull ghcr.io/cdeving/vibecpp:latest
```
To start docker in your current folder:
```bash
docker run --rm -it \
-v ~/.vibecpp:/root/.vibecpp \
-v ./:/app/project \
ghcr.io/cdeving/vibecpp:latest
```
This is a minimal container for running vibecpp. To interact with additional tools, you will have to modify the image.

## Overview

vibecpp leverages LLM to automate repetitive tasks, with a focus on code generation and editing. It supports multiple LLM providers including local Ollama instances and cloud providers (OpenAI compatible API), making it versatile for different deployment scenarios.

## Key Features

- **Multi-Provider Support**: Connect to Ollama (local) or OpenAI-compatible APIs
- **Flexible Interaction:**
Interactive chat mode for conversation-based tasks
Single message input via stdin `(cat somefile.txt | vibecpp)`
- **Customizable Workflows:** Extend functionality with custom tools using `.custom.vibecpp`

## Configuration System

JSON config system: `~/.vibecpp` and ability to override values with flags and options:
```bash
-h, --help  # Print this help message and exit
# Core settings
--type      # LLM client type (ollama|openai)
--model     # Specific LLM model name
--server    # API endpoint URL
--api-key   # Authentication key for cloud providers (can be empty/random for ollama)

# Debugging
-d          # Enable debug logging
```

## Usage Examples

```bash
# Local Ollama setup
./vibecpp --type ollama --model qwen3

# Cloud OpenAI setup
./vibecpp --type openai --model gpt-4 --api-key YOUR_API_KEY

# Pipe input for automation
echo "Write a Python function to sort a list" | ./vibecpp --type ollama --model qwen3
```

## Building vibecpp

### Prerequisites
Since this project uses Git submodules, make sure to clone recursively:
```bash
git clone --recursive
```
To build this project, you'll need the following dependencies on Ubuntu:
```bash
sudo apt-get update
sudo apt-get install libcurl4-openssl-dev libssl-dev libpthread-stubs0-dev
```

### Build Instructions
The project uses a Makefile with the following targets:
```bash
# Build both debug and release versions (default)
make all
# Build debug version only
make debug
# Build release version only
make release
# Clean object files
make clean
# Remove all generated files
make distclean
```