# 🧾 Invoice OCR & Field Extraction Pipeline

### Overview
This repository implements an **end-to-end invoice extraction system** that runs entirely on local or hybrid cloud infrastructure.  
It combines **vision-language models (Donut + LayoutLMv3)** with **document layout analysis (DocTR)** and **fallback OCR (AWS Textract + regex + Tesseract)** to extract structured fields from invoices, match them against a **PostgreSQL database**, and return ranked similarity scores.

The pipeline is modular, offline-capable, and production-ready — ideal for invoice reconciliation, expense tracking, or supplier analytics.

---

## 🚀 Features

- **Multimodal OCR Stack**
  - **Donut (VisionEncoderDecoderModel)** for high-level text and layout understanding  
  - **LayoutLMv3** for fine-grained token classification and spatial field mapping  
  - **DocTR** for candidate company name detection and fallback extraction  
  - **AWS Lambda (Textract + Comprehend)** integration for cloud-based inference and validation  
  - **Regex / spaCy / pytesseract** as a final local fallback  

- **Dynamic Fallback Logic**
  - Retries invalid or incomplete fields (e.g., missing company name, bad invoice number)  
  - Excludes previously invalid results to prevent infinite loops  

- **PostgreSQL Matching**
  - Extracted fields (`invoice_number`, `invoice_date`, `total_amount`, `company_name`) are compared with local DB records  
  - Results returned in **ascending order of similarity** with match percentages  

- **Result Logging**
  - Every run is logged into PostgreSQL with:
    - Source of extraction (Donut / LayoutLMv3 / DocTR / AWS / Regex)
    - Confidence score
    - Timestamp and file path  

---

## 🧱 System Architecture

```text
            +--------------------+
            |  Upload JPEG File  |
            +--------------------+
                       │
                       ▼
         +---------------------------+
         |  Flask / API Gateway      |
         |  (receives invoice image) |
         +---------------------------+
                       │
                       ▼
        +----------------------------------------+
        |    Donut Model (primary extraction)     |
        +----------------------------------------+
                       │
                Invalid / Missing fields?
                       ▼
        +----------------------------------------+
        | LayoutLMv3 + DocTR (layout refinement) |
        +----------------------------------------+
                       │
                Invalid company name?
                       ▼
        +-------------------------------------------+
        | AWS Lambda / Textract / Comprehend fallback |
        +-------------------------------------------+
                       │
                       ▼
            +----------------------------+
            | Regex / pytesseract backup |
            +----------------------------+
                       │
                       ▼
          +--------------------------------+
          | PostgreSQL field comparison    |
          +--------------------------------+
                       │
                       ▼
          +--------------------------------+
          | JSON Response with Match Scores |
          +--------------------------------+
