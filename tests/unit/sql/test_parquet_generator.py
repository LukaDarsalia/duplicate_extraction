import pandas as pd
import sqlite3

# Original Data
data = {
    'domain': [
        'domain1.com',
        'domain1.com',
        'domain2.com',
        'domain3.com',
        'domain1.com',
        'domain2.com'
    ],
    'content': [
        'First document content',
        'Second document from domain1',
        'Document from domain2',
        'გამარჯობა from domain3',
        'Third document from domain1',
        'Another document from domain2'
    ],
    'category': [
        'news', 'news', 'blog', 'news', 'blog', 'news'
    ]
}

# Create DataFrame
df = pd.DataFrame(data)

# Save to SQLite
df.to_parquet('test_documents.parquet')

print("Database created with schema:")
print(df.dtypes)
print("\nSample data:")
print(df.head())