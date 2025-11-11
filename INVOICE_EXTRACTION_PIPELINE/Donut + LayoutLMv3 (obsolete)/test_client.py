import requests

url = "http://127.0.0.1:5000/upload"
files = {"file": open("sample_invoice.jpg", "rb")}
response = requests.post(url, files=files)

print("Response:")
print(response.json())
