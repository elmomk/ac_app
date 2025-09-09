from diagrams import Diagram, Edge, Cluster
from diagrams.onprem.client import Client
from diagrams.generic.os import Raspbian

# This script generates a single system diagram from Python using the 'diagrams' library.
# To run this, you will need to install the library:
# pip install diagrams

# It also requires Graphviz to be installed on your system.
# See the official docs for installation: https://graphviz.org/download/

with Diagram("AC Controller Architecture", show=False, direction="LR"):
    # Define the layers using Clusters to keep them in one diagram
    with Cluster("Hardware Layer"):
        esp32_sensor = Client("ESP32: Temperature Sensor")
        esp32_actuator = Client("ESP32: AC Controller")

    with Cluster("Server Layer"):
        rpi_backend = Raspbian(
            "\nRaspberry Pi: Backend\n(Logger / AC and Temperature Logic)"
        )

    with Cluster("Client Layer"):
        rpi_frontend = Raspbian(
            "\nRaspberry Pi: Frontend\n(UI to send actions to ESP32)"
        )

    # Define the connections (flow) between the layers
    # Use the >> operator to show the direction of data flow

    # Data flow from sensor to backend
    esp32_sensor >> Edge(label="Reads temperature") >> rpi_backend

    # Command flow from backend to actuator
    rpi_backend >> Edge(label="Sends commands based on logic") >> esp32_actuator

    # User actions from frontend to actuator
    rpi_frontend >> Edge(label="Sends user actions/Displays data") >> rpi_backend
