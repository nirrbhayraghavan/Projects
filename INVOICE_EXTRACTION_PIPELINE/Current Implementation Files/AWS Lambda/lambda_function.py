import boto3
import json
import base64
import requests
import re

s3 = boto3.client('s3')
textract = boto3.client('textract')
comprehend = boto3.client('comprehend')

S3_BUCKET = 'invoice-ocr-uploads1'
FORWARD_API_URL = 'https://dcd4cvmjb2.execute-api.us-east-1.amazonaws.com/prod/upload'


def extract_company_name_from_candidates(candidates):
    comprehend = boto3.client("comprehend")
    text_blob = "\n".join(candidates)[:5000]

    try:
        response = comprehend.detect_entities(Text=text_blob, LanguageCode='en')
    except Exception as e:
        print(f"❌ Comprehend call failed: {e}")
        return "", []

    entities = response.get("Entities", [])
    org_entities = [e["Text"] for e in entities if e["Type"] == "ORGANIZATION"]

    for org in org_entities:
        for candidate in candidates:
            if org.lower() in candidate.lower() or candidate.lower() in org.lower():
                return candidate, org_entities

    return (org_entities[0] if org_entities else ""), org_entities



def extract_kv_from_forms(blocks):
    fields = {
        'invoice_number': None,
        'invoice_date': None,
        'transaction_amount': None,
        'company_name': None
    }
    key_map, block_map = {}, {}

    for block in blocks:
        block_map[block['Id']] = block
        if block['BlockType'] == 'KEY_VALUE_SET' and 'KEY' in block.get('EntityTypes', []):
            key_map[block['Id']] = block

    for key_id, key_block in key_map.items():
        key_text = extract_text(key_block, block_map).lower()
        val_block_id = next((rel['Ids'][0] for rel in key_block.get('Relationships', []) if rel['Type'] == 'VALUE'), None)
        val_block = block_map.get(val_block_id)
        val_text = extract_text(val_block, block_map).strip() if val_block else ""

        if any(k in key_text for k in ['invoice number', 'document number']):
            fields['invoice_number'] = val_text
        elif any(k in key_text for k in ['invoice date', 'document date']):
            fields['invoice_date'] = val_text
        elif re.fullmatch(r'total', key_text) and not fields['transaction_amount']:
            try:
                val = float(val_text.replace(',', ''))
                if val > 0:
                    fields['transaction_amount'] = val_text
            except:
                pass
        elif 'company' in key_text or 'vendor' in key_text:
            fields['company_name'] = val_text

    return fields

def extract_text(block, block_map):
    text = ''
    for rel in block.get('Relationships', []):
        if rel['Type'] == 'CHILD':
            for cid in rel['Ids']:
                word = block_map.get(cid)
                if word and word['BlockType'] == 'WORD':
                    text += word['Text'] + ' '
    return text.strip()

def fallback_extract_from_text(text, fields):
    lines = [line.strip() for line in text.split('\n') if line.strip()]
    all_numbers = []

    ########
    if not fields['invoice_date']:
        for line in lines:
            if any(k in line.lower() for k in ['invoice date', 'document date', 'date']):
                match = re.search(r'(\d{1,2}[ \-/][A-Za-z]{3,9}[ \-/]\d{2,4}|\d{1,2}[ \-/]\d{1,2}[ \-/]\d{2,4})', line)
                if match:
                    fields['invoice_date'] = match.group(1).strip()
                    break

    ########
    if not fields['company_name'] or fields['company_name'].lower() in ['ads', 'invoice', 'bill', '', 'receipt']:
        found = False

        ########
        for i, line in enumerate(lines):
            if re.fullmatch(r'from', line.lower().strip()):
                for j in range(i + 1, i + 3):
                    if j < len(lines):
                        next_line = lines[j].strip()
                        if len(next_line.split()) >= 2 and not re.search(r'\d{2,}', next_line) and not re.search(r'invoice|bill|address|date|to', next_line.lower()):
                            fields['company_name'] = next_line
                            found = True
                            break
                if found:
                    break

        ########
        if not found:
            for i, line in enumerate(lines):
                if 'seller' in line.lower():
                    for j in range(i + 1, i + 4):
                        if j < len(lines):
                            possible = lines[j].strip()
                            if len(possible.split()) > 1 and not re.search(r'\d{2,}', possible) and not re.search(r'invoice|total|address|city|state|pin|phone', possible.lower()):
                                fields['company_name'] = possible
                                found = True
                                break
                    if found:
                        break

        ########
        if not found:
            for line in lines[:6]:
                if re.search(r'\b(a\+e|architects|design|solutions|group|inc|llc|ltd|associates|corp|consulting|technologies|foundation|partners|company|national park)\b', line, re.IGNORECASE):
                    if not re.search(r'invoice|number|bill|date|address|statement|to', line.lower()):
                        fields['company_name'] = line.strip()
                        found = True
                        break

        ########
        if not found:
            for i, line in enumerate(lines[-10:]):
                if 'make payments to' in line.lower():
                    for j in range(i + 1, i + 4):
                        if j < len(lines[-10:]):
                            possible = lines[-10:][j]
                            if len(possible.split()) > 1 and not re.search(r'\d', possible):
                                fields['company_name'] = possible.strip()
                                found = True
                                break
                    if found:
                        break

        ########
        if not found:
            for line in lines:
                if re.search(r'\b(inc|llc|corp|ltd|company|group|partners|systems|foundation|technologies|national park)\b', line, re.IGNORECASE):
                    if not re.search(r'\b(invoice|number|date|billed|address|total|due|pin|mobile|city|state|landmark|order id)\b', line.lower()):
                        fields['company_name'] = line.strip()
                        found = True
                        break

        ########
        if not found:
            for line in lines[:10]:
                if len(line.split()) <= 3 and line.isupper():
                    if not re.search(r'\d|invoice|date|due|total|amount|qty|price|card|note|bank|account', line.lower()):
                        fields['company_name'] = line.strip()
                        break

    ########
    if not fields['transaction_amount']:
        for line in reversed(lines):
            if re.search(r'\btotal\b', line.lower()) and not re.search(r'sub[-\s]?total', line.lower()):
                matches = re.findall(r'(\d{1,3}(?:[,]\d{3})*(?:[.]\d{2}))', line)
                for amt in matches:
                    try:
                        val = float(amt.replace(',', ''))
                        if 0 < val < 1000000:
                            fields['transaction_amount'] = f"{val:.2f}"
                            return fields
                    except:
                        continue
        nums = re.findall(r'(\d{1,3}(?:[,]\d{3})*(?:[.]\d{2}))', text)
        for n in nums:
            try:
                val = float(n.replace(',', ''))
                if 0 < val < 1000000:
                    all_numbers.append(val)
            except:
                continue
        if all_numbers:
            fields['transaction_amount'] = f"{max(all_numbers):.2f}"

    return fields



def enhance_with_comprehend(text, fields):
    try:
        response = comprehend.detect_entities(Text=text[:5000], LanguageCode='en')
        entities = response.get("Entities", [])

        if not fields['company_name'] or fields['company_name'].lower() in ['ads', '', 'invoice', 'receipt']:
            for e in entities:
                if e["Type"] == "ORGANIZATION" and len(e["Text"].split()) > 1:
                    fields['company_name'] = e["Text"]
                    break

        if not fields['transaction_amount']:
            for e in entities:
                if e["Type"] == "QUANTITY":
                    context = text[e['BeginOffset']-20:e['EndOffset']+20].lower()
                    if re.search(r'\btotal\b', context):
                        amt = e["Text"].replace("$", "").replace(",", "")
                        try:
                            val = float(amt)
                            if val > 0:
                                fields['transaction_amount'] = f"{val:.2f}"
                                break
                        except:
                            continue
    except Exception as e:
        print(f"Comprehend enhancement failed: {str(e)}")
    return fields

def lambda_handler(event, context):
    try:
        if isinstance(event.get("body"), str):
            body = json.loads(event["body"])
        elif isinstance(event.get("body"), dict):
            body = event["body"]
        else:
            body = {}
    except Exception as e:
        print(f"Failed to parse event['body']: {e}")
        return {
            "statusCode": 400,
            "body": json.dumps({"error": "Invalid or missing body"})
        }

    if body.get("mode") == "extract_company_name":
        candidates = body.get("candidates", [])
        if not isinstance(candidates, list) or not candidates:
            return {
                "statusCode": 400,
                "body": json.dumps({"error": "Missing or invalid 'candidates' list"})
            }

        try:
            best_match, orgs = extract_company_name_from_candidates(candidates)
            return {
                "statusCode": 200,
                "body": json.dumps({
                    "best_match": best_match,
                    "organization_entities": orgs
                })
            }
        except Exception as e:
            print(f"Comprehend block failed: {str(e)}")
            return {
                "statusCode": 500,
                "body": json.dumps({"error": "Comprehend error", "details": str(e)})
            }
        
    try:
        filename = body['filename']
        file_data = base64.b64decode(body['file'])

        s3.put_object(Bucket=S3_BUCKET, Key=filename, Body=file_data)
        print(f"Uploaded {filename} to S3")

        extracted_fields = {}

        ######## AnalyzeDocument
        try:
            response = textract.analyze_document(
                Document={'S3Object': {'Bucket': S3_BUCKET, 'Name': filename}},
                FeatureTypes=['FORMS', 'TABLES']
            )
            extracted_fields = extract_kv_from_forms(response['Blocks'])
        except Exception as e:
            print(f"AnalyzeDocument failed: {e}")

        ######## AnalyzeExpense fallback
        if not extracted_fields or all(v is None for v in extracted_fields.values()):
            try:
                exp_response = textract.analyze_expense(
                    Document={'S3Object': {'Bucket': S3_BUCKET, 'Name': filename}}
                )
                summary_fields = exp_response['ExpenseDocuments'][0]['SummaryFields']
                extracted_fields = extract_invoice_fields(summary_fields)
            except Exception as e:
                print(f"AnalyzeExpense failed: {e}")

        ######## Raw OCR + NLP fallback
        try:
            text_response = textract.detect_document_text(
                Document={'S3Object': {'Bucket': S3_BUCKET, 'Name': filename}}
            )
            full_text = '\n'.join([
                block['Text'].strip() for block in text_response['Blocks']
                if block['BlockType'] == 'LINE' and block['Text'].strip()
            ])
            extracted_fields = fallback_extract_from_text(full_text, extracted_fields)
            extracted_fields = enhance_with_comprehend(full_text, extracted_fields)
        except Exception as e:
            print(f"Raw OCR fallback failed: {e}")

        print("Final extracted fields:", extracted_fields)

        ######## Forward results
        try:
            res = requests.post(FORWARD_API_URL, json={
                'filename': filename,
                'fields': extracted_fields
            })
            print(f"Forwarded to backend: {res.status_code}")
        except Exception as e:
            print(f"Forwarding failed: {str(e)}")

        return {
            'statusCode': 200,
            'body': json.dumps({'fields': extracted_fields})
        }

    except Exception as e:
        print(f"Fatal error: {str(e)}")
        return {
            'statusCode': 500,
            'body': json.dumps({'error': str(e)})
        }
