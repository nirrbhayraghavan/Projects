from flask import Flask, request, jsonify
import pytesseract
from utils import preprocess_image, extract_fields, match_invoice_to_db

app = Flask(__name__)

@app.route("/upload", methods=["POST"])
def upload_invoice():
    if "file" not in request.files:
        return jsonify({"error": "No file part"}), 400
    file = request.files["file"]
    if file.filename == "":
        return jsonify({"error": "No selected file"}), 400

    img = preprocess_image(file)
    text = pytesseract.image_to_string(img)
    extracted = extract_fields(text)

    results = match_invoice_to_db(extracted)
    formatted = [
        {
            "company": row[0],
            "invoice_number": row[1],
            "invoice_date": row[2],
            "transaction_amount": row[3],
            "match_score": score
        }
        for row, score in results
    ]
    return jsonify({
        "extracted": extracted,
        "matches": formatted
    })

if __name__ == "__main__":
    app.run(debug=True)
