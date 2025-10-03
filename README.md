# AC Control Application

A web-based interface to control an AC unit, featuring a modern UI and a FastAPI backend.

## Features

- 🌡️ Control AC temperature (16°C - 30°C)
- 🔄 Multiple operation modes (Cool, Heat, Fan, Dry, Auto)
- 🌬️ Adjustable fan speed (Low, Medium, High, Auto)
- ⚡ Power on/off control
- 📱 Responsive design that works on desktop and mobile
- 🔄 Real-time state updates
- 🐳 Dockerized for easy setup and deployment

## Project Structure

```
/
├── backend/
│   ├── Dockerfile
│   ├── main.py
│   └── requirements.txt
├── pwa/
│   ├── Dockerfile
│   ├── index.html
│   ├── manifest.json
│   └── sw.js
├── assets/
│   └── ac_controller_architecture.png
├── docker-compose.yml
└── README.md
```

## Running the Application with Docker Compose

This is the recommended way to run the application.

### Prerequisites

- [Docker](https://docs.docker.com/get-docker/)
- [Docker Compose](https://docs.docker.com/compose/install/)

### Instructions

1.  Clone the repository:
    ```bash
    git clone <repository-url>
    cd ac_app
    ```

2.  Build and start the containers:
    ```bash
    docker-compose up --build
    ```

3.  Access the application:
    -   **Frontend:** [http://localhost:8080](http://localhost:8001)
    -   **Backend API:** [http://localhost:8000](http://localhost:8000)

4.  To stop the application:
    ```bash
    docker-compose down
    ```

## API Endpoints

The backend provides the following API endpoint:

-   `POST /api/set-state`: Update the AC state.

    The request body should be a JSON object with the following structure:

    ```json
    {
      "isOn": boolean,
      "temperature": number (16-30),
      "mode": string ("Cool", "Heat", "Fan", "Dry", "Auto"),
      "fanSpeed": string ("Low", "Medium", "High", "Auto")
    }
    ```

## Development

For development, you can run the backend and frontend services separately.

### Backend (FastAPI)

1.  Navigate to the `backend` directory:
    ```bash
    cd backend
    ```

2.  Install the required Python packages:
    ```bash
    pip install -r requirements.txt
    ```

3.  Start the FastAPI backend server:
    ```bash
    uvicorn main:app --reload
    ```

    The API will be available at `http://127.0.0.1:8000`.

### Frontend (PWA)

1.  Navigate to the `pwa` directory:
    ```bash
    cd pwa
    ```

2.  Open the `index.html` file in your web browser. You can use a local server like VS Code Live Server or Python's built-in HTTP server:
    ```bash
    python -m http.server 8001
    ```

    Then navigate to `http://localhost:8001`.

## License

This project is open source and available under the [MIT License](LICENSE).

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.