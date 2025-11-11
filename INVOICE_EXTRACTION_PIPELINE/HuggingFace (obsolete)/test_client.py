import requests

with open("sample_invoice.jpg", "rb") as f:
    response = requests.post("http://127.0.0.1:5000/upload", files={"file": f})

print("Status Code:", response.status_code)

if response.status_code == 200:
    data = response.json()
    print("\n Matches (Sorted by Match Score Ascending):\n")

    for idx, match in enumerate(data["matches"], start=1):
        print(f"--- Match #{idx} ---")
        print(f"Company:          {match['company']}")
        print(f"Invoice Number:   {match['invoice_number']}")
        print(f"Invoice Date:     {match['invoice_date']}")
        print(f"Transaction Amt:  {match['transaction_amount']}")
        print(f"Match Score:      {match['match_score']:.3f}")
        print()

    print(" Extracted Fields:")
    for key, value in data["extracted"].items():
        print(f"{key.replace('_', ' ').title()}: {value}")
else:
    print("Error:", response.text)
