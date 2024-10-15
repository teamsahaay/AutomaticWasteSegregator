import requests
import socket
import cv2
import numpy as np
import tensorflow as tf
from tensorflow.keras.layers import TFSMLayer
import zipfile
import os
import sys
import serial
import time
import requests

# Function to map predicted class to corresponding object and print the result
def map_predicted_class(predicted_class):
    object_classes = ['wood', 'glass', 'metal', 'paper', 'plastic', 'mixed']
    predicted_object = object_classes[predicted_class]
    print(f"The predicted object is: {predicted_object}")
    
    # Print specific numbers based on the predicted object
    object_number = None
    if predicted_object == "metal":
        object_number = 1
    elif predicted_object == "wood":
        object_number = 2
    elif predicted_object == "paper":
        object_number = 3
    elif predicted_object == "mixed":
        object_number = 4
    elif predicted_object == "glass":
        object_number = 5
    elif predicted_object == "plastic":
        object_number = 6
    
    return object_number

# Unzip the model file
model_zip_path = r'C:\Users\shrut\Downloads\aws_model.zip'  # Ensure the path is raw string
model_dir = 'AWS_model'

if not os.path.exists(model_dir):
    with zipfile.ZipFile(model_zip_path, 'r') as zip_ref:
        zip_ref.extractall(model_dir)
    print("Model unzipped successfully.")
else:
    print("Model directory already exists.")

# Load the unzipped model for inference
try:
    aws_model = TFSMLayer(model_dir, call_endpoint='serving_default')
    print("Model loaded successfully for inference.")
except Exception as e:
    print(f"Error: Could not load the model. {str(e)}")
    sys.exit()

# Define the URL of the image
url = 'http://192.168.176.1:8080/camera'  # Replace with the actual URL

# Send a GET request to the URL
response = requests.get(url, stream=True)

# Check if the request was successful
if response.status_code == 200:
    # Open a file in binary write mode and save the image
    with open('downloaded_image_1.jpg', 'wb') as file:
        file.write(response.content)
    print('Image downloaded successfully.')
else:
    print('Failed to download image. Status code:', response.status_code)



# Replace with the actual IP address of your ESP32
esp32_ip = "192.168.176.1"  # Update this to match your ESP32's IP address
esp32_port = 8081
# Read the saved image
frame = cv2.imread(r"C:\Users\shrut\OneDrive\Desktop\downloaded_image_1.jpg")
if frame is None:
    print("Error: Could not read image file 'photo.jpg'. Make sure the file exists in the working directory.")
    sys.exit()

# Preprocess the captured image
resized_frame = cv2.resize(frame, (224, 224))
resized_frame = resized_frame.astype('float32') / 255.0
input_image = np.expand_dims(resized_frame, axis=0)

# Make a prediction using the AWS model
predictions = aws_model(input_image)

# Debug: Print the structure of the predictions object
print(f"Predictions object type: {type(predictions)}")
print(f"Predictions content: {predictions}")

# Extracting the actual prediction values
# This section will need to be adjusted based on the printed structure
if isinstance(predictions, tf.Tensor):
    predictions = predictions.numpy()
elif isinstance(predictions, dict):
    predictions = list(predictions.values())[0]

print(f"Processed predictions content: {predictions}")

# Determine the predicted class
predicted_class = np.argmax(predictions)

# Map the predicted class to the corresponding object and print the result
object_number = map_predicted_class(predicted_class)
print(object_number)
# Data to send
data = str(object_number)

# Create a socket object
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    # Connect to the ESP32
    client_socket.connect((esp32_ip, esp32_port))
    print("Connected to ESP32")

    # Send data
    request = f"GET /value?data={data} HTTP/1.1\r\nHost: {esp32_ip}\r\n\r\n"
    client_socket.sendall(request.encode())
    print(f"Data sent: {data}")

    # Receive acknowledgment from ESP32
    response = client_socket.recv(1024)
    print(f"Response from ESP32: {response.decode()}")

finally:
    # Close the socket
    client_socket.close()
    print("Connection closed")
