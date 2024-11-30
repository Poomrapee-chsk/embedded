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

image_path = "Path to images folder"

while(True):
    file_list = [os.path.join(image_path, path) for path in os.listdir(image_path)]
    gfile = drive.CreateFile({'parents': [{'id': ''}]})
    for file in file_list:
    # check if current path is a file
        if os.path.isfile(file):
            if detect_person(file):
                gfile.SetContentFile(file)
                gfile.Upload()
                gfile.content.close()
            else:
                print("No person detected in ",file)
                res = requests.put(headers={"Authorization":""},url="https://api.netpie.io/v2/device/message/light",data="off")

    
    for file in file_list:
        os.remove(file)
        
