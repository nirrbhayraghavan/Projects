import psycopg2

conn = psycopg2.connect(
    dbname="invoice_db",
    user="postgres",
    password="1234",
    host="localhost",
    port="5432"
)
cur = conn.cursor()

cur.execute("DROP TABLE IF EXISTS invoices")
cur.execute("""
    CREATE TABLE invoices (
        id SERIAL PRIMARY KEY,
        invoice_number TEXT,
        invoice_date DATE,
        transaction_amount NUMERIC(10, 2),
        company TEXT
    )
""")

cur.execute("""
    INSERT INTO invoices (invoice_number, invoice_date, transaction_amount, company)
    VALUES (%s, %s, %s, %s)
""", ('US-001', '11/02/2019', 154.00, 'East Repair Inc'))

dummy_data = [
    ('INV-1001', '2023-11-01', 430.50, 'East Repair Inc'),
    ('#9981', '2024-01-19', 1024.99, 'Summit Foods Ltd.'),
    ('INV-7782', '2024-05-03', 612.00, 'Brightlane Logistics'),
    ('#8833', '2023-10-22', 78.40, 'Noble Ridge Corp'),
    ('INV-0099', '2023-08-14', 312.75, 'Silverpoint Realty'),
    ('#7611', '2024-04-10', 999.99, 'Crimson Creek Apparel'),
    ('INV-5501', '2023-12-02', 124.00, 'Verde Supply Co.'),
    ('#3704', '2023-09-27', 45.25, 'Hearthstone Ventures'),
    ('INV-9990', '2024-06-20', 880.00, 'Claymore Design Inc.')
]

for record in dummy_data:
    cur.execute("""
        INSERT INTO invoices (invoice_number, invoice_date, transaction_amount, company)
        VALUES (%s, %s, %s, %s)
    """, record)

conn.commit()
cur.close()
conn.close()

print("Database initialized with 10 records.")
