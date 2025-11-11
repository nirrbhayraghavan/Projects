from flask import Flask, request, jsonify
import os
from werkzeug.utils import secure_filename
from utils import extract_invoice_fields

app = Flask(__name__)
UPLOAD_FOLDER = "invoices"
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

@app.route("/upload", methods=["POST"])
def upload_invoice():
    file = request.files.get("file")
    if not file:
        return jsonify({"error": "No file provided"}), 400

    filename = os.path.join(UPLOAD_FOLDER, secure_filename(file.filename))
    file.save(filename)

    try:
        result = extract_invoice_fields(filename)
        return jsonify(result)
    except Exception as e:
        print("❌ ERROR:", e)
        return jsonify({"error": str(e)}), 500

if __name__ == "__main__":
    app.run(debug=True)
