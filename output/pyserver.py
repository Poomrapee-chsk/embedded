import socket
from PIL import Image
import numpy as np
import time

server_ip = '0.0.0.0' # Listen on all interfaces
server_port = 81
# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind((server_ip, server_port))
sock.listen(1)

print(f'Server listening on {server_ip}:{server_port}')

count = 0
## QQVGA frame size
frame_width = 160
frame_height = 120
frame_size = frame_width * frame_height * 2
buffer_size = 1024

def receive_frame(connection, frame_size):
    header = b'\xAA\x55'  # The expected 2-byte header
    received_data = bytearray()

    # Wait for the header
    while len(received_data) < 2:
        chunk = connection.recv(2 - len(received_data))
        if not chunk:
            raise ConnectionError("Connection lost before receiving header")
        received_data.extend(chunk)

    if received_data[:2] != header:
        raise ValueError("Invalid header received")

    # Receive the frame data
    frame_data = bytearray()
    while len(frame_data) < frame_size:
        chunk = connection.recv(min(1024, frame_size - len(frame_data)))
        if not chunk:
            raise ConnectionError("Connection lost before receiving frame data")
        frame_data.extend(chunk)

    return frame_data

def rgb565_to_rgb888(pixels, xres, yres, flag):
    # Create an empty image buffer (RGBA)
    img_data = np.zeros((yres, xres, 4), dtype=np.uint8)

    for y in range(yres):
        for x in range(xres):
            i = (y * xres + x) * 2
            # Correct pixel alignment
            pixel16 = pixels[i] | (pixels[i + 1] << 8)
            # Extract and scale RGB components
            img_data[y, x, 0] = ((((pixel16 >> 11) & 0x1F) * 527) + 23) >> 6  # Red
            img_data[y, x, 1] = ((((pixel16 >> 5) & 0x3F) * 259) + 33) >> 6   # Green
            img_data[y, x, 2] = (((pixel16 & 0x1F) * 527) + 23) >> 6          # Blue
            img_data[y, x, 3] = 255                                           # Alpha channel

    if flag == 0xFF:  # Last block
        return img_data

# # Test frame
# test_frame = np.zeros((frame_height, frame_width), dtype=np.uint16)
# test_frame[:] = 0xF800  # Pure red in RGB565

# # Convert
# test_frame_bytes = test_frame.tobytes()
# test_image = rgb565_to_rgb888(test_frame_bytes, frame_width, frame_height, 0xFF)

# # Display
# Image.fromarray(test_image, 'RGBA').save('test_frame.png')


while True:
    connection, client_address = sock.accept()
    try:
        received_data = bytearray()
        now = int(time.time())
        while len(received_data) < frame_size:
            data = receive_frame(connection, frame_size)
            if not data:
                break
            print("Received frame data")
            received_data.extend(data)
        
        # name = f'output_frame_{now}.bin'
        # with open(name, 'wb') as f:
        #     f.write(received_data)
        if len(received_data) == frame_size:
            # Convert the frame data to RGB888
            rgb888_data = rgb565_to_rgb888(received_data, frame_width, frame_height, 0xFF)

            # Create an image from the RGB888 data
            name = f'output_frame_{now}.png'
            image = Image.fromarray(rgb888_data, 'RGBA')
            image.save(name)
        else:
            print('Incomplete frame data received')
    
    finally:
        connection.close()