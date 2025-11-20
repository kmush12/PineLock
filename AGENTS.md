# PineLock Agent Guidelines

## Build/Lint/Test Commands

### Server (Python/FastAPI)
- **Install dependencies**: `pip install -r requirements.txt`
- **Run server**: `uvicorn app.main:app --host 0.0.0.0 --port 8000`
- **Run with reload**: `uvicorn app.main:app --reload`
- **Run tests**: `pytest tests/` (run single test: `pytest tests/test_file.py::test_function`)
- **No linting configured** - use `flake8` or `black` if added

### Firmware (C++/PlatformIO)
- **Build**: `pio run`
- **Build and upload**: `pio run --target upload`
- **Monitor serial**: `pio device monitor`
- **Clean build**: `pio run --target clean`
- **No linting or testing configured**

## Code Style Guidelines

### Python (Server)
- **Imports**: Standard library first, then third-party, then local imports. Use absolute imports.
- **Naming**: snake_case for variables/functions, PascalCase for classes, UPPER_CASE for constants
- **Types**: Use type hints for function parameters and return values
- **Async**: Use async/await for I/O operations, context managers for resources
- **Error handling**: Use try/except blocks, log errors with appropriate levels
- **Logging**: Use Python logging module, not print statements
- **Docstrings**: Use triple quotes for module/class/function documentation
- **Formatting**: 4 spaces indentation, line length ~88 chars (Black style)

### C++ (Firmware/Arduino)
- **Includes**: Arduino.h first, then third-party libraries, then local headers
- **Naming**: camelCase for variables/functions, PascalCase for classes/structs, UPPER_CASE for constants
- **Braces**: Opening brace on same line, consistent indentation
- **Error handling**: Return boolean/error codes, use Serial.println for debugging
- **Memory**: Be mindful of ESP32 constraints, use const where possible
- **Comments**: Brief comments for complex logic, no excessive commenting
- **Formatting**: Consistent spacing, align similar constructs

### General
- **Security**: Never commit secrets, use environment variables
- **Commits**: Write clear commit messages explaining what and why
- **Testing**: Add tests for new features, run existing tests before committing
- **Documentation**: Update README/docs for API/firmware changes</content>
<parameter name="filePath">/home/kmush/Desktop/Work/Other_repo/PineLock/AGENTS.md