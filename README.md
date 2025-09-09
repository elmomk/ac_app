# AC Control Application

A web-based interface to control an AC unit, featuring a modern UI and a FastAPI backend.

## Features

- 🌡️ Control AC temperature (16°C - 30°C)
- 🔄 Multiple operation modes (Cool, Heat, Fan, Dry, Auto)
- 🌬️ Adjustable fan speed (Low, Medium, High, Auto)
- ⚡ Power on/off control
- 📱 Responsive design that works on desktop and mobile
- 🔄 Real-time state updates

## Prerequisites

- Python 3.7+
- pip (Python package manager)
- Modern web browser (Chrome, Firefox, Safari, Edge)

## Installation

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd ac_app
   ```

2. Install the required Python packages:
   ```bash
   pip install fastapi uvicorn pydantic
   ```

## Running the Application

1. Start the FastAPI backend server:
   ```bash
   uvicorn main:app --reload
   ```
   The API will be available at `http://127.0.0.1:8000`

2. Open the web interface:
   - Simply open `index.html` in your web browser
   - Or use a local server like VS Code Live Server or Python's built-in HTTP server:
     ```bash
     python -m http.server 8001
     #or
     livereload -d -p 8001
     ```
     Then navigate to `http://localhost:8001`

## Running with Docker Compose

1. Make sure you have [Docker](https://docs.docker.com/get-docker/) and [Docker Compose](https://docs.docker.com/compose/install/) installed

2. Build and start the containers:
   ```bash
   docker-compose up --build
   ```

3. Access the application:
   - Frontend: http://localhost:8001
   - Backend API: http://localhost:8000

4. To stop the application:
   ```bash
   docker-compose down
   ```

## API Endpoints

- `POST /api/set-state` - Update the AC state
  - Request body should be a JSON object with:
    ```json
    {
      "isOn": boolean,
      "temperature": number (16-30),
      "mode": string ("cool", "heat", "fan", "dry", "auto"),
      "fanSpeed": string ("low", "medium", "high", "auto")
    }
    ```

## Project Structure

- `main.py` - FastAPI backend server
- `index.html` - Web interface

## Development

This project uses:
- Backend: FastAPI
- Frontend: Vanilla JavaScript with Tailwind CSS

## License

This project is open source and available under the [MIT License](LICENSE).

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Acknowledgments

- [FastAPI](https://fastapi.tiangolo.com/)
- [Tailwind CSS](https://tailwindcss.com/)
- [Inter Font](https://rsms.me/inter/)
