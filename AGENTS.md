# PineLock Agent Guidelines

## Project Structure

### Root Directory
- **`server/`**: Python FastAPI backend. Handles API requests, MQTT communication, and serves the web UI.
- **`firmware/`**: C++ PlatformIO project for the ESP32 lock node.
- **`AGENTS.md`**: This file. Guidelines for AI agents.
- **`API_EXAMPLES.md`**: Examples of how to use the server API.
- **`WEB_UI.md`**: Documentation for the Web UI routes and templates.
- **`ARCHITECTURE.md`**: High-level system architecture documentation.
- **`PROJECT_SUMMARY.md`**: Summary of the project status and features.

### Key Components

#### Server (`server/app/`)
- **`main.py`**: Application entry point.
- **`models.py`**: SQLAlchemy database models (User, Lock, Log, etc.).
- **`database.py`**: Database connection and session management.
- **`mqtt_client.py`**: MQTT client wrapper for communicating with the lock.
- **`routes.py`**: API routes for controlling the lock and managing users.
- **`ui_routes.py`**: Routes for serving HTML templates.
- **`templates/`**: Jinja2 HTML templates for the UI.

#### Firmware (`firmware/lock_node/`)
- **`src/main.cpp`**: Main firmware logic (setup and loop).

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
- **Documentation**: Update README/docs for API/firmware changes