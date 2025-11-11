from transformers import LayoutLMTokenizer, LayoutLMForTokenClassification
from PIL import Image
import torch
import pytesseract

tokenizer = LayoutLMTokenizer.from_pretrained("microsoft/layoutlm-base-uncased")
model = LayoutLMForTokenClassification.from_pretrained("microsoft/layoutlm-base-uncased")
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
model.to(device)

def normalize_bbox(bbox, size):
    width, height = size
    return [
        int(1000 * bbox[0] / width),
        int(1000 * bbox[1] / height),
        int(1000 * bbox[2] / width),
        int(1000 * bbox[3] / height),
    ]

def extract_invoice_fields(image_path):
    image = Image.open(image_path).convert("RGB")
    width, height = image.size
    data = pytesseract.image_to_data(image, output_type=pytesseract.Output.DICT)

    words, boxes = [], []
    for i in range(len(data["text"])):
        if int(data["conf"][i]) > 0 and data["text"][i].strip() != "":
            words.append(data["text"][i])
            x, y, w, h = data["left"][i], data["top"][i], data["width"][i], data["height"][i]
            boxes.append(normalize_bbox([x, y, x + w, y + h], (width, height)))

    if not words:
        return {}

    encoding = tokenizer(words, boxes=boxes, return_tensors="pt", padding="max_length", truncation=True)
    encoding = {k: v.to(device) for k, v in encoding.items()}
    outputs = model(**encoding)
    predictions = outputs.logits.argmax(-1).squeeze().tolist()
    labels = [model.config.id2label[pred] for pred in predictions]

    extracted = {}
    for token, label in zip(words, labels):
        if label != "O":
            key = label.split("-")[-1].lower()
            extracted.setdefault(key, "")
            extracted[key] += token + " "

    return {k: v.strip() for k, v in extracted.items()}
