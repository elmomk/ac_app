# First, install the necessary libraries using pip:
# pip install fastapi uvicorn pydantic

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel

# import uvicorn
import time

# Create the FastAPI application instance
app = FastAPI(title="Raspberry Pi AC Control API")

# --- CORS Middleware Configuration ---
# This middleware allows the API to accept requests from your web app.
origins = [
    "http://localhost",  # Allow requests from localhost
    "http://localhost:8000",
    "http://127.0.0.1:5500",  # If you're using a live server like VS Code Live Server, you may need to add its address.
]

app.add_middleware(
    CORSMiddleware,
    allow_origins=[
        "*"
    ],  # For development, it's easier to allow all origins. You can make this more specific later.
    allow_credentials=True,
    allow_methods=["*"],  # Allow all HTTP methods (GET, POST, etc.)
    allow_headers=["*"],  # Allow all headers
)


# Define a Pydantic model for the incoming AC state data.
# This ensures that the data received from the web app is structured correctly.
class ACState(BaseModel):
    isOn: bool
    temperature: int
    mode: str
    fanSpeed: str


# Define the API endpoint to receive AC control commands
@app.post("/api/set-state")
async def set_ac_state(state: ACState):
    """
    Simulates sending a command to an Arduino to control an AC unit.

    In a real-world scenario, you would replace the print statements
    with actual code to communicate with your Arduino via a serial port
    or another communication protocol.
    """
    print("--------------------------------------------------")
    print(f"Received command from web app:")
    print(f"AC Power: {'ON' if state.isOn else 'OFF'}")
    print(f"Temperature: {state.temperature}°C")
    print(f"Mode: {state.mode}")
    print(f"Fan Speed: {state.fanSpeed}")

    # --- SIMULATED ARDUINO COMMUNICATION ---
    # This is where you would add the logic to send the command to the Arduino.
    # For example:
    # import serial
    # ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
    # command_string = f"{state.isOn},{state.temperature},{state.mode},{state.fanSpeed}\n"
    # ser.write(command_string.encode('utf-8'))
    # time.sleep(0.1) # Give the Arduino time to respond
    # response = ser.readline().decode('utf-8').strip()
    # ser.close()

    # We will simulate a successful response for this example.
    print("Simulating command sent to Arduino...")
    # Delay to represent the time it takes for the Arduino to receive and process the command
    time.sleep(1)
    print("Command acknowledged by simulated Arduino.")
    print("--------------------------------------------------")

    # Return a success message to the web app
    return {"message": "AC state updated successfully", "status": "ok"}


# Instructions to run the API:
# Save this file as main.py.
# Open your terminal and navigate to the directory where you saved it.
# Run the command:
# uvicorn main:app --reload

# The server will start at http://127.0.0.1:8000.
# The endpoint is http://127.0.0.1:8000/api/set-state
# Remember to replace the fake API URL in your HTML file with this address.
