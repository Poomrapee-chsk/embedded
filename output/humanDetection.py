from transformers import pipeline
from pydrive.auth import GoogleAuth 
from pydrive.drive import GoogleDrive
import os
import requests


gauth = GoogleAuth()
drive = GoogleDrive(gauth)
def detect_person(image_path):
    object_detector = pipeline("object-detection", model="hustvl/yolos-tiny")

    results = object_detector(image_path)

    for result in results:
        if result['label'] == 'person' and result['score'] > 0.5:
            return True
    return False

image_path = "images"

while(True):
    file_list = [os.path.join(image_path, path) for path in os.listdir(image_path)]
    gfile = drive.CreateFile({'parents': [{'id': '1JHcgdEVg76rGVJuKUBf3Ol7CYN9cviQ-'}]})
    for file in file_list:
    # check if current path is a file
        if os.path.isfile(file):
            if detect_person(file):
                gfile.SetContentFile(file)
                gfile.Upload()
                gfile.content.close()
            else:
                print("No person detected in ",file)
                res = requests.put(headers={"Authorization":"Device 1428f47f-fe35-4010-8537-ac411a92ef51:PS42qjzn1vjEehJUAPMjqW1jCx65F5te"},url="https://api.netpie.io/v2/device/message/light",data="off")

    
    for file in file_list:
        os.remove(file)
        
