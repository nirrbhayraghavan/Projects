import re
import spacy
import psycopg2
from difflib import SequenceMatcher
from PIL import Image, ImageEnhance, ImageFilter

nlp = spacy.load("en_core_web_sm")

def preprocess_image(path):
    img = Image.open(path).convert("L")
    img = img.filter(ImageFilter.SHARPEN)
    return img

def extract_fields(text):
    fields = {
        'invoice_number': None,
        'invoice_date': None,
        'transaction_amount': None,
        'company': None
    }

    invoice_num_match = re.search(r'(Invoice|Inv)[\s#:]*([A-Z0-9-]+)', text, re.IGNORECASE)
    date_match = re.search(r'(Invoice Date|Date)[\s:]*([\d/\-.]+)', text, re.IGNORECASE)
    total_match = re.search(r'(Total|Amount Due)[\s:]*\$?([\d,]+\.\d{2})', text, re.IGNORECASE)

    fields['invoice_number'] = invoice_num_match.group(2) if invoice_num_match else None
    fields['invoice_date'] = date_match.group(2) if date_match else None
    fields['transaction_amount'] = total_match.group(2) if total_match else None

    doc = nlp(text)
    for ent in doc.ents:
        if ent.label_ == "ORG":
            fields['company'] = ent.text
            break

    return fields

def match_invoice_to_db(extracted):
    conn = psycopg2.connect(
        dbname="invoice_db",
        user="postgres",
        password="1234",
        host="localhost",
        port="5432"
    )
    cur = conn.cursor()
    cur.execute("SELECT company, invoice_number, invoice_date, transaction_amount FROM invoices")
    rows = cur.fetchall()

    def score(row):
        keys = ['company', 'invoice_number', 'invoice_date', 'transaction_amount']
        return sum(SequenceMatcher(None, str(extracted[k] or '').lower(), str(v or '').lower()).ratio()
                   for k, v in zip(keys, row)) / len(keys)

    scored_rows = [(row, round(score(row), 3)) for row in rows]
    scored_rows.sort(key=lambda x: x[1], reverse=True)  

    cur.close()
    conn.close()

    return scored_rows  
