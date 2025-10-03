
import base64
from PIL import Image, ImageDraw

def create_icon(size, file_path):
    image = Image.new("RGB", (size, size), "blue")
    draw = ImageDraw.Draw(image)
    image.save(file_path, "PNG")

create_icon(192, "/home/salomo/Documents/fun/home_automation/ac_app/pwa/images/icon-192.png")
create_icon(512, "/home/salomo/Documents/fun/home_automation/ac_app/pwa/images/icon-512.png")
